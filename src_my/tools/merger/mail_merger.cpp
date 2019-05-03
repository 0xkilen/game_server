#include "mail_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "config/mail_ptts.hpp"
#include "config/utils.hpp"
#include "utils/gid.hpp"

namespace nora {

        void mail_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge mail begin";
                                PTTS_LOAD(mail);
                                load_next_page();
                        });
        }

        void mail_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_mails",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_mails_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void mail_merger::on_db_get_mails_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "merge mails done";
                        if (done_cb_) {
                                done_cb_();
                        }
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 3);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_mail",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto gid = boost::any_cast<uint64_t>(i[0]);
                        auto role_gid = boost::any_cast<uint64_t>(i[1]);
                        ILOG << "process mail " << gid << " " << role_gid;
                        db_msg->push_param(gid);
                        db_msg->push_param(role_gid);
                        db_msg->push_param(boost::any_cast<string>(i[2]));
                        to_db_->push_message(db_msg, st_);
                }

        }

        pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data) {
                const auto& ptt = PTTS_GET(mail, pttid);
                auto events = ptt.events();
                pd::mail m;
                m.set_gid(gid::instance().new_gid(gid_type::MAIL));
                if (dynamic_data.has_time()) {
                        m.set_time(dynamic_data.time());
                } else {
                        m.set_time(system_clock::to_time_t(system_clock::now()));
                }
                m.set_pttid(pttid);
                *m.mutable_dynamic_data() = dynamic_data;
                *m.mutable_events() = events;
                return m;
        }
}
