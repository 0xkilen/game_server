#include "scene.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_dbcache.pb.h"
#include "proto/data_db.pb.h"
#include "utils/assert.hpp"
#include "utils/proto_utils.hpp"
#include "utils/game_def.hpp"

namespace nora {
        namespace dbcache {

                scene::scene(const string& name, const shared_ptr<service_thread>& st) :
                        name_(name),
                        st_(st),
                        nsv_(make_shared<net::server<net::CONN>>("dbcache-scene-netserver")) {
                        auto ptt = PTTS_GET_COPY(options, 1u);

                        for (auto i = 0u; i < ptt.dbcache_info().parse_thread_count(); ++i) {
                                parse_sts_.push_back(make_shared<service_thread>("parse"));
                        }
                        next_parse_st_ = 0;

                        for (auto i = static_cast<int>(GDT_ROLE); i < static_cast<int>(GDT_COUNT); ++i) {
                                auto type = static_cast<gamedb_table>(i);
                                gamedbs_[type] = make_shared<db::connector>(ptt.game_db().ipport(),
                                                                            ptt.game_db().user(),
                                                                            ptt.game_db().password(),
                                                                            ptt.game_db().database());
                        }

                        serialize_st_ = make_shared<service_thread>("serialize");

                        arena_rank_ = make_shared<dbc_object_single<pd::arena_rank>>("arena_rank", st_);

                        gladiators_ = make_shared<dbc_object_mgr<pd::gladiator>>("gladiator", st_, ICT_DBC_GLADIATOR, ptt.dbcache_info().max_gladiators());

                        child_mgr_ = make_shared<dbc_object_single<pd::child_mgr>>("child_mgr", st_);

                        children_ = make_shared<dbc_object_mgr<pd::child>>("child", st_, ICT_DBC_CHILD, ptt.dbcache_info().max_children());

                        league_war_ = make_shared<dbc_object_single<pd::league_war>>("league_war", st_);

                        fiefs_ = make_shared<dbc_object_mgr<pd::fief>>("fief", st_, ICT_DBC_FIEF, ptt.dbcache_info().max_fiefs());

                        mansion_mgr_ = make_shared<dbc_object_single<pd::mansion_mgr>>("mansion_mgr", st_);

                        mansions_ = make_shared<dbc_object_mgr<pd::mansion>>("mansion", st_, ICT_DBC_MANSION, ptt.dbcache_info().max_mansions());

                        guanpin_ = make_shared<dbc_object_single<pd::guanpin>>("guanpin", st_);

                        marriage_ = make_shared<dbc_object_single<pd::marriage_data>>("marriage", st_);

                        huanzhuang_pvp_ = make_shared<dbc_object_single<pd::huanzhuang_pvp>>("huanzhuang_pvp", st_);

                        guans_ = make_shared<dbc_object_mgr<pd::guan>>("guan", st_, ICT_DBC_GUAN, ptt.dbcache_info().max_guans());

                        leagues_ = make_shared<dbc_object_mgr<pd::league>>("league", st_, ICT_DBC_LEAGUE, 100);
                        league_separations_ = make_shared<dbc_object_mgr<pd::league_separation>>("league_separation", st_, ICT_DBC_LEAGUE_SEPARATION, 1000);
                        cities_ = make_shared<dbc_object_mgr<pd::city>>("city", st_, ICT_DBC_CITY, 100);

                        league_mgr_ = make_shared<dbc_object_single<pd::league_mgr>>("league_mgr", st_);

                        activity_mgr_ = make_shared<dbc_object_single<pd::activity_mgr>>("activity_mgr", st_);

                        rank_ = make_shared<dbc_object_mgr<pd::rank>>("rank", st_, ICT_DBC_RANK, pd::RT_COUNT);

                        register_proto_handler(ps::s2d_load_ranks_req::descriptor(), &scene::process_load_ranks_req);
                        register_proto_handler(ps::s2d_create_role_req::descriptor(), &scene::process_create_role_req);
                        register_proto_handler(ps::s2d_find_role_req::descriptor(), &scene::process_find_role_req);
                        register_proto_handler(ps::s2d_find_role_by_gid_req::descriptor(), &scene::process_find_role_by_gid_req);
                        register_proto_handler(ps::s2d_update_role_req::descriptor(), &scene::process_update_role_req);
                        register_proto_handler(ps::s2d_create_fief_req::descriptor(), &scene::process_create_fief_req);
                        register_proto_handler(ps::s2d_find_fief_req::descriptor(), &scene::process_find_fief_req);
                        register_proto_handler(ps::s2d_update_fief_req::descriptor(), &scene::process_update_fief_req);
                        register_proto_handler(ps::s2d_create_mansion_req::descriptor(), &scene::process_create_mansion_req);
                        register_proto_handler(ps::s2d_find_mansion_req::descriptor(), &scene::process_find_mansion_req);
                        register_proto_handler(ps::s2d_update_mansion_req::descriptor(), &scene::process_update_mansion_req);
                        register_proto_handler(ps::s2d_load_huanzhuang_pvp_req::descriptor(), &scene::process_load_huanzhuang_pvp_req);
                        register_proto_handler(ps::s2d_save_huanzhuang_pvp_req::descriptor(), &scene::process_save_huanzhuang_pvp_req);
                        register_proto_handler(ps::s2d_load_marriage_req::descriptor(), &scene::process_load_marriage_req);
                        register_proto_handler(ps::s2d_save_marriage_req::descriptor(), &scene::process_save_marriage_req);
                        register_proto_handler(ps::s2d_save_mansion_mgr_req::descriptor(), &scene::process_save_mansion_mgr_req);
                        register_proto_handler(ps::s2d_load_mansion_mgr_req::descriptor(), &scene::process_load_mansion_mgr_req);
                        register_proto_handler(ps::s2d_create_league_req::descriptor(), &scene::process_create_league_req);
                        register_proto_handler(ps::s2d_get_league_req::descriptor(), &scene::process_get_league_req);
                        register_proto_handler(ps::s2d_update_league_req::descriptor(), &scene::process_update_league_req);
                        register_proto_handler(ps::s2d_get_league_separations_req::descriptor(), &scene::process_get_league_separations_req);
                        register_proto_handler(ps::s2d_delete_league_req::descriptor(), &scene::process_delete_league_req);
                        register_proto_handler(ps::s2d_add_cities_req::descriptor(), &scene::process_add_cities_req);
                        register_proto_handler(ps::s2d_get_city_req::descriptor(), &scene::process_get_city_req);
                        register_proto_handler(ps::s2d_update_city_req::descriptor(), &scene::process_update_city_req);
                        register_proto_handler(ps::s2d_update_rank_req::descriptor(), &scene::process_update_rank_req);
                        register_proto_handler(ps::s2d_update_whole_rank_req::descriptor(), &scene::process_update_whole_rank_req);
                        register_proto_handler(ps::s2d_get_arena_rank_req::descriptor(), &scene::process_get_arena_rank_req);
                        register_proto_handler(ps::s2d_update_arena_rank_req::descriptor(), &scene::process_update_arena_rank_req);
                        register_proto_handler(ps::s2d_add_gladiators_req::descriptor(), &scene::process_add_gladiators_req);
                        register_proto_handler(ps::s2d_get_gladiator_req::descriptor(), &scene::process_get_gladiator_req);
                        register_proto_handler(ps::s2d_update_gladiator_req::descriptor(), &scene::process_update_gladiator_req);
                        register_proto_handler(ps::s2d_load_guanpin_req::descriptor(), &scene::process_load_guanpin_req);
                        register_proto_handler(ps::s2d_guanpin_add_guan_req::descriptor(), &scene::process_guanpin_add_guan_req);
                        register_proto_handler(ps::s2d_guanpin_get_guan_req::descriptor(), &scene::process_guanpin_get_guan_req);
                        register_proto_handler(ps::s2d_save_guanpin_req::descriptor(), &scene::process_save_guanpin_req);
                        register_proto_handler(ps::s2d_guanpin_update_guan_req::descriptor(), &scene::process_guanpin_update_guan_req);
                        register_proto_handler(ps::s2d_load_league_war_req::descriptor(), &scene::process_load_league_war_req);
                        register_proto_handler(ps::s2d_save_league_war_req::descriptor(), &scene::process_save_league_war_req);
                        register_proto_handler(ps::s2d_add_child_req::descriptor(), &scene::process_add_child_req);
                        register_proto_handler(ps::s2d_find_child_req::descriptor(), &scene::process_find_child_req);
                        register_proto_handler(ps::s2d_save_child_req::descriptor(), &scene::process_save_child_req);
                        register_proto_handler(ps::s2d_load_child_mgr_req::descriptor(), &scene::process_load_child_mgr_req);
                        register_proto_handler(ps::s2d_save_child_mgr_req::descriptor(), &scene::process_save_child_mgr_req);
                        register_proto_handler(ps::s2d_load_activity_mgr_req::descriptor(), &scene::process_load_activity_mgr_req);
                        register_proto_handler(ps::s2d_save_activity_mgr_req::descriptor(), &scene::process_save_activity_mgr_req);
                        register_proto_handler(ps::s2d_load_league_mgr_req::descriptor(), &scene::process_load_league_mgr_req);
                        register_proto_handler(ps::s2d_save_league_mgr_req::descriptor(), &scene::process_save_league_mgr_req);
                }

                void scene::start(const string& ip, unsigned short port) {
                        for (const auto& i : gamedbs_) {
                                i.second->start();
                        }
                        for (const auto& i : parse_sts_) {
                                i->start();
                        }
                        serialize_st_->start();
                        auto self = shared_from_this();
                        nsv_->new_conn_cb_ = [this, self] (const auto& c) -> shared_ptr<service_thread> {
                                if (!conn_) {
                                        DBCACHE_DLOG << *this << " got new conn " << *c;
                                        conn_ = c;
                                        return st_;
                                }
                                return nullptr;
                        };
                        nsv_->msg_cb_ = [this, self] (const auto& c, const auto& msg) {
                                if (c != conn_) {
                                        return;
                                }
                                this->process_msg(msg);
                        };
                        nsv_->error_cb_ = [this, self] (const auto& c) {
                                DBCACHE_DLOG << *this << " got error";
                                conn_.reset();
                        };

                        nsv_->start();
                        nsv_->listen(ip, port);

                        DBCACHE_ILOG << *this << " start";
                }

                void scene::try_stop() {
                        if (stop_) {
                                return;
                        }
                        stop_ = true;

                        add_stop_timer();
                }

                void scene::add_stop_timer() {
                        ADD_TIMER(
                                st_,
                                ([this, self = shared_from_this()] (auto cancel, const auto& timer) {
                                        if (!cancel) {
                                                DBCACHE_ILOG << *this << " try stop";
                                                if (this->can_stop()) {
                                                        this->stop();
                                                } else {
                                                        this->add_stop_timer();
                                                }
                                        }
                                }),
                                1s);
                }

                void scene::stop() {
                        if (nsv_) {
                                nsv_->stop();
                                nsv_.reset();
                        }
                        for (auto& i : parse_sts_) {
                                i->stop();
                        }
                        parse_sts_.clear();
                        if (serialize_st_) {
                                serialize_st_->stop();
                                serialize_st_.reset();
                        }
                        for (auto& i : gamedbs_) {
                                i.second->stop();
                        }
                        gamedbs_.clear();

                        DBCACHE_ILOG << *this << " stop";
                        ASSERT(stop_cb_);
                        stop_cb_();
                }

                void scene::process_msg(const shared_ptr<net::recvstream>& msg) {
                        ASSERT(st_->check_in_thread());
                        parse_sts_[next_parse_st_]->async_call(
                                [this, self = shared_from_this(), msg, msg_id = msg_id_] {
                                        ps::base m;
                                        m.ParseFromIstream(msg.get());
                                        DBCACHE_TLOG << *this << " got msg:\n" << m.DebugString();
                                        lock_guard<mutex> lock(msgs_lock_);
                                        ASSERT(msgs_.count(msg_id) == 0);
                                        msgs_[msg_id] = m;
                                        process_msgs();
                                });
                        msg_id_ = max(1u, msg_id_ + 1);
                        next_parse_st_ = (next_parse_st_ + 1) % parse_sts_.size();
                }

                void scene::process_msgs() {
                        st_->async_call(
                                [this, self = shared_from_this()] {
                                        ps::base m;
                                        {
                                                lock_guard<mutex> lock(msgs_lock_);
                                                if (msgs_.count(process_msg_id_) == 0) {
                                                        return;
                                                }
                                                m = msgs_.at(process_msg_id_);
                                                msgs_.erase(process_msg_id_);
                                        }

                                        process(this, &m);
                                        process_msg_id_ = max(1u, process_msg_id_ + 1);
                                        process_msgs();
                                });
                }

                void scene::process_load_ranks_req(const ps::base *msg) {
                        auto db_msg = make_shared<db::message>("get_ranks",
                                                               db::message::req_type::rt_select,
                                                               [this, self = shared_from_this()](const shared_ptr<db::message>& msg) {
                                                                       on_db_load_ranks_done(msg->results());
                                                               });
                        gamedbs_[GDT_RANK]->push_message(db_msg, st_);
                }

                void scene::process_create_role_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_create_role_req::scrr);
                        const auto& username = req.username();
                        if (username2role_.count(username) > 0) {
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_rsp::dfrr);
                                rsp->set_username(username);
                                rsp->set_result(pd::OK);
                                *rsp->mutable_role() = username2role_.at(username)->data();
                                send_to_scene(rsp_msg);
                                return;
                        }

                        DBCACHE_DLOG << *this << " create role " << req.role().gid();
                        string role_str;
                        req.role().SerializeToString(&role_str);
                        auto db_msg = make_shared<db::message>("add_role",
                                                               db::message::req_type::rt_insert,
                                                               [this, self = shared_from_this(), username, data = req.role()] (const shared_ptr<db::message>& msg) {
                                                                       on_db_add_role_done(username, data, msg->update_got_duplicate());
                                                               });
                        db_msg->push_param(username);
                        db_msg->push_param(req.role().gid());
                        db_msg->push_param(req.role().data().name());
                        ASSERT(req.role().actors().actors_size() > 0);
                        db_msg->push_param(role_str);
                        gamedbs_[GDT_ROLE]->push_message(db_msg, st_);
                }

                void scene::process_find_role_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_find_role_req::sfrr);
                        const auto& username = req.username();
                        if (username2role_.count(username) > 0) {
                                const auto& role = username2role_.at(username);
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_rsp::dfrr);
                                rsp->set_username(username);
                                rsp->set_result(pd::OK);
                                *rsp->mutable_role() = role->data();
                                if (req.has_sid()) {
                                        rsp->set_sid(req.sid());
                                        rsp->set_gm_id(req.gm_id());
                                }
                                send_to_scene(rsp_msg);

                                role_free_list_.touch(role->gid());
                        } else {
                                if (finding_roles_.count(username) <= 0) {
                                        DBCACHE_DLOG << *this << " find role " << username;
                                        finding_roles_.insert(username);
                                        auto db_msg = make_shared<db::message>("find_role",
                                                                               db::message::req_type::rt_select,
                                                                               [this, self = shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                                       on_db_find_role_done(boost::any_cast<string>(msg->params()[0]), msg->results(), msg->client_data());
                                                                               });
                                        pd::db_client_data dcd;
                                        dcd.set_sid(req.sid());
                                        dcd.set_gm_id(req.gm_id());
                                        db_msg->set_client_data(dcd);
                                        db_msg->push_param(req.username());
                                        gamedbs_[GDT_ROLE]->push_message(db_msg, st_);
                                }
                        }
                }

                void scene::process_find_role_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_find_role_by_gid_req::sfrbgr);
                        auto gid = req.gid();
                        if (gid2username_.count(gid) > 0) {
                                const auto& role = username2role_.at(gid2username_.at(gid));
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_by_gid_rsp::dfrbgr);
                                rsp->set_gid(gid);
                                rsp->set_result(pd::OK);
                                *rsp->mutable_role() = role->data();
                                send_to_scene(rsp_msg);

                                role_free_list_.touch(gid);
                        } else {
                                if (finding_by_gid_roles_.count(gid) <= 0) {
                                        finding_by_gid_roles_.insert(gid);
                                        auto db_msg = make_shared<db::message>("find_role_by_gid",
                                                                               db::message::req_type::rt_select,
                                                                               [this, self = shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                                       auto gid = boost::any_cast<uint64_t>(msg->params()[0]);
                                                                                       on_db_find_role_by_gid_done(gid, msg->results());

                                                                                       ps::base rsp_msg;
                                                                                       auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_by_gid_rsp::dfrbgr);
                                                                                       rsp->set_gid(gid);
                                                                                       if (msg->results().size() <= 0) {
                                                                                               rsp->set_result(pd::NOT_FOUND);
                                                                                       } else {
                                                                                               rsp->set_result(pd::OK);
                                                                                               ASSERT(gid2username_.count(gid) > 0);
                                                                                               const auto& username = gid2username_.at(gid);
                                                                                               ASSERT(username2role_.count(username) > 0);

                                                                                               rsp->set_username(username);
                                                                                               *rsp->mutable_role() = username2role_.at(username)->data();
                                                                                       }
                                                                                       send_to_scene(rsp_msg);
                                                                               });

                                        db_msg->push_param(gid);
                                        gamedbs_[GDT_ROLE]->push_message(db_msg, st_);
                                }
                        }
                }

                void scene::update_role(uint64_t gid, const pd::role& data) {
                        ASSERT(st_->check_in_thread());
                        DBCACHE_DLOG << *this << " update role " << gid;
                        ASSERT(gid2username_.count(gid) > 0);
                        auto& role = username2role_.at(gid2username_.at(gid));
                        role->update(data);
                        role_free_list_.touch(gid);

                        if (role_dirty_list_.has(gid)) {
                                if (role->dirty() < 8) {
                                        role_dirty_list_.touch(gid);
                                }
                        } else {
                                role_dirty_list_.push(gid);
                        }

                        try_save_role();
                }

                void scene::process_update_role_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_role_req::surr);
                        const auto& data = req.role();
                        auto gid = data.gid();
                        if (gid2username_.count(gid) > 0) {
                                update_role(gid, data);
                        } else {
                                if (finding_by_gid_roles_.count(gid) <= 0) {
                                        DBCACHE_DLOG << *this << " find role " << gid;
                                        finding_by_gid_roles_.insert(gid);
                                        auto db_msg = make_shared<db::message>("find_role_by_gid",
                                                                               db::message::req_type::rt_select,
                                                                               [this, self = shared_from_this(), data] (const shared_ptr<db::message>& msg) {
                                                                                       auto gid = boost::any_cast<uint64_t>(msg->params()[0]);
                                                                                       on_db_find_role_by_gid_done(gid, msg->results());

                                                                                       update_role(gid, data);
                                                                               });

                                        db_msg->push_param(gid);
                                        gamedbs_[GDT_ROLE]->push_message(db_msg, st_);
                                }
                        }
                }

                void scene::try_save_role() {
                        ASSERT(st_->check_in_thread());
                        if (role_dirty_list_.empty() || saving_role_) {
                                return;
                        }

                        auto gid = role_dirty_list_.pop();
                        auto& role = username2role_.at(gid2username_.at(gid));
                        DBCACHE_DLOG << *this << " save role " << gid;
                        const auto& role_data = role->data();
                        auto db_msg = make_shared<db::message>("update_role",
                                                               db::message::req_type::rt_update,
                                                               [this, self = shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                       saving_role_ = false;
                                                                       try_save_role();
                                                               });

                        db_msg->push_param(gid);
                        db_msg->push_param(role_data.data().name());
                        ASSERT(role_data.actors().actors_size() > 0);
                        string data_str;
                        role_data.SerializeToString(&data_str);
                        db_msg->push_param(data_str);
                        gamedbs_[GDT_ROLE]->push_message(db_msg, st_);

                        saving_role_ = true;
                        role->reset_dirty();
                }

                void scene::process_create_fief_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_create_fief_req::scfr);
                        fiefs_->add(req.fief(),
                                       [this, self = shared_from_this(), data = req.fief()] () {
                                               ps::base rsp_msg;
                                               auto *rsp = rsp_msg.MutableExtension(ps::d2s_create_fief_rsp::dcfr);
                                               rsp->set_gid(data.gid());
                                               rsp->set_result(pd::OK);
                                               *rsp->mutable_fief() = data;
                                               this->send_to_scene(rsp_msg);
                                       });
                }

                void scene::process_find_fief_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_find_fief_req::sffr);
                        fiefs_->get(req.gid(),
                                       [this, self = shared_from_this(), gid = req.gid()] (auto result, const auto& fief) {
                                               ps::base rsp_msg;
                                               auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_fief_rsp::dffr);
                                               rsp->set_gid(gid);
                                               rsp->set_result(result);
                                               if (result == pd::OK) {
                                                       *rsp->mutable_fief() = fief;
                                               }
                                               this->send_to_scene(rsp_msg);
                                       });
                }

                void scene::process_update_fief_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_fief_req::sufr);
                        fiefs_->update(req.fief().gid(), req.fief());
                }

                void scene::process_create_mansion_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_create_mansion_req::scmr);
                        mansions_->add(req.mansion(),
                                          [this, self = shared_from_this(), mansion = req.mansion()] {
                                                  ps::base rsp_msg;
                                                  auto *rsp = rsp_msg.MutableExtension(ps::d2s_create_mansion_rsp::dcmr);
                                                  rsp->set_gid(mansion.gid());
                                                  rsp->set_result(pd::OK);
                                                  *rsp->mutable_mansion() = mansion;
                                                  this->send_to_scene(rsp_msg);
                                          });
                }

                void scene::process_find_mansion_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_find_mansion_req::sfmr);
                        auto gid = req.gid();

                        ASSERT(mansions_);
                        mansions_->get(gid,
                                       [this, self = shared_from_this(), gid] (auto result, const auto& data) {
                                               ps::base rsp_msg;
                                               auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_mansion_rsp::dfmr);
                                               rsp->set_gid(gid);
                                               rsp->set_result(result);
                                               if (result == pd::OK) {
                                                       *rsp->mutable_mansion() = data;
                                               }
                                               this->send_to_scene(rsp_msg);
                                       });
                }

                void scene::process_update_mansion_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_mansion_req::sumr);
                        const auto& data = req.mansion();
                        auto gid = data.gid();

                        ASSERT(mansions_);
                        mansions_->update(gid, data);
                }

                void scene::process_load_huanzhuang_pvp_req(const ps::base *msg) {
                        ASSERT(huanzhuang_pvp_);
                        huanzhuang_pvp_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_huanzhuang_pvp_rsp::dlhpr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::huanzhuang_pvp data;
                                                huanzhuang_pvp_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_huanzhuang_pvp_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_huanzhuang_pvp_req::sshpr);

                        ASSERT(huanzhuang_pvp_);
                        huanzhuang_pvp_->update(req.data());
                }

                void scene::process_load_marriage_req(const ps::base *msg) {
                        ASSERT(marriage_);
                        marriage_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_marriage_rsp::dlmr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::marriage_data data;
                                                marriage_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_marriage_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_marriage_req::ssmr);

                        ASSERT(marriage_);
                        marriage_->update(req.data());
                }

                void scene::process_save_mansion_mgr_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_mansion_mgr_req::ssmmr);

                        ASSERT(mansion_mgr_);
                        mansion_mgr_->update(req.data());
                }

                void scene::process_load_mansion_mgr_req(const ps::base *msg) {
                        ASSERT(mansion_mgr_);
                        mansion_mgr_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_mansion_mgr_rsp::dlmmr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::mansion_mgr data;
                                                mansion_mgr_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_create_league_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_create_league_req::sclr);
                        const auto& league = req.updatable().league();
                        auto gid = league.gid();
                        const auto& separation = req.updatable().separation();

                        ASSERT(leauge_operate_.count(gid) <= 0);
                        leauge_operate_[gid] = 2;
                        leagues_->add(league,
                                        [this, self = shared_from_this(), league, separation] () {
                                                ASSERT(st_->check_in_thread());
                                                auto gid = league.gid();
                                                leauge_operate_[gid]--;
                                                if (leauge_operate_[gid] == 0) {
                                                        on_db_add_league_done(league, separation, false);
                                                }
                                        });

                        league_separations_->add(separation,
                                        [this, self = shared_from_this(), league, separation] () {
                                                ASSERT(st_->check_in_thread());
                                                auto gid = league.gid();
                                                leauge_operate_[gid]--;
                                                if (leauge_operate_[gid] == 0) {
                                                        on_db_add_league_done(league, separation, false);
                                                }
                                        });
                }

                void scene::process_get_league_req(const ps::base *msg) {
                        if (getting_all_leagues) {
                                return;
                        }
                        const auto& req = msg->GetExtension(ps::s2d_get_league_req::sglr);
                        auto gid = req.gid();

                        leagues_->get(gid,
                                        [this, self = shared_from_this(), gid = req.gid()] (auto result, const auto& league) {
                                                ps::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_get_league_rsp::dglr);
                                                rsp->set_gid(gid);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_league() = league;
                                                }
                                                this->send_to_scene(rsp_msg);
                                        });
                }

                void scene::process_update_league_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_league_req::sulr);
                        auto gid = req.updatable().league().gid();
                        leagues_->update(gid, req.updatable().league());
                        league_separations_->update(gid, req.updatable().separation());
                }

                void scene::process_get_league_separations_req(const ps::base *msg) {
                        if (getting_all_league_separations_) {
                                return;
                        }
                        auto db_msg = make_shared<db::message>("get_league_separations",
                                                                db::message::req_type::rt_select,
                                                                [this, self = shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                        ASSERT(st_->check_in_thread());
                                                                        on_db_get_league_separations_done(msg->results());
                                                                });
                        gamedbs_[GDT_LEAGUE]->push_message(db_msg, st_);
                        getting_all_league_separations_ = true;
                }

                void scene::process_delete_league_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_delete_league_req::sdlr);
                        leagues_->remove(req.gid());
                        league_separations_->remove(req.gid());
                }

                void scene::process_add_cities_req(const ps::base *msg) {
                        ASSERT(cities_);
                        const auto& req = msg->GetExtension(ps::s2d_add_cities_req::sacr);
                        const auto& cities = req.cities().cities();
                        vector<pd::city> cities_v;
                        for(const auto& city_data : cities) {
                                cities_v.push_back(city_data);
                        }

                        cities_->add(
                                cities_v,
                                [this, self = this->shared_from_this()] (auto count) {
                                        ASSERT(st_->check_in_thread());
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_add_cities_rsp::dacr);
                                        rsp->set_count(count);
                                        this->send_to_scene(rsp_msg);
                                });

                }

                void scene::process_get_city_req(const ps::base *msg) {
                        ASSERT(cities_);
                        const auto& req = msg->GetExtension(ps::s2d_get_city_req::sgcr);

                        uint64_t gid = static_cast<uint64_t>(req.pttid());

                        cities_->get(gid,
                                [this, self = shared_from_this(), pttid = req.pttid()] (auto result, const auto& data) {
                                        ASSERT(st_->check_in_thread());
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_get_city_rsp::dgcr);
                                        rsp->set_result(result);
                                        rsp->set_pttid(pttid);
                                        if (result == pd::OK) {
                                                *rsp->mutable_city() = data;
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_update_city_req(const ps::base *msg) {
                        ASSERT(cities_);
                        const auto& req = msg->GetExtension(ps::s2d_update_city_req::sucr);
                        const auto& data = req.city();
                        cities_->update(data.gid(), data);
                }

                void scene::process_update_rank_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_rank_req::surr);
                        const auto& update_data = req.rank();
                        
                        ASSERT(rank_);
                        rank_->update(update_data.gid(),
                                [this, self = shared_from_this(), update_data] (pd::rank& data) {
                                        auto *e = data.mutable_entities();
                                        for (auto i = 0; i < update_data.indexs_size(); i++) {
                                                ASSERT(update_data.entities().entities(i).has_gid());
                                                if (update_data.indexs(i) >= data.entities().entities_size()) {
                                                        ASSERT(update_data.indexs(i) == data.entities().entities_size());
                                                        *e->add_entities() = update_data.entities().entities(i);
                                                } else {
                                                        *e->mutable_entities(update_data.indexs(i)) = update_data.entities().entities(i);
                                                }
                                        }
                        });
                }

                void scene::process_update_whole_rank_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_whole_rank_req::suwrr);
                        const auto& data = req.rank();
                        auto gid = data.gid();

                        ASSERT(rank_);
                        rank_->update(gid, data);
                }

                void scene::process_get_arena_rank_req(const ps::base *msg) {
                        ASSERT(arena_rank_);
                        arena_rank_->get(
                                [this, self = shared_from_this()] (auto result, const auto& data) {
                                        ASSERT(st_->check_in_thread());
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_get_arena_rank_rsp::dgarr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_add_gladiators_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_add_gladiators_req::sagr);
                        const auto& gladiators = req.gladiators();

                        ASSERT(arena_rank_);
                        arena_rank_->get(
                                [this, self = this->shared_from_this(), gladiators] (auto result, const auto& data) {
                                        ASSERT(st_->check_in_thread());

                                        auto ar_data = data;
                                        if (ar_data.rank().gids_size() == 0) {
                                                ar_data.mutable_rank()->add_gids(0);
                                        }

                                        vector<pd::gladiator> glad_data;
                                        for (const auto& i : gladiators.gladiators()) {
                                                glad_data.push_back(i);
                                                ar_data.mutable_rank()->add_gids(i.gid());
                                        }

                                        ASSERT(gladiators_);
                                        gladiators_->add(
                                                glad_data,
                                                [this, self = this->shared_from_this()] (auto count) {
                                                        ps::base rsp_msg;
                                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_add_gladiators_rsp::dagr);
                                                        rsp->set_count(count);
                                                        this->send_to_scene(rsp_msg);
                                                });

                                        if (result == pd::OK) {
                                                arena_rank_->update(ar_data);
                                        } else {
                                                arena_rank_->create(ar_data);
                                        }
                                });
                }

                void scene::process_get_gladiator_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_get_gladiator_req::sggr);
                        auto gid = req.gid();

                        ASSERT(gladiators_);
                        gladiators_->get(gid,
                                            [this, self = shared_from_this(), gid] (auto result, const auto& data) {
                                                    ps::base rsp_msg;
                                                    auto *rsp = rsp_msg.MutableExtension(ps::d2s_get_gladiator_rsp::dggr);
                                                    rsp->set_gid(gid);
                                                    rsp->set_result(result);
                                                    if (result == pd::OK) {
                                                            *rsp->mutable_gladiator() = data;
                                                    }
                                                    this->send_to_scene(rsp_msg);
                                            });
                }

                void scene::process_update_arena_rank_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_arena_rank_req::suarr);

                        ASSERT(arena_rank_);
                        arena_rank_->update(req.data());
                }

                void scene::process_update_gladiator_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_update_gladiator_req::sugr);
                        const auto& data = req.gladiator();
                        auto gid = data.gid();

                        ASSERT(gladiators_);
                        gladiators_->update(gid, data);
                }

                void scene::process_load_guanpin_req(const ps::base *msg) {
                        ASSERT(guanpin_);
                        guanpin_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_guanpin_rsp::dlgr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::guanpin data;
                                                guanpin_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_guanpin_add_guan_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_guanpin_add_guan_req::sgagr);
                        const auto& data = req.guan();

                        ASSERT(guans_);
                        guans_->add(data, nullptr);
                }

                void scene::process_guanpin_get_guan_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_guanpin_get_guan_req::sgggr);
                        auto gid = req.gid();
                        ASSERT(guans_);
                        guans_->get(gid,
                                    [this, self = shared_from_this(), gid] (auto result, const auto& data) {
                                            ps::base rsp_msg;
                                            auto *rsp = rsp_msg.MutableExtension(ps::d2s_guanpin_get_guan_rsp::dgggr);
                                            rsp->set_gid(gid);
                                            rsp->set_result(result);
                                            if (result == pd::OK) {
                                                    *rsp->mutable_guan() = data;
                                            }
                                            this->send_to_scene(rsp_msg);
                                    });
                }

                void scene::process_save_guanpin_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_guanpin_req::ssgr);

                        ASSERT(guanpin_);
                        guanpin_->update(req.data());
                }

                void scene::process_guanpin_update_guan_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_guanpin_update_guan_req::sgugr);
                        const auto& guan = req.guan();
                        auto gid = guan.gid();
                        ASSERT(guans_);

                        guans_->update(gid, guan);
                }

                void scene::process_load_league_war_req(const ps::base *msg) {
                        ASSERT(league_war_);
                        league_war_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_league_war_rsp::dllwr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::league_war data;
                                                league_war_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_league_war_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_league_war_req::sslwr);

                        ASSERT(league_war_);
                        league_war_->update(req.data());
                }

                void scene::process_add_child_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_add_child_req::sacr);

                        ASSERT(children_);
                        children_->add(req.data(), nullptr);
                }

                void scene::process_find_child_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_find_child_req::sfcr);
                        auto gid = req.gid();

                        ASSERT(children_);
                        children_->get(gid,
                                       [this, self = shared_from_this(), gid] (auto result, const auto& data) {
                                               ps::base rsp_msg;
                                               auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_child_rsp::dfcr);
                                               rsp->set_gid(gid);
                                               rsp->set_result(result);
                                               if (result == pd::OK) {
                                                       *rsp->mutable_data() = data;
                                               }
                                               this->send_to_scene(rsp_msg);
                                       });
                }

                void scene::process_save_child_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_child_req::sscr);
                        const auto& data = req.data();
                        auto gid = data.gid();

                        ASSERT(children_);
                        children_->update(gid, data);
                }

                void scene::process_load_child_mgr_req(const ps::base *msg) {
                        ASSERT(child_mgr_);
                        child_mgr_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_child_mgr_rsp::dlcmr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::child_mgr data;
                                                child_mgr_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_child_mgr_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_child_mgr_req::sscmr);

                        ASSERT(child_mgr_);
                        child_mgr_->update(req.data());
                }

                void scene::process_load_activity_mgr_req(const ps::base *msg) {
                        ASSERT(activity_mgr_);
                        activity_mgr_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_activity_mgr_rsp::dlamr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::activity_mgr data;
                                                activity_mgr_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_activity_mgr_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_activity_mgr_req::ssamr);

                        ASSERT(activity_mgr_);
                        activity_mgr_->update(req.data());
                }

                void scene::process_load_league_mgr_req(const ps::base *msg) {
                        ASSERT(league_mgr_);
                        league_mgr_->get(
                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                        ps::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_league_mgr_rsp::dllmr);
                                        rsp->set_result(result);
                                        if (result == pd::OK) {
                                                *rsp->mutable_data() = data;
                                        } else if (result == pd::NOT_FOUND) {
                                                pd::league_mgr data;
                                                league_mgr_->create(data);
                                        }
                                        this->send_to_scene(rsp_msg);
                                });
                }

                void scene::process_save_league_mgr_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2d_save_league_mgr_req::sslmr);

                        ASSERT(league_mgr_);
                        league_mgr_->update(req.data());
                }

                void scene::on_db_add_role_done(const string& username, const pd::role& data, bool already_exist) {
                        ASSERT(st_->check_in_thread());
                        if (already_exist) {
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_create_role_rsp::dcrr);
                                rsp->set_username(username);
                                rsp->set_result(pd::NAME_EXIST);
                                send_to_scene(rsp_msg);
                        } else {
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_rsp::dfrr);
                                rsp->set_username(username);
                                rsp->set_result(pd::OK);
                                auto r = make_unique<dbc_object<pd::role>>(ICT_DBC_ROLE);
                                r->set(data);
                                *rsp->mutable_role() = r->data();

                                auto gid = r->gid();
                                gid2username_[gid] = username;
                                username2role_[username].swap(r);

                                DBCACHE_DLOG << *this << " created role " << gid;

                                if (role_free_list_.has(gid)) {
                                        role_free_list_.touch(gid);
                                } else {
                                        role_free_list_.push(gid);
                                }
                                clean_roles();
                                send_to_scene(rsp_msg);
                        }
                }

                void scene::on_db_find_role_done(const string& username, const vector<vector<boost::any>>& results, const pd::db_client_data& db_client_data) {
                        ASSERT(st_->check_in_thread());
                        ASSERT(results.size() <= 1);

                        ps::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_find_role_rsp::dfrr);
                        rsp->set_username(username);
                        if (db_client_data.has_sid()) {
                                rsp->set_sid(db_client_data.sid());
                                rsp->set_gm_id(db_client_data.gm_id());
                        }
                        finding_roles_.erase(username);
                        if (results.empty()) {
                                rsp->set_result(pd::NOT_FOUND);
                        } else {
                                rsp->set_result(pd::OK);

                                const auto& result = results.front();
                                auto r = make_unique<dbc_object<pd::role>>(ICT_DBC_ROLE);
                                r->parse(boost::any_cast<string>(result[2]));
                                *rsp->mutable_role() = r->data();

                                auto gid = r->gid();
                                gid2username_[gid] = username;
                                username2role_[username].swap(r);

                                DBCACHE_DLOG << *this << " found role " << username << " " << gid;

                                if (role_free_list_.has(gid)) {
                                        role_free_list_.touch(gid);
                                } else {
                                        role_free_list_.push(gid);
                                }
                                clean_roles();
                        }
                        send_to_scene(rsp_msg);
                }

                void scene::on_db_find_role_by_gid_done(uint64_t gid, const vector<vector<boost::any>>& results) {
                        ASSERT(st_->check_in_thread());
                        ASSERT(results.size() <= 1);

                        finding_by_gid_roles_.erase(gid);
                        if (results.size() > 0) {
                                const auto& result = results.front();
                                auto username = boost::any_cast<string>(result[3]);
                                auto r = make_unique<dbc_object<pd::role>>(ICT_DBC_ROLE);
                                r->parse(boost::any_cast<string>(result[2]));

                                gid2username_[gid] = username;
                                username2role_[username].swap(r);

                                DBCACHE_DLOG << *this << " found role " << gid << " " << username;

                                if (role_free_list_.has(gid)) {
                                        role_free_list_.touch(gid);
                                } else {
                                        role_free_list_.push(gid);
                                }
                                clean_roles();
                        }
                }

                void scene::clean_roles() {
                        ASSERT(st_->check_in_thread());
                        if (cleaning_roles_) {
                                return;
                        }

                        cleaning_roles_ = true;
                        ADD_TIMER(
                                st_,
                                ([this, self = shared_from_this()] (auto canceled, const auto& timer) {
                                        cleaning_roles_ = false;
                                        if (!canceled) {
                                                auto ptt = PTTS_GET_COPY(options, 1u);
                                                auto max_try = role_free_list_.size();
                                                while (role_free_list_.size() > ptt.dbcache_info().max_roles() && max_try > 0) {
                                                        auto gid = role_free_list_.pop();
                                                        ASSERT(gid2username_.count(gid) > 0);
                                                        const auto& username = gid2username_.at(gid);
                                                        ASSERT(username2role_.count(username) > 0);
                                                        if (username2role_.at(username)->dirty() > 0) {
                                                                role_free_list_.push(gid);
                                                        } else {
                                                                gid2username_.erase(gid);
                                                                username2role_.erase(username);
                                                        }
                                                        max_try -= 1;
                                                }
                                        }
                                }),
                                3s);
                }

                void scene::on_db_add_league_done(const pd::league& data, const pd::league_separation& separation, bool already_exist) {
                        ASSERT(st_->check_in_thread());
                        ASSERT(!already_exist);

                        ps::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_create_league_rsp::dclr);
                        rsp->set_gid(data.gid());
                        rsp->set_result(pd::OK);
                        *rsp->mutable_league() = data;
                        *rsp->mutable_separation() = separation;
                        send_to_scene(rsp_msg);
                }

                void scene::on_db_get_league_separations_done(const vector<vector<boost::any>>& results) {
                        ASSERT(st_->check_in_thread());
                        ps::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::d2s_get_league_separations_rsp::dglsr);
                        rsp->set_result(pd::OK);
                        auto* separations = rsp->mutable_separations();
                        for (const auto& i : results) {
                                ASSERT(i.size() == 2);
                                pd::league_separation separation;
                                separation.ParseFromString(boost::any_cast<string>(i[1]));
                                *separations->add_separations() = separation;
                        }
                        getting_all_league_separations_ = false;
                        send_to_scene(rsp_msg);
                }

                void scene::on_db_load_ranks_done(const vector<vector<boost::any>>& results) {
                        ASSERT(st_->check_in_thread());

                        map<pd::rank_type, pd::rank> ranks;

                        for (auto i = static_cast<int>(pd::RT_LEVEL); i < static_cast<int>(pd::RT_COUNT); ++i) {
                                pd::rank rank;
                                rank.set_type(static_cast<pd::rank_type>(i));
                                rank.set_gid(i);
                                ranks[static_cast<pd::rank_type>(i)] = rank;
                        }

                        set<pd::rank_type> used_rank;

                        if (!results.empty()) {
                                for (const auto& i : results) {
                                        ASSERT(i.size() == 2);
                                        pd::rank rank;
                                        rank.ParseFromString(boost::any_cast<string>(i[1]));
                                        DBCACHE_DLOG << *this << " load rank rank_type is " << pd::rank_type_Name(rank.type());
                                        ranks[rank.type()] = rank;

                                        used_rank.insert(rank.type());
                                }
                        }

                        if (used_rank.size() != ranks.size()) {
                                ASSERT(used_rank.size() < ranks.size());
                                ASSERT(rank_);
                                auto count = ranks.size() - used_rank.size();
                                for (auto& i : ranks) {
                                        if (used_rank.count(i.first) > 0) {
                                                continue;
                                        }
                                        rank_->add(i.second, 
                                                [this, self = this->shared_from_this(), ranks, count] () {
                                                        this->add_rank_size_++;
                                                        if (this->add_rank_size_ == count) {
                                                                ps::base rsp_msg;
                                                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_ranks_rsp::dlrr);
                                                                for (const auto& i : ranks) {
                                                                        *rsp->mutable_ranks()->add_ranks() = i.second;
                                                                }
                                                                this->send_to_scene(rsp_msg);
                                                        }
                                                });
                                }
                        } else {
                                ps::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(ps::d2s_load_ranks_rsp::dlrr);
                                for (const auto& i : ranks) {
                                        *rsp->mutable_ranks()->add_ranks() = i.second;
                                }
                                send_to_scene(rsp_msg);
                        }
                }

                void scene::send_to_scene(const ps::base& msg) {
                        serialize_st_->call_in_thread(
                                [this, self = shared_from_this(), msg] {
                                        DBCACHE_TLOG << *this << " send " << msg.DebugString();
                                        auto ss = make_shared<net::sendstream>();
                                        msg.SerializeToOstream(ss.get());
                                        nsv_->send(conn_, ss);
                                });
                }

                bool scene::can_stop() const {
                        if (nsv_ && nsv_->conn_count() > 0) {
                                DBCACHE_ILOG << *this << " still connected by scene, cannot stop";
                                return false;
                        }
                        if (!role_dirty_list_.empty() || saving_role_) {
                                DBCACHE_ILOG << *this << " role cannot stop";
                                return false;
                        }
                        if (!arena_rank_->can_stop()) {
                                DBCACHE_ILOG << *this << " arena_rank cannot stop";
                                return false;
                        }
                        if (!gladiators_->can_stop()) {
                                DBCACHE_ILOG << *this << " gladiators cannot stop";
                                return false;
                        }
                        if (!fiefs_->can_stop()) {
                                DBCACHE_ILOG << *this << " fiefs cannot stop";
                                return false;
                        }
                        if (!mansions_->can_stop()) {
                                DBCACHE_ILOG << *this << " mansions cannot stop";
                                return false;
                        }
                        if (!mansion_mgr_->can_stop()) {
                                DBCACHE_ILOG << *this << " mansion_mgr cannot stop";
                                return false;
                        }
                        if (!guans_->can_stop()) {
                                DBCACHE_ILOG << *this << " guans cannot stop";
                                return false;
                        }
                        if (!league_dirty_list_.empty() || saving_league_) {
                                DBCACHE_ILOG << *this << " league cannot stop";
                                return false;
                        }
                        if (!children_->can_stop()) {
                                DBCACHE_ILOG << *this << " children cannot stop";
                                return false;
                        }
                        if (!child_mgr_->can_stop()) {
                                DBCACHE_ILOG << *this << " child_mgr cannot stop";
                                return false;
                        }
                        if (!activity_mgr_->can_stop()) {
                                DBCACHE_ILOG << *this << " activity_mgr cannot stop";
                                return false;
                        }
                        return true;
                }

                ostream& operator<<(ostream& os, const scene& s) {
                        return os << s.name_;
                }

        }
}
