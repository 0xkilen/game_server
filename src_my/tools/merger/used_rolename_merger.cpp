#include "used_rolename_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"
#include "proto/data_global.pb.h"

namespace nora {

        void used_rolename_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge used_rolenames begin";
                                load();
                        });
        }

        void used_rolename_merger::load() {
                ASSERT(st_->check_in_thread());
                loading_count_ += 2;
                auto from_db_msg = make_shared<db::message>("load_used_rolenames",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_load_done(msg);
                                                       });
                from_db_->push_message(from_db_msg, st_);

                auto to_db_msg = make_shared<db::message>("load_used_rolenames",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_load_done(msg);
                                                       });
                to_db_->push_message(to_db_msg, st_);
        }

        void used_rolename_merger::on_db_load_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                loading_count_ -= 1;
                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 1);
                        const auto& data = boost::any_cast<string>(i[0]);
                        pd::used_rolename_array used_rolenames;
                        used_rolenames.ParseFromString(data);
                        for (const auto& name : used_rolenames.used_names()) {
                                used_rolenames_.insert(name);
                        }
                }
                if (loading_count_ == 0) {
                        auto db_msg = make_shared<db::message>("update_used_rolenames",
                                                       db::message::req_type::rt_update,
                                                       [this] (const auto& msg) {
                                                               TOOL_TLOG << "merge used_rolenames done";
                                                               if (done_cb_) {
                                                                       done_cb_();
                                                               }
                                                       });
                        pd::used_rolename_array used_rolenames;
                        for (const auto& name : used_rolenames_) {
                                used_rolenames.add_used_names(name);
                        }

                        string data;
                        used_rolenames.SerializeToString(&data);
                        db_msg->push_param(data);
                        to_db_->push_message(db_msg, st_);
                }
        }

}
