#include "role_merger.hpp"
#include "prepare.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/gid.hpp"
#include "config/player_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "config/utils.hpp"

namespace nora {
        namespace {

                string fix_collide_name(uint64_t gid, const string& name) {
                        string ret = name;
                        ret += 's';
                        ret += to_string(gid::instance().get_server_id(gid));
                        return ret;
                }

        }

        void role_merger::start() {
                ASSERT(st_);
                st_->async_call(
                        [this] {
                                ILOG << "merge role begin";
                                PTTS_LOAD(role_logic);
                                PTTS_LOAD(mail);
                                load_next_page();
                        });
        }

        void role_merger::load_next_page() {
                ASSERT(st_->check_in_thread());
                auto db_msg = make_shared<db::message>("get_roles",
                                                       db::message::req_type::rt_select,
                                                       [this] (const auto& msg) {
                                                               this->on_db_get_roles_done(msg);
                                                       });
                db_msg->push_param(cur_page_ * page_size_);
                db_msg->push_param(page_size_);
                from_db_->push_message(db_msg, st_);
                cur_page_ += 1;
        }

        void role_merger::on_db_get_roles_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                if (msg->results().empty()) {
                        ILOG << "from merge to to done";
                        fix_to_names();
                        return;
                }

                for (const auto& i : msg->results()) {
                        ASSERT(i.size() == 4);
                        adding_count_ += 1;
                        auto db_msg = make_shared<db::message>("add_role",
                                                               db::message::req_type::rt_insert,
                                                               [this] (const auto& msg) {
                                                                       adding_count_ -= 1;
                                                                       if (adding_count_ == 0) {
                                                                               this->load_next_page();
                                                                       }
                                                               });
                        auto username = boost::any_cast<string>(i[0]);
                        auto gid = boost::any_cast<uint64_t>(i[1]);
                        db_msg->push_param(username);
                        db_msg->push_param(gid);

                        auto rolename = boost::any_cast<string>(i[2]);
                        ILOG << "process role " << username << " " << rolename << " " << gid;
                        auto iter = prepare::instance().to_rolename2gid().find(rolename);
                        if (iter != prepare::instance().to_rolename2gid().end()) {
                                ILOG << rolename << " already exist, add server id suffix";
                                auto new_rolename = fix_collide_name(gid, rolename);
                                fix_name_to_roles_.insert(iter->second);

                                pd::role data;
                                data.ParseFromString(boost::any_cast<string>(i[3]));
                                data.mutable_data()->set_name(new_rolename);
                                string new_data_str;
                                data.SerializeToString(&new_data_str);
                                db_msg->push_param(new_rolename);
                                db_msg->push_param(new_data_str);

                                const auto& ptt = PTTS_GET(role_logic, 1);
                                send_mail(gid, ptt.merge_change_name_mail());
                        } else {
                                db_msg->push_param(rolename);
                                db_msg->push_param(boost::any_cast<string>(i[3]));
                        }
                        to_db_->push_message(db_msg, st_);
                }
        }

        void role_merger::fix_to_names() {
                ASSERT(st_->check_in_thread());
                if (fix_name_to_roles_.empty()) {
                        if (done_cb_) {
                                ILOG << "role merge done";
                                done_cb_();
                        }
                        return;
                }
                
                for (auto i : fix_name_to_roles_) {
                        auto db_msg = make_shared<db::message>("find_role_by_gid",
                                                               db::message::req_type::rt_select,
                                                               [this] (const auto& msg) {
                                                                       this->on_db_find_fix_name_to_role_done(msg);
                                                               });
                        db_msg->push_param(i);
                        to_db_->push_message(db_msg, st_);
                }
        }

        void role_merger::on_db_find_fix_name_to_role_done(const shared_ptr<db::message>& msg) {
                ASSERT(st_->check_in_thread());
                ASSERT(msg->results().size() == 1);
                const auto& result = msg->results()[0];
                ASSERT(result.size() == 4);
                auto gid = boost::any_cast<uint64_t>(result[0]);
                auto db_msg = make_shared<db::message>("update_role",
                                                       db::message::req_type::rt_update,
                                                       [this, gid] (const auto& msg) {
                                                               fix_name_to_roles_.erase(gid);
                                                               if (fix_name_to_roles_.empty()) {
                                                                       if (done_cb_) {
                                                                              ILOG << "role merge done";
                                                                              done_cb_();
                                                                       }
                                                               }
                                                       });
                pd::role data;
                data.ParseFromString(boost::any_cast<string>(result[2]));
                auto rolename = fix_collide_name(gid, boost::any_cast<string>(result[1]));
                data.mutable_data()->set_name(rolename);
                string data_str;
                data.SerializeToString(&data_str);

                db_msg->push_param(gid);
                db_msg->push_param(rolename);
                db_msg->push_param(data_str);
                to_db_->push_message(db_msg, st_);

                const auto& ptt = PTTS_GET(role_logic, 1);
                send_mail(gid, ptt.merge_change_name_mail());
        }

        void role_merger::send_mail(uint64_t role, uint32_t mail_id) {
                ILOG << "send role change names mail to role: " << role;
                auto m = mail_new_mail(role, mail_id, pd::dynamic_data());
                auto db_msg = make_shared<db::message>("add_mail", db::message::req_type::rt_insert);
                db_msg->push_param(m.gid());
                db_msg->push_param(role);
                string mail_str;
                m.SerializeToString(&mail_str);
                db_msg->push_param(mail_str);
                to_db_->push_message(db_msg);
        }

        pd::mail role_merger::mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data) {
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
