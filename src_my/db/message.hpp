#pragma once

#include "proto/data_db.pb.h"
#include "utils/instance_counter.hpp"
#include <string>
#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include <functional>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace db {

                class message : public enable_shared_from_this<message>,
                                public boost::noncopyable,
                                private instance_countee {
                public:
                        enum req_type {
                                rt_insert,
                                rt_select,
                                rt_update,
                                rt_delete,
                                rt_control,
                        };
                        message(const string& procedure,
                                req_type rt,
                                function<void(const shared_ptr<message>&)> finish_handler) :
                                instance_countee(ICT_DB_MESSAGE),
                                procedure_(procedure),
                                rt_(rt),
                                finish_handler_(finish_handler) {
                        }
                        message(const string& procedure,
                                req_type rt) :
                                instance_countee(ICT_DB_MESSAGE),
                                procedure_(procedure),
                                rt_(rt) {
                        }
                        template <typename T>
                        void push_param(T&& param) {
                                parameters_.push_back(forward<T>(param));
                        }
                        const string& procedure() const {
                                return procedure_;
                        }
                        const vector<boost::any>& params() const {
                                return parameters_;
                        }
                        req_type get_req_type() const {
                                return rt_;
                        }
                        vector<vector<boost::any>>& results() {
                                return results_;
                        }
                        void finish() {
                                if (finish_handler_) {
                                        finish_handler_(shared_from_this());
                                }
                        }
                        bool has_finish_handler() {
                                return finish_handler_ != nullptr;
                        }
                        void set_update_got_duplicate() {
                                update_got_duplicate_ = true;
                        }
                        bool update_got_duplicate() const {
                                return update_got_duplicate_;
                        }
                        void set_client_data(const pd::db_client_data& data) {
                                client_data_ = data;
                        }
                        const pd::db_client_data& client_data() const {
                                return client_data_;
                        }
                        friend ostream& operator<<(ostream& os, const message& msg) {
                                os << "<dbmessage:" << msg.procedure_ << ">";
                                return os;
                        }
                        stringstream *new_stringstream() {
                                sss_.push_back(make_unique<stringstream>());
                                return sss_.back().get();
                        }
                private:
                        const string procedure_;
                        req_type rt_;
                        vector<boost::any> parameters_;
                        vector<vector<boost::any>> results_;
                        bool update_got_duplicate_ = false;
                        function<void(const shared_ptr<message>&)> finish_handler_;
                        vector<unique_ptr<stringstream>> sss_;
                        pd::db_client_data client_data_;
                };

        }
}
