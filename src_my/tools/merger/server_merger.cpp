#include "server_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"
#include <string>

namespace nora {

        void server_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge servers begin";
                                load();
                        });
        }

        void server_merger::load() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("load_server",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_server_ = make_unique<pd::server>();
                                                                    from_server_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_server_) {
                                                                            this->merge();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("load_server",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_server_ = make_unique<pd::server>();
                                                                  to_server_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_server_) {
                                                                          this->merge();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }


        void server_merger::merge() {
                ASSERT(st_->check_in_thread());

                pd::server server_data;
                set<string> used_white_list;
                for (const auto& i : from_server_->white_list()) {
                        if (used_white_list.count(i) > 0) {
                                continue;
                        }
                        used_white_list.insert(i);
                        server_data.add_white_list(i);
                }
                for (const auto& i : to_server_->white_list()) {
                        if (used_white_list.count(i) > 0) {
                                continue;
                        }
                        used_white_list.insert(i);
                        server_data.add_white_list(i);
                }
                server_data.set_open_day(to_server_->open_day());
                server_data.set_marriage_count(from_server_->marriage_count() + to_server_->marriage_count());

                auto db_msg = make_shared<db::message>("update_server",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               TOOL_TLOG << "merge servers done";
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                string data;
                server_data.SerializeToString(&data);
                db_msg->push_param(data);
                to_db_->push_message(db_msg, st_);
        }

}
