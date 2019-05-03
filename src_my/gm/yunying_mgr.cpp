#include "yunying_mgr.hpp"
#include "yunying.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/assert.hpp"
#include "scene_mgr_class.hpp"
#include "login_mgr_class.hpp"

namespace nora {
        namespace gm {

                yunying_mgr::yunying_mgr() {
                        name_ = "gm-yunyingmgr";
                        st_ = make_shared<service_thread>("yunyingmgr");
                        nsv_ = make_shared<net::server<net::HTTP_CONN>>("gm-yunyingmgr-netserver");
                        st_->start();
                }

                void yunying_mgr::start(const string& ip, unsigned short port) {
                        nsv_->new_conn_cb_ = [this] (const auto& c) -> shared_ptr<service_thread> {
                                GM_DLOG << *this << " got new conn " << *c;

                                auto s = make_shared<yunying>(st_, nsv_, c, sid_seed_);
                                lock_guard<mutex> lock(lock_);
                                if (sid_seed_ >= 10000) {
                                        sid_seed_ = 1;
                                }
                                sid2yunying_[sid_seed_] = s;
                                sid_seed_ = max(sid_seed_ + 1, 1u);
                                conn2yunying_[c] = s;
                                s->gag_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role, auto gag_until_time, const auto& reason) {
                                        scene_mgr_class::instance().gag_role_by_gid(sid, server_id, role, gag_until_time, reason);
                                };
                                s->remove_gag_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role) {
                                        scene_mgr_class::instance().remove_gag_role_by_gid(sid, server_id, role);
                                };
                                s->ban_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role, auto ban_until_time, const auto& reason) {
                                        scene_mgr_class::instance().ban_role_by_gid(sid, server_id, role, ban_until_time, reason);
                                };
                                s->remove_ban_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role) {
                                        scene_mgr_class::instance().remove_ban_role_by_gid(sid, server_id, role);
                                };
                                s->kick_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role, const auto& reason) {
                                        scene_mgr_class::instance().kick_role_by_gid(sid, server_id, role, reason);
                                };
                                s->find_role_by_gid_func_ = [this] (auto sid, auto server_id, auto role) {
                                        scene_mgr_class::instance().find_role_by_gid(sid, server_id, role);
                                };
                                s->find_role_by_rolename_func_ = [this] (auto sid, auto server_id, const auto& rolename) {
                                        scene_mgr_class::instance().find_role_by_rolename(sid, server_id, rolename);
                                };
                                s->fetch_world_chat_func_ = [this] (auto sid, auto server_id) {
                                        scene_mgr_class::instance().fetch_world_chat(sid, server_id);
                                };
                                s->fetch_rank_list_func_ = [this] (auto sid, auto server_id, const auto& rank_type, auto page_size) {
                                        scene_mgr_class::instance().fetch_rank_list(sid, server_id, rank_type, page_size);
                                };
                                s->internal_recharge_func_ = [this] (auto sid, auto server_id, auto role, auto recharge_id) {
                                        scene_mgr_class::instance().internal_recharge(sid, server_id, role, recharge_id);
                                };
                                s->reissue_recharge_func_ = [this] (auto sid, auto server_id, auto role, auto order) {
                                        scene_mgr_class::instance().reissue_recharge(sid, server_id, role, order);
                                };
                                s->recharge_func_ = [this] (auto sid, auto order, const auto& yy_orderno, const auto& currency, auto price, auto paytime, const auto& product_name, const auto& ext_info) {
                                        scene_mgr_class::instance().recharge(sid, order, yy_orderno, currency, price, paytime, product_name, ext_info);
                                };
                                s->send_mail_func_ = [this] (auto sid, auto server_id, auto channel_id, const auto& roles, auto mail_id, const auto& title, const auto& content, const auto& events) {
                                        scene_mgr_class::instance().send_mail(sid, server_id, channel_id, roles, mail_id, title, content, events);
                                };
                                s->fetch_log_func_ = [this] (auto sid, auto server_id, auto role, auto item_id, auto start_time, auto end_time, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_log(sid, server_id, role, item_id, start_time, end_time, page_idx, page_size);
                                };
                                s->add_to_white_list_func_ = [this] (auto sid, auto server_id, const auto& ips) {
                                        login_mgr_class::instance().add_to_white_list(sid, server_id, ips);
                                };
                                s->remove_from_white_list_func_ = [this] (auto sid, auto server_id, const auto& ips) {
                                        login_mgr_class::instance().remove_from_white_list(sid, server_id, ips);
                                };
                                s->fetch_white_list_func_ = [this] (auto sid, auto server_id) {
                                        login_mgr_class::instance().fetch_white_list(sid, server_id);
                                };
                                s->manage_announcement_func_ = [this] (auto sid, const auto& announcement) {
                                        scene_mgr_class::instance().manage_announcement(sid, announcement);
                                };
                                s->server_images_announcement_func_ = [this] (auto sid, auto server_id, const auto& announcements) {
                                        scene_mgr_class::instance().server_images_announcement(sid, server_id, announcements);
                                };
                                s->fetch_punished_func_ = [this] (auto sid, auto server_id, const auto& role, auto start_time, auto end_time, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_punished(sid, server_id, role, start_time, end_time, page_idx, page_size);
                                };
                                s->fetch_sent_mail_func_ = [this] (auto sid, auto server_id, const auto& role, auto start_time, auto end_time, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_sent_mail(sid, server_id, role, start_time, end_time, page_idx, page_size);
                                };
                                s->fetch_login_record_func_ = [this] (auto sid, auto server_id, const auto& role, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_login_record(sid, server_id, role, page_idx, page_size);
                                };
                                s->fetch_recharge_record_func_ = [this] (auto sid, auto server_id, auto channel_id, auto order, const auto& role, auto start_time, auto end_time, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_recharge_record(sid, server_id, channel_id, order, role, start_time, end_time, page_idx, page_size);
                                };
                                s->fetch_currency_record_func_ = [this] (auto sid, auto server_id, const auto& type, const auto& role, auto start_time, auto end_time, auto page_idx, auto page_size) {
                                        scene_mgr_class::instance().fetch_currency_record(sid, server_id, type, role, start_time, end_time, page_idx, page_size);
                                };
                                s->login_gonggao_func_ = [this] (auto sid, const auto& gonggao) {
                                        login_mgr_class::instance().login_gonggao(sid, gonggao);
                                };
                                s->fetch_role_list_func_ = [this] (auto sid, const auto& username) {
                                        scene_mgr_class::instance().fetch_role_list(sid, username);
                                };
                                s->add_stuff_by_gid_func_ = [this] (auto sid, auto server_id, auto role, const auto& events) {
                                        scene_mgr_class::instance().add_stuff_by_gid(sid, server_id, role, events);
                                };
                                s->add_stuff_by_rolename_func_ = [this] (auto sid, auto server_id, auto rolename, const auto& events) {
                                        scene_mgr_class::instance().add_stuff_by_rolename(sid, server_id, rolename, events);
                                };
                                s->dec_stuff_by_gid_func_ = [this] (auto sid, auto server_id, auto role, const auto& events) {
                                        scene_mgr_class::instance().dec_stuff_by_gid(sid, server_id, role, events);
                                };
                                s->dec_stuff_by_rolename_func_ = [this] (auto sid, auto server_id, auto rolename, const auto& events) {
                                        scene_mgr_class::instance().dec_stuff_by_rolename(sid, server_id, rolename, events);
                                };
                                s->operates_activity_func_ = [this] (auto sid, auto activity, auto start_time, auto duration, auto type, const auto& server_ids, auto open_activity) {
                                        scene_mgr_class::instance().operates_activity(sid, activity, start_time, duration, type, server_ids, open_activity);
                                };
                                s->stop_cb_ = [this] (const auto& c){
                                        if (conn2yunying_.count(c) == 0) {
                                                return;
                                        }
                                        const auto& yy = conn2yunying_.at(c);
                                        ASSERT(sid2yunying_.count(yy->sid()) > 0);
                                        sid2yunying_.erase(yy->sid());
                                        conn2yunying_.erase(c);
                                };
                                return st_;
                        };
                        nsv_->msg_cb_ = [this] (const auto& c, const auto& msg) {
                                lock_guard<mutex> lock(lock_);
                                if (conn2yunying_.count(c) == 0) {
                                        return;
                                }
                                const auto& yunying = conn2yunying_.at(c);
                                GM_DLOG << *this << " got msg from " << *yunying;
                                yunying->process_msg(msg);
                        };
                        nsv_->error_cb_ = [this] (const auto& c) {
                                lock_guard<mutex> lock(lock_);
                                const auto& yunying = conn2yunying_.at(c);
                                GM_DLOG << *this << " got error from " << *yunying;
                                sid2yunying_.erase(yunying->sid());
                                yunying->stop();
                                conn2yunying_.erase(c);
                        };

                        nsv_->start();
                        nsv_->listen(ip, port);
                        GM_ILOG << *this << " start";
                }

                void yunying_mgr::stop() {
                        auto c2s = conn2yunying_;
                        for_each(c2s.begin(), c2s.end(),
                                 [this] (const auto& i) {
                                         i.second->stop();
                                 });
                        c2s.clear();
                        conn2yunying_.clear();
                        if (nsv_) {
                                nsv_->stop();
                                nsv_.reset();
                        }
                        GM_DLOG << *this << " stop";
                }

                ostream& operator<<(ostream& os, const yunying_mgr& ym) {
                        return os << ym.name_;
                }

                void yunying_mgr::gag_role_by_gid_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->gag_role_by_gid_done(result);
                }

                void yunying_mgr::ban_role_by_gid_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->ban_role_by_gid_done(result);
                }

                void yunying_mgr::remove_punish_by_gid_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->remove_punish_by_gid_done(result);
                }

                void yunying_mgr::kick_role_by_gid_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->kick_role_by_gid_done(result);
                }

                void yunying_mgr::fetch_world_chat_done(uint32_t sid, const pd::world_chat_array& world_chat, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_world_chat_done(world_chat, result);
                }

                void yunying_mgr::find_role_done(uint32_t sid, const pd::yunying_role_info& role, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->find_role_done(role, result);
                }

                void yunying_mgr::fetch_rank_list_done(uint32_t sid, pd::result result, const pd::rank_list_array& rank_list_array) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_rank_list_done(result, rank_list_array);
                }

                void yunying_mgr::recharge_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->recharge_done(result);
                }

                void yunying_mgr::send_mail_done(uint32_t sid, pd::result result, const pd::gid_array& roles) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->send_mail_done(result, roles);
                }

                void yunying_mgr::fetch_log_done(uint32_t sid, pd::result result, const pd::yunying_item_change_array& item_changes) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_log_done(result, item_changes);
                }

                void yunying_mgr::add_to_white_list_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->add_to_white_list_done(result);
                }

                void yunying_mgr::remove_from_white_list_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->remove_from_white_list_done(result);
                }

                void yunying_mgr::fetch_white_list_done(uint32_t sid, pd::result result, const pd::white_list_array& white_lists) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_white_list_done(result, white_lists);
                }

                void yunying_mgr::manage_announcement_done(uint32_t sid, pd::result result, const vector<int>& keys) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->manage_announcement_done(result, keys);
                }

                void yunying_mgr::server_images_announcements_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->server_images_announcements_done(result);
                }

                void yunying_mgr::fetch_punished_done(uint32_t sid, pd::result result, const pd::yunying_fetch_punished_info_array& yspia) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_punished_done(result, yspia);
                }

                void yunying_mgr::fetch_sent_mail_done(uint32_t sid, pd::result result, const pd::yunying_fetch_sent_mail_info_array& yssmia) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_sent_mail_done(result, yssmia);
                }

                void yunying_mgr::fetch_login_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_login_record_array& yflra) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_login_record_done(result, yflra);
                }

                void yunying_mgr::fetch_recharge_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_recharge_record_array& yfrra) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_recharge_record_done(result, yfrra);
                }

                void yunying_mgr::fetch_currency_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_currency_record_array& yfcra) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_currency_record_done(result, yfcra);
                }

                void yunying_mgr::login_gonggao_done(uint32_t sid) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->login_gonggao_done();
                }

                void yunying_mgr::fetch_role_list_done(uint32_t sid, const pd::yunying_role_simple_info_array& role_infos, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->fetch_role_list_done(role_infos, result);
                }

                void yunying_mgr::add_stuff_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->add_stuff_done(result);
                }

                void yunying_mgr::dec_stuff_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->dec_stuff_done(result);
                }

                void yunying_mgr::operates_activity_done(uint32_t sid, pd::result result) {
                        lock_guard<mutex> lock(lock_);
                        if (sid2yunying_.count(sid) == 0) {
                                return;
                        }
                        sid2yunying_.at(sid)->operates_activity_done(result);
                }
        }
}
