#include "arena_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/gid.hpp"

namespace nora {

        void arena_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge arena begin";
                                load_rank();
                        });
        }

        void arena_merger::load_rank() {
                ASSERT(st_->check_in_thread());
                auto from_db_msg = make_shared<db::message>("get_arena_rank",
                                                            db::message::req_type::rt_select,
                                                            [this] (const auto& msg) {
                                                                    ASSERT(msg->results().size() == 1);

                                                                    from_arena_ = make_unique<pd::arena_rank>();
                                                                    from_arena_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                    if (to_arena_) {
                                                                            this->merge_rank();
                                                                    }
                                                            });
                from_db_->push_message(from_db_msg, st_);
                auto to_db_msg = make_shared<db::message>("get_arena_rank",
                                                          db::message::req_type::rt_select,
                                                          [this] (const auto& msg) {
                                                                  ASSERT(msg->results().size() == 1);

                                                                  to_arena_ = make_unique<pd::arena_rank>();
                                                                  to_arena_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                  if (from_arena_) {
                                                                          this->merge_rank();
                                                                  }
                                                          });
                to_db_->push_message(to_db_msg, st_);
        }


        void arena_merger::merge_rank() {
                ASSERT(st_->check_in_thread());
                auto from_idx = 1;
                auto to_idx = 1;
                pd::gid_array new_rank;
                new_rank.add_gids(0);
                do {
                        if (to_idx < to_arena_->rank().gids_size()) {
                                new_rank.add_gids(to_arena_->rank().gids(to_idx));
                                to_idx += 1;
                        }
                        if (from_idx < from_arena_->rank().gids_size()) {
                                auto from_gid = from_arena_->rank().gids(from_idx);
                                if (gid::instance().get_type(from_gid) != gid_type::GLADIATOR) {
                                        new_rank.add_gids(from_gid);
                                }
                                from_idx += 1;
                        }
                } while (to_idx < to_arena_->rank().gids_size() || from_idx < from_arena_->rank().gids_size());

                *to_arena_->mutable_rank() = new_rank;
                auto db_msg = make_shared<db::message>("save_arena_rank",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               this->load_to_db_next_page();
                                                       });
                string data_str;
                to_arena_->SerializeToString(&data_str);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);
        }

        void arena_merger::load_to_db_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_gladiators",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_to_db_gladiators_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                to_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void arena_merger::on_db_get_to_db_gladiators_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        cur_page_ = 0;
                        this->load_next_page();
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 2);
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        if (gid::instance().get_type(gid) == gid_type::GLADIATOR) {
                                continue;
                        }

                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("update_gladiator",
                                                               db::message::req_type::rt_update,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_to_db_next_page();
                                                                       }
                                                               });
                        
                        ILOG << "process gladiator " << gid;
                        pd::gladiator data;
                        data.ParseFromString(boost::any_cast<string>(i[1]));

                        data.mutable_data()->clear_visible_ranks();
                        string data_str;
                        data.SerializeToString(&data_str);

                        db_msg->push_param(gid);
                        db_msg->push_param(data_str);
                        to_db_->push_message(db_msg, st_);
                }

                if (adding_count_ == 0) {
                        this->load_to_db_next_page();
                }
        }

        void arena_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_gladiators",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_gladiators_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void arena_merger::on_db_get_gladiators_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge arena done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 2);
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        if (gid::instance().get_type(gid) == gid_type::GLADIATOR) {
                                continue;
                        }

                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_gladiator",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        
                        ILOG << "process gladiator " << gid;
                        pd::gladiator data;
                        data.ParseFromString(boost::any_cast<string>(i[1]));

                        data.mutable_data()->clear_visible_ranks();
                        string data_str;
                        data.SerializeToString(&data_str);

                        db_msg->push_param(gid);
                        db_msg->push_param(data_str);
                        to_db_->push_message(db_msg, st_);
                }

                if (adding_count_ == 0) {
                        this->load_next_page();
                }
        }
}
