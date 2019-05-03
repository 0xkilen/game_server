#pragma once

#include "dbcache_logic.hpp"
#include "utils/free_list.hpp"
#include "utils/assert.hpp"
#include "dbcache_logic.hpp"
#include "log.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"
#include <boost/any.hpp>
#include <vector>
#include <memory>

using namespace std;

namespace nora {
        namespace dbcache {

                template <typename T>
                class dbc_object_single : public enable_shared_from_this<dbc_object_single<T>> {
                public:
                        dbc_object_single(const string& name,
                                       const shared_ptr<service_thread>& st) :
                                name_(name),
                                st_(st) {
                                auto ptt = PTTS_GET_COPY(options, 1u);
                                db_connector_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                                           ptt.game_db().user(),
                                                                           ptt.game_db().password(),
                                                                           ptt.game_db().database());
                                db_connector_->start();
                        }
                        void get(const function<void(pd::result, const T& data)>& cb) {
                                ASSERT(cb);
                                ASSERT(st_->check_in_thread());
                                if (object_) {
                                        cb(pd::OK, object_->data());
                                } else {
                                        dbc_object_call_funcs_.push_back(cb);
                                        if (!finding_) {
                                                finding_ = true;
                                                auto db_msg = make_shared<db::message>(string("get_") + name_,
                                                                               db::message::req_type::rt_select,
                                                                               [this, self = this->shared_from_this(), cb] (const shared_ptr<db::message>& msg) {
                                                                                       ASSERT(st_->check_in_thread());
                                                                                       auto result = on_db_find_done(msg->results());

                                                                                       try {
                                                                                                for (const auto& i : this->dbc_object_call_funcs_) {
                                                                                                        if (result == pd::OK) {
                                                                                                                ASSERT(object_);
                                                                                                                i(result, object_->data());
                                                                                                        } else {
                                                                                                                i(result, T());
                                                                                                        }
                                                                                                }
                                                                                       } catch (...) {
                                                                                                DBCACHE_ELOG << "dbc object single call func when find done throw";
                                                                                       }

                                                                                       this->dbc_object_call_funcs_.clear();
                                                                               });
                                                db_connector_->push_message(db_msg, st_);
                                        }
                                        
                                }
                        }
                        void create(const T& data) {
                                ASSERT(st_->check_in_thread());
                                ASSERT(!object_);

                                object_ = make_unique<dbc_object<T>>(ICT_IGNORE);
                                object_->set(data);

                                DBCACHE_DLOG << *this << " create";
                                auto db_msg = make_shared<db::message>(string("save_") + name_, db::message::req_type::rt_update);

                                string data_str;
                                data.SerializeToString(&data_str);
                                db_msg->push_param(data_str);
                                db_connector_->push_message(db_msg);
                        }
                        void update(const T& data) {
                                ASSERT(st_->check_in_thread());

                                get(
                                        [this, self = this->shared_from_this(), new_data = data] (auto result, const T& data) {
                                                ASSERT(result == pd::OK);
                                                ASSERT(object_);

                                                object_->update(new_data);

                                                if (update_timer_) {
                                                        return;
                                                }

                                                update_timer_ = ADD_TIMER(
                                                        st_,
                                                        ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                                                ASSERT(!canceled);
                                                                DBCACHE_DLOG << *this << " update";
                                                                auto db_msg = make_shared<db::message>(string("save_") + name_, db::message::req_type::rt_update);

                                                                ASSERT(object_);
                                                                string data_str;
                                                                object_->data().SerializeToString(&data_str);
                                                                db_msg->push_param(data_str);
                                                                db_connector_->push_message(db_msg);

                                                                update_timer_.reset();
                                                        }),
                                                        1s);
                                        });
                        }
                        bool can_stop() const {
                                return !update_timer_;
                        }
                        friend ostream& operator<<(ostream& os, const dbc_object_single<T>& dos) {
                                return os << dos.name_ << "_single";
                        }
                private:
                        pd::result on_db_find_done(const vector<vector<boost::any>>& results) {
                                ASSERT(st_->check_in_thread());
                                ASSERT(results.size() <= 1);

                                finding_ = false;
                                if (results.size() > 0) {
                                        const auto& result = results.front();
                                        object_ = make_unique<dbc_object<T>>(ICT_IGNORE);
                                        object_->parse(boost::any_cast<string>(result[0]));
                                        return pd::OK;
                                } else {
                                        return pd::NOT_FOUND;
                                }
                        }

                        string name_;
                        shared_ptr<service_thread> st_;
                        unique_ptr<dbc_object<T>> object_;
                        shared_ptr<timer_type> update_timer_;
                        shared_ptr<db::connector> db_connector_;
                        bool finding_ = false;
                        vector<function<void(pd::result, const T& data)>> dbc_object_call_funcs_;
                };

        }
}
