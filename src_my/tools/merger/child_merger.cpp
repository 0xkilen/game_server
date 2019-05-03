#include "child_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"

namespace nora {

        void child_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge child begin";
                                load_mgr();
                        });
        }

        void child_merger::load_mgr() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_child_mgr",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_child_mgr_ = make_unique<pd::child_mgr>();
                                                                    from_child_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_child_mgr_) {
                                                                            this->merge_mgr();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_child_mgr",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_child_mgr_ = make_unique<pd::child_mgr>();
                                                                  to_child_mgr_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_child_mgr_) {
                                                                          this->merge_mgr();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }


        void child_merger::merge_mgr() {
                ASSERT(st_->check_in_thread());
                for (const auto& i : from_child_mgr_->owners().owners()) {
                        *to_child_mgr_->mutable_owners()->add_owners() = i;
                }

                auto db_msg = make_shared<db::message>("save_child_mgr",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               this->load_next_page();
                                                       });
                string data_str;
                to_child_mgr_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

        void child_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_children",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_children_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void child_merger::on_db_get_children_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge children done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 2);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_child",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        ILOG << "process child " << gid;
                        db_msg->push_param(gid);
                        db_msg->push_param(boost::any_cast<string>(i[1]));
                        to_db_->push_message(db_msg, st_);
                }
        }
}
