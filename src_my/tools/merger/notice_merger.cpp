#include "notice_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"

namespace nora {

        void notice_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge notices begin";
                                load_next_page();
                        });
        }

        void notice_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_notices",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_notices_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void notice_merger::on_db_get_notices_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge notices done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 4);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_notice",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        auto role_gid = boost::any_cast<uint64_t>(i[1]);
                        ILOG << "process notice " << gid << " " << role_gid;
                        db_msg->push_param(gid);
                        db_msg->push_param(role_gid);
                        db_msg->push_param(boost::any_cast<uint32_t>(i[2]));
                        db_msg->push_param(boost::any_cast<string>(i[3]));
                        to_db_->push_message(db_msg, st_);
                }

        }
}
