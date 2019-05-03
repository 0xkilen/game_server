#include "tower_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"
#include "utils/proto_utils.hpp"

namespace nora {

        void tower_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge tower begin";
                                load_next_level();
                        });
        }

        void tower_merger::load_next_level() {
                ASSERT(st_->check_in_thread());

                from_tower_end_ = false;
                to_tower_end_ = false;
                
                if (from_tower_) {
                        from_tower_.reset();
                }

                if (to_tower_) {
                        to_tower_.reset();
                }

                auto from_db_msg = make_shared<db::message>("get_tower_records",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                                auto& results = msg->results();
                                                                if (results.empty()) {
                                                                        from_tower_end_ = true;
                                                                        if (to_tower_end_) {
                                                                                ILOG << "merge tower done";
                                                                                if (done_cb_) {
                                                                                        done_cb_();
                                                                                }
                                                                        } else if (to_tower_){
                                                                                this->merge();
                                                                        }
                                                                        return;
                                                                }
                                                                ASSERT(msg->results().size() == 1);

                                                                from_tower_ = make_unique<pd::tower_record_array>();
                                                                from_tower_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                
                                                                if (to_tower_ || to_tower_end_) {
                                                                        this->merge();
                                                                }
                                                       });
                from_db_msg->push_param(cur_level_);
                from_db_->push_message(from_db_msg, st_);

                auto to_db_msg = make_shared<db::message>("get_tower_records",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                                auto& results = msg->results();
                                                                if (results.empty()) {
                                                                        to_tower_end_ = true;
                                                                        if (from_tower_end_) {
                                                                                ILOG << "merge tower done";
                                                                                if (done_cb_) {
                                                                                        done_cb_();
                                                                                }
                                                                        } else if (from_tower_) {
                                                                                this->merge();
                                                                        }
                                                                        return;
                                                                }

                                                                to_tower_ = make_unique<pd::tower_record_array>();
                                                                to_tower_->ParseFromString(boost::any_cast<string>(msg->results()[0][0]));
                                                                
                                                                if (from_tower_ || from_tower_end_) {
                                                                        this->merge();
                                                                } 
                                                       });
                to_db_msg->push_param(cur_level_);
                to_db_->push_message(to_db_msg, st_);
        }

        void tower_merger::merge() {
                pd::tower_record_array new_records;

                if (to_tower_) {
                        for (const auto& i : to_tower_->records()) {
                                *new_records.add_records() = i;
                        }
                }

                if (from_tower_) {
                        for (const auto& i : from_tower_->records()) {
                                *new_records.add_records() = i;
                        }
                }

                list<pd::tower_record> rl;
                set<uint64_t> roles;
                for (const auto& i : new_records.records()) {
                        rl.push_back(i);
                }
                rl.sort([] (const auto& a, const auto& b) {
                                return a < b;
                        });
                new_records.clear_records();
                for (const auto& i : rl) {
                        if (new_records.records_size() >= 3) {
                                break;
                        }
                        if (roles.count(i.role()) == 0) {
                                roles.insert(i.role());
                                *new_records.add_records() = i;
                        }
                }

                to_tower_ = make_unique<pd::tower_record_array>(new_records);

                auto db_msg = make_shared<db::message>("update_tower_records", db::message::req_type::rt_update);
                string str;
                to_tower_->SerializeToString(&str);
                db_msg->push_param(cur_level_);
                db_msg->push_param(str);
                to_db_->push_message(db_msg);

                cur_level_++;
                load_next_level();
        }
}