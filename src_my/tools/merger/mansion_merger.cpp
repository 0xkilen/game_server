#include "mansion_merger.hpp"
#include "tools/log.hpp"
#include "utils/assert.hpp"
#include "proto/data_mansion.pb.h"

namespace pd = proto::data;

namespace nora {

        void mansion_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge mansion begin";
                                load_next_page();
                        });
        }

        void mansion_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_mansions",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_mansions_done(msg);
                                                       });
                db_msg->push_param(cur_page_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += page_size_;
        }

        void mansion_merger::on_db_get_mansions_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        TOOL_TLOG << "merge mansions done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 2);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_mansion",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        const auto& data = boost::any_cast<string>(i[1]);
                        pd::mansion mansion_data;
                        mansion_data.ParseFromString(data);
                        TOOL_DLOG << "process mansion " << mansion_data.DebugString();
                        db_msg->push_param(gid);
                        db_msg->push_param(data);
                        to_db_->push_message(db_msg, st_);
                }
        }

}
