#include "connector.hpp"
#include "log.hpp"
#include "utils/assert.hpp"
#include <cppconn/prepared_statement.h>
#include <istream>
#include <sstream>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace db {

                inline string build_procedure_string_(const message *message) {
                        string ret = "CALL " + message->procedure() + "(";
                        for (size_t i = 0; i < message->params().size(); ++i) {
                                if (i != message->params().size() - 1) {
                                        ret += "?,";
                                } else {
                                        ret += "?";
                                }
                        }
                        ret += ")";
                        return ret;
                }

                class data_stream : public iostream {
                public:
                        data_stream() : iostream(&streambuf_) {}
                        size_t size() {
                                return streambuf_.size();
                        }
                        ba::streambuf streambuf_;
                };

                inline void fill_params_(sql::PreparedStatement *pstmt, message *message) {
                        pstmt->clearParameters();
                        const auto& params = message->params();
                        for (auto i = 0ul; i < params.size(); ++i) {
                                auto *str_param = boost::any_cast<string>(&params[i]);
                                if (str_param) {
                                        pstmt->setString(i + 1, *str_param);
                                        continue;
                                }

                                auto *u64_param = boost::any_cast<uint64_t>(&params[i]);
                                if (u64_param) {
                                        pstmt->setUInt64(i + 1, *u64_param);
                                        continue;
                                }

                                auto *u32_param = boost::any_cast<uint32_t>(&params[i]);
                                if (u32_param) {
                                        pstmt->setUInt(i + 1, *u32_param);
                                        continue;
                                }

                                auto *int_param = boost::any_cast<int>(&params[i]);
                                if (int_param) {
                                        pstmt->setInt(i + 1, *int_param);
                                        continue;
                                }

                                DB_ELOG << "got invalid procedure parameter type";
                                ASSERT(0);
                        }
                }

                inline void process_select_(sql::PreparedStatement *pstmt, message *message) {
                        unique_ptr<sql::ResultSet> rs(pstmt->executeQuery());
                        sql::ResultSetMetaData *meta = rs->getMetaData();
                        unsigned int col_count = meta->getColumnCount();
                        while (rs->next()) {
                                message->results().push_back(vector<boost::any>());
                                for (unsigned int i = 1; i <= col_count; ++i) {
                                        switch (meta->getColumnType(i)) {
                                        case sql::DataType::CHAR:
                                        case sql::DataType::VARCHAR:
                                                message->results().back().push_back(rs->getString(i).asStdString());
                                                break;
                                        case sql::DataType::INTEGER:
                                                if (meta->isSigned(i)) {
                                                        message->results().back().push_back(rs->getInt(i));
                                                } else {
                                                        message->results().back().push_back(rs->getUInt(i));
                                                }
                                                break;
                                        case sql::DataType::BIGINT:
                                                if (meta->isSigned(i)) {
                                                        message->results().back().push_back(rs->getInt64(i));
                                                } else {
                                                        message->results().back().push_back(rs->getUInt64(i));
                                                }
                                                break;
                                        case sql::DataType::VARBINARY:
                                        case sql::DataType::LONGVARBINARY: {
                                                message->results().back().push_back(rs->getString(i).asStdString());
                                                break;
                                        }
                                        default:
                                                ASSERT(0);
                                        }
                                }
                        }

                        // we assume we will never use stored procedures with multi result set
                        // we still need to drain all the results because stored procedure +
                        // prepared statement seems to have this problem:
                        // http://forums.mysql.com/read.php?167,416487,416487
                        while (pstmt->getMoreResults()) {
                                rs.reset(pstmt->getResultSet());
                        }
                }

                inline void process_update_(sql::PreparedStatement *pstmt, message *msg) {
                        try {
                                pstmt->executeUpdate();
                        } catch (const sql::SQLException& e) {
                                auto ec = e.getErrorCode();
                                if (ec == 1062) {
                                        msg->set_update_got_duplicate();
                                        return;
                                } else {
                                        throw;
                                }
                        }
                }

                connector::connector(const string& ipport,
                                     const string& user,
                                     const string& passwd,
                                     const string& database) :
                        st_(make_shared<service_thread>("db-connector")),
                        ipport_(ipport),
                        user_(user),
                        passwd_(passwd),
                        database_(database) {
                        sql::mysql::MySQL_Driver *driver = sql::mysql::get_driver_instance();
                        conn_ = unique_ptr<sql::Connection>(driver->connect(ipport_, user_, passwd_));
                        conn_->setSchema(database_);
                }

                connector::~connector() {
                        stop();
                }

                void connector::start() {
                        st_->start();
                }

                void connector::push_message(const shared_ptr<message>& msg, const shared_ptr<service_thread>& st) {
                        ASSERT(!msg->has_finish_handler() || st);
                        st_->async_call(
                                [this, msg, st] {
                                        DB_DLOG << *msg;
                                        while (true) {
                                                try {
                                                        shared_ptr<sql::PreparedStatement> pstmt;
                                                        if (pstmts_.count(msg->procedure()) > 0) {
                                                                pstmt = pstmts_.at(msg->procedure());
                                                                fill_params_(pstmt.get(), msg.get());
                                                        } else {
                                                                string pstmtString = build_procedure_string_(msg.get());
                                                                pstmt.reset(conn_->prepareStatement(pstmtString));
                                                                pstmts_[msg->procedure()] = pstmt;
                                                                fill_params_(pstmt.get(), msg.get());
                                                        }

                                                        switch (msg->get_req_type()) {
                                                        case message::rt_select:
                                                                process_select_(pstmt.get(), msg.get());
                                                                break;
                                                        case message::rt_insert:
                                                        case message::rt_update:
                                                        case message::rt_delete:
                                                                process_update_(pstmt.get(), msg.get());
                                                                break;
                                                        case message::rt_control:
                                                                break;
                                                        }
                                                        if (pstmt) {
                                                                pstmt->clearParameters();
                                                        }
                                                } catch (const sql::SQLException& e) {
                                                        auto ec = e.getErrorCode();
                                                        if (ec == 2013 || ec == 2006) {
                                                                this_thread::sleep_for(1ms);
                                                                pstmts_.clear();
                                                                conn_->reconnect();
                                                                conn_->setSchema(database_);
                                                                DB_ILOG << "db lost connection, reconnect to mysql";
                                                                continue;
                                                        } else {
                                                                DB_ELOG << *msg << " got exception: " << e.what();
                                                                return;
                                                        }
                                                }
                                                break;
                                        }

                                        if (st) {
                                                st->async_call(
                                                        [msg] {
                                                                msg->finish();
                                                        });
                                        } else {
                                                msg->finish();
                                        }
                                });
                }

                void connector::stop() {
                        if (stopped_) {
                                return;
                        }

                        stopped_ = true;
                        st_->stop();
                        conn_->close();
                        conn_.reset();
                }

        }
}
