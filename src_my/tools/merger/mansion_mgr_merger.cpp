#include "mansion_mgr_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"

namespace nora {

        void mansion_mgr_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge mansion_mgr begin";
                                load();
                        });
        }

        void mansion_mgr_merger::load() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_mansion_mgr",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_mansion_mgr_ = make_unique<pd::mansion_mgr>();
                                                                    from_mansion_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_mansion_mgr_) {
                                                                            this->merge();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);

                auto to_db_msg = make_shared<db::message>("get_mansion_mgr",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_mansion_mgr_ = make_unique<pd::mansion_mgr>();
                                                                  to_mansion_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_mansion_mgr_) {
                                                                          this->merge();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }

        void mansion_mgr_merger::merge() {
                ASSERT(st_->check_in_thread());

                for (const auto& i : from_mansion_mgr_->banquets().banquets()) {
                        *to_mansion_mgr_->mutable_banquets()->add_banquets() = i;
                }

                auto db_msg = make_shared<db::message>("save_mansion_mgr",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               TOOL_TLOG << "merge mansion_mgrs done";
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                string data_str;
                to_mansion_mgr_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

}
