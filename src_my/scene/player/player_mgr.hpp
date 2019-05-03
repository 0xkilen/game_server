#pragma once

#include "player.hpp"
#include "scene/item.hpp"
#include "scene/mail/mail.hpp"
#include "scene/common_logic.hpp"
#include "proto/data_server.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/data_mirror.pb.h"
#include "proto/data_global.pb.h"
#include "log.hpp"
#include "net/websocket/server_conn.hpp"
#include "net/server_ssl.hpp"
#include "db/connector.hpp"
#include "utils/service_thread.hpp"
#include "utils/game_def.hpp"
#include "utils/gid.hpp"
#include "utils/time_utils.hpp"
#include "utils/name_utils.hpp"
#include "utils/proto_utils.hpp"
#include "utils/assert.hpp"
#include "utils/pool.hpp"
#include "config/options_ptts.hpp"
#include "config/serverlist_ptts.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <mutex>
#include <google/protobuf/descriptor.h>

using namespace std;
namespace pb = google::protobuf;
namespace pd = proto::data;

#define PLAYER_CALL_FUNC_LOG SPLAYER_DLOG << "player call func " << __func__ << " " << __LINE__;

namespace nora {
        namespace scene {

                using gm_mgr = server_process_mgr<gm>;
                using mirror_mgr = server_process_mgr<mirror>;
                using login_mgr = server_process_mgr<login>;

                template <typename T>
                class player_mgr {
                public:
                        player_mgr() : st_idx_(0), stop_(false) {
                                st_ = make_shared<service_thread>("player_mgr");
                                st_->start();
                                auto ptt = PTTS_GET_COPY(options, 1u);
                                for (auto i = 0; i < int(ptt.scene_info().player_thread_count()); ++i) {
                                        string name(string("player-thread-") + to_string(i));
                                        sts_.push_back(make_shared<service_thread>(name));
                                }

                                gamedb_notice_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                                            ptt.game_db().user(),
                                                                            ptt.game_db().password(),
                                                                            ptt.game_db().database());
                                gamedb_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                                     ptt.game_db().user(),
                                                                     ptt.game_db().password(),
                                                                     ptt.game_db().database());
                                gamedb_->start();
                                gamedb_notice_->start();

                                player<T>::static_init();
                                SPLAYER_DLOG << *this << " create";
                        }

                        static player_mgr<T>& instance() {
                                static player_mgr<T> inst;
                                return inst;
                        }

                        void start(const string& client_ip, unsigned short client_port) {
                                client_ip_ = client_ip;
                                client_port_ = client_port;
                                auto db_msg = make_shared<db::message>("load_server",
                                                                       db::message::req_type::rt_select,
                                                                       [this] (const shared_ptr<db::message>& msg) {
                                                                               lock_guard<mutex> lock(lock_);
                                                                               auto& results = msg->results();
                                                                               if (results.size() <= 0) {
                                                                                       server_open_day_ = refresh_day();
                                                                                       this->update_server_db();
                                                                               } else {
                                                                                       const auto& result = results.front();
                                                                                       ASSERT(result.size() == 1);
                                                                                       pd::server server;
                                                                                       server.ParseFromString(boost::any_cast<string>(result[0]));
                                                                                       server_open_day_ = server.open_day();
                                                                               }
                                                                       });
                                gamedb_->push_message(db_msg, st_);
                                db_msg = make_shared<db::message>("load_gid_and_rolename",
                                                                  db::message::req_type::rt_select,
                                                                  [this] (const shared_ptr<db::message>& msg) {
                                                                          lock_guard<mutex> lock(lock_);
                                                                          auto& results = msg->results();
                                                                          for (const auto& i : results) {
                                                                                  ASSERT(i.size() == 3);
                                                                                  auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                                  auto rolename = boost::any_cast<string>(i[1]);
                                                                                  auto username = boost::any_cast<string>(i[2]);
                                                                                  username2gid_[username] = gid;
                                                                                  gid2rolename_[gid] = rolename;
                                                                                  rolename2gid_[rolename] = gid;
                                                                                  SPLAYER_DLOG << "add gid and rolename " << gid << " " << rolename;
                                                                          }
                                                                          mail_ = make_shared<mail>(st_);
                                                                          mail_->start();
                                                                          mail_->find_mails_done_cb_ = [this] (auto role, const auto& mails) {
                                                                                  lock_guard<mutex> lock(lock_);
                                                                                  if (gid2playing_player_.count(role) <= 0) {
                                                                                          return;
                                                                                  }
                                                                                  gid2playing_player_.at(role)->add_mails(mails);
                                                                                  for (const auto& i : mails.mails()) {
                                                                                          mail_->delete_mail(i.gid());
                                                                                  }
                                                                          };
                                                                  });
                                gamedb_->push_message(db_msg, st_);
                                db_msg = make_shared<db::message>("load_used_rolenames",
                                                                  db::message::req_type::rt_select,
                                                                  [this] (const shared_ptr<db::message>& msg) {
                                                                          lock_guard<mutex> lock(lock_);
                                                                          auto& results = msg->results();
                                                                          if (results.size() > 0) {
                                                                                  const auto& result = results.front();
                                                                                  ASSERT(result.size() == 1);
                                                                                  pd::used_rolename_array used_rolenames;
                                                                                  used_rolenames.ParseFromString(boost::any_cast<string>(result[0]));
                                                                                  for (const auto& name : used_rolenames.used_names()) {
                                                                                          used_rolenames_.insert(name);
                                                                                  }
                                                                          }
                                                                  });
                                gamedb_->push_message(db_msg, st_);
                                send_username_to_logind();
                        }

                        void send_username_to_logind() {
                                ps::base sync_msg;
                                auto *sync = sync_msg.MutableExtension(ps::s2l_username_sync::sus);
                                const auto& ptt = PTTS_GET(options, 1);
                                sync->set_server_id(ptt.scene_info().server_id());
                                for (const auto& i : username2gid_) {
                                        sync->add_username(i.first);
                                }
                                login_mgr::instance().broadcast_msg(sync_msg);
                        }

                        void update_server_db() {
                                auto db_msg = make_shared<db::message>("update_server", db::message::req_type::rt_update);
                                pd::server server;
                                server.set_open_day(server_open_day_);
                                /*if (gongdou_effect_.from() != 0) {
                                        *server.mutable_gongdou() = gongdou_effect_;
                                }*/
                                //server.set_marriage_count(marriage_count_);

                                string str;
                                server.SerializeToString(&str);
                                db_msg->push_param(str);
                                gamedb_->push_message(db_msg);
                        }

                        void update_used_rolenames_db() {
                                auto db_msg = make_shared<db::message>("update_used_rolenames", db::message::req_type::rt_update);
                                pd::used_rolename_array used_rolenames;
                                for (const auto& name : used_rolenames_) {
                                        used_rolenames.add_used_names(name);
                                }

                                string str;
                                used_rolenames.SerializeToString(&str);
                                db_msg->push_param(str);
                                gamedb_->push_message(db_msg);
                        }

                        void check_stopped() {
                                if (conn2player_.empty() && stop_.load()) {
                                        ASSERT(username2player_.empty());
                                        ASSERT(gid2playing_player_.empty());
                                        if (!gid2player_.empty()) {
                                                gid2player_.clear();
                                        }
                                        for (const auto& i : sts_) {
                                                i->stop();
                                        }
                                        sts_.clear();

                                        if (nsv_) {
                                                nsv_->stop();
                                                nsv_.reset();
                                        }
                                        if (gamedb_) {
                                                gamedb_->stop();
                                                gamedb_.reset();
                                        }
                                        if (gamedb_notice_) {
                                                gamedb_notice_->stop();
                                                gamedb_notice_.reset();
                                        }
                                        if (mail_) {
                                                mail_->stop();
                                                mail_.reset();
                                        }

                                        ASSERT(stopped_cb_);
                                        stopped_cb_();
                                }
                        }

                        void init_player(const shared_ptr<player<T>>& pl) {
                                pl->init();
                                init_player_cbs(pl);
                                pl->login_cb_ = [this] (const auto& pl) {
                                        SPLAYER_DLOG << *pl << " login cb";
                                        lock_guard<mutex> lock(lock_);
                                        const auto& username = pl->username();
                                        if (username2player_.count(username) != 0) {
                                                SPLAYER_DLOG << *pl << " login cb elsewhere";
                                                auto& old_pl = username2player_.at(username);
                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                                notice->set_reason(pd::LOGIN_ELSEWHERE);
                                                notice->set_same_ip(old_pl->ip() == pl->ip());
                                                old_pl->send_to_client(msg, true);
                                                old_pl->quit();
                                                return pd::AGAIN;
                                        }
                                        auto options_ptt = PTTS_GET_COPY(options, 1);
                                        if (gid2playing_player_.size() >= options_ptt.scene_info().max_roles()) {
                                                auto mirror_roles = this->get_online_mirror_roles();
                                                if (mirror_roles.size() == 0) {
                                                        SPLAYER_DLOG << *pl << " login cb full";
                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                                        notice->set_reason(pd::LOGIN_SERVER_FULL);
                                                        pl->send_to_client(msg, true);
                                                        pl->quit();
                                                        return pd::FAILED;
                                                } else {
                                                        auto iter = mirror_roles.begin();
                                                        advance(iter, rand() % mirror_roles.size());
                                                        ASSERT(gid2playing_player_.count(*iter) > 0);
                                                        gid2playing_player_.at(*iter)->quit();
                                                        username2player_[pl->username()] = pl;
                                                        return pd::OK;
                                                }
                                        }
                                        username2player_[pl->username()] = pl;
                                        return pd::OK;
                                };
                                pl->start_fetch_data_cb_ = [this] (const auto& pl) {
                                        SPLAYER_DLOG << *pl << " start fetch data cb";
                                        lock_guard<mutex> lock(lock_);
                                        auto *role = pl->get_role();
                                        auto gid = role->gid();
                                        if (gid2player_.count(gid) > 0) {
                                                SPLAYER_DLOG << *pl << " start fetch data cb gid exist";
                                                return false;
                                        }
                                        ASSERT(gid2fetching_data_player_.count(pl->gid()) == 0);
                                        gid2fetching_data_player_[pl->gid()] = pl;
                                        return true;
                                };
                                pl->login_done_cb_ = [this] (const auto& pl, const auto& rolename, const auto& username) {
                                        SPLAYER_DLOG << *pl << " login done cb";
                                        lock_guard<mutex> lock(lock_);
                                        auto *role = pl->get_role();
                                        auto gid = role->gid();
                                        if (gid2player_.count(gid) > 0) {
                                                SPLAYER_DLOG << *pl << " login done cb gid exist";
                                                return false;
                                        }
                                        if (gid2rolename_.count(gid) <= 0) {
                                                SPLAYER_DLOG << "add rolename, username, gid " << rolename << " " << username << " " << gid;
                                                gid2rolename_[gid] = rolename;
                                                ASSERT(rolename2gid_.count(rolename) <= 0);
                                                rolename2gid_[rolename] = gid;
                                                ASSERT(username2gid_.count(username) <= 0);
                                                username2gid_[username] = gid;
                                                this->send_username_to_logind();
                                        }
                                        gid2player_[gid] = pl;
                                        gid2playing_player_[gid] = pl;
                                        level2playing_player_[pl->level()].insert(pl->gid());
                                        gender2playing_player_[pl->gender()].insert(pl->gid());
                                        guanpin2playing_player_[pl->guanpin()].insert(pl->gid());
                                        gid2fetching_data_player_.erase(pl->gid());

                                        pd::role data;
                                        role->serialize(&data);
                                        ASSERT(role_login_cb_);
                                        role_login_cb_(gid, data);

                                        mail_->find_mails(gid);

                                        const auto& ptt = PTTS_GET(options, 1);
                                        gm_mgr_class::instance().admin_role_online(ptt.scene_info().server_id(), gid);

                                        return true;
                                };
                                pl->start_playing_cb_ = [this] (const auto& pl) {
                                        SPLAYER_DLOG << *pl << " start playing cb";
                                        lock_guard<mutex> lock(lock_);
                                        auto iter = player_call_funcs_.find(pl->gid());
                                        if (iter != player_call_funcs_.end()) {
                                                for (const auto& i : iter->second) {
                                                        i(pl);
                                                }
                                        }
                                        player_call_funcs_.erase(pl->gid());
                                };
                                pl->stop_playing_cb_ = [this] (const auto& pl) {
                                        SPLAYER_DLOG << *pl << " stop playing cb";
                                        lock_guard<mutex> lock(lock_);
                                        auto gid = pl->gid();
                                        auto iter = player_call_funcs_.find(gid);
                                        if (iter != player_call_funcs_.end()) {
                                                for (const auto& i : iter->second) {
                                                        i(pl);
                                                }
                                                player_call_funcs_.erase(iter);
                                                return false;
                                        }
                                        if (pl->has_async_call()) {
                                                return false;
                                        }
                                        gid2playing_player_.erase(gid);
                                        level2playing_player_[pl->level()].erase(pl->gid());
                                        gender2playing_player_[pl->gender()].erase(pl->gid());
                                        guanpin2playing_player_[pl->guanpin()].erase(pl->gid());

                                        ASSERT(role_logout_cb_);
                                        role_logout_cb_(pl->gid(), pl->spouse());

                                        const auto& ptt = PTTS_GET(options, 1);
                                        gm_mgr_class::instance().admin_role_offline(ptt.scene_info().server_id(), gid);

                                        if (!pl->mirror_role()) {
                                                ps::base msg;
                                                auto *notice = msg.MutableExtension(ps::sm_role_event::sre);
                                                auto *event = notice->mutable_event();
                                                event->set_gid(pl->gid());
                                                event->set_type(pd::RMET_LOGOUT);
                                                server_process_mgr<mirror>::instance().random_send_msg(msg);
                                        }
                                        return true;
                                };
                                pl->quit_cb_ = [this] (const auto& pl) {
                                        SPLAYER_DLOG << *pl << " quit cb";
                                        lock_guard<mutex> lock(lock_);
                                        auto username = pl->username();
                                        conn2player_.erase(pl->conn());
                                        auto iter = username2player_.find(username);
                                        if (iter != username2player_.end() && iter->second == pl) {
                                                username2player_.erase(iter);
                                        }
                                        if (pl->get_role()) {
                                                gid2fetching_data_player_.erase(pl->gid());
                                                // player call func interrupt fetching data player
                                                if (gid2player_.count(pl->gid()) > 0 && gid2player_.at(pl->gid()) == pl) {
                                                        gid2player_.erase(pl->gid());
                                                        player_call_funcs_.erase(pl->gid());
                                                }
                                        }
                                        this->check_stopped();
                                };
                                pl->process_msg_cb_ = [this] (const auto *desc, auto size) {
                                        lock_guard<mutex> lock(up_msg_stats_lock_);
                                        if (up_msg_stats_.msgs_.count(desc) == 0) {
                                                up_msg_stats_.msgs_[desc].desc_ = desc;
                                        }
                                        auto& msg = up_msg_stats_.msgs_[desc];
                                        msg.total_size_ += size;
                                        msg.max_size_ = max(msg.max_size_, size);
                                        msg.count_ += 1;

                                        up_msg_stats_.total_size_ += size;
                                        up_msg_stats_.total_count_ += 1;

                                        if (up_msg_stats_.total_count_ % 30000 == 0) {
                                                this->print_up_msg_stats();
                                        }
                                };
                                pl->send_msg_cb_ = [this] (const auto *desc, auto size) {
                                        lock_guard<mutex> lock(down_msg_stats_lock_);
                                        if (down_msg_stats_.msgs_.count(desc) == 0) {
                                                down_msg_stats_.msgs_[desc].desc_ = desc;
                                        }
                                        auto& msg = down_msg_stats_.msgs_[desc];
                                        msg.total_size_ += size;
                                        msg.max_size_ = max(msg.max_size_, size);
                                        msg.count_ += 1;

                                        down_msg_stats_.total_size_ += size;
                                        down_msg_stats_.total_count_ += 1;

                                        if (down_msg_stats_.total_count_ % 30000 == 0) {
                                                this->print_down_msg_stats();
                                        }
                                };
                        }

                        void init_player_cbs(const shared_ptr<player<T>>& pl) {
                                pl->rolename2gid_func_ = [this] (const auto& rolename) {
                                        lock_guard<mutex> lock(lock_);
                                        auto it = rolename2gid_.find(rolename);
                                        if (it != rolename2gid_.end()) {
                                                return it->second;
                                        } else {
                                                return 0ul;
                                        }
                                };
                                pl->gid2rolename_func_ = [this] (auto gid) {
                                        return this->gid2rolename_lock(gid);
                                };
                                pl->random_role_name_func_ = [this] (auto pool_start) {
                                        return this->random_role_name(pool_start);
                                };
                                pl->role_change_name_func_ = [this] (auto gid, const auto& name, auto is_former_name, const auto& friends, const auto& event_res) {
                                        pd::dynamic_data mdd;
                                        auto result = pd::OK;
                                        {
                                                lock_guard<mutex> lock(lock_);
                                                ASSERT(gid2rolename_.count(gid) > 0);
                                                do {
                                                        if (!is_former_name) {
                                                                if (used_rolenames_.count(name) > 0) {
                                                                        result = pd::OTHER_FORMER_NAME;
                                                                        break;
                                                                }
                                                        }

                                                        if (rolename2gid_.count(name) > 0) {
                                                                result = pd::NAME_EXIST;
                                                                break;
                                                        }
                                                } while (false);

                                                if (result == pd::OK) {
                                                        auto old_name = gid2rolename_.at(gid);
                                                        mdd.add_name(old_name);
                                                        mdd.add_name(name);
                                                        rolename2gid_.erase(old_name);
                                                        rolename2gid_[name] = gid;
                                                        gid2rolename_[gid] = name;
                                                        used_rolenames_.insert(old_name);
                                                        this->update_used_rolenames_db();
                                                }
                                        }

                                        if (result == pd::OK) {
                                                const auto& ptt = PTTS_GET(role_logic, 1);
                                                if (ptt.has_change_name_mail()) {
                                                        for (const auto& i : friends) {
                                                                this->send_mail(i, ptt.change_name_mail(), mdd, pd::event_array());
                                                        }
                                                }
                                        }

                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(gid,
                                                               [name, result, event_res] (const auto& pl) {
                                                                       pl->role_change_name_done(name, result, event_res);
                                                               });
                                };
                                pl->role_online_func_ = [this] (auto gid) {
                                        return this->role_online(gid);
                                };
                                pl->get_online_roles_func_ = [this] {
                                        return this->get_online_roles();
                                };
                                pl->check_role_exists_func_ = [this] (auto gid) {
                                        return this->check_role_exists(gid);
                                };
                                pl->server_open_day_func_ = [this] {
                                        lock_guard<mutex> lock(lock_);
                                        return server_open_day_;
                                };
                                pl->get_images_announcements_func_ = [this] {
                                        return gm_mgr_class::instance().get_images_announcements();
                                };
                                pl->add_marriage_count_func_ = [this] {
                                        return add_marriage_count();
                                };
                                pl->marriage_count_func_ = [this] {
                                        return marriage_count();
                                };
                                pl->relation_send_gift_func_ = [this] (auto from, auto to_gid, auto item, auto gift_count, const auto& event_res) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(to_gid,
                                                               [from, item, gift_count, event_res] (const auto& pl) {
                                                                       pl->relation_receive_gift_from_other(from, item, gift_count, event_res);
                                                               });
                                };
                                pl->relation_send_gift_done_cb_ = [this] (auto from_gid, const auto& to, auto item, auto gift_count, const auto& event_res, auto result, auto to_league) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(from_gid,
                                                               [to, item, gift_count, event_res, result, to_league] (const auto& pl) {
                                                                       pl->relation_send_gift_done(to, item, gift_count, event_res, result, to_league);
                                                               });
                                };
                                pl->levelup_cb_ = [this] (auto role, auto old_level, auto level, auto has_mansion) {
                                        {
                                                lock_guard<mutex> lock(lock_);

                                                level2playing_player_[old_level].erase(role);
                                                level2playing_player_[level].insert(role);
                                        }
                                        ASSERT(this->role_levelup_cb_);
                                        this->role_levelup_cb_(role, old_level, level, has_mansion);
                                };
                                pl->add_notice_cb_ = [this] (auto notice_gid, auto role_gid) {
                                        lock_guard<mutex> lock(this->lock_);
                                        if (this->gid2playing_player_.count(role_gid) > 0 && this->gid2playing_player_.at(role_gid)->conn()) {
                                                this->gid2playing_player_.at(role_gid)->on_new_notice(notice_gid);
                                        }
                                };
                                pl->private_chat_func_ = [this] (const auto& from, auto time, auto to, const auto& content, const auto& voice) {
                                        if (this->role_online(to)) {
                                                lock_guard<mutex> lock(lock_);
                                                gid2playing_player_.at(to)->send_chat(pd::CT_PRIVATE, from, time, content, voice, 0, to);
                                        } else {
                                                this->player_call_func(to,
                                                                       [time, content, voice, from_gid = from.simple_info().gid()] (const auto& pl) {
                                                                               pl->notice_offline_private_chat(from_gid, time, content, voice);
                                                                       });
                                        }
                                };
                                pl->world_chat_func_ = [this] (auto type, const auto& from, auto time, const auto& content, const auto& voice) {
                                        lock_guard<mutex> lock(lock_);
                                        for (const auto& i : gid2playing_player_) {
                                                i.second->send_chat(type, from, time, content, voice);
                                        }
                                        this->push_world_chat(type, from, time, content);
                                };
                                pl->world_chat_role_list_func_ = [this] {
                                        lock_guard<mutex> lock(lock_);
                                        set<uint64_t> ret;
                                        for (const auto& i : world_chat_list_) {
                                                ret.insert(i.gid());
                                        }
                                        return ret;
                                };
                                pl->comment_func_ = [this] (auto from, auto to, auto reply_to, const auto& content) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(to,
                                                               [from, reply_to, content] (const auto& pl) {
                                                                       pl->comment_by_other(from, reply_to, content);
                                                               });
                                };
                                pl->comment_done_cb_ = [this] (auto from, auto result, auto to, const auto& comment) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(from) > 0) {
                                                gid2playing_player_.at(from)->comment_done(result, to, comment);
                                        }
                                };
                                pl->fetch_comments_func_ = [this] (auto fetcher, auto fetchee) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(fetchee,
                                                               [fetcher] (const auto& pl) {
                                                                       pl->other_fetch_comments(fetcher);
                                                               });
                                };
                                pl->fetch_comments_done_cb_ = [this] (auto fetcher, auto fetchee, const auto& comments) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(fetcher) > 0) {
                                                gid2playing_player_.at(fetcher)->fetch_comments_done(fetchee, comments);
                                        }
                                };
                                pl->delete_comment_func_ = [this] (auto deleter, auto role, auto gid) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(role,
                                                               [deleter, gid] (const auto& pl) {
                                                                       pl->delete_comment(deleter, gid);
                                                               });
                                };
                                pl->delete_comment_done_cb_ = [this] (auto deleter, auto result, auto role, auto gid) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(deleter) > 0) {
                                                gid2playing_player_.at(deleter)->delete_comment_done(result, role, gid);
                                        }
                                };
                                pl->relation_apply_friend_func_ = [this] (auto adder, auto addee) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(addee,
                                                               [adder] (const auto& pl) {
                                                                       pl->relation_add_friend_application(adder);
                                                               });
                                };
                                pl->relation_add_friend_func_ = [this] (auto adder, auto addee, auto had_apply) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(addee,
                                                               [adder, had_apply] (const auto& pl) {
                                                                       pl->relation_add_friend(adder, had_apply);
                                                               });
                                };
                                pl->relation_add_friend_done_cb_ = [this] (auto addee, auto adder, auto had_apply, auto result) {
                                        if (result == pd::OK) {
                                                lock_guard<mutex> lock(lock_);
                                                if (gid2playing_player_.count(adder) > 0) {
                                                        gid2playing_player_.at(adder)->relation_add_friend_done(addee, result, had_apply);
                                                }
                                        } else {
                                                PLAYER_CALL_FUNC_LOG;
                                                this->player_call_func(adder,
                                                                       [addee, result] (const auto& pl) {
                                                                               pl->relation_add_friend_done(addee, result);
                                                                       });
                                        }
                                };
                                pl->relation_apply_friend_done_cb_ = [this] (auto addee, auto adder, auto result) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(adder) > 0) {
                                                gid2playing_player_.at(adder)->relation_add_friend_application_done(addee, result);
                                        }
                                };
                                pl->relation_add_in_blacklist_func_ = [this] (auto adder, auto addee) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(addee,
                                                               [adder] (const auto& pl) {
                                                                       pl->relation_add_in_blacklist(adder);
                                                               });
                                };
                                pl->relation_remove_friend_func_ = [this] (auto remover, auto removee) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(removee,
                                                               [remover] (const auto& pl) {
                                                                       pl->relation_removed_by_friend(remover);
                                                               });
                                };
                                pl->get_info_func_ = [this] (auto to , auto from, auto simple) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(from,
                                                               [to, simple] (const auto& pl) {
                                                                       pl->send_info_to(to, simple);
                                                               });
                                };
                                pl->get_role_info_by_str_func_ = [this] (auto to, const auto& str, const auto& gids, const auto& data) {
                                        if (gids.empty()) {
                                                lock_guard<mutex> lock(lock_);
                                                if (gid2playing_player_.count(to) <= 0) {
                                                        return;
                                                }
                                                gid2playing_player_.at(to)->get_role_info_by_str_done(str, data);
                                        } else {
                                                PLAYER_CALL_FUNC_LOG;
                                                this->player_call_func(gids.front(),
                                                                       [to, str, gids, data] (const auto& pl) {
                                                                               pl->get_role_info_by_str_to(to, str, gids, data);
                                                                       });
                                        }
                                };
                                pl->send_info_func_ = [this] (auto to, const auto& data, auto simple) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(to) <= 0) {
                                                return;
                                        }
                                        gid2playing_player_.at(to)->send_info(data, simple);
                                };
                                pl->suggest_friends_func_ = [this] (auto role,
                                                                          auto gender,
                                                                          auto level,
                                                                          auto origin,
                                                                          auto& candidates,
                                                                          const auto& exclude) {
                                        lock_guard<mutex> lock(lock_);
                                        ASSERT(gid2playing_player_.count(role) > 0);

                                        if (origin != pd::RSO_GONGDOU) {
                                                auto min_level = 1u;
                                                if (level > 5) {
                                                        min_level = level - 5;
                                                }
                                                for (auto i = min_level; i <= level + 5; ++i) {
                                                        if (level2playing_player_.count(i) == 0) {
                                                                continue;
                                                        }
                                                        const auto& level_players = level2playing_player_.at(i);
                                                        for (auto i : level_players) {
                                                                if (exclude.count(i) > 0) {
                                                                        continue;
                                                                }
                                                                candidates[pd::RST_LEVEL].insert(i);
                                                        }
                                                }
                                                candidates[pd::RST_LEVEL].erase(role);
                                                for (auto i : candidates[pd::RST_LEVEL]) {
                                                        SPLAYER_DLOG << *this << " level " << i;
                                                }

                                                const auto& gender_players = gender == pd::MALE ?
                                                        gender2playing_player_[pd::FEMALE] : gender2playing_player_[pd::MALE];
                                                for (auto i : gender_players) {
                                                        if (exclude.count(i) > 0) {
                                                                continue;
                                                        }
                                                        candidates[pd::RST_ROMANCE].insert(i);
                                                        SPLAYER_DLOG << *this << " romance " << i;
                                                }
                                        }

                                        auto try_times = 0;
                                        while (candidates[pd::RST_STRANGER].size() < 8 && try_times < 16) {
                                                try_times += 1;

                                                auto iter = gid2playing_player_.begin();
                                                advance(iter, rand() % gid2playing_player_.size());
                                                if (iter->first == role || exclude.count(iter->first) > 0) {
                                                        continue;
                                                }
                                                candidates[pd::RST_STRANGER].insert(iter->first);
                                                SPLAYER_DLOG << *this << " stranger " << iter->first;
                                        }

                                        gid2playing_player_.at(role)->suggest_friends_done(origin, candidates);
                                };
                                pl->ai_suggest_friend_func_ = [this] (auto role, const auto& friends) {
                                        lock_guard<mutex> lock(lock_);
                                        if (gid2playing_player_.count(role) <= 0) {
                                                return;
                                        }

                                        vector<uint64_t> roles;
                                        for (const auto& i : gid2playing_player_) {
                                                if (i.first == role) {
                                                        continue;
                                                }
                                                if (friends.count(i.first) > 0) {
                                                        continue;
                                                }
                                                roles.push_back(i.first);
                                        }

                                        if (roles.empty()) {
                                                gid2playing_player_.at(role)->ai_suggest_friend_done(0);
                                        } else {
                                                auto iter = roles.begin();
                                                advance(iter, rand() % roles.size());
                                                gid2playing_player_.at(role)->ai_suggest_friend_done(*iter);
                                        }
                                };
                                pl->yunying_recharge_done_cb_ = [this] (auto sid, auto gm_id, auto result) {
                                        gm_mgr_class::instance().yunying_recharge_done(sid, gm_id, result);
                                };
                                pl->yunying_cdkey_func_ = [this] (const auto& username, auto role, const auto& yci, const auto& ip, auto level, const auto& code) {
                                        ASSERT(gid2rolename_.count(role) > 0);
                                        /*bi::instance().cdkey(username, gid2rolename_.at(role), yci, ip, role, level, code,
                                                             [this, role] (auto result, const auto& events, const auto& msg) {
                                                                     PLAYER_CALL_FUNC_LOG;
                                                                     this->player_call_func(role,
                                                                                            [this, result, events, msg] (const auto& pl) {
                                                                                                    pl->yunying_cdkey_done(result, events, msg);
                                                                                            });
                                                             });*/
                                };
                                pl->yunying_add_stuff_done_cb_ = [this] (auto sid, auto gm_id, auto result) {
                                        gm_mgr_class::instance().yunying_add_stuff_done(sid, gm_id, result);
                                };
                                pl->yunying_dec_stuff_done_cb_ = [this] (auto sid, auto gm_id, auto result) {
                                        gm_mgr_class::instance().yunying_dec_stuff_done(sid, gm_id, result);
                                };

                                pl->vip_exp_sync_to_spouse_func_ = [this] (auto spouse, auto role, auto vip_exp) {
                                        PLAYER_CALL_FUNC_LOG;
                                        this->player_call_func(spouse,
                                                               [role, vip_exp] (const auto& pl) {
                                                                       pl->marriage_update_spouse_vip_exp(role, vip_exp);
                                                               });
                                };
                        }

                        void start_rest() {
                                clock::instance().register_hour_cb(5,
                                                                   [this] {
                                                                           lock_guard<mutex> lock(lock_);
                                                                           for (const auto& i : gid2playing_player_) {
                                                                                   i.second->on_new_day();
                                                                           }
                                                                   });

                                nsv_ = make_shared<net::server<T>>("playermgr-netserver");
                                nsv_->new_conn_cb_ = [this] (const auto& c) -> shared_ptr<service_thread> {
                                        lock_guard<mutex> lock(lock_);
                                        auto st = sts_.at(st_idx_);
                                        st_idx_++;
                                        st_idx_ %= sts_.size();
                                        auto pl = make_shared<player<T>>(st,
                                                                         nsv_,
                                                                         c,
                                                                         gamedb_notice_,
                                                                         false,
                                                                         true);
                                        this->init_player(pl);
                                        conn2player_[c] = pl;
                                        return st;
                                };
                                nsv_->msg_cb_ = [this] (const auto& c, const auto& msg) {
                                        lock_guard<mutex> lock(lock_);
                                        if (conn2player_.count(c) == 0) {
                                                return;
                                        }

                                        conn2player_.at(c)->process_msg(msg);
                                };
                                nsv_->error_cb_ = [this] (const auto& c) {
                                        lock_guard<mutex> lock(lock_);
                                        if (conn2player_.count(c) == 0) {
                                                return;
                                        }
                                        conn2player_.at(c)->quit();
                                        conn2player_.erase(c);
                                };

                                for_each(sts_.begin(), sts_.end(), [] (const auto& i) { i->start(); });
                                nsv_->start();
                                nsv_->listen(client_ip_, client_port_);

                                SPLAYER_ILOG << *this << " start";
                        }

                        void stop() {
                                lock_guard<mutex> lock(lock_);
                                if (stop_.load()) {
                                    return;
                                }
                                stop_.store(true);

                                nsv_->stop();

                                pcs::base msg;
                                auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                notice->set_reason(pd::SERVER_CLOSE);
                                auto c2p = conn2player_;
                                for (const auto& i : c2p) {
                                        i.second->send_to_client(msg);
                                        i.second->quit();
                                }
                                if (tower_) {
                                        tower_->stop();
                                }

                                check_stopped();
                                SPLAYER_DLOG << *this << " stop";
                        }

                        void kick_player_by_username(const string& username) {
                                lock_guard<mutex> lock(lock_);
                                if (username2player_.count(username) == 0) {
                                        return;
                                }
                                pcs::base msg;
                                auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                notice->set_reason(pd::KICKED);
                                username2player_.at(username)->send_to_client(msg);
                                username2player_.at(username)->quit();
                        }
                        void kick_player_by_rolename(const string& rolename) {
                                lock_guard<mutex> lock(lock_);
                                if (rolename2gid_.count(rolename) <= 0) {
                                        return;
                                }
                                auto gid = rolename2gid_.at(rolename);
                                if (gid2player_.count(gid) <= 0) {
                                        return;
                                }
                                pcs::base msg;
                                auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                notice->set_reason(pd::KICKED);
                                gid2player_.at(gid)->quit();
                        }

                        void gag_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid, uint32_t gag_until_time, const string& reason) {
                                if(!check_role_exists(gid)) {
                                        gm_mgr_class::instance().gag_role_by_gid_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(gid,
                                                [this, sid, gm_id, gag_until_time, gid, reason] (const auto& pl) {
                                                        pl->change_punish_gag(gag_until_time, reason);
                                                        gm_mgr_class::instance().gag_role_by_gid_done(sid, gm_id, pd::OK);
                                                });
                        }

                        void remove_gag_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid) {
                                if(!check_role_exists(gid)) {
                                        gm_mgr_class::instance().remove_punish_role_by_gid_done(sid, gm_id, pd::NOT_FOUND, pd::AERT_GAG);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(gid,
                                                [this, sid, gm_id, gid] (const auto& pl) {
                                                        pl->reset_gag_time();
                                                        gm_mgr_class::instance().remove_punish_role_by_gid_done(sid, gm_id, pd::OK, pd::AERT_GAG);

                                                });
                        }

                        void ban_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid, uint32_t ban_until_time, const string& reason) {
                                if(!check_role_exists(gid)) {
                                        gm_mgr_class::instance().ban_role_by_gid_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(gid,
                                                [this, ban_until_time, sid, gm_id, gid, reason] (const auto& pl) {
                                                        pl->change_punish_ban(ban_until_time, reason);
                                                        pl->quit();
                                                        gm_mgr_class::instance().ban_role_by_gid_done(sid, gm_id, pd::OK);
                                                });
                        }

                        void remove_ban_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid) {
                                if(!check_role_exists(gid)) {
                                        gm_mgr_class::instance().remove_punish_role_by_gid_done(sid, gm_id, pd::NOT_FOUND, pd::AERT_BAN);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(gid,
                                                [this, sid, gm_id, gid] (const auto& pl) {
                                                        pl->remove_ban_time();
                                                        gm_mgr_class::instance().remove_punish_role_by_gid_done(sid, gm_id, pd::OK, pd::AERT_BAN);
                                                });
                        }

                        void kick_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid, const string& reason) {
                                if(!check_role_exists(gid)) {
                                        gm_mgr_class::instance().kick_role_by_gid_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(gid,
                                                 [this, sid, gm_id, gid, reason] (const auto& pl) {
                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_quit::sq);
                                                        notice->set_reason(pd::KICKED);
                                                        pl->send_to_client(msg, true);
                                                        pl->quit();
                                                        gm_mgr_class::instance().kick_role_by_gid_done(sid, gm_id, pd::OK);
                                                        db_log::instance().log_punish_role_by_gid(*pl->get_role(), reason, pd::log::PUNISH_ROLE_KICK);
                                                });
                        }

                        void fetch_world_chat(uint32_t sid, uint32_t gm_id) {
                                lock_guard<mutex> lock(lock_);
                                pd::world_chat_array world_chat;
                                for (const auto& i : world_chat_list_) {
                                        *world_chat.add_world_chat() = i;
                                }
                                world_chat_list_.clear();
                                gm_mgr_class::instance().fetch_world_chat_done(sid, gm_id, world_chat, pd::OK);
                        }

                        void push_world_chat(pd::chat_type type, const pd::role_info& from, uint32_t time, const string& content) {
                                pd::world_chat ycr;
                                ycr.set_type(type);
                                ycr.set_gid(from.simple_info().gid());
                                ycr.set_rolename(from.simple_info().name());
                                ycr.set_time(time);
                                ycr.set_content(content);

                                world_chat_list_.push_back(ycr);
                                while (world_chat_list_.size() > 50) {
                                        world_chat_list_.pop_front();
                                }
                        }

                        void yunying_find_role_by_gid(uint32_t sid, uint32_t gm_id, uint64_t gid) {
                                if (check_role_exists(gid)) {
                                        PLAYER_CALL_FUNC_LOG;
                                        uint32_t online = 0;
                                        {
                                                lock_guard<mutex> lock(lock_);
                                                if (gid2playing_player_.count(gid) > 0) {
                                                        online = 1;
                                                }
                                        }
                                        player_call_func(gid,
                                                         [this, sid, gm_id, online] (const auto& pl) {
                                                                 pl->async_call(
                                                                         [this, pl, sid, gm_id, online] {
                                                                                 pd::role role;
                                                                                 pl->get_role()->serialize(&role);
                                                                                 gm_mgr_class::instance().find_role_by_gid_done(sid, gm_id, role, pd::OK, online);
                                                                         });
                                                         });
                                } else {
                                        gm_mgr_class::instance().find_role_by_gid_done(sid, gm_id, pd::role(), pd::NOT_FOUND, 0);
                                }
                        }
                        void yunying_find_role_by_rolename(uint32_t sid, uint32_t gm_id, const string& rolename) {
                                if (check_role_exists(rolename)) {
                                        auto gid = rolename2gid(rolename);
                                        PLAYER_CALL_FUNC_LOG;
                                        player_call_func(gid,
                                                         [this, sid, gm_id] (const auto& pl) {
                                                                 pl->async_call(
                                                                         [this, pl, sid, gm_id] {
                                                                                 pd::role role;
                                                                                 pl->get_role()->serialize(&role );
                                                                                 if (gid2playing_player_.count(role.gid()) <= 0) {
                                                                                         gm_mgr_class::instance().find_role_by_gid_done(sid, gm_id, role, pd::OK, 0);
                                                                                 } else {
                                                                                         gm_mgr_class::instance().find_role_by_gid_done(sid, gm_id, role, pd::OK, 1);
                                                                                 }
                                                                         });
                                                         });
                                } else {
                                        gm_mgr_class::instance().find_role_by_gid_done(sid, gm_id, pd::role(), pd::NOT_FOUND, 0);
                                }
                        }

                        void yunying_fetch_role_list(uint32_t sid, uint32_t gm_id, const string& username) {
                                if (!check_player_exists(username)) {
                                        auto msg = dbcache::gen_find_role_req(username, sid, gm_id);
                                        dbcache::instance().send_to_dbcached(msg);
                                } else {
                                        uint64_t gid = 0;
                                        {
                                                lock_guard<mutex> lock(lock_);
                                                gid = username2player_.at(username)->gid();
                                        }
                                        if (gid == 0) {
                                                gm_mgr_class::instance().fetch_role_list_done(sid, gm_id, pd::role(), pd::NOT_FOUND);
                                        } else {
                                                PLAYER_CALL_FUNC_LOG;
                                                player_call_func(gid,
                                                                [this, sid, gm_id] (const auto& pl) {
                                                                        pd::role role;
                                                                        pl->get_role()->serialize(&role);
                                                                        gm_mgr_class::instance().fetch_role_list_done(sid, gm_id, role, pd::OK);
                                                                });
                                        }
                                }
                        }

                        void yunying_add_stuff_by_gid(uint32_t sid, uint32_t gm_id, uint64_t role, const pd::event_array& events) {
                                if (!check_role_exists(role)) {
                                        gm_mgr_class::instance().yunying_add_stuff_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                player_call_func(role,
                                                 [gm_id, sid, events] (const auto& pl) {
                                                         pl->yunying_add_stuff(sid, gm_id, events);
                                                 });
                        }
                        void yunying_add_stuff_by_rolename(uint32_t sid, uint32_t gm_id, const string& rolename, const pd::event_array& events) {
                                if (!check_role_exists(rolename)) {
                                        gm_mgr_class::instance().yunying_add_stuff_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                auto role = rolename2gid(rolename);
                                player_call_func(role,
                                                 [sid, gm_id, events] (const auto& pl) {
                                                         pl->yunying_add_stuff(sid, gm_id, events);
                                                 });
                        }

                        void yunying_dec_stuff_by_gid(uint32_t sid, uint32_t gm_id, uint64_t role, const pd::event_array& events) {
                                if (!check_role_exists(role)) {
                                        gm_mgr_class::instance().yunying_dec_stuff_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                player_call_func(role,
                                                 [sid, gm_id, events] (const auto& pl) {
                                                         pl->yunying_dec_stuff(sid, gm_id, events);
                                                 });
                        }
                        void yunying_dec_stuff_by_rolename(uint32_t sid, uint32_t gm_id, const string& rolename, const pd::event_array& events) {
                                if (!check_role_exists(rolename)) {
                                        gm_mgr_class::instance().yunying_dec_stuff_done(sid, gm_id, pd::NOT_FOUND);
                                        return;
                                }
                                auto role = rolename2gid(rolename);
                                player_call_func(role,
                                                 [sid, gm_id, events] (const auto& pl) {
                                                         pl->yunying_dec_stuff(sid, gm_id, events);
                                                 });
                        }

                        bool check_role_exists(uint64_t role) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2rolename_.count(role) <= 0) {
                                        return false;
                                }
                                return true;
                        }
                        bool check_role_exists(const string& rolename) {
                                lock_guard<mutex> lock(lock_);
                                if (rolename2gid_.count(rolename) <= 0) {
                                        return false;
                                }
                                return true;
                        }
                        bool check_player_exists(const string& username) {
                                lock_guard<mutex> lock(lock_);
                                if (username2player_.count(username) <= 0) {
                                        return false;
                                }
                                return true;
                        }
                        string random_role_name(uint32_t pool_start) {
                                lock_guard<mutex> lock(lock_);
                                return random_name(pool_start, rolename2gid_);
                        }
                        bool check_used_rolenames_exist(const string& rolename) {
                                lock_guard<mutex> lock(lock_);
                                return used_rolenames_.count(rolename) > 0 ? true : false;
                        }
                        uint64_t rolename2gid(const string& rolename) const {
                                lock_guard<mutex> lock(lock_);
                                ASSERT(rolename2gid_.count(rolename) > 0);
                                return rolename2gid_.at(rolename);
                        }

                        void yunying_fetch_rank_list(uint32_t sid, uint32_t gm_id, uint32_t page_size, pd::rank_type rank_type) {
                                rank_mgr::instance().get_ranking_list(rank_type,
                                                                       [this, sid, gm_id, page_size] (auto entities){
                                                                        pd::rank_list_array lists;
                                                                        uint32_t rank = 1;
                                                                        for (const auto& i : entities) {
                                                                                auto *list = lists.add_rank_list();
                                                                                list->set_role(i.gid());
                                                                                ASSERT(gid2rolename_.count(i.gid()) > 0);
                                                                                list->set_rolename(gid2rolename_.at(i.gid()));
                                                                                list->set_rank(rank);
                                                                                if (i.values_size() > 1) {
                                                                                        list->set_value_1(i.values(0));
                                                                                        list->set_value_2(i.values(1));
                                                                                } else if (i.values_size() == 1) {
                                                                                        list->set_value_1(i.values(0));
                                                                                }
                                                                                if (rank >= page_size) {
                                                                                        break;
                                                                                }
                                                                                ++rank;
                                                                        }
                                                                        gm_mgr_class::instance().yunying_fetch_rank_list_done(sid, gm_id, lists);
                                });
                        }

                        void yunying_send_mail(uint32_t sid, uint32_t gm_id, uint32_t server_id, const pd::gid_array& roles, uint64_t mail_id, const string& title, const string& content, const pd::event_array& events) {
                                vector<uint64_t> no_roles;
                                vector<uint64_t> all_roles;
                                pd::dynamic_data dynamic_data ;
                                dynamic_data.set_yunying_mail_id(mail_id);
                                {
                                        lock_guard<mutex> lock(lock_);
                                        for (auto i : roles.gids()) {
                                                if (gid2rolename_.count(i) == 0) {
                                                        no_roles.push_back(i);
                                                }
                                        }
                                        if (!no_roles.empty()) {
                                                pd::gid_array gids;
                                                for (auto i : no_roles) {
                                                        gids.add_gids(i);
                                                }
                                                gm_mgr_class::instance().send_mail_done(sid, gm_id, pd::NOT_FOUND, gids);
                                                return;
                                        }

                                        if (roles.gids_size() == 0) {
                                                for (const auto& i : gid2rolename_) {
                                                        all_roles.push_back(i.first);
                                                }
                                        } else {
                                                for (auto i : roles.gids()) {
                                                        all_roles.push_back(i);
                                                }
                                        }
                                }

                                for (auto i : all_roles) {
                                        send_mail(i, title, content, dynamic_data, events);
                                }
                                gm_mgr_class::instance().send_mail_done(sid, gm_id, pd::OK, pd::gid_array());
                        }
                        void yunying_manage_announcement(const pd::announcement& announcement) {
                                pd::dynamic_data dd;
                                bool send = false;
                                if (announcement.type() == pd::SYSTEM_ANNOUNCEMENT) {
                                        dd.set_yunying_system_announcement_id(announcement.system_id());
                                        dd.add_position(pd::MP_SYSTEM_CHAT);
                                        send = true;
                                } else if (announcement.type() == pd::FLOW_LIGHT_ANNOUNCEMENT) {
                                        auto server_id = PTTS_GET(options, 1).scene_info().server_id();
                                        if (!PTTS_HAS(serverlist, server_id)) {
                                                return;
                                        }
                                        if (announcement.channel_id() == 0) {
                                                send = true;
                                        } else {
                                                const auto& server_list_ptt = PTTS_GET(serverlist, server_id);
                                                for (auto i : server_list_ptt.channel_ids()) {
                                                        if (announcement.channel_id() == i) {
                                                                send = true;
                                                                break;
                                                        }
                                                }
                                        }
                                        if (send) {
                                                dd.set_yunying_flow_announcement_id(announcement.system_id());
                                                dd.add_position(pd::MP_CENTRE_DOWN);
                                                dd.add_position(pd::MP_SYSTEM_CHAT);
                                        }
                                }
                                if (send) {
                                        send_system_chat(0, dd, announcement.content());
                                }
                        }

                        void yunying_recharge(uint32_t sid, uint32_t gm_id, uint64_t order, const string& yy_orderno, const string& currency, uint32_t price, uint32_t paytime, const string& product_name, const string& ext_info) {
                                auto role = stoull(ext_info);
                                if (!check_role_exists(role)) {
                                        gm_mgr_class::instance().yunying_recharge_done(sid, gm_id, pd::RECHARGE_NO_SUCH_ROLE);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(role,
                                                 [sid, gm_id, order, yy_orderno, currency, price, paytime, product_name] (const auto& pl) {
                                                         pl->yunying_recharge(sid, gm_id, order, yy_orderno, currency, price, paytime, product_name);
                                                 });
                        }
                        void yunying_reissue_recharge(uint32_t sid, uint32_t gm_id, uint64_t role, uint64_t order) {
                                if (!check_role_exists(role)) {
                                        gm_mgr_class::instance().yunying_recharge_done(sid, gm_id, pd::RECHARGE_NO_SUCH_ROLE);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                player_call_func(role,
                                                 [sid, gm_id, role, order] (const auto& pl) {
                                                         pl->yunying_reissue_recharge(sid, gm_id, order);
                                                 });
                        }
                        void yunying_internal_recharge(uint32_t sid, uint32_t gm_id, uint64_t role, uint32_t recharge_id) {
                                if (!check_role_exists(role)) {
                                        gm_mgr_class::instance().yunying_recharge_done(sid, gm_id, pd::RECHARGE_NO_SUCH_ROLE);
                                        return;
                                }
                                PLAYER_CALL_FUNC_LOG;
                                this->player_call_func(role,
                                                        [sid, gm_id, recharge_id] (const auto& pl) {
                                                                pl->yunying_internal_recharge(sid, gm_id, recharge_id);
                                                        });
                        }
                        void room_chat(uint64_t to, const pd::role_info& from, uint32_t time, const string& content, const pd::voice& voice) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(to) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(to)->send_chat(pd::CT_ROOM, from, time, content, voice);
                        }
                        void enter_room_done(uint64_t role, uint64_t room, pd::room_type type, pd::result result, const pd::room& data) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->enter_room_done(room, type, result, data);
                        }
                        void leave_room_done(uint64_t role, uint64_t room, pd::result result) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->leave_room_done(room, result);
                        }
                        void kick_room_done(uint64_t role, uint64_t room, uint64_t kick_role, pd::result result) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->kick_room_done(room, kick_role, result);
                        }
                        void close_pit_done(uint64_t role, uint64_t room, pd::result result) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->close_pit_done(room, result);
                        }
                        void open_pit_done(uint64_t role, uint64_t room, pd::result result) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->open_pit_done(room, result);
                        }
                        void room_invite_other_done(uint64_t role, uint64_t invitee, pd::result result, uint32_t time) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->room_invite_other_done(invitee, result, time);
                        }
                        void room_invited_by_other(uint64_t role, uint64_t from, const pd::room& room, uint32_t timer) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->room_invited_by_other(from, room, timer);
                        }
                        void room_new_role(uint64_t role, uint64_t room, const pd::role_simple_info& new_role, const pd::room_data& data, uint32_t close_time) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->room_new_role(room, new_role, data, close_time);
                        }
                        void room_remove_role(uint64_t role, uint64_t room, uint64_t remove_role) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->room_remove_role(room, remove_role);
                        }
                        void room_close(uint64_t role, uint64_t room, pd::room_type type, pd::result reason) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->room_close(room, type, reason);
                        }
                        void send_system_chat(uint32_t system_chat, const pd::dynamic_data& dd, const string& content = "") {
                                lock_guard<mutex> lock(lock_);
                                for (auto i : gid2playing_player_) {
                                        i.second->send_system_chat(content, system_chat, dd);
                                }
                        }
                        void system_chat_to_player_offline(uint32_t system_chat, uint64_t role,const pd::dynamic_data& dd) {
                                player_call_func(role,
                                                [system_chat, dd] (const auto& pl) {
                                                        pl->offline_system_chat_add_notice(pl->get_role()->marriage_spouse(), system_chat, dd);
                                                });
                        }
                        void create_role_done(const string& username, pd::result result) {
                                lock_guard<mutex> lock(lock_);
                                if (username2player_.count(username) <= 0) {
                                        return;
                                }
                                username2player_.at(username)->create_role_done(result);
                        }
                        void find_role_done(const string& username, pd::result result, const pd::role& role, uint32_t sid, uint32_t gm_id) {
                                if (sid == 0) {
                                        lock_guard<mutex> lock(lock_);
                                        if (username2player_.count(username) <= 0) {
                                                return;
                                        }
                                        username2player_.at(username)->find_role_done(result, role);
                                } else {
                                        gm_mgr_class::instance().fetch_role_list_done(sid, gm_id, role, pd::OK);
                                }
                        }
                        void find_role_by_gid_done(uint64_t gid, pd::result result, const string& username, const pd::role& role) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2player_.count(gid) <= 0) {
                                        return;
                                }
                                gid2player_.at(gid)->find_role_by_gid_done(gid, result, username, role);
                        }

                        uint32_t server_open_day() const {
                                lock_guard<mutex> lock(lock_);
                                return server_open_day_;
                        }
                        void activity_fetch_data_done(uint64_t role, const pd::activity_mgr& data) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2fetching_data_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2fetching_data_player_.at(role)->activity_fetch_data_done(data);
                        }

                        void activity_start_prize_wheel(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_prize_wheel(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_prize_wheel() {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_prize_wheel();
                                }
                        }

                        void activity_start_tea_party(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_tea_party(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_tea_party() {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_tea_party();
                                }
                        }


                        void activity_start_discount_goods(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_discount_goods(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_discount_goods() {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_discount_goods();
                                }
                        }

                        void activity_start_continue_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_continue_recharge(start_day, duration, pttid);
                                }
                        }
                        void activity_stop_continue_recharge(uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_continue_recharge(pttid);
                                }
                        }

                        void activity_start_limit_play(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_limit_play(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_limit_play(uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_limit_play(pttid);
                                }
                        }

                        void activity_start_leiji_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_leiji_recharge(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_leiji_recharge(uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_leiji_recharge(pttid);
                                }
                        }

                        void activity_start_leiji_consume(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_leiji_consume(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_leiji_consume(uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_leiji_consume(pttid);
                                }
                        }

                        void activity_start_festival(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_festival(start_day, duration, pttid);
                                }
                        }

                        void activity_stop_festival(uint32_t pttid) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_festival(pttid);
                                }
                        }

                        void activity_start_recharge_rank(uint32_t start_day, uint32_t duration) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_start_recharge_rank(start_day, duration);
                                }
                        }

                        void activity_stop_recharge_rank() {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->activity_stop_recharge_rank();
                                }
                        }


                        string gid2rolename_lock(uint64_t gid) const {
                                lock_guard<mutex> lock(lock_);
                                ASSERT(gid2rolename_.count(gid) > 0);
                                return gid2rolename_.at(gid);
                        }

                        string gid2rolename_unlock(uint64_t gid) const {
                                ASSERT(gid2rolename_.count(gid) > 0);
                                return gid2rolename_.at(gid);
                        }

                        bool role_online(uint64_t gid) const {
                                lock_guard<mutex> lock(lock_);
                                auto it = gid2playing_player_.find(gid);
                                if (it == gid2playing_player_.end()) {
                                        return false;
                                }
                                return it->second->online();
                        }
                        bool role_state_playing(uint64_t gid) const {
                                lock_guard<mutex> lock(lock_);
                                auto it = gid2playing_player_.find(gid);
                                if (it == gid2playing_player_.end()) {
                                        return false;
                                }
                                return true;
                        }
                        set<uint64_t> get_online_roles() const {
                                lock_guard<mutex> lock(lock_);
                                set<uint64_t> gids;
                                for (const auto& i : gid2playing_player_) {
                                        gids.insert(i.first);
                                }
                                return gids;
                        }
                        set<uint64_t> get_online_roles_except_mirror() const {
                                set<uint64_t> gids;
                                for (const auto& i : gid2playing_player_) {
                                        if (!i.second->mirror_role()) {
                                                gids.insert(i.first);
                                        }
                                }
                                return gids;
                        }

                        void send_mail(uint64_t gid, uint32_t pttid, const pd::dynamic_data& dd, const pd::event_array& events) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(gid) > 0) {
                                        gid2playing_player_.at(gid)->send_mail(pttid, dd, drop_to_event(events));
                                } else {
                                        auto m = mail_new_mail(gid, pttid, dd, drop_to_event(events));
                                        mail_->add_mail(gid, m);
                                }
                        }
                        void send_mail(uint64_t gid, const string& title, const string& content, const pd::dynamic_data& dd, const pd::event_array& events) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(gid) > 0) {
                                        gid2playing_player_.at(gid)->send_mail(title, content, dd, drop_to_event(events));
                                } else {
                                        auto m = mail_new_mail(gid, title, content, dd, drop_to_event(events));
                                        mail_->add_mail(gid, m);
                                }
                        }
                        void playing_player_call_func(uint64_t gid, const function<void(const shared_ptr<player<T>>&)>& func) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(gid) <= 0) {
                                        return;
                                }
                                func(gid2playing_player_.at(gid));
                        }
                        void player_call_func(uint64_t gid, const function<void(const shared_ptr<player<T>>&)>& func) {
                                SPLAYER_DLOG << *this << " player call func " << gid;
                                lock_guard<mutex> lock(lock_);
                                ASSERT(gid2rolename_.count(gid) > 0);
                                if (gid2playing_player_.count(gid) > 0) {
                                        func(gid2playing_player_.at(gid));
                                } else if (player_call_funcs_.count(gid) > 0 || gid2fetching_data_player_.count(gid) > 0) {
                                        player_call_funcs_[gid].push_back(func);
                                } else {
                                        auto st = sts_.at(st_idx_);
                                        st_idx_++;
                                        st_idx_ %= sts_.size();
                                        auto pl = make_shared<player<T>>(st,
                                                                         nullptr,
                                                                         nullptr,
                                                                         gamedb_notice_,
                                                                         false,
                                                                         false);
                                        pl->init();
                                        init_player_cbs(pl);
                                        pl->login_done_cb_ = [this] (const auto& pl, const auto& rolename, const auto& username) {
                                                SPLAYER_DLOG << pl << " player call func login done cb";
                                                lock_guard<mutex> lock(lock_);
                                                auto gid = pl->gid();
                                                ASSERT(gid2rolename_.count(gid) > 0);
                                                ASSERT(gid2player_[gid] == pl);
                                                gid2playing_player_[gid] = pl;
                                                gender2playing_player_[pl->gender()].insert(gid);
                                                return true;
                                        };
                                        pl->start_playing_cb_ = [this] (const auto& pl) {
                                                SPLAYER_DLOG << pl << " player start playing cb";
                                                lock_guard<mutex> lock(lock_);
                                                for (const auto& i : player_call_funcs_.at(pl->gid())) {
                                                        i(pl);
                                                }
                                                player_call_funcs_.erase(pl->gid());
                                                pl->quit();
                                        };
                                        pl->stop_playing_cb_ = [this] (const auto& pl) {
                                                SPLAYER_DLOG << pl << " player stop playing cb";
                                                lock_guard<mutex> lock(lock_);
                                                auto gid = pl->gid();
                                                auto iter = player_call_funcs_.find(gid);
                                                if (iter != player_call_funcs_.end()) {
                                                        try {
                                                                for (const auto& i : iter->second) {
                                                                        i(pl);
                                                                }
                                                        } catch (...) {
                                                                SPLAYER_ELOG << "player call func when find done throw";
                                                        }
                                                        player_call_funcs_.erase(iter);
                                                        return false;
                                                }
                                                if (pl->has_async_call()) {
                                                        return false;
                                                }
                                                gid2playing_player_.erase(gid);
                                                gender2playing_player_[pl->gender()].erase(gid);
                                                level2playing_player_[pl->level()].erase(pl->gid());
                                                guanpin2playing_player_[pl->guanpin()].erase(pl->gid());
                                                player_call_funcs_.erase(gid);
                                                return true;
                                        };
                                        pl->quit_cb_ = [this, gid] (const auto& pl) {
                                                SPLAYER_DLOG << pl << " player call func quit cb";
                                                lock_guard<mutex> lock(lock_);

                                                if (gid2playing_player_.count(gid) > 0) {
                                                        ASSERT(gid2playing_player_.at(gid) != pl);
                                                }
                                                if (gid2player_.count(gid) > 0 && gid2player_.at(gid) == pl) {
                                                        gid2player_.erase(gid);
                                                        player_call_funcs_.erase(gid);
                                                }

                                                this->check_stopped();
                                        };

                                        player_call_funcs_[gid].push_back(func);
                                        gid2player_[gid] = pl;
                                        pl->login_by_gid(gid);
                                }
                        }

                        void print_up_msg_stats() {
                                vector<const Descriptor *> descs;
                                for (const auto& i : up_msg_stats_.msgs_) {
                                        descs.push_back(i.first);
                                }
                                sort(descs.begin(), descs.end(),
                                     [this] (const auto *a, const auto *b) {
                                             return up_msg_stats_.msgs_[a].total_size_ > up_msg_stats_.msgs_[b].total_size_;
                                     });

                                stringstream ss;
                                ss << "\n====== up msg stats: total count: " << up_msg_stats_.total_count_ << " total size: " << bytes_to_string(up_msg_stats_.total_size_) << "\n";
                                ss << setw(STATS_WIDTH) << "name" << setw(STATS_WIDTH) << "total size" << setw(STATS_WIDTH) << "total count" << "\n";
                                for (const auto& i : descs) {
                                        ss << setw(STATS_WIDTH) << i->name() << setw(STATS_WIDTH) << bytes_to_string(up_msg_stats_.msgs_[i].total_size_) << setw(STATS_WIDTH) << up_msg_stats_.msgs_[i].count_ << "\n";
                                }
                                ILOG << ss.str();
                        }
                        void print_down_msg_stats() {
                                vector<const Descriptor *> descs;
                                for (const auto& i : down_msg_stats_.msgs_) {
                                        descs.push_back(i.first);
                                }
                                sort(descs.begin(), descs.end(),
                                     [this] (const auto *a, const auto *b) {
                                             return down_msg_stats_.msgs_[a].total_size_ > down_msg_stats_.msgs_[b].total_size_;
                                     });

                                stringstream ss;
                                ss << "\n====== down msg stats: total count: " << down_msg_stats_.total_count_ << " total size: " << bytes_to_string(down_msg_stats_.total_size_) << "\n";
                                ss << setw(STATS_WIDTH) << "name" << setw(STATS_WIDTH) << "total size" << setw(STATS_WIDTH) << "total count" << "\n";
                                for (const auto& i : descs) {
                                        ss << setw(STATS_WIDTH) << i->name() << setw(STATS_WIDTH) << bytes_to_string(down_msg_stats_.msgs_[i].total_size_) << setw(STATS_WIDTH) << down_msg_stats_.msgs_[i].count_ << "\n";
                                }
                                ILOG << ss.str();
                        }
                        void cross_world_chat(const pd::role_info& from, uint32_t time, uint32_t from_server, const string& content, const pd::voice& voice) {
                                lock_guard<mutex> lock(lock_);
                                for (const auto& i : gid2playing_player_) {
                                        i.second->send_chat(pd::CT_CROSS_WORLD, from, time, content, voice, from_server);
                                }
                        }
                        void server_roles_mirror_event(const pd::server_mirror_event& event) {
                                switch (event.type()) {
                                case pd::RMET_HOLD_ONLINE:
                                        ASSERT(event.has_gid_array());
                                        server_roles_hold_online(event.gid_array());
                                        break;
                                default:
                                        break;
                                }
                        }
                        void server_roles_hold_online(const pd::gid_array& gid_array) {
                                lock_guard<mutex> lock(lock_);
                                for (auto i : gid_array.gids()) {
                                        if (gid2playing_player_.count(i) == 0) {
                                                continue;
                                        }
                                        gid2playing_player_.at(i)->clear_timeout();
                                }
                        }
                        uint32_t server_open_day() {
                                lock_guard<mutex> lock(lock_);
                                return server_open_day_;
                        }
                        void role_mirror_event(const pd::role_mirror_event& event) {
                                auto gid = gid::instance().change_type(event.gid(), gid_type::MIRROR_ROLE);
                                switch (event.type()) {
                                case pd::RMET_LOGIN:
                                        if (mirror_role_login_check()) {
                                                role_mirror_event_login(gid, event);
                                        }
                                        break;
                                case pd::RMET_LOGOUT:
                                        role_mirror_event_logout(gid);
                                        break;
                                case pd::RMET_FRIEND_APPLY:
                                        role_mirror_event_friend_apply(gid);
                                        break;
                                case pd::RMET_SEND_GIFT:
                                        role_mirror_event_send_gift(gid);
                                        break;
                                case pd::RMET_MARRIAGE_DIVORCE:
                                        role_mirror_event_divorce(gid, event.spouce());
                                        break;
                                default:
                                        break;
                                }
                        }
                        void role_mirror_event_login(uint64_t gid, const pd::role_mirror_event& event) {
                                lock_guard<mutex> lock(lock_);
                                auto st = sts_.at(st_idx_);
                                st_idx_++;
                                st_idx_ %= sts_.size();
                                auto pl = make_shared<player<T>>(st,
                                                                 nullptr,
                                                                 nullptr,
                                                                 gamedb_notice_,
                                                                 true,
                                                                 true);
                                this->init_player(pl);
                                pl->set_mirror_role_data(gid, event.role());

                                if (event.has_mansion()) {
                                        mansion_mgr::instance().mirror_update_mansion(gid, event.mansion());
                                }
                                if (event.has_fief_data()) {
                                        fief_mgr::instance().mirror_update_fief(gid, event.fief_data());
                                }
                                pl->login("mirror_" + to_string(gid), false);
                        }
                        void role_mirror_event_logout(uint64_t gid) {
                                lock_guard<mutex> lock(lock_);
                                auto iter = gid2playing_player_.find(gid);
                                if (iter == gid2playing_player_.end()) {
                                        return;
                                }
                                iter->second->quit();
                        }
                        void role_mirror_event_friend_apply(uint64_t role) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->role_mirror_event_friend_apply();
                        }
                        void role_mirror_event_send_gift(uint64_t role) {
                                lock_guard<mutex> lock(lock_);
                                if (gid2playing_player_.count(role) <= 0) {
                                        return;
                                }
                                gid2playing_player_.at(role)->role_mirror_event_send_gift();
                        }

                        void role_mirror_event_broadcast(uint64_t role, uint32_t system_chat) {
                                pd::dynamic_data dd;
                                dd.add_name(gid2rolename_lock(role));
                                send_system_chat(system_chat, dd);
                        }

                        bool mirror_role_login_check() {
                                lock_guard<mutex> lock(lock_);
                                const auto& ptt = PTTS_GET_COPY(options, 1);
                                if (gid2playing_player_.size() >= ptt.scene_info().max_roles()) {
                                        return false;
                                }
                                return true;
                        }

                        void lock_fanli_done(const string& uid, const pd::fanli_array& fanlis) {
                                const auto& ptt = PTTS_GET(options, 1);
                                auto username = uid + "_" + to_string(ptt.scene_info().server_id());
                                {
                                        lock_guard<mutex> lock(lock_);
                                        ASSERT(username2gid_.count(username) > 0);
                                }
                                player_call_func(username2gid_.at(username),
                                                 [fanlis] (const auto& pl) {
                                                         pl->shandang_fanli_done(fanlis);
                                                 });
                        }

                        void lock_come_back_done(const string& uid) {
                                const auto& ptt = PTTS_GET(options, 1);
                                auto username = uid + "_" + to_string(ptt.scene_info().server_id());
                                {
                                        lock_guard<mutex> lock(lock_);
                                        ASSERT(username2gid_.count(username) > 0);
                                }
                                player_call_func(username2gid_.at(username),
                                                 [] (const auto& pl) {
                                                         pl->come_back_done();
                                                 });
                        }

                        vector<string> username() {
                                lock_guard<mutex> lock(lock_);
                                vector<string> usernames;
                                for (const auto& i : username2player_) {
                                        usernames.push_back(i.first);
                                }
                                return usernames;
                        }
                        friend ostream& operator<<(ostream& os, const player_mgr& pm) {
                                return os << "playermgr";
                        }

                        function<void()> init_done_cb_;
                        function<void()> stopped_cb_;
                        function<void(uint64_t, uint32_t, uint32_t, bool)> role_levelup_cb_;
                        function<void(const string&, uint32_t, const pd::role_info_league&, const pd::event_res&, bool)> create_league_func_;
                        function<void(uint64_t, pd::room_type, uint64_t, uint32_t)> room_invite_other_func_;
                        function<void(const pd::role_info&)> create_fief_func_;
                        
                        function<void(const string&, const pd::role& data)> create_role_func_;
                        function<void(uint64_t, const pd::role&)> role_login_cb_;
                        function<void(uint64_t, uint64_t)> role_logout_cb_;

                        int player_count() const {
                                lock_guard<mutex> lock(lock_);
                                return gid2playing_player_.size();
                        }
                        int player_register_count() const {
                                lock_guard<mutex> lock(lock_);
                                return gid2rolename_.size();
                        }
                private:
                        set<uint64_t> get_online_mirror_roles() const {
                                set<uint64_t> gids;
                                for (const auto& i : gid2playing_player_) {
                                        if (i.second->mirror_role()) {
                                                gids.insert(i.first);
                                        }
                                }
                                return gids;
                        }

                        shared_ptr<service_thread> st_;
                        mutable mutex lock_;
                        string client_ip_;
                        unsigned short client_port_;
                        shared_ptr<db::connector> gamedb_;
                        shared_ptr<db::connector> gamedb_notice_;
                        vector<shared_ptr<service_thread>> sts_;
                        int st_idx_;
                        shared_ptr<net::server<T>> nsv_;

                        map<shared_ptr<T>, shared_ptr<player<T>>> conn2player_;
                        map<string, shared_ptr<player<T>>> username2player_;
                        map<uint64_t, shared_ptr<player<T>>> gid2fetching_data_player_;
                        map<uint64_t, shared_ptr<player<T>>> gid2player_;
                        map<uint64_t, shared_ptr<player<T>>> gid2playing_player_;
                        map<uint32_t, set<uint64_t>> level2playing_player_;
                        map<pd::gender, set<uint64_t>> gender2playing_player_;
                        map<int, set<uint64_t>> guanpin2playing_player_;
                        map<string, uint64_t> username2gid_;
                        map<uint64_t, string> gid2rolename_;
                        map<string, uint64_t> rolename2gid_;
                        map<uint64_t, vector<function<void(const shared_ptr<player<T>>&)>>> player_call_funcs_;
                        shared_ptr<mail> mail_;
                        //pd::gongdou_effect gongdou_effect_;
                        set<string> used_rolenames_;
                        uint32_t server_open_day_ = 0;
                        atomic<bool> stop_;
                        list<pd::world_chat> world_chat_list_;
                        struct msg_stat {
                                const Descriptor *desc_ = nullptr;
                                size_t total_size_ = 0;
                                size_t max_size_ = 0;
                                size_t count_ = 0;
                        };
                        struct msg_stats {
                                size_t total_size_ = 0;
                                size_t total_count_ = 0;
                                map<const Descriptor *, msg_stat> msgs_;
                        };
                        mutable mutex up_msg_stats_lock_;
                        msg_stats up_msg_stats_;
                        mutable mutex down_msg_stats_lock_;
                        msg_stats down_msg_stats_;
                };

        }
}
