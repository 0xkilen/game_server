#include "mail.hpp"
#include "config/options_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "utils/proto_utils.hpp"
#include "utils/assert.hpp"
#include <limits>

namespace nora {
        namespace scene {

                mail::mail(const shared_ptr<service_thread>& st) {
                        st_ = st;
                        auto ptt = PTTS_GET_COPY(options, 1u);
                        gamedb_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                             ptt.game_db().user(),
                                                             ptt.game_db().password(),
                                                             ptt.game_db().database());
                        gamedb_->start();
                }

                void mail::start() {
                }

                void mail::add_mail(uint64_t role, const pd::mail& mail) const {
                        ASSERT(gamedb_);
                        auto db_msg = make_shared<db::message>("add_mail", db::message::req_type::rt_insert);
                        db_msg->push_param(mail.gid());
                        db_msg->push_param(role);
                        string mail_str;
                        mail.SerializeToString(&mail_str);
                        db_msg->push_param(mail_str);
                        gamedb_->push_message(db_msg);
                }

                void mail::find_mails(uint64_t role) const {
                        ASSERT(gamedb_);
                        auto db_msg = make_shared<db::message>("find_mails",
                                                               db::message::req_type::rt_select,
                                                               [this, self = shared_from_this(), role] (const auto& msg) {
                                                                       if (msg->results().size() > 0) {
                                                                               pd::mail_array mails;
                                                                               for (const auto& i : msg->results()) {
                                                                                       ASSERT(i.size() == 1);
                                                                                       mails.add_mails()->ParseFromString(boost::any_cast<string>(i[0]));
                                                                               }
                                                                               ASSERT(find_mails_done_cb_);
                                                                               find_mails_done_cb_(role, mails);
                                                                       }
                                                               });
                        db_msg->push_param(role);
                        gamedb_->push_message(db_msg, st_);
                }

                void mail::delete_mail(uint64_t gid) const {
                        ASSERT(gamedb_);
                        auto db_msg = make_shared<db::message>("delete_mail", db::message::req_type::rt_update);
                        db_msg->push_param(gid);
                        gamedb_->push_message(db_msg);
                }

                void mail::stop() {
                        if (gamedb_) {
                                gamedb_->stop();
                        }
                }

        }
}
