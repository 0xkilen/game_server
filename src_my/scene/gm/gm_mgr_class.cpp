#include "gm_mgr_class.hpp"
#include "log.hpp"
#include "scene/scened.hpp"
#include "scene/recharge.hpp"
#include "scene/db_log.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/game_def.hpp"
#include "config/serverlist_ptts.hpp"
#include "scene/player/player_mgr.hpp"
#include "proto/data_yunying.pb.h"
#include "scene/arena/arena.hpp"

namespace nora {
        namespace scene {

                using gm_mgr = server_process_mgr<gm>;
                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                gm_mgr_class::gm_mgr_class() {
                        st_ = make_shared<service_thread>("gm_mgr_class");
                        st_->start();
                }

                gm_mgr_class::gm_mgr_class(const shared_ptr<service_thread>& st) : st_(st) {
                        ASSERT(st_);
                }

                void gm_mgr_class::start() {
                        auto ptt = PTTS_GET_COPY(options, 1);
                        game_db_ = make_shared<db::connector>(ptt.game_db().ipport(),
                                                              ptt.game_db().user(),
                                                              ptt.game_db().password(),
                                                              ptt.game_db().database());
                        game_db_->start();

                        load_announcement();

                        gm_mgr::static_init();
                        gm_mgr::instance().init(ptt.scene_info().server_id());
                }

                void gm_mgr_class::in_second() {
                        announcement_timer_ = ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (announcement_timer_ == timer) {
                                                announcement_timer_->cancel();
                                                announcement_timer_.reset();
                                        }
                                        if (!canceled) {
                                                for (auto i : time_point2ids_) {
                                                        if (system_clock::to_time_t(i.first) < system_clock::to_time_t(system_clock::now())) {
                                                                time_point2ids_.erase(i.first);
                                                        } else if (system_clock::to_time_t(i.first) == system_clock::to_time_t(system_clock::now())) {
                                                                this->publish_announcement(i.first);
                                                        }
                                                }
                                        }
                                        this->in_second();
                                }),
                                1s);
                }

                void gm_mgr_class::load_announcement() {
                        auto db_msg = make_shared<db::message>("load_announcement",
                                                               db::message::req_type::rt_select,
                                                               [this] (const shared_ptr<db::message>& msg) {
                                                                       auto& results = msg->results();
                                                                       for (const auto& i : results) {
                                                                               ASSERT(i.size() == 2);
                                                                               pd::announcement ann;
                                                                               ann.ParseFromString(boost::any_cast<string>(i[1]));
                                                                               if (ann.start_time() + (ann.times() * ann.interval_time()) >= system_clock::to_time_t(system_clock::now())) {
                                                                                       this->add_announcement(ann);
                                                                               } else {
                                                                                       auto db_msg = make_shared<db::message>("delete_announcement", db::message::req_type::rt_insert);
                                                                                       db_msg->push_param(ann.system_id());
                                                                                       game_db_->push_message(db_msg, st_);
                                                                               }
                                                                       }
                                                                       this->in_second();
                                                               });
                        game_db_->push_message(db_msg, st_);

                        db_msg = make_shared<db::message>("load_images_announcement",
                                                                db::message::req_type::rt_select,
                                                                [this] (const shared_ptr<db::message>& msg) {
                                                                        auto& results = msg->results();
                                                                        pd::images_announcement_array anns;
                                                                        for (const auto& i : results) {
                                                                                ASSERT(i.size() == 1);
                                                                                anns.ParseFromString(boost::any_cast<string>(i[0]));
                                                                        }
                                                                        for (const auto& i : anns.images()) {
                                                                                images_announcements_[i.content()] = i.until_time();
                                                                        }
                                                                });
                        game_db_->push_message(db_msg, st_);
                }

                void gm_mgr_class::stop() {
                        if (st_) {
                                st_->stop();
                                st_.reset();
                        }
                        if (announcement_timer_) {
                                announcement_timer_->cancel();
                                announcement_timer_.reset();
                        }
                        for (auto& i : sid2timeout_timer_) {
                                i.second->cancel();
                                i.second.reset();
                        }
                        sid2timeout_timer_.clear();
                        if (game_db_) {
                                game_db_->stop();
                                game_db_.reset();
                        }
                        gm_mgr::instance().stop();
                        SGMMGR_ILOG << *this << " stop";
                }

                ostream& operator<<(ostream& os, const gm_mgr_class& gms) {
                        return os << "gmmgrs";
                }

                void gm_mgr_class::add_images_announcement(const pd::images_announcement_array& images_announcements) {
                        st_->async_call(
                                [this, images_announcements] {
                                        auto db_msg = make_shared<db::message>("add_images_announcement",
                                                db::message::req_type::rt_insert,
                                                [this, images_announcements] (const shared_ptr<db::message>& msg) {
                                                        images_announcements_.clear();
                                                        for (const auto& i : images_announcements.images()) {
                                                                images_announcements_[i.content()] = i.until_time();
                                                        }
                                                });

                                        string str;
                                        images_announcements.SerializeToString(&str);
                                        db_msg->push_param(str);
                                        game_db_->push_message(db_msg, st_);
                                });

                }

                void gm_mgr_class::update_images_announcement(const map<string, uint32_t>& images_announcements) {
                        st_->async_call(
                                [this, images_announcements] {
                                        pd::images_announcement_array iaa;
                                        for (const auto& i : images_announcements) {
                                                auto* image = iaa.add_images();
                                                image->set_content(i.first);
                                                image->set_until_time(i.second);
                                        }
                                        auto db_msg = make_shared<db::message>("update_images_announcement",
                                                db::message::req_type::rt_update,
                                                [this, images_announcements] (const shared_ptr<db::message>& msg) {
                                                });

                                        string str;
                                        iaa.SerializeToString(&str);
                                        db_msg->push_param(str);
                                        game_db_->push_message(db_msg, st_);
                                });

                }

                void gm_mgr_class::manage_announcement_done(uint32_t sid, uint32_t gm_id, const pd::announcement& ann) {
                        st_->async_call(
                                [this, sid, gm_id, ann] {
                                        auto procedure = "add_announcement";
                                        if (!ann.publish()) {
                                                procedure = "delete_announcement";
                                        }
                                        auto db_msg = make_shared<db::message>(procedure,
                                                                               db::message::req_type::rt_insert,
                                                                               [this, sid, gm_id] (const shared_ptr<db::message>& msg) {
                                                                                        ps::base rsp_msg;
                                                                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_manage_announcement_rsp::smar);
                                                                                        rsp->set_result(pd::OK);
                                                                                        rsp->set_sid(sid);
                                                                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                                                               });

                                        db_msg->push_param(ann.system_id());
                                        if (ann.publish()) {
                                                string str;
                                                ann.SerializeToString(&str);
                                                db_msg->push_param(str);
                                        }
                                        game_db_->push_message(db_msg, st_);
                                        add_announcement(ann);
                                });
                }

                void gm_mgr_class::add_announcement(const pd::announcement& announcement) {
                        if (id2ann_.count(announcement.system_id()) > 0) {
                                for (auto& i : time_point2ids_) {
                                        if (i.second.count(announcement.system_id()) > 0) {
                                                i.second.erase(announcement.system_id());
                                        }
                                }
                        }
                        id2ann_[announcement.system_id()] = announcement;
                        if (announcement.publish()) {
                                for (auto i = 0u; i < announcement.times(); ++i) {
                                        auto start_time = announcement.start_time() + (i * announcement.interval_time());
                                        if (start_time >= announcement.stop_time()) {
                                                break;
                                        }
                                        const auto& start_point = system_clock::from_time_t(start_time);
                                        if (time_point2ids_.count(start_point) <= 0) {
                                                set<uint32_t> ids;
                                                ids.insert(announcement.system_id());
                                                time_point2ids_[start_point] = ids;
                                        } else {
                                                time_point2ids_[start_point].insert(announcement.system_id());
                                        }
                                }
                        } else {
                                if (id2ann_.count(announcement.system_id()) > 0) {
                                        id2ann_.erase(announcement.system_id());
                                }
                        }
                }

                void gm_mgr_class::publish_announcement(const system_clock::time_point& time_point) {
                        SGMMGR_DLOG << "publish announcement";
                        ASSERT(time_point2ids_.count(time_point) > 0);
                        for (const auto& i : time_point2ids_[time_point]) {
                                ASSERT(id2ann_.count(i) > 0);
                                player_mgr_class::instance().yunying_manage_announcement(id2ann_[i]);
                        }
                        time_point2ids_.erase(time_point);
                }

                void gm_mgr_class::set_item_change(const pd::log& log, pd::yunying_item_change_array& item_changes) {
                        auto *item_change = item_changes.add_item_change();
                        item_change->set_roleid(log.item_change().role());
                        item_change->set_rolename(log.item_change().role_name());
                        item_change->set_itemid(stoul(yunying_serialize_item(log.item_change().pttid())));
                        item_change->set_itemname(log.item_change().name());
                        item_change->set_count(log.item_change().count());
                        item_change->set_reason(log.item_change().origin());
                        item_change->set_time(log.item_change().time());
                        if (log.item_change().type() == pd::ADD_ITEM) {
                                item_change->set_type(1);
                        } else if (log.item_change().type() == pd::DECREASE_ITEM) {
                                item_change->set_type(-1);
                        }
                }

                void gm_mgr_class::set_item_use(const pd::log& log, pd::yunying_item_change_array& item_changes) {
                        auto *item_change = item_changes.add_item_change();
                        item_change->set_roleid(log.item_use().role());
                        item_change->set_rolename(log.item_use().role_name());
                        item_change->set_itemid(stoul(yunying_serialize_item(log.item_use().pttid())));
                        item_change->set_itemname(log.item_use().name());
                        item_change->set_count(log.item_use().count());
                        item_change->set_reason(pd::ce_origin::CO_ITEM_USE);
                        item_change->set_time(log.item_use().time());
                        item_change->set_type(-1);
                }

                void gm_mgr_class::fetch_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t item_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, item_id, procedure, role, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, item_id, procedure, role, start_time, end_time, page_idx, page_size] (const auto& results) {
                                                          pd::yunying_item_change_array item_changes;
                                                          pd::log_array logs;
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                          }
                                                          if (item_id == 0) {
                                                                  for (auto i : logs.logs()) {
                                                                          this->set_item_change(i, item_changes);
                                                                  }
                                                          } else {
                                                                  for (auto i : logs.logs()) {
                                                                          if (item_id == stoul(yunying_serialize_item(i.item_change().pttid()))) {
                                                                                  this->set_item_change(i, item_changes);
                                                                          }
                                                                  }
                                                          }
                                                          db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                                  [this, sid, gm_id, item_id, item_changes] (const auto& results) mutable {
                                                                          pd::log_array logs;
                                                                          for (const auto& i : results) {
                                                                                  ASSERT(i.size() == 3);
                                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                                          }
                                                                          if (item_id == 0) {
                                                                                  for (auto i : logs.logs()) {
                                                                                          this->set_item_use(i, item_changes);
                                                                                  }
                                                                          } else {
                                                                                  for (auto i : logs.logs()) {
                                                                                          if (item_id == stoul(yunying_serialize_item(i.item_change().pttid()))) {
                                                                                                  this->set_item_use(i, item_changes);
                                                                                          }
                                                                                  }
                                                                          }
                                                                          this->yunying_fetch_log_done(sid, gm_id, item_changes);
                                                                  }, {pd::log::ITEM_USE});
                                                  }, {pd::log::ITEM_CHANGE});
                                });

                }

                void gm_mgr_class::set_punished(const pd::punish_role& pr, pd::yunying_fetch_punished_info_array& yspia, int server_id) {
                        auto *fetch_punished_info = yspia.add_fetch_punished_info();
                        fetch_punished_info->set_roleid(pr.role());
                        fetch_punished_info->set_rolename(pr.rolename());
                        fetch_punished_info->set_serverid(server_id);
                        fetch_punished_info->set_servername("");
                        fetch_punished_info->set_rolestatus(pr.type());
                        fetch_punished_info->set_opttime(pr.operate_time());
                        fetch_punished_info->set_lasttime((pr.punish_time() - pr.operate_time()) / 60);
                        fetch_punished_info->set_reason(pr.reason());
                }

                void gm_mgr_class::fetch_punished_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, server_id, procedure, role, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, server_id] (const auto& results) {
                                                          pd::log_array logs;
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                          }
                                                          auto now = system_clock::to_time_t(system_clock::now());
                                                          map<uint64_t, pd::punish_role> bans;
                                                          map<uint64_t, pd::punish_role> gags;
                                                          for (auto i : logs.logs()) {
                                                                  if (i.has_punish_role()) {
                                                                          if (i.punish_role().has_gag_role()) {
                                                                                  if (i.punish_role().gag_role().punish_time() > now) {
                                                                                          gags[i.punish_role().gag_role().role()] = i.punish_role().gag_role();
                                                                                  }
                                                                          } else if (i.punish_role().has_ban_role()) {
                                                                                  if (i.punish_role().ban_role().punish_time() > now) {
                                                                                          bans[i.punish_role().ban_role().role()] = i.punish_role().ban_role();
                                                                                  }
                                                                          }
                                                                  } else if (i.has_unpunish_role()) {
                                                                          auto unpunish = i.unpunish_role();
                                                                          if (bans.count(unpunish.role()) > 0) {
                                                                                  if (unpunish.operate_time() >= bans[unpunish.role()].operate_time()) {
                                                                                          bans.erase(unpunish.role());
                                                                                  }
                                                                          } else if (gags.count(unpunish.role()) > 0) {
                                                                                  if (unpunish.operate_time() >= gags[unpunish.role()].operate_time()) {
                                                                                           gags.erase(unpunish.role());
                                                                                  }
                                                                          }
                                                                  }
                                                          }

                                                          pd::yunying_fetch_punished_info_array yspia;
                                                          for (const auto& i : bans) {
                                                                  this->set_punished(i.second, yspia, server_id);
                                                          }
                                                          for (const auto& i : gags) {
                                                                  this->set_punished(i.second, yspia, server_id);
                                                          }
                                                          this->fetch_punished_done(sid, gm_id, yspia);

                                                  }, {pd::log::PUNISH_ROLE_GAG, pd::log::PUNISH_ROLE_BAN, pd::log::PUNISH_ROLE_KICK, pd::log::UNPUNISH_ROLE_GAG, pd::log::UNPUNISH_ROLE_BAN});
                                });

                }

                void gm_mgr_class::fetch_punished_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_punished_info_array& punished_info) {
                        st_->async_call(
                                [this, sid, gm_id, punished_info] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_punished_rsp::sfpr);
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        *rsp->mutable_fetch_punished_infos() = punished_info;
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::fetch_mail_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, server_id, role, procedure, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, server_id, role] (const auto& results) {
                                                          pd::log log;
                                                          vector<uint64_t> mailids;
                                                          map<uint64_t, pd::yunying_fetch_sent_mail_info> mail_infos;
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  log.ParseFromString(boost::any_cast<string>(i[2]));
                                                                  pd::yunying_fetch_sent_mail_info mail_info;
                                                                  for (const auto& j : log.mail().mail_id()) {
                                                                        auto type = static_cast<pd::log::log_type>(boost::any_cast<uint32_t>(i[1]));
                                                                        if (j.gid() == 0 && j.id() == 0) {
                                                                                continue;
                                                                        }
                                                                        if (mail_infos.count(j.gid()) > 0) {
                                                                              if (type == pd::log::SEND_MAIL) {
                                                                                      auto& mail = mail_infos.at(j.gid());
                                                                                      mail.set_createdat(log.mail().mail().time());
                                                                                      *mail.mutable_events() = log.mail().mail().events();
                                                                              }
                                                                              continue;
                                                                        }
                                                                        mail_info.set_roleid(boost::any_cast<uint64_t> (i[0]));
                                                                        mail_info.set_mailid(j.id());
                                                                        mail_info.set_mail_gid(j.gid());
                                                                        mail_info.set_serverid(log.mail().server_id());
                                                                        mail_info.set_content(log.mail().mail().content());
                                                                        if (type == pd::log::MAIL_DELETE) {
                                                                                mail_info.set_status("0");
                                                                        } else if (type == pd::log::MAIL_FETCH) {
                                                                                mail_info.set_status("3");
                                                                        } else if (type == pd::log::MAIL_SET_ALREADY_READ) {
                                                                                mail_info.set_status("2");
                                                                        } else if (type == pd::log::SEND_MAIL) {
                                                                                mail_info.set_status("1");
                                                                                mail_info.set_createdat(log.mail().mail().time());
                                                                        }
                                                                        mail_infos[j.gid()] = mail_info;
                                                                        mailids.push_back(j.gid());
                                                                }
                                                          }
                                                          this->fetch_sent_mail_done(sid, gm_id, mailids, mail_infos);
                                                  }, {pd::log::SEND_MAIL, pd::log::MAIL_SET_ALREADY_READ, pd::log::MAIL_FETCH, pd::log::MAIL_DELETE});
                        });
                }

                void gm_mgr_class::fetch_sent_mail_done(uint32_t sid, uint32_t gm_id, const vector<uint64_t>& mailids, const map<uint64_t, pd::yunying_fetch_sent_mail_info>& mail_infos) {
                        st_->async_call(
                                [this, sid, gm_id, mailids, mail_infos] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_sent_mail_rsp::sfsmr);
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        for (auto i : mailids) {
                                                for (const auto& j : mail_infos) {
                                                        if (j.first == i) {
                                                                *rsp->mutable_fetch_sent_mail_infos()->add_fetch_sent_mail_info() = j.second;
                                                        }
                                                }
                                        }
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::fetch_login_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this,  sid, gm_id, role, server_id, procedure, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, server_id] (const auto& results) {
                                                          pd::log_array logs;
                                                          auto logs_size = 0;
                                                          pd::yunying_fetch_login_record_array yflra;
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  auto *login_record = yflra.add_fetch_login_record();
                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                                  if (logs.logs_size() > 0) {
                                                                          const auto& ptt = PTTS_GET(serverlist, server_id);
                                                                          login_record->set_servername(ptt.servername());
                                                                          login_record->set_time(boost::any_cast<uint32_t>(i[0]));
                                                                          if (logs.logs(logs_size).has_login()) {
                                                                                  const auto& log = logs.logs(logs_size).login();
                                                                                  login_record->set_roleid(log.role());
                                                                                  login_record->set_rolename(log.rolename());
                                                                                  login_record->set_channelname(to_string(log.channel()));
                                                                                  login_record->set_ip(log.ip());
                                                                                  login_record->set_logintype(pd::log_log_type_Name(pd::log::LOGIN));
                                                                          } else if (logs.logs(logs_size).has_logout()) {
                                                                                  const auto& log = logs.logs(logs_size).logout();
                                                                                  login_record->set_roleid(log.role());
                                                                                  login_record->set_rolename(log.rolename());
                                                                                  login_record->set_channelname(to_string(log.channel()));
                                                                                  login_record->set_ip(log.ip());
                                                                                  login_record->set_logintype(pd::log_log_type_Name(pd::log::LOGOUT));
                                                                          }
                                                                  }
                                                                  ++ logs_size;
                                                          }
                                                          this->fetch_login_records_done(sid, gm_id, yflra);
                                                  }, {pd::log::LOGIN, pd::log::LOGOUT});
                                });
                }

                void gm_mgr_class::fetch_login_records_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_login_record_array& login_records) {
                        st_->async_call(
                                [this, sid, gm_id, login_records] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_login_record_rsp::sflrr);
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        *rsp->mutable_fetch_login_records() = login_records;
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                });
                }

                void gm_mgr_class::set_recharge_record(pd::yunying_fetch_recharge_record *recharge_record, const pd::log_recharge& log, uint32_t time, const string& server_name) {
                        recharge_record->set_servername(server_name);
                        recharge_record->set_roleid(log.role());
                        recharge_record->set_rolename(log.rolename());
                        recharge_record->set_orderno(log.order());
                        recharge_record->set_money(log.price());
                        recharge_record->set_time(time);
                }

                void gm_mgr_class::fetch_recharge_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, server_id, procedure, role, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, server_id] (const auto& results) {
                                                          pd::log_array logs;
                                                          auto logs_size = 0;
                                                          pd::yunying_fetch_recharge_record_array recharge_records;
                                                          const auto& ptt = PTTS_GET(serverlist, server_id);
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  auto *recharge_record = recharge_records.add_fetch_recharge_record();
                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                                  if (logs.logs_size() > 0) {
                                                                          if (logs.logs(logs_size).has_recharge()) {
                                                                                  this->set_recharge_record(recharge_record, logs.logs(logs_size).recharge(), boost::any_cast<uint32_t>(i[0]), ptt.servername());
                                                                          } else if (logs.logs(logs_size).has_internal_recharge()) {
                                                                                  this->set_recharge_record(recharge_record, logs.logs(logs_size).internal_recharge(), boost::any_cast<uint32_t>(i[0]), ptt.servername());
                                                                          } else if (logs.logs(logs_size).has_reissue_recharge()) {
                                                                                  this->set_recharge_record(recharge_record, logs.logs(logs_size).reissue_recharge(), boost::any_cast<uint32_t>(i[0]), ptt.servername());
                                                                          }
                                                                  }
                                                                  ++ logs_size;
                                                          }
                                                          this->fetch_recharge_record_done(sid, gm_id, recharge_records);
                                                  }, {pd::log::RECHARGE, pd::log::INTERNAL_RECHARGE, pd::log::REISSUE_RECHARGE});
                                });
                }

                void gm_mgr_class::fetch_recharge_record_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_recharge_record_array& recharge_records) {
                        st_->async_call(
                                [this, sid, gm_id, recharge_records] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_recharge_record_rsp::sfrrr);
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        *rsp->mutable_fetch_recharge_records() = recharge_records;
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                });
                }

                void gm_mgr_class::set_currency_record(pd::yunying_fetch_currency_record *currency_record, const pd::log_currency_change& log, uint32_t time) {
                        ASSERT(st_->check_in_thread());
                        currency_record->set_roleid(log.role());
                        currency_record->set_rolename(log.rolename());
                        currency_record->set_count(log.currency_count());
                        if (log.change_type() == pd::DECREASE_CURRENCY) {
                                currency_record->set_type(-1);
                        } else if (log.change_type() == pd::ADD_CURRENCY) {
                                currency_record->set_type(1);
                        }
                        currency_record->set_reason(log.reason());
                        currency_record->set_time(time);
                }

                void gm_mgr_class::fetch_currency_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, const vector<pd::log::log_type>& types, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, server_id, types, procedure, role, start_time, end_time, page_idx, page_size] {
                                        db_log::instance().fetch_log(procedure, role, start_time, end_time, page_idx, page_size, st_,
                                                  [this, sid, gm_id, server_id, types] (const auto& results) {
                                                        pd::log_array logs;
                                                          pd::yunying_fetch_currency_record_array currency_records;
                                                          auto logs_size = 0;
                                                          for (const auto& i : results) {
                                                                  ASSERT(i.size() == 3);
                                                                  auto *currency_record = currency_records.add_fetch_currency_record();
                                                                  logs.add_logs()->ParseFromString(boost::any_cast<string>(i[2]));
                                                                  if (logs.logs_size() > 0) {
                                                                          if (logs.logs(logs_size).has_currency_gold()) {
                                                                                  this->set_currency_record(currency_record, logs.logs(logs_size).currency_gold(), boost::any_cast<uint32_t>(i[0]));
                                                                                  const auto& ptt = PTTS_GET(resource, pd::GOLD);
                                                                                  currency_record->set_moneytype(yunying_serialize_resource(pd::GOLD));
                                                                                  currency_record->set_moneyname(ptt.name());
                                                                          } else if (logs.logs(logs_size).has_currency_diamond()) {
                                                                                  this->set_currency_record(currency_record, logs.logs(logs_size).currency_diamond(), boost::any_cast<uint32_t>(i[0]));
                                                                                  const auto& ptt = PTTS_GET(resource, pd::DIAMOND);
                                                                                  currency_record->set_moneytype(yunying_serialize_resource(pd::DIAMOND));
                                                                                  currency_record->set_moneyname(ptt.name());
                                                                          }
                                                                  }
                                                                  ++ logs_size;
                                                          }
                                                          this->fetch_currency_record_done(sid, gm_id, currency_records);
                                                  }, types);
                                });
                }

                void gm_mgr_class::fetch_currency_record_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_currency_record_array& currency_records) {
                        st_->async_call(
                                [this, sid, gm_id, currency_records] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_currency_record_rsp::sfcrr);
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        *rsp->mutable_fetch_currency_records() = currency_records;
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                });
                }

                void gm_mgr_class::fetch_ranK_list(uint32_t sid, uint32_t gm_id, const string& type, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, gm_id, type, page_size] {
                                        if (type == "ARENA") {
                                                arena::instance().yunying_fetch_arena_rank_list(sid, gm_id, page_size);
                                        } else {
                                                pd::rank_type rank_type;
                                                pd::rank_type_Parse(type, &rank_type);
                                                player_mgr_class::instance().yunying_fetch_rank_list(sid, gm_id, page_size, rank_type);
                                        }
                                        sid2timeout_timer_[sid] = ADD_TIMER(
                                                st_,
                                                ([this, sid, gm_id] (auto canceled, const auto& timer)  {
                                                        if (sid2timeout_timer_.count(sid) > 0 && !canceled) {
                                                                ps::base rsp_msg;
                                                                auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_rank_list_rsp::sfrlr);
                                                                rsp->set_result(pd::OK);
                                                                rsp->set_sid(sid);
                                                                gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                                                sid2timeout_timer_.erase(sid);
                                                                SGMMGR_DLOG << "fetch rank list timeout";
                                                        }
                                                }),
                                                3s);
                        });
                }

                void gm_mgr_class::gag_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_gag_role_by_gid_rsp::sgrbgr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::ban_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_ban_role_by_gid_rsp::sbrbgr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::kick_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_kick_role_by_gid_rsp::skrbgr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::remove_punish_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result, pd::admin_edit_role_type edit_type) {
                        st_->async_call(
                                [this, sid, gm_id, result, edit_type] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_remove_punish_by_gid_rsp::srpbgr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        rsp->set_type(edit_type);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::fetch_world_chat_done(uint32_t sid, uint32_t gm_id, const pd::world_chat_array& world_chat, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, world_chat, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_world_chat_rsp::sfwcr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        *rsp->mutable_world_chat_array() = world_chat;
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::set_role_info(const pd::role& role, pd::yunying_role_info& role_info, uint32_t online) {
                        ASSERT(st_->check_in_thread());
                        role_info.set_roleid(role.gid());
                        role_info.set_rolename(role.data().name());
                        auto uid = role.data().username();
                        auto sp_uid = split_string(uid, '_');
                        if(sp_uid.size() == 2) {
                                uid = sp_uid[0];
                        }
                        role_info.set_uid(uid);
                        role_info.set_rolerank(role.data().level());
                        role_info.set_roletype(role.data().gender());
                        role_info.set_channelid(role.data().channel_id());
                        role_info.set_isonline(online);
                        role_info.set_lastlogintime(role.data().last_login_time());
                        role_info.set_sumpaidmoney(role.recharge().paid_money());
                        uint32_t diamond_count = 0;
                        uint32_t vip_level = 0;
                        for (const auto& i : role.resources().values()) {
                                if (i.type() == pd::DIAMOND) {
                                        diamond_count = i.value();
                                }
                                if (i.type() == pd::VIP_EXP) {
                                        vip_level = vip_exp_to_level(i.value());
                                }
                        }
                        role_info.set_viprank(vip_level);
                        role_info.set_lastdiamond(diamond_count);
                        role_info.set_gameunion(role.other_data().league().league_name());
                        role_info.set_regtime(role.data().first_login_time());
                        if (role.has_resources()) {
                                for (const auto& i : role.resources().values()) {
                                        auto *y_items = role_info.add_items();
                                        const auto& ptts = PTTS_GET_ALL(resource);
                                        for (const auto& j : ptts) {
                                                if (j.second.type() == i.type()) {
                                                        y_items->set_itemname(j.second.name());
                                                }
                                        }
                                        y_items->set_itemid(yunying_serialize_resource(i.type()));
                                        y_items->set_itemnum(i.value());
                                        y_items->set_itemtype("2");
                                }
                        }
                        if (role.has_items()) {
                                for (const auto& i : role.items().items()) {
                                        auto *y_items = role_info.add_items();
                                        const auto& ptt = PTTS_GET(item, i.pttid());
                                        y_items->set_itemname(ptt.name());
                                        y_items->set_itemid(stoul(yunying_serialize_item(i.pttid())));
                                        y_items->set_itemnum(i.count());
                                        y_items->set_itemtype("1");
                                }
                        }
                        if (role.has_equipments()) {
                                for (const auto& i : role.equipments().equipments()) {
                                        auto *y_items = role_info.add_items();
                                        const auto& ptt = PTTS_GET(equipment, i.pttid());
                                        y_items->set_itemname(ptt.name());
                                        y_items->set_itemid(stoul(yunying_serialize_equipment(i.pttid())));
                                        y_items->set_itemnum(1);
                                        y_items->set_itemtype("1");
                                }
                        }
                        if (role.has_spine()) {
                                for (const auto& i : role.spine().collection()) {
                                        auto *y_items = role_info.add_items();
                                        switch (i.part()) {
                                        case pd::SP_HONGMO: {
                                                const auto& ptt = PTTS_GET(hongmo, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_YANJING: {
                                                const auto& ptt = PTTS_GET(yanjing, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_YANSHENGUANG: {
                                                const auto& ptt = PTTS_GET(yanshenguang, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_SAIHONG: {
                                                const auto& ptt = PTTS_GET(saihong, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_MEIMAO: {
                                                const auto& ptt = PTTS_GET(meimao, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_YANZHUANG: {
                                                const auto& ptt = PTTS_GET(yanzhuang, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_CHUNCAI: {
                                                const auto& ptt = PTTS_GET(chuncai, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_TIEHUA: {
                                                const auto& ptt = PTTS_GET(tiehua, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_LIANXING: {
                                                const auto& ptt = PTTS_GET(lianxing, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_DEFORM_FACE: {
                                                const auto& ptt = PTTS_GET(spine_deform_face, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_DEFORM_HEAD: {
                                                const auto& ptt = PTTS_GET(spine_deform_head, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_ZUI: {
                                                const auto& ptt = PTTS_GET(zui, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_HUZI: {
                                                const auto& ptt = PTTS_GET(huzi, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        case pd::SP_LIAN: {
                                                const auto& ptt = PTTS_GET(lian, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_spine(i.part(), ptt.id())));
                                                break;
                                        }
                                        }
                                        y_items->set_itemnum(1);
                                        y_items->set_itemtype("1");
                                }
                        }
                        if (role.has_huanzhuang()) {
                                for (const auto& i : role.huanzhuang().collection()) {
                                        auto *y_items = role_info.add_items();
                                        switch (i.part()) {
                                        case pd::HP_YIFU: {
                                                const auto& ptt = PTTS_GET(yifu, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_TOUFA: {
                                                const auto& ptt = PTTS_GET(toufa, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_JIEZHI: {
                                                const auto& ptt = PTTS_GET(jiezhi, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_EDAI: {
                                                const auto& ptt = PTTS_GET(edai, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_FAZAN: {
                                                const auto& ptt = PTTS_GET(fazan, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_GUANSHI: {
                                                const auto& ptt = PTTS_GET(guanshi, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_TOUJIN: {
                                                const auto& ptt = PTTS_GET(toujin, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_ERHUAN: {
                                                const auto& ptt = PTTS_GET(erhuan, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_XIANGLIAN: {
                                                const auto& ptt = PTTS_GET(xianglian, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_BEISHI: {
                                                const auto& ptt = PTTS_GET(beishi, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_GUANGHUAN: {
                                                const auto& ptt = PTTS_GET(guanghuan, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_SHOUCHI: {
                                                const auto& ptt = PTTS_GET(shouchi, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_DEFORM_FACE: {
                                                const auto& ptt = PTTS_GET(spine_deform_face, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        case pd::HP_DEFORM_HEAD: {
                                                const auto& ptt = PTTS_GET(spine_deform_head, i.pttid());
                                                y_items->set_itemname(ptt.name());
                                                y_items->set_itemid(stoul(yunying_serialize_huanzhuang(i.part(), ptt.id())));
                                        }
                                                break;
                                        }
                                        y_items->set_itemnum(1);
                                        y_items->set_itemtype("1");
                                }
                        }
                        role_info.set_ext(role_info.ext());
                }

                void gm_mgr_class::find_role_by_gid_done(uint32_t sid, uint32_t gm_id, const pd::role& role, pd::result result, uint32_t online) {
                        st_->async_call(
                                [this, sid, gm_id, role, result, online] {
                                        pd::yunying_role_info role_info;
                                        this->set_role_info(role, role_info, online);
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_find_role_rsp::sfrr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        if (result == pd::OK) {
                                                if (sid <= MAX_GMD_SID) {
                                                        *rsp->mutable_role() = role_info;
                                                } else {
                                                        *rsp->mutable_role_data() = role;
                                                        const auto& ptt = PTTS_GET(options, 1);
                                                        rsp->set_server_id(ptt.scene_info().server_id());
                                                }
                                        }
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::yunying_fetch_rank_list_done(uint32_t sid, uint32_t gm_id, const pd::rank_list_array& rank_lists) {
                        st_->async_call(
                                [this, sid, gm_id, rank_lists] {
                                        if (sid2timeout_timer_.count(sid) > 0) {
                                                ps::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_rank_list_rsp::sfrlr);
                                                rsp->set_result(pd::OK);
                                                rsp->set_sid(sid);
                                                if (rank_lists.rank_list_size() > 0) {
                                                        *rsp->mutable_rank_list_array() = rank_lists;
                                                }
                                                gm_mgr::instance().send_msg(gm_id, rsp_msg);
                                                sid2timeout_timer_.at(sid)->cancel();
                                                sid2timeout_timer_.erase(sid);
                                        }
                        });
                }

                void gm_mgr_class::yunying_recharge_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_recharge_rsp::srr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::send_mail_done(uint32_t sid, uint32_t gm_id, pd::result result, const pd::gid_array& roles) {
                         st_->async_call(
                                [this, sid, gm_id, result, roles] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_send_mail_rsp::ssmr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        if (result != pd::OK) {
                                                *rsp->mutable_roles() = roles;
                                        }
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::yunying_fetch_log_done(uint32_t sid, uint32_t gm_id, const pd::yunying_item_change_array& item_changes) {
                        st_->async_call(
                                [sid, gm_id, item_changes] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_log_rsp::sflr);
                                        *rsp->mutable_item_changes() = item_changes;
                                        rsp->set_result(pd::OK);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::set_role_simple_info(const pd::role& role, pd::yunying_role_simple_info& role_simple_info) {
                        ASSERT(st_->check_in_thread());
                        role_simple_info.set_roleid(role.gid());
                        role_simple_info.set_rolename(role.data().name());
                        role_simple_info.set_rolerank(role.data().level());
                        role_simple_info.set_roletype(role.data().gender());
                        role_simple_info.set_serverid(PTTS_GET(options, 1).scene_info().server_id());
                        role_simple_info.set_regtime(role.data().first_login_time());
                }

                void gm_mgr_class::fetch_role_list_done(uint32_t sid, uint32_t gm_id, const pd::role& role, pd::result result) {
                         st_->async_call(
                                [this, sid, gm_id, result, role] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_fetch_role_list_rsp::sfrlr);
                                        rsp->set_sid(sid);
                                        if (result == pd::OK && role.gid() > 0) {
                                                pd::yunying_role_simple_info role_simple_info;
                                                set_role_simple_info(role, role_simple_info);
                                                *rsp->mutable_role_simple_info() = role_simple_info;
                                        }
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::yunying_add_stuff_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                         st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_add_stuff_rsp::sasr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::yunying_dec_stuff_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                         st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_dec_stuff_rsp::sdsr);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                void gm_mgr_class::yunying_operates_activity_done(uint32_t sid, uint32_t gm_id, pd::result result) {
                        st_->async_call(
                                [this, sid, gm_id, result] {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::s2g_operates_activity_rsp::soar);
                                        rsp->set_result(result);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gm_id, rsp_msg);
                        });
                }

                // for admin_mgr
                void gm_mgr_class::admin_role_online(uint32_t server_id, uint64_t gid) {
                        st_->async_call(
                                [this, server_id, gid] {
                                        ps::base msg;
                                        auto *notice = msg.MutableExtension(ps::s2g_admin_role_online_notice::saron);
                                        notice->set_server_id(server_id);
                                        notice->set_role_gid(gid);
                                        gm_mgr::instance().broadcast_msg(msg);
                        });
                }

                void gm_mgr_class::admin_role_offline(uint32_t server_id, uint64_t gid) {
                        st_->async_call(
                                [this, server_id, gid] {
                                        ps::base msg;
                                        auto *notice = msg.MutableExtension(ps::s2g_admin_role_offline_notice::saron);
                                        notice->set_server_id(server_id);
                                        notice->set_role_gid(gid);
                                        gm_mgr::instance().broadcast_msg(msg);
                        });
                }

                void gm_mgr_class::admin_update_register_count(uint32_t server_id, uint32_t register_count) {
                        st_->async_call(
                                [this, server_id, register_count] {
                                        ps::base msg;
                                        auto *notice = msg.MutableExtension(ps::s2g_admin_role_register_count_update_notice::sarrcun);
                                        notice->set_server_id(server_id);
                                        notice->set_register_count(register_count);
                                        gm_mgr::instance().broadcast_msg(msg);
                        });
                }
        }
}
