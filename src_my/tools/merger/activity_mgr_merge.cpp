#include "activity_mgr_merge.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/proto_utils.hpp"
#include "utils/assert.hpp"

namespace nora {

        void activity_mgr_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge activity begin";
                                load_mgr();
                        });
        }

        void activity_mgr_merger::load_mgr() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_activity_mgr",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);
                                                                    from_activity_mgr_ = make_unique<pd::activity_mgr>();
                                                                    from_activity_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));

                                                                    if (to_activity_mgr_) {
                                                                            this->merge_mgr();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_activity_mgr",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);
                                                                  to_activity_mgr_ = make_unique<pd::activity_mgr>();
                                                                  to_activity_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                
                                                                  if (from_activity_mgr_) {
                                                                            this->merge_mgr();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }

        void activity_mgr_merger::merge_mgr() {
                ASSERT(st_->check_in_thread());
                if (to_activity_mgr_->prize_wheel() != from_activity_mgr_->prize_wheel()) {
                        ELOG << "activity mgr prize wheel mismatch:\n" <<
                                "from:\n" << from_activity_mgr_->prize_wheel().DebugString() <<
                                "\nto:\n" << to_activity_mgr_->prize_wheel().DebugString();
                        exit(1);
                }

                to_activity_mgr_->mutable_fund()->set_value(to_activity_mgr_->fund().value() + from_activity_mgr_->fund().value());

                auto db_msg = make_shared<db::message>("save_activity_mgr",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               this->on_db_save_activity_mgr_done(msg);
                                                       });
                string str;
                to_activity_mgr_->SerializeToString(&str);
                db_msg->push_param(str);
                to_db_->push_message(db_msg, st_);
        }


        void activity_mgr_merger::on_db_save_activity_mgr_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge activity_fund done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }
        }
}
