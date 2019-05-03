#pragma once

#include "role.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"
#include "scene/log.hpp"
#include "scene/common_logic.hpp"
#include "scene/item_logic.hpp"
#include "scene/shop.hpp"
#include "scene/xinshou.hpp"
#include "scene/event.hpp"
#include "scene/quest.hpp"
#include "scene/player/role_logic.hpp"
#include "scene/relation_logic.hpp"
#include "scene/activity/activity_logic.hpp"
#include "scene/activity/activity_mgr.hpp"
#include "scene/db_log.hpp"
//#include "scene/yunying/sdk.hpp"
//#include "scene/yunying/bi.hpp"
#include "scene/recharge.hpp"
#include "scene/mail/mail_logic.hpp"
#include "scene/behavior_tree.hpp"
#include "scene/condevent.hpp"
#include "scene/mirror.hpp"
#include "scene/chat.hpp"
#include "scene/dbcache/dbcache.hpp"
#include "scene/common_resource/common_resource.hpp"
#include "scene/item.hpp"
#include "utils/state_machine.hpp"
#include "utils/server_process_mgr.hpp"
#include "utils/dirty_word_filter.hpp"
#include "utils/service_thread.hpp"
#include "utils/json2pb.hpp"
#include "utils/string_utils.hpp"
#include "utils/game_def.hpp"
#include "utils/instance_counter.hpp"
#include "utils/assert.hpp"
#include "utils/time_utils.hpp"
#include "utils/random_utils.hpp"
#include "utils/client_version.hpp"
#include "config/utils.hpp"
#include "config/player_ptts.hpp"
#include "config/drop_ptts.hpp"
#include "config/shop_ptts.hpp"
#include "config/item_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "config/resource_ptts.hpp"
#include "config/chat_ptts.hpp"
#include "config/options_ptts.hpp"
#include "config/behavior_tree_ptts.hpp"
#include "config/relation_ptts.hpp"
#include "config/activity_ptts.hpp"
#include "config/title_ptts.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "proto/cs_login.pb.h"
#include "proto/cs_base.pb.h"
#include "proto/cs_shop.pb.h"
#include "proto/cs_chat.pb.h"
#include "proto/cs_relation.pb.h"
#include "proto/cs_gm.pb.h"
#include "proto/cs_notice.pb.h"
#include "proto/cs_mail.pb.h"
#include "proto/cs_room.pb.h"
#include "proto/cs_quest.pb.h"
#include "proto/cs_item.pb.h"
#include "proto/cs_player.pb.h"
#include "proto/cs_activity.pb.h"
#include "proto/cs_title.pb.h"
#include "proto/cs_xinshou.pb.h"
#include "proto/cs_recharge.pb.h"
#include "proto/cs_yunying.pb.h"
#include "proto/ss_chat.pb.h"
#include "proto/ss_common_resource.pb.h"
#include "proto/data_notice.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_player.pb.h"
#include "proto/processor.hpp"
#include <string>
#include <memory>
#include <functional>
#include <boost/any.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
using namespace chrono;
namespace pcs = proto::cs;
namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace scene {

                using mirror_mgr = server_process_mgr<mirror>;
                using chat_mgr = server_process_mgr<chat>;
                using common_resource_mgr = server_process_mgr<common_resource>;

                template <typename T>
                class player : public enable_shared_from_this<player<T>>,
                               public proto::processor<player<T>, pcs::base>,
                               private boost::noncopyable,
                               private instance_countee {
                public:
                        player(const shared_ptr<service_thread>& st,
                               const shared_ptr<net::server<T>>& nsv,
                               const shared_ptr<T>& conn,
                               const shared_ptr<db::connector>& gamedb_notice,
                               bool mirror_role,
                               bool online) :
                                instance_countee(ICT_PLAYER),
                                st_(st),
                                nsv_(nsv),
                                conn_(conn),
                                gamedb_notice_(gamedb_notice),
                                mirror_role_(mirror_role),
                                online_(online) {
                                if (conn_) {
                                        ip_ = conn_->remote_ip();
                                }
                        }

                        void init() {
                                sm_ = make_shared<state_machine<player<T>>>(this->shared_from_this(), st_);
                                sm_->start(state_init::instance());
                                const auto& ptt = PTTS_GET(behavior_tree_root, pc::BTRT_TICK);
                                bt_ = make_shared<behavior_tree<player<T>>>(this, st_, ptt.node());
                                this->set_proto_min_frequency(milliseconds(100));
                                this->add_ignore_min_frequency(pcs::c2s_role_get_info_req::descriptor());
                        }

                        static void static_init() {
                            //register some handler
                                player::register_proto_handler(pcs::c2s_login_req::descriptor(), &player::process_login_req);
                                player::register_proto_handler(pcs::c2s_create_role_req::descriptor(), &player::process_create_role_req);
                                player::register_proto_handler(pcs::c2s_ping_req::descriptor(), &player::process_ping_req);
                                player::register_proto_handler(pcs::c2s_mail_fetch_req::descriptor(), &player::process_mail_fetch_req);
                                player::register_proto_handler(pcs::c2s_mail_delete_req::descriptor(), &player::process_mail_delete_req);
                                // ai_action
                                //player::ai_action_handlers_[pd::PAT_ARENA_JOIN] = &player::ai_action_join_arena;

                                // ai_condition
                                //player::ai_condition_handlers_[pd::PCT_MANSION_CREATED] = &player::ai_condition_mansion_created;
                        }

                        const string& username() const {
                                return username_;
                        }

                        role *get_role() {
                                return role_.get();
                        }

                        void ai_stop() {
                                bt_->stop();
                        }

                        uint64_t gid() const {
                                if (role_) {
                                        return role_->gid();
                                } else {
                                        return 0;
                                }
                        }

                        uint32_t level() const {
                                ASSERT(role_);
                                return role_->level();
                        }

                        pd::gender gender() const {
                                ASSERT(role_);
                                return role_->gender();
                        }

                        bool online() const {
                                return online_;
                        }

                        const string& ip() const {
                                return ip_;
                        }

                        bool mirror_role() const {
                                return mirror_role_;
                        }

                        bool ai_is_active() const {
                                return bt_->running();
                        }

                        void ai_set_active(bool active) {
                                if (bt_->running() == active) {
                                        return;
                                }
                                if (active) {
                                        bt_->run();
                                } else {
                                        bt_->stop();
                                }
                        }

                        void set_next_frequency(const system_clock::duration& du) {
                                bt_->set_next_frequency(du);
                        }

                        void set_cur_root(uint32_t children_node) {
                                const auto& ptt = PTTS_GET(behavior_tree_root, pc::BTRT_TICK);
                                if (ptt.node() != children_node) {
                                        bt_->rebuild_root_node(ptt.node(), children_node);
                                }
                        }

                        void add_async_call_count() {
                                async_call_count_ += 1;
                        }

                        void dec_async_call_count() {
                                ASSERT(async_call_count_.load() > 0);
                                async_call_count_ -= 1;
                        }

                        bool has_async_call() const {
                                return async_call_count_.load() > 0;
                        }

                        void async_call(const function<void()>& func) {
                                add_async_call_count();
                                st_->async_call(
                                        [this, self = this->shared_from_this(), func = func] {
                                                this->dec_async_call_count();
                                                func();
                                                update_role_db();
                                        });
                        }

                        const shared_ptr<T>& conn() const {
                                return conn_;
                        }

                        void send_to_client(pcs::base& msg, bool no_lag = false) {
                                auto ptt = PTTS_GET_COPY(options, 1);
                                if (ptt.scene_info().lag() > 0 && !no_lag) {
                                        ADD_TIMER(
                                                st_,
                                                ([this, self = this->shared_from_this(), msg] (auto canceled, const auto& timer) mutable {
                                                        if (!canceled) {
                                                                this->send_to_client_inner(msg);
                                                        }
                                                }),
                                                milliseconds(ptt.scene_info().lag()));
                                } else {
                                        async_call(
                                                [this, self = this->shared_from_this(), msg] () mutable {
                                                        this->send_to_client_inner(msg);
                                                });
                                }
                        }

                        void send_to_client_inner(pcs::base& msg) {
                                ASSERT(st_->check_in_thread());
                                if (!conn_) {
                                        return;
                                }
                                msg.set_time(system_clock::to_time_t(system_clock::now()));
                                SPLAYER_TLOG << *this << " send " << msg.DebugString();

                                ASSERT(send_msg_cb_);
                                const Reflection *reflection = msg.GetReflection();
                                vector<const FieldDescriptor *> fields;
                                reflection->ListFields(msg, &fields);
                                ASSERT(fields.size() == 2);
                                const Descriptor *desc = fields.back()->message_type();
                                send_msg_cb_(desc, msg.ByteSize());

                                auto ss = make_shared<net::sendstream>();
                                msg.SerializeToOstream(ss.get());
                                nsv_->send(conn_, ss);
                        }

                        void set_mirror_role_data(uint64_t gid, const pd::role& data) {
                                mirror_role_data_ = make_unique<pd::role>(data);
                                mirror_role_data_->set_gid(gid);
                        }

                        void login(const string& username, bool check_sdk = true) {
                                async_call(
                                        [this, self = this->shared_from_this(), username, check_sdk] {
                                                event_login event(username, check_sdk);
                                                sm_->on_event(&event);
                                        });
                        }

                        void create_role(const pd::role& data) {
                                ASSERT(st_->check_in_thread());
                                async_call(
                                        [this, self = this->shared_from_this(), data] {
                                                auto msg = dbcache::gen_create_role_req(username_, data);
                                                dbcache::instance().send_to_dbcached(msg);

                                                event_client_create_role event(username_);
                                                sm_->on_event(&event);
                                        });
                        }

                        void quit() {
                                async_call(
                                        [this, self = this->shared_from_this()] {
                                                event_logout event;
                                                sm_->on_event(&event);
                                        });
                        }

                        bool stop_playing() {
                                auto pl = this->shared_from_this();
                                if (!stop_playing_cb_(pl)) {
                                        SPLAYER_DLOG << *this << " " << *this << " reset timeout";
                                        set_quiting_playing();
                                        sm_->reset_timeout(1s);
                                        return false;
                                }

                                if (online()) {
                                        if (role_->marriage_married()) {
                                                ASSERT(marriage_notice_spouse_logout_func_);
                                                marriage_notice_spouse_logout_func_(role_->gid(), role_->marriage_spouse());
                                        }

                                        role_->logout();
                                        update_role_db();

                                        stop_resource_grow_timer();
                                        /*bi::instance().logout(username(),
                                                              yci_,
                                                              ip_,
                                                              role_->gid(),
                                                              role_->last_login_time(),
                                                              pd::ce_origin(),
                                                              role_->level(),
                                                              role_->get_resource(pd::resource_type::EXP),
                                                              role_->get_resource(pd::resource_type::DIAMOND),
                                                              role_->get_resource(pd::resource_type::GOLD),
                                                              role_->calc_max_zhanli(),
                                                              role_->name(),
                                                              pd::gender_Name(role_->gender()),
                                                              role_->get_resource(pd::resource_type::VIP_EXP),
                                                              role_->vip_level());*/

                                        db_log::instance().log_logout(*role_, ip_);
                                }
                                return true;
                        }

                        void quit_timeout() {
                                ASSERT(st_->check_in_thread());

                                ai_stop();
                                stop_resource_grow_timer();
                                stop_mirror_timers();
                                sm_->stop();
                                fetch_room_list_call_funcs_.clear();
                                fetch_room_info_call_funcs_.clear();
                                if (quit_cb_) {
                                        quit_cb_(this->shared_from_this());
                                }
                                if (role_) {
                                        role_.reset();
                                }
                                if (nsv_ && conn_) {
                                        nsv_->disconnect(conn_);
                                        conn_.reset();
                                }
                        }

                        void handle_finished_db_msg(const shared_ptr<db::message>& msg) {
                                ASSERT(st_->check_in_thread());
                                event_db_rsp event(msg);
                                sm_->on_event(&event);
                        }

                        void process_msg(const shared_ptr<net::recvstream>& msg) {
                                async_call(
                                        [this, self = this->shared_from_this(), msg] {
                                                pcs::base cs_msg;
                                                if (cs_msg.ParseFromIstream(msg.get())) {
                                                        SPLAYER_TLOG << *this << " recv " << cs_msg.DebugString();
                                                        this->process(this, &cs_msg);
#ifndef _DEBUG
                                                        if (this->check_state_playing() && !quiting_playing_) {
                                                                this->sm_->reset_timeout(5min);
                                                        }
#endif
                                                } else {
                                                        SPLAYER_DLOG << *this << " parse msg failed";
                                                }
                                        });
                        }

                        void process_msg(const pcs::base& msg) {
                                async_call(
                                        [this, self = this->shared_from_this(), msg] {
                                                this->process(this, &msg);
                                        });
                        }

                        void process_login_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_init()) {
                                        return;
                                }
                                const auto& req = msg->GetExtension(pcs::c2s_login_req::clr);

                                auto result = pd::OK;
                                if (req.client_version_size() != 3) {
                                        result = pd::CLIENT_VERSION_INVAILD;
                                } else {
                                        auto ptt = PTTS_GET_COPY(options, 1);
                                        vector<uint32_t> minimum_version_vec;
                                        result = parse_str_version_to_vector(ptt.login_info().client_version(), minimum_version_vec);
                                        if (result == pd::OK) {
                                                vector<uint32_t> client_version_vec;
                                                for (auto small_version : req.client_version()) {
                                                        client_version_vec.push_back(small_version);
                                                }
                                                if (compare_client_version(client_version_vec, minimum_version_vec) < 0) {
                                                        result = pd::CLIENT_VERSION_TOO_OLD;
                                                }
                                        }
                                }
                                if (result != pd::OK) {
                                        pcs::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                        rsp->set_username(req.username());
                                        rsp->set_result(result);
                                        send_to_client(rsp_msg);
                                        return;
                                }

                                login(req.username() + "_" + to_string(req.server_id()));

                                ai_.robot_role_ = req.robot_role();
                                ai_.behavior_children_node_ = req.tree_children_node();

                                yci_ = req.client_info();
                                origin_username_ = req.username();
                                server_id_ = req.server_id();
                        }

                        void set_username(const string& username) {
                                username_ = username;
                        }

                        void process_create_role_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_client_create_role()) {
                                        return;
                                }

                                const auto& req = msg->GetExtension(pcs::c2s_create_role_req::ccrr);
                                auto result = check_name(req.name(), forbid_names(), !req.robot());
                                if (result != pd::OK) {
                                        SPLAYER_DLOG << " invalid name " << pd::result_Name(result);
                                        return;
                                }

                                auto options_ptt = PTTS_GET_COPY(options, 1);
                                auto today = refresh_day();
                                auto now = system_clock::to_time_t(system_clock::now());
                                pd::role data;
                                pc::create_role *ptt = nullptr;
                                if (ai_.robot_role_ && options_ptt.scene_info().allow_robot()) {
                                        ptt = req.gender() == pd::FEMALE ? &PTTS_GET(create_role, 3) : &PTTS_GET(create_role, 4);
                                } else if (options_ptt.scene_info().testin()) {
                                        ptt = &PTTS_GET(create_role, 5);
                                } else {
                                        ptt = req.gender() == pd::FEMALE ? &PTTS_GET(create_role, 1) : &PTTS_GET(create_role, 2);
                                }
                                ASSERT(ptt);
                                data.set_gid(gid::instance().new_gid(gid_type::ROLE));
                                auto *role_data = data.mutable_data();
                                if (req.has_channel_id()) {
                                        role_data->set_channel_id(req.channel_id());
                                }
                                role_data->set_name(req.name());
                                role_data->set_username(username_);
                                role_data->set_gender(req.gender() == pd::FEMALE ? pd::FEMALE : pd::MALE);
                                role_data->set_icon(0);
                                for (auto i = 0; i < PLAYER_CUSTOM_ICON_COUNT; ++i) {
                                        role_data->add_custom_icons("");
                                }
                                if (ptt->has_level()) {
                                        role_data->set_level(ptt->level());
                                }
                                role_data->set_last_levelup_time(now);
                                auto *role_other_data = data.mutable_other_data();
                                role_other_data->mutable_league()->set_leave_league_time(0);
                                role_other_data->mutable_league()->set_latest_refresh_day(today);

                                role r(data.gid(), yci_, ip_);
                                r.username_func_ = [this] {
                                        return username();
                                };

                                for (const auto& i : ptt->resources()) {
                                        r.set_resource(i.type(), i.value());
                                }
                                r.reset_resources();
                                const auto& resource_ptts = PTTS_GET_ALL(resource);
                                for (const auto& i : resource_ptts) {
                                        if (i.second.grow_minutes() > 0) {
                                                r.set_resource_grow_time(i.second.type(), now);
                                        }
                                }
                                r.serialize_resources(data.mutable_resources());

                                pd::spine_item_array col;
                                *col.mutable_items() = ptt->spine_collection();
                                for (auto i = 0u; i < SPINE_SELECTION_COUNT; ++i) {
                                        *spine->add_selections() = spine_check_and_fix_sel(req.spine_selection(), col);
                                }
                                spine->set_cur_selection(0);
                                *spine->mutable_collection() = ptt->spine_collection();
                                r.parse_spine(&data.spine());

                                set<uint32_t> slot;
                                vector<uint32_t> skills;
                                for (auto i : ptt->grids()) {
                                        const auto& actor_ptt = PTTS_GET(actor, i.actor());
                                        if (actor_ptt.type() == pd::actor::ZHU) {
                                                for(auto j : actor_ptt.skill()) {
                                                        const auto& skill_ptt = PTTS_GET(skill, j);
                                                        if (skill_ptt.has_slot() && slot.count(skill_ptt.slot()) <= 0) {
                                                                slot.insert(skill_ptt.slot());
                                                                skills.push_back(j);
                                                        }
                                                }
                                                break;
                                        }
                                }
                                auto *battle = data.mutable_battle();
                                for (auto i = 0; i < 4; ++i) {
                                        set<uint32_t> skill_slots;
                                        auto *formation = battle->add_formations();
                                        *formation->mutable_grids()->mutable_grids() = ptt->grids();
                                        for (auto j : skills) {
                                                const auto& skill_ptt = PTTS_GET(skill, j);
                                                if (skill_ptt.has_slot() && skill_slots.count(skill_ptt.slot()) <= 0) {
                                                        skill_slots.insert(skill_ptt.slot());
                                                        formation->add_skills(j);
                                                }
                                        }
                                }
                                r.parse_battle(&data.battle());

                                for (const auto& i : ptt->huanzhuang_collection()) {
                                        r.huanzhuang_add_collection(i.part(), i.pttid());
                                }
                                pd::huanzhuang_item_id_array selection;
                                *selection.mutable_items() = ptt->huanzhuang_selection();
                                for (auto i = 0u; i < HUANZHUANG_SELECTION_COUNT; ++i) {
                                        r.huanzhuang_update_selection(i, selection);
                                }
                                r.huanzhuang_set_cur_selection(0);
                                r.serialize_huanzhuang(data.mutable_huanzhuang());

                                for (auto i : ptt->items()) {
                                        r.add_item(i.pttid(), i.count(), true);
                                }
                                r.serialize_items(data.mutable_items());

                                for (const auto& i : ptt->equipments()) {
                                        for (auto j = 0; j < i.count(); ++j) {
                                                r.add_equipment(i.pttid(), i.quality(), true);
                                        }
                                }
                                r.serialize_equipments(data.mutable_equipments());

                                r.serialize_jades(data.mutable_jades());

                                for (auto i : ptt->titles()) {
                                        r.add_title(i);
                                }
                                r.serialize_titles(data.mutable_titles());

                                for (auto i : ptt->quests()) {
                                        r.quest_accept(i);
                                }
                                if (today <= server_open_day_func_() + 6) {
                                        const auto& al_ptt = PTTS_GET(activity_logic, 1);
                                        const auto& alr_ptt = PTTS_GET(activity_leiji_recharge, al_ptt.activity_start().leiji_recharge());
                                        for (auto i : alr_ptt.quests()) {
                                                r.quest_accept(i.auto_quest());
                                                r.quest_accept(i.event_quest());
                                        }

                                        const auto& alc_ptt = PTTS_GET(activity_leiji_consume, al_ptt.activity_start().leiji_consume());
                                        for (auto i : alc_ptt.quests()) {
                                                r.quest_accept(i.auto_quest());
                                                r.quest_accept(i.event_quest());
                                        }
                                        const auto& dyr_ptt = PTTS_GET(activity_daiyanren, 1);
                                        for (auto i : dyr_ptt.fuli_quests()) {
                                                r.quest_accept(i);
                                        }
                                        for (auto i : dyr_ptt.duihuan_quests()) {
                                                r.quest_accept(i);
                                        }
                                } else {
                                        r.activity_daiyanren_set_finished();
                                }

                                for (auto i : ptt->mansion_cook_recipes()) {
                                        r.mansion_cook_add_recipe(i);
                                }
                                r.serialize_mansion_data(data.mutable_mansion_data());

                                r.guanpin_update_battle_seed();
                                r.serialize_guanpin_data(data.mutable_guanpin_data());

                                r.serialize_records(data.mutable_records());

                                pd::event_res event_res;
                                r.refresh_everyday_quests(event_res);
                                r.accept_achievement_quest();
                                r.accept_ui_quest();
                                r.serialize_quests(data.mutable_quests());

                                auto m = mail_new_mail(data.gid(), CREATE_ROLE_FIRST_MAIL_PTTID, pd::dynamic_data(), pd::event_array());
                                r.add_mail(m);
                                r.serialize_mails(data.mutable_mails());

                                create_role(data);

                                if (!req.robot()) {
                                        ADD_TIMER(
                                                st_,
                                                ([this, self = this->shared_from_this(), data] (auto canceled, const auto& timer) {
                                                        if (!canceled) {
                                                                auto username = split_string(username_, '_');
                                                                ASSERT(username.size() == 2);
                                                                ps::base fanli_msg;
                                                                auto *fanli = fanli_msg.MutableExtension(ps::s2cr_lock_fanli_req::slfr);
                                                                fanli->set_username(username[0]);
                                                                common_resource_mgr::instance().random_send_msg(fanli_msg);

                                                                ps::base come_back_msg;
                                                                auto *come_back = come_back_msg.MutableExtension(ps::s2cr_lock_come_back_req::slcbr);
                                                                come_back->set_username(username[0]);
                                                                common_resource_mgr::instance().random_send_msg(come_back_msg);
                                                        }
                                                }),
                                                1s);
                                }
                        }

                        void process_ping_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing() && !check_state_create_role() && !check_state_client_create_role()) {
                                        return;
                                }

                                pcs::base rsp_msg;
                                rsp_msg.MutableExtension(pcs::s2c_ping_rsp::spr);
                                send_to_client(rsp_msg);
                        }

                        //void process_challenge_adventure(const pcs::base *msg) {
                        //}


                        /*void process_shop_check_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }

                                const auto& req = msg->GetExtension(pcs::c2s_shop_check_req::cscr);
                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_shop_check_rsp::sscr);
                                do {
                                        if (!PTTS_HAS(shop, req.pttid())) {
                                                rsp->set_result(pd::SHOP_NO_SUCH_SHOP);
                                                break;
                                        }

                                        auto ptt = PTTS_GET_COPY(shop, req.pttid());
                                        rsp->set_pttid(ptt.id());
                                        if (ptt.version() != req.version()) {
                                                rsp->set_result(pd::SHOP_NEED_UPDATE);
                                                *rsp->mutable_shop() = ptt;
                                                break;
                                        }

                                        rsp->set_result(pd::OK);
                                } while (false);
                                send_to_client(rsp_msg);
                        }

                        void process_shop_buy_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }

                                const auto& req = msg->GetExtension(pcs::c2s_shop_buy_req::csbr);
                                if (req.buy_goods_size() <= 0) {
                                        return;
                                }

                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_shop_buy_rsp::ssbr);
                                bool bought = false;
                                pd::event_res event_res;
                                bool send_bi = false;
                                for (const auto& i : req.buy_goods()) {
                                        for (auto j = 0u; j < i.count(); ++j) {
                                                auto result = shop_check_buy(i.shop(), i.good_id(), *role_);
                                                rsp->set_result(result);
                                                if (result != pd::OK) {
                                                        break;
                                                }
                                                if (j + 1 >= i.count()) {
                                                        send_bi = true;
                                                }
                                                event_res = shop_buy(i.shop(), i.good_id(), *role_, &event_res, send_bi);
                                                bought = true;
                                        }
                                        if (bought) {
                                                pd::yunying_bi_info bi;
                                                auto ptt = PTTS_GET_COPY(shop, i.shop());
                                                for (const auto& j : ptt.goods()) {
                                                        if (j.id() == i.good_id()) {
                                                                bi.set_is_discount(j.discount());
                                                                bi.set_is_promotion(0);
                                                                bi::instance().shop_stream(username(), yci_, ip_, role_->gid(), role_->league_gid(), bi, j.type(), i.shop(), i.good_id(), event_res);
                                                        }
                                                }
                                        }
                                }
                                if (bought) {
                                        *rsp->mutable_res() = event_res;
                                        db_log::instance().log_shop_buy(event_res, *role_);
                                }
                                send_to_client(rsp_msg);
                        }

                        void process_mys_shop_refresh_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }

                                const auto& req = msg->GetExtension(pcs::c2s_mys_shop_refresh_req::cmsrr);
                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_mys_shop_refresh_rsp::smsrr);
                                rsp->set_pttid(req.pttid());
                                rsp->set_free(req.free());
                                pd::event_res event_res;
                                auto result = pd::OK;
                                if (req.free()) {
                                        result = mys_shop_free_refresh_goods(req.pttid(), &event_res, *role_);
                                } else {
                                        result = mys_shop_refresh_goods(req.pttid(), &event_res, *role_);
                                }
                                rsp->set_result(result);
                                if (result == pd::OK) {
                                        *rsp->mutable_event_res() = event_res;
                                        role_->serialize_mys_shop(req.pttid(), rsp->mutable_mys_shop());
                                        db_log::instance().log_mys_shop_refresh(req.pttid(), event_res, rsp->mys_shop(), *role_);
                                }

                                send_to_client(rsp_msg);
                        }

                        void process_mys_shop_buy_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }

                                const auto& req = msg->GetExtension(pcs::c2s_mys_shop_buy_req::cmsbr);
                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_mys_shop_buy_rsp::smsbr);
                                rsp->set_pttid(req.pttid());
                                auto result = mys_shop_check_buy(req.pttid(), req.good_id(), *role_);
                                rsp->set_good_id(req.good_id());
                                rsp->set_result(result);
                                if (result == pd::OK) {
                                        *rsp->mutable_event_res() = mys_shop_buy(req.pttid(), req.good_id(), *role_);
                                        db_log::instance().log_mys_shop_buy(req.pttid(), req.good_id(), &rsp->event_res(), *role_);
                                }

                                send_to_client(rsp_msg);
                        }

                        void mirror_broad_system_chat(uint32_t system_chat) {
                                auto delay = rand() % 60 + 10;
                                add_mirror_timer(delay, [this, self = this->shared_from_this(), system_chat] {
                                        pd::role_mirror_event event;
                                        event.set_gid(this->gid());
                                        event.set_type(pd::RMET_BROADCAST);
                                        event.set_system_chat(system_chat);
                                        this->send_mirror_role_event_msg(event);
                                });
                        }
*/

/*                        void process_chat_req(const pcs::base *msg) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }

                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_chat_rsp::scr);
                                rsp->set_result(pd::OK);
                                do {
                                        if (role_->gag_until_time() >= system_clock::to_time_t(system_clock::now())) {
                                                rsp->set_result(pd::CHAT_GAG);
                                                break;
                                        }

                                        const auto& req = msg->GetExtension(pcs::c2s_chat_req::ccr);
                                        auto result = check_content(req.content(), false);
                                        if (result != pd::OK) {
                                                rsp->set_result(result);
                                                break;
                                        }
                                        bool all_space = true;
                                        for (auto i : req.content()) {
                                                if (!isspace(i)) {
                                                        all_space = false;
                                                        break;
                                                }
                                        }
                                        if (all_space) {
                                                rsp->set_result(pd::CHAT_CONTENT_ALL_SPACE);
                                                break;
                                        }

                                        auto content = req.content();
                                        auto options_ptt = PTTS_GET_COPY(options, 1u);
                                        if (options_ptt.scene_info().dev_gm()) {
                                                if (try_exec_chat_gm(content)) {
                                                        return;
                                                }
                                        }

                                        const auto& ptt = PTTS_GET(chat, req.type());
                                        result = condition_check(ptt.conditions(), *role_);
                                        if (ptt.has_send_cd() && role_->get_last_chat_time(req.type()) > 0) {
                                                if (system_clock::to_time_t(system_clock::now()) <= role_->get_last_chat_time(req.type()) + ptt.send_cd()) {
                                                        result = pd::CHAT_IN_CD;
                                                }
                                        }

                                        if (result == pd::OK) {
                                                role_->update_chat_data(req.type());
                                                pd::ce_env ce;
                                                ce.set_origin(pd::CO_CHAT);
                                                *rsp->mutable_event_res() = event_process(ptt._events(), *role_, ce);
                                        } else {
                                                rsp->set_result(result);
                                                break;
                                        }

                                        content = dirty_word_filter::instance().fix(content);
                                        auto voice = req.voice();
                                        pd::role_info ri;
                                        role_->serialize(ri.mutable_simple_info());
                                        auto time = system_clock::to_time_t(system_clock::now());
                                        switch (req.type()) {
                                        case pd::CT_PRIVATE: {
                                                if (!req.has_to()) {
                                                        rsp->set_result(pd::CHAT_NEED_TO);
                                                        break;
                                                }
                                                if (role_->relation_in_blacklist(req.to())) {
                                                        rsp->set_result(pd::CHAT_TO_IN_BLACKLIST);
                                                        break;
                                                }
                                                ASSERT(private_chat_func_);
                                                private_chat_func_(ri, time, req.to(), content, voice);
                                                send_chat(pd::CT_PRIVATE, ri, time, content, voice, 0, req.to());
                                                role_->on_event(ECE_PRIVATE_CHAT, {});
                                                break;
                                        }
                                        case pd::CT_LEAGUE: {
                                                ASSERT(league_chat_func_);
                                                league_chat_func_(ri, time, content, voice);
                                                break;
                                        }
                                        case pd::CT_WORLD:
                                        case pd::CT_WORLD_LABA: {
                                                ASSERT(world_chat_func_);
                                                world_chat_func_(req.type(), ri, time, content, voice);
                                                role_->on_event(ECE_WORLD_CHAT, {});
                                                break;
                                        }
                                        case pd::CT_BANQUET: {
                                                if (!req.has_to()) {
                                                        rsp->set_result(pd::CHAT_NEED_TO);
                                                        break;
                                                }
                                                if (mansion_check_enter_banquet_time() != pd::OK) {
                                                        rsp->set_result(pd::CHAT_NOT_BANQUET_TIME);
                                                        break;
                                                }
                                                ASSERT(banquet_chat_func_);
                                                banquet_chat_func_(ri, time, req.to(), content, voice);
                                                break;
                                        }
                                        case pd::CT_ROOM: {
                                                if (!req.has_to()) {
                                                        rsp->set_result(pd::CHAT_NEED_TO);
                                                        break;
                                                }
                                                room_mgr::instance().room_chat(ri, time, req.to(), content, voice);
                                                break;
                                        }
                                        case pd::CT_CROSS_WORLD: {
                                                ps::base msg;
                                                auto *req = msg.MutableExtension(ps::sch_cross_world_chat::scwc);
                                                *req->mutable_from() = ri;
                                                req->set_time(time);
                                                req->set_from_server(options_ptt.scene_info().server_id());
                                                req->set_content(content);
                                                *req->mutable_voice() = voice;
                                                chat_mgr::instance().broadcast_msg(msg);
                                                break;
                                        }
                                        default:
                                                break;
                                        }
                                        db_log::instance().log_chat(req.content(), rsp->event_res(), *role_);
                                } while (false);

                                send_to_client(rsp_msg);
                        }
*/
                        /*

                        void add_resource_grow_timer(pd::resource_type type) {
                                if (!check_state_fetch_data() && !check_state_playing()) {
                                        return;
                                }

                                const auto& ptt = PTTS_GET(resource, type);
                                ASSERT(ptt.grow_minutes() > 0);
                                resource_grow_timer_[type] = ADD_TIMER(
                                        st_,
                                        ([this, self = this->shared_from_this(), type = ptt.type()] (auto canceled, const auto& timer) {
                                                auto it = resource_grow_timer_.find(type);
                                                if (it == resource_grow_timer_.end() && it->second == timer) {
                                                        resource_grow_timer_.erase(type);
                                                }
                                                if (!canceled) {
                                                        if (!this->check_state_fetch_data() && !this->check_state_playing()) {
                                                                return;
                                                        }

                                                        auto now = system_clock::to_time_t(system_clock::now());
                                                        auto old_value = role_->get_resource(type);
                                                        role_->grow_resource(type);
                                                        this->add_resource_grow_timer(type);
                                                        this->update_role_db();

                                                        auto new_value = role_->get_resource(type);
                                                        if (old_value != new_value && this->check_state_playing()) {
                                                                pcs::base msg;
                                                                auto *notice = msg.MutableExtension(pcs::s2c_role_update_resource::srur);
                                                                notice->set_type(type);
                                                                notice->set_value(new_value);
                                                                notice->set_time(now);
                                                                this->send_to_client(msg);
                                                        }
                                                }
                                        }),
                                        system_clock::from_time_t(role_->resource_grow_time(type)) + minutes(ptt.grow_minutes()));
                        }

                        void stop_resource_grow_timer() {
                                for (auto& i : resource_grow_timer_) {
                                        i.second->cancel();
                                        i.second.reset();
                                }
                                resource_grow_timer_.clear();
                        }

                        void stop_mirror_timers() {
                                for (auto i : mirror_timers_) {
                                        i->cancel();
                                        i.reset();
                                }
                                mirror_timers_.clear();
                        }

                        void add_rand_quit_timer() {
                                rand_quit_timer_ = ADD_TIMER(
                                        st_,
                                        ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                                if (rand_quit_timer_ == timer) {
                                                        rand_quit_timer_.reset();
                                                }
                                                if (!canceled && this->check_state_playing()) {
                                                        if (rand() % 5 == 0) {
                                                                this->quit();
                                                        } else {
                                                                this->add_rand_quit_timer();
                                                        }
                                                }
                                        }),
                                        5s);
                        }

                        void remove_rand_quit_timer() {
                                ASSERT(st_->check_in_thread());
                                if (rand_quit_timer_) {
                                        rand_quit_timer_->cancel();
                                        rand_quit_timer_.reset();
                                }
                        }
*/
                        void create_role_done(pd::result result) {
                                async_call(
                                        [this, self = this->shared_from_this(), result] {
                                                SPLAYER_DLOG << *this << " create role done " << pd::result_Name(result);
                                                event_create_role_done event(result);
                                                sm_->on_event(&event);
                                        });
                        }

                        void find_role_done(pd::result result, const pd::role& role) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, role] {
                                                SPLAYER_DLOG << *this << " find role done " << pd::result_Name(result);
                                                event_find_role_done event(result, role);
                                                sm_->on_event(&event);
                                        });
                        }

                        void find_role_by_gid_done(uint64_t gid, pd::result result, const string& username, const pd::role& role) {
                                async_call(
                                        [this, self = this->shared_from_this(), gid, result, username, role] {
                                                SPLAYER_DLOG << *this << " find role by gid done " << gid << " " << pd::result_Name(result);
                                                event_find_role_by_gid_done event(result, gid, username, role);
                                                sm_->on_event(&event);
                                        });
                        }

                     
                        void login_by_gid(uint64_t gid) {
                                async_call(
                                        [this, self = this->shared_from_this(), gid] {
                                                SPLAYER_DLOG << *this << " login by gid " << gid;
                                                event_login_by_gid event(gid);
                                                sm_->on_event(&event);
                                        });
                        }

                        void send_info_to(uint64_t to, bool simple) {
                                async_call(
                                        [this, self = this->shared_from_this(), to, simple] {
                                                SPLAYER_DLOG << *this << " send info to " << to;
                                                ASSERT(check_state_playing());

                                                pd::role_info data;
                                                role_->serialize_role_info_for_client(&data, simple);
                                                send_info_func_(to, data, simple);
                                        });
                        }

                        

                        void get_role_info_by_str_to(uint64_t to, const string& str, const queue<uint64_t>& gids, const pd::role_info_array& data) {
                                async_call(
                                        [this, self = this->shared_from_this(), to, str, gids = gids, data = data] () mutable {
                                                SPLAYER_DLOG << *this << " get role info by str to " << to;
                                                ASSERT(check_state_playing());
                                                ASSERT(gids.front() == role_->gid());

                                                gids.pop();
                                                role_->serialize_role_info_for_client(data.add_infos(), true);

                                                get_role_info_by_str_func_(to, str, gids, data);
                                        });
                        }

                        void send_info(const pd::role_info& data, bool simple) {
                                async_call(
                                        [this, self = this->shared_from_this(), data, simple] {
                                                SPLAYER_DLOG << *this << " send info";
                                                if (!check_state_playing()) {
                                                        return;
                                                }
                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_role_get_info_rsp::srgir);
                                                rsp->set_result(pd::OK);
                                                rsp->set_gid(data.simple_info().gid());
                                                rsp->set_simple(simple);
                                                *rsp->mutable_info() = data;
                                                send_to_client(msg);
                                        });
                        }

                        void get_role_info_by_str_done(const string& str, const pd::role_info_array& data) {
                                async_call(
                                        [this, self = this->shared_from_this(), str, data] {
                                                SPLAYER_DLOG << *this << " get role info by str done";
                                                if (!check_state_playing()) {
                                                        return;
                                                }
                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_role_get_info_by_str_rsp::srgibsr);
                                                rsp->set_str(str);
                                                rsp->set_result(pd::OK);
                                                *rsp->mutable_infos() = data;
                                                send_to_client(msg);
                                        });
                        }



                        friend ostream& operator<<(ostream& os, const player& pl) {
                                ASSERT(pl.st_->check_in_thread());
                                auto username = pl.username().empty() ? "noname" : pl.username();
                                return os << username << " " << pl.gid();
                        }

                        // events
                        class event_db_rsp : public event_base {
                        public:
                                event_db_rsp(const shared_ptr<db::message>& db_msg) :
                                        db_msg_(db_msg) {}
                                shared_ptr<db::message> db_msg_;
                                friend ostream& operator<<(ostream& os, const event_db_rsp& e) {
                                        return os << "<event:db_rsp>" << *e.db_msg_;
                                }
                        };

                        class event_create_role_done : public event_base {
                        public:
                                event_create_role_done(pd::result result) : result_(result) {}
                                pd::result result_;
                                friend ostream& operator<<(ostream& os, const event_create_role_done& e) {
                                        return os << "<event:create_role_done> " << pd::result_Name(e.result_);
                                }
                        };

                        class event_check_login_done : public event_base {
                        public:
                                event_check_login_done(pd::result result, const string& username) :
                                        result_(result), username_(username) {}
                                pd::result result_;
                                const string username_;
                                friend ostream& operator<<(ostream& os, const event_check_login_done& e) {
                                        return os << "<event:check_login_done> " << pd::result_Name(e.result_);
                                }
                        };

                        class event_find_role_done : public event_base {
                        public:
                                event_find_role_done(pd::result result, const pd::role& role) :
                                        result_(result), role_(role) {}
                                pd::result result_;
                                pd::role role_;
                                friend ostream& operator<<(ostream& os, const event_find_role_done& e) {
                                        return os << "<event:find_role_done> " << pd::result_Name(e.result_);
                                }
                        };

                        class event_find_role_by_gid_done : public event_base {
                        public:
                                event_find_role_by_gid_done(pd::result result, uint64_t gid, const string& username, const pd::role& role) :
                                        result_(result), gid_(gid), username_(username), role_(role) {}
                                pd::result result_;
                                uint64_t gid_;
                                string username_;
                                pd::role role_;
                                friend ostream& operator<<(ostream& os, const event_find_role_by_gid_done& e) {
                                        return os << "<event:find_role_by_gid_done> " << pd::result_Name(e.result_);
                                }
                        };

                        class event_net_rsp : public event_base {
                        public:
                                event_net_rsp(const string& rolename) :
                                        rolename_(rolename) {
                                }
                                const string rolename_;
                                friend ostream& operator<<(ostream& os, const event_net_rsp& e) {
                                        return os << "<event:net_rsp><role:" << e.rolename_ << ">";
                                }
                        };

                        class event_client_create_role : public event_base {
                        public:
                                event_client_create_role(const string& username) :
                                        username_(username) {
                                }
                                const string username_;
                                friend ostream& operator<<(ostream& os, const event_client_create_role& e) {
                                        return os << "<event:client_create_role><role:" << e.username_ << ">";
                                }
                        };

                        class event_fetch_data_done : public event_base {
                                friend ostream& operator<<(ostream& os, const event_fetch_data_done& e) {
                                        return os << "<event:fetch_data_done>";
                                }
                        };

                        class event_logout : public event_base {
                                friend ostream& operator<<(ostream& os, const event_logout& e) {
                                        return os << "<event:logout>";
                                }
                        };

                        class event_login : public event_base {
                        public:
                                event_login(const string& username, bool check_sdk) : username_(username), check_sdk_(check_sdk) {
                                }
                                const string username_;
                                bool check_sdk_ = true;
                                friend ostream& operator<<(ostream& os, const event_login& el) {
                                        return os << "<event:login>";
                                }
                        };

                        class event_login_by_gid : public event_base {
                        public:
                                event_login_by_gid(uint64_t gid) : gid_(gid) {
                                }
                                uint64_t gid_;
                                friend ostream& operator<<(ostream& os, const event_login_by_gid& elbg) {
                                        return os << "<event:login_by_gid " << elbg.gid_ << ">";
                                }
                        };

                        // states
                        class state_init : public state_base<player> {
                        public:
                                static state_init *instance() {
                                        static state_init inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) override {
                                        instance_counter::instance().add(ICT_INIT_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) override {
                                        if (auto *e = dynamic_cast<event_login *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                auto options_ptt = PTTS_GET_COPY(options, 1);
                                                if (!options_ptt.scene_info().sdk_check_login() || !e->check_sdk_) {
                                                        pl->username_ = e->username_;
                                                        ASSERT(pl->login_cb_);
                                                        auto result = pl->login_cb_(pl);
                                                        if (result != pd::OK) {
                                                                pcs::base msg;
                                                                auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                                rsp->set_username(pl->username_);
                                                                rsp->set_result(result);
                                                                pl->send_to_client(msg);
                                                                return state_quit::instance();
                                                        }
                                                        auto msg = dbcache::gen_find_role_req(pl->username_, 0, 0);
                                                        dbcache::instance().send_to_dbcached(msg);
                                                        return state_find_role::instance();
                                                } else {
                                                        sdk::instance().check_login(pl->origin_username_,
                                                                                    [pl] (auto result, auto username) {
                                                                                            pl->async_call(
                                                                                                    [pl, result, username] {
                                                                                                            if (!pl->check_state_check_login()) {
                                                                                                                    return;
                                                                                                            }
                                                                                                            event_check_login_done event(result, username + "_" + to_string(pl->server_id_));
                                                                                                            pl->sm_->on_event(&event);
                                                                                                    });
                                                                                    });
                                                        return state_check_login::instance();
                                                }
                                        } else if (auto *e = dynamic_cast<event_login_by_gid *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                auto msg = dbcache::gen_find_role_by_gid_req(e->gid_);
                                                dbcache::instance().send_to_dbcached(msg);
                                                return state_find_role::instance();
                                        } else if (auto *e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }
                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) override {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) override {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_INIT_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 5s;
                                }
                                friend ostream& operator<<(ostream& os, const state_init& s) {
                                        return os << "<state:init>";
                                }
                        };

                        class state_check_login : public state_base<player> {
                        public:
                                static state_check_login *instance() {
                                        static state_check_login inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_CHECK_LOGIN_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto *e = dynamic_cast<event_check_login_done *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;

                                                if (e->result_ == pd::OK) {
                                                        pl->username_ = e->username_;
                                                        ASSERT(pl->login_cb_);
                                                        pd::result result = pl->login_cb_(pl);
                                                        if (result != pd::OK) {
                                                                pl->login_cb_(pl);
                                                                pcs::base msg;
                                                                auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                                rsp->set_username(e->username_);
                                                                rsp->set_result(result);
                                                                pl->send_to_client(msg);
                                                                return state_quit::instance();
                                                        }
                                                        auto msg = dbcache::gen_find_role_req(e->username_, 0, 0);
                                                        dbcache::instance().send_to_dbcached(msg);
                                                        return state_find_role::instance();
                                                } else {
                                                        pcs::base msg;
                                                        auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                        rsp->set_username(pl->username_);
                                                        rsp->set_result(e->result_);
                                                        pl->send_to_client(msg);
                                                        return state_quit::instance();
                                                }
                                        } else if (auto *e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }

                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_CHECK_LOGIN_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 5s;
                                }
                                friend ostream& operator<<(ostream& os, const state_check_login& s) {
                                        return os << "<state:check_login>";
                                }
                        };

                        class state_find_role : public state_base<player> {
                        public:
                                static state_find_role *instance() {
                                        static state_find_role inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_FIND_ROLE_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto *e = dynamic_cast<event_find_role_done *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;

                                                if (e->result_ == pd::NOT_FOUND) {
                                                        pl->login_no_role();
                                                        return player::state_client_create_role::instance();
                                                } else {
                                                        ASSERT(e->result_ == pd::OK);
                                                        if (!pl->init_role(e->role_)) {
                                                                return player::state_quit::instance();
                                                        } else {
                                                                return player::state_fetch_data::instance();
                                                        }
                                                }
                                        } else if (auto *e = dynamic_cast<event_find_role_by_gid_done *>(event)) {
                                                if (e->result_ == pd::NOT_FOUND) {
                                                        return player::state_quit::instance();
                                                } else {
                                                        ASSERT(e->result_ == pd::OK);
                                                        pl->username_ = e->username_;
                                                        ASSERT(pl->init_role(e->role_));
                                                        return player::state_fetch_data::instance();
                                                }
                                        } else if (auto *e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }

                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_FIND_ROLE_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 5s;
                                }
                                friend ostream& operator<<(ostream& os, const state_find_role& s) {
                                        return os << "<state:find_role>";
                                }
                        };

                        class state_fetch_data : public state_base<player> {
                        public:
                                static state_fetch_data *instance() {
                                        static state_fetch_data inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_FETCH_DATA_PLAYER);
                                        pl->fetch_data();
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto *e = dynamic_cast<event_fetch_data_done *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                if (!pl->login_done()) {
                                                        return player::state_quit::instance();
                                                }
                                                return player::state_playing::instance();
                                        } else if (auto *e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }

                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_FETCH_DATA_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 5s;
                                }
                                friend ostream& operator<<(ostream& os, const state_fetch_data& s) {
                                        return os << "<state:fetch_data>";
                                }
                        };

                        class state_client_create_role : public state_base<player> {
                        public:
                                static state_client_create_role *instance() {
                                        static state_client_create_role inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_CLIENT_CREATE_ROLE_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto e = dynamic_cast<event_client_create_role *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_create_role::instance();
                                        } else if (auto e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }

                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_CLIENT_CREATE_ROLE_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 1h;
                                }
                                friend ostream& operator<<(ostream& os, const state_client_create_role& s) {
                                        return os << "<state::client_create_role>";
                                }
                        };

                        class state_create_role : public state_base<player> {
                        public:
                                static state_create_role *instance() {
                                        static state_create_role inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_CREATE_ROLE_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto e = dynamic_cast<event_create_role_done *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                ASSERT(e->result_ != pd::OK);
                                                pcs::base msg;
                                                auto rsp = msg.MutableExtension(pcs::s2c_create_role_rsp::scrr);
                                                rsp->set_result(e->result_);
                                                pl->send_to_client(msg);
                                                return player::state_client_create_role::instance();
                                        } else if (auto *e = dynamic_cast<event_find_role_done *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                if (e->result_ != pd::OK) {
                                                        return nullptr;
                                                }
                                                pl->init_role(e->role_);
                                                db_log::instance().log_login(*pl->role_, pl->ip_);
                                                pl->role_->on_event(ECE_ZHANLI_CHANGE, {});
                                                db_log::instance().log_create_role(e->role_, *pl->role_);

                                                auto opt_ptt = PTTS_GET_COPY(options, 1);
                                                if (!pl->mirror_role()) {
                                                        /*bi::instance().create_role(pl->username(),
                                                                                   pl->yci_,
                                                                                   pl->ip_,
                                                                                   pl->role_->gid(),
                                                                                   pl->role_->name());*/
                                                        const auto& mails = pl->role_->mails();
                                                        ASSERT(mails.size() == 1);
                                                        db_log::instance().log_send_mail(pl->role_->gid(), mails.begin()->first, opt_ptt.scene_info().server_id(),  mails.begin()->second);
                                                } else {
                                                        auto m = mail_new_mail(pl->role_->gid(), CREATE_ROLE_FIRST_MAIL_PTTID, pd::dynamic_data(), pd::event_array());
                                                        db_log::instance().log_send_mail(pl->role_->gid(), m.gid(), opt_ptt.scene_info().server_id(),  m);
                                                }
                                                return player::state_fetch_data::instance();
                                        } else if (auto e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit::instance();
                                        }
                                        return nullptr;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit::instance();
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_CREATE_ROLE_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 5s;
                                }
                                friend ostream& operator<<(ostream& os, const state_create_role& s) {
                                        return os << "<state:db_create_role>";
                                }
                        };

                        class state_playing : public state_base<player> {
                        public:
                                static state_playing *instance() {
                                        static state_playing inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_PLAYING_PLAYER);

                                        pl->start_playing_cb_(pl);

                                        auto ptt = PTTS_GET_COPY(options, 1);
                                        if (ptt.scene_info().weak_connection()) {
                                                pl->add_rand_quit_timer();
                                        }
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;

                                                if (!pl->stop_playing()) {
                                                        return nullptr;
                                                }
                                                return player::state_quit::instance();
                                        } else if (auto e = dynamic_cast<event_db_rsp *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                if (e->db_msg_->procedure() == "add_notice") {
                                                        const auto& params = e->db_msg_->params();
                                                        auto notice_gid = boost::any_cast<uint64_t>(params[0]);
                                                        auto role_gid = boost::any_cast<uint64_t>(params[1]);
                                                        ASSERT(pl->add_notice_cb_);
                                                        pl->add_notice_cb_(notice_gid, role_gid);
                                                } else if (e->db_msg_->procedure() == "find_notice" || e->db_msg_->procedure() == "get_notice") {
                                                        if (!e->db_msg_->results().empty()) {
                                                                auto now = duration_cast<hours>(system_clock::now().time_since_epoch());
                                                                pcs::base msg;
                                                                auto *ssn = msg.MutableExtension(pcs::s2c_sync_notices::ssn);
                                                                auto& results = e->db_msg_->results();
                                                                for (const auto& i : results) {
                                                                        auto gid = boost::any_cast<uint64_t>(i[0]);
                                                                        auto str = boost::any_cast<string>(i[2]);
                                                                        pd::notice notice;
                                                                        notice.ParseFromString(str);
                                                                        // todo number limit
                                                                        if (now - hours(notice.time()) > 28 * 24h) {
                                                                                pl->notice_delete(gid);
                                                                        } else {
                                                                                auto *cs_notice = ssn->add_notices();
                                                                                cs_notice->set_gid(gid);
                                                                                cs_notice->set_already_read(boost::any_cast<uint32_t>(i[1]) != 0);
                                                                                *cs_notice->mutable_data() = notice;
                                                                        }
                                                                }

                                                                if (ssn->notices_size() > 0) {
                                                                        pl->send_to_client(msg);
                                                                }
                                                        }
                                                        SPLAYER_DLOG << *this << " find notice done";
                                                } else if (e->db_msg_->procedure() == "notice_set_already_read") {
                                                        const auto& params = e->db_msg_->params();
                                                        auto gid = boost::any_cast<uint64_t>(params[0]);
                                                        pcs::base msg;
                                                        auto *rsp = msg.MutableExtension(pcs::s2c_notice_set_already_read_rsp::snsarr);
                                                        rsp->set_gid(gid);
                                                        pl->send_to_client(msg);
                                                }
                                        }
                                        return nullptr;
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " out";
                                        instance_counter::instance().dec(ICT_PLAYING_PLAYER);
                                        auto ptt = PTTS_GET_COPY(options, 1);
                                        if (ptt.scene_info().weak_connection()) {
                                                pl->remove_rand_quit_timer();
                                        }
                                }
#ifndef _DEBUG
                                system_clock::duration timeout() override {
                                        return 5min;
                                }
#endif
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";

                                        if (!pl->stop_playing()) {
                                                return nullptr;
                                        }
                                        return player::state_quit::instance();
                                }
                                friend ostream& operator<<(ostream& os, const state_playing& s) {
                                        return os << "<state:playing>";
                                }
                        };

                        class state_quit : public state_base<player> {
                        public:
                                static state_quit *instance() {
                                        static state_quit inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                        instance_counter::instance().add(ICT_QUIT_PLAYER);
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        if (auto *e = dynamic_cast<event_logout *>(event)) {
                                                SPLAYER_DLOG << *pl << " " << *this << " on " << *e;
                                                return player::state_quit_done::instance();
                                        }
                                        return nullptr;
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        pl->quit_timeout();
                                        instance_counter::instance().dec(ICT_QUIT_PLAYER);
                                }
                                system_clock::duration timeout() override {
                                        return 100ms;
                                }
                                state_base<player> *on_timeout(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " timeout";
                                        return player::state_quit_done::instance();
                                }
                                friend ostream& operator<<(ostream& os, const player::state_quit& s) {
                                        return os << "<state:quit>";
                                }
                        };

                        class state_quit_done : public state_base<player> {
                        public:
                                static state_quit_done *instance() {
                                        static state_quit_done inst;
                                        return &inst;
                                }
                                void on_in(const shared_ptr<player<T>>& pl) {
                                        SPLAYER_DLOG << *pl << " " << *this << " in";
                                }
                                state_base<player> *on_event(const shared_ptr<player<T>>& pl, event_base *event) {
                                        return nullptr;
                                }
                                void on_out(const shared_ptr<player<T>>& pl) {
                                        ASSERT(false);
                                }
                                friend ostream& operator<<(ostream& os, const player::state_quit_done& s) {
                                        return os << "<state:quit_done>";
                                }
                        };

#define NOTICE_COMMON(type, rolegid, self) auto db_msg = make_shared<db::message>("add_notice", \
                                                                                  db::message::req_type::rt_insert, \
                                                                                  [self] (const shared_ptr<db::message>& db_msg) { \
                                                                                          self->handle_finished_db_msg(db_msg); \
                                                                                  }); \
                        auto gid = gid::instance().new_gid(gid_type::NOTICE); \
                        db_msg->push_param(gid);                        \
                        db_msg->push_param(rolegid);                    \
                        db_msg->push_param(0);                          \
                        pd::notice notice;                              \
                        notice.set_type(type);                          \
                        notice.set_gid(gid);                            \
                        notice.set_time(system_clock::to_time_t(system_clock::now()));

                        void notice_offline_private_chat(uint64_t from_gid, uint32_t time, const string& content, const pd::voice& voice) {
                                async_call(
                                        [this, self = this->shared_from_this(), from_gid, time, content, voice] {
                                                ASSERT(role_);
                                                if (!role_->relation_in_blacklist(from_gid)) {
                                                        SPLAYER_DLOG << *this << " notice offline private chat " << role_->gid();
                                                        NOTICE_COMMON(pd::notice::OFFLINE_PRIVATE_CHAT, role_->gid(), self);
                                                        auto *opc = notice.mutable_offline_private_chat();
                                                        opc->set_from(from_gid);
                                                        opc->set_time(time);
                                                        opc->set_content(content);
                                                        *opc->mutable_voice() = voice;

                                                        string str;
                                                        notice.SerializeToString(&str);
                                                        db_msg->push_param(str);
                                                        gamedb_notice_->push_message(db_msg, st_);

                                                        SPLAYER_DLOG << *this << " add notice: " << db_msg;
                                                }
                                        });
                        }

                        void relation_notice_removed_by_friend(uint64_t to_gid) {
                                async_call(
                                        [this, self = this->shared_from_this(), to_gid] {
                                                SPLAYER_DLOG << *self << " notice " << to_gid << " removed by friend";
                                                NOTICE_COMMON(pd::notice::REMOVED_BY_FRIEND, to_gid, self);
                                                auto *rbf = notice.mutable_removed_by_friend();
                                                rbf->set_from(role_->gid());

                                                string str;
                                                notice.SerializeToString(&str);
                                                db_msg->push_param(str);
                                                self->gamedb_notice_->push_message(db_msg, self->st_);

                                                SPLAYER_DLOG << *self << " add notice: " << db_msg;
                                        });
                        }

                        void send_gift_offline_add_notice(uint64_t from, uint32_t gift, uint32_t count) {
                                auto self = this->shared_from_this();
                                NOTICE_COMMON(pd::notice::OFFLINE_SEND_GIFT, role_->gid(), self);
                                auto *osg = notice.mutable_offline_send_gift();
                                osg->set_from(from);
                                osg->set_gift(gift);
                                osg->set_count(count);

                                string str;
                                notice.SerializeToString(&str);
                                db_msg->push_param(str);
                                self->gamedb_notice_->push_message(db_msg, self->st_);

                                SPLAYER_DLOG << *self << " add notice: " << db_msg;
                        }

                        void offline_system_chat_add_notice(uint64_t spouse, uint32_t system_chat, const pd::dynamic_data& dd) {
                                auto self = this->shared_from_this();
                                NOTICE_COMMON(pd::notice::OFFLINE_SYSTEM_CHAT, role_->gid(), self);
                                auto *osc = notice.mutable_offline_system_chat();
                                osc->set_from(spouse);
                                osc->set_system_chat(system_chat);
                                *osc->mutable_data() = dd;

                                string str;
                                notice.SerializeToString(&str);
                                db_msg->push_param(str);
                                self->gamedb_notice_->push_message(db_msg, self->st_);

                                SPLAYER_DLOG << *self << " system chat add notice: " << db_msg;
                        }

                        void comment_by_other(uint64_t from, uint64_t reply_to, const string& content) {
                                async_call(
                                        [this, self = this->shared_from_this(), from, reply_to, content] {
                                                SPLAYER_DLOG << *this << " comment by " << from;

                                                ASSERT(check_state_playing());

                                                uint64_t gid;
                                                auto result = role_comment_by_other(from, reply_to, content, *role_, gid);
                                                ASSERT(comment_done_cb_);

                                                if (result == pd::OK) {
                                                        const auto& comment = role_->get_comment(gid);

                                                        if (from != role_->gid()) {
                                                                pcs::base msg;
                                                                auto *notice = msg.MutableExtension(pcs::s2c_role_comment_by_other::srcbo);
                                                                notice->set_role(from);
                                                                *notice->mutable_comment() = comment;
                                                                send_to_client(msg);
                                                        }

                                                        comment_done_cb_(from, result, role_->gid(), comment);
                                                } else {
                                                        comment_done_cb_(from, result, role_->gid(), pd::comment());
                                                }
                                        });
                        }

                        void comment_done(pd::result result, uint64_t to, const pd::comment& comment) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, to, comment] {
                                                ASSERT(check_state_playing());

                                                SPLAYER_DLOG << *this << " comment done " << pd::result_Name(result);
                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_role_comment_rsp::srcr);
                                                rsp->set_role(to);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_comment() = comment;
                                                }
                                                send_to_client(msg);
                                        });
                        }

                        void other_fetch_comments(uint64_t role) {
                                async_call(
                                        [this, self = this->shared_from_this(), role] {
                                                SPLAYER_DLOG << *this << " fetch comments by " << role;

                                                ASSERT(check_state_playing());

                                                pd::comment_array comments;
                                                role_->serialize_comments(&comments);
                                                comments.clear_gid_seed();
                                                ASSERT(fetch_comments_done_cb_);
                                                fetch_comments_done_cb_(role, role_->gid(), comments);
                                        });
                        }

                        void fetch_comments_done(uint64_t role, const pd::comment_array& comments) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, comments] {
                                                SPLAYER_DLOG << *this << " fetch comments done " << role;

                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_role_fetch_comments_rsp::srfcr);
                                                notice->set_role(role);
                                                notice->set_result(pd::OK);
                                                *notice->mutable_comments() = comments;
                                                send_to_client(msg);
                                        });
                        }

                        void delete_comment(uint64_t role, uint64_t gid) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, gid] {
                                                SPLAYER_DLOG << *this << " delete comment by " << role;

                                                ASSERT(check_state_playing());

                                                auto result = pd::OK;
                                                do {
                                                        if (!role_->has_comment(gid)) {
                                                                result = pd::ROLE_COMMENT_NO_SUCH_COMMENT;
                                                                break;
                                                        }
                                                        const auto& comment = role_->get_comment(gid);
                                                        if (role != role_->gid() && comment.role() != role) {
                                                                result = pd::ROLE_COMMENT_NEITHER_OWNER_NOR_WRITER;
                                                                break;
                                                        }

                                                        role_->delete_comment(gid);


                                                        if (role != role_->gid()) {
                                                                pcs::base msg;
                                                                auto *notice = msg.MutableExtension(pcs::s2c_role_other_deleted_comment::srodc);
                                                                notice->set_role(role);
                                                                notice->set_gid(gid);
                                                                send_to_client(msg);
                                                        }
                                                } while (false);

                                                ASSERT(delete_comment_done_cb_);
                                                delete_comment_done_cb_(role, result, role_->gid(), gid);
                                        });
                        }

                        void delete_comment_done(pd::result result, uint64_t role, uint64_t gid) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, role, gid] {
                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_role_delete_comment_rsp::srdcr);
                                                rsp->set_role(role);
                                                rsp->set_gid(gid);
                                                rsp->set_result(result);
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void upvote_by_other(uint64_t upvoter) {
                                async_call(
                                        [this, self = this->shared_from_this(), upvoter] {
                                                SPLAYER_DLOG << *this << " upvote by " << upvoter;

                                                ASSERT(check_state_playing());

                                                uint32_t present = 0;
                                                auto result = role_check_upvote_by_other(upvoter, *role_);
                                                if (result == pd::OK) {
                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_role_upvoted_by_other::srubo);
                                                        notice->set_role(upvoter);
                                                        *notice->mutable_event_res() = role_upvote_by_other(upvoter, *role_, present);
                                                        send_to_client(msg);

                                                }

                                                uint32_t present_count = 0;
                                                if (role_->has_present()) {
                                                        present_count = role_->present_count(role_->get_present());
                                                }

                                                ASSERT(upvote_done_cb_);
                                                upvote_done_cb_(upvoter, result, role_->gid(), present, present_count);
                                        });
                        }

                        void upvote_done(pd::result result, uint64_t upvotee, uint32_t present, uint32_t present_count) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, upvotee, present, present_count] {
                                                SPLAYER_DLOG << *this << " upvote done " << pd::result_Name(result);

                                                if (present > 0) {
                                                        ASSERT(check_state_playing());
                                                } else {
                                                        if (!check_state_playing()) {
                                                                return;
                                                        }
                                                }

                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_role_upvote_rsp::srur);
                                                rsp->set_result(result);
                                                rsp->set_role(upvotee);
                                                rsp->set_present_count(present_count);
                                                if (result == pd::OK && present > 0) {
                                                        const auto& ptt = PTTS_GET(role_present, present);
                                                        auto events = drop_process(PTTS_GET(drop, ptt.drop()));

                                                        pd::ce_env ce;
                                                        ce.set_origin(pd::CO_ROLE_UPVOTE_GET_PRESENT);
                                                        *rsp->mutable_event_res() = event_process(events, *role_, ce);
                                                        db_log::instance().log_upvote(upvotee, present_count, rsp->event_res(), *role_);
                                                }
                                                send_to_client(msg);

                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_ROLE_UPVOTE) {
                                                                can->mutable_cache()->set_status(result == pd::OK ? pd::OK : pd::FAILED);
                                                        }
                                                }
                                        });
                        }

                        void gongdou_by_other(uint64_t role, const pd::gongdou_data& data, const pd::event_res& event_res, uint64_t call_helper, int desc_idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, data, event_res, call_helper, desc_idx] {
                                                SPLAYER_DLOG << *this << " gongdou by other " << role << " " << pd::gongdou_type_Name(data.type());
						ASSERT(check_state_playing());

                                                auto result = gongdou_target_check(data, *role_);
                                                if (result != pd::OK) {
                                                        ASSERT(gongdou_done_cb_);
                                                        gongdou_done_cb_(role, role_->gid(), data.type(), result, pd::gongdou_effect(), event_res, call_helper, role_->league_gid());
                                                        return;
                                                }
                                                if (data.type() == pd::GT_SLANDER) {
                                                        pd::gongdou_base_data base_data;
                                                        base_data.set_from(role);
                                                        base_data.set_target(role_->gid());
                                                        base_data.set_type(data.type());
                                                        base_data.set_time(system_clock::to_time_t(system_clock::now()));
                                                        base_data.set_slander_idx(desc_idx);
                                                        gongdou_slander_vote_start_func_(base_data);
                                                        gongdou_done_cb_(role, role_->gid(), data.type(), pd::CONTINUE, pd::gongdou_effect(), event_res, call_helper, role_->league_gid());
                                                        return;
                                                }

                                                const auto& ptt = PTTS_GET(gongdou, data.type());
                                                auto last_minutes = ptt.last_minutes();
                                                if (role_->gongdou_has_effect(pd::GT_PRISON)) {
                                                        if (ptt.effect_type() == pd::GET_HARMFUL) {
                                                                last_minutes *= 2;
                                                        }
                                                }
                                                auto record = role_->gongdou_add_effect(role, data.type(), last_minutes * 60);
                                                role_->change_gongdou_by_other_times(data.type(), 1);

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_gongdou_by_other::sgbo);
                                                *notice->mutable_effect() = role_->gongdou_effect(data.type());
                                                *notice->mutable_record() = record;
                                                *notice->mutable_event_res() = gongdou_effect_event_process(data, *role_, event_res);

                                                if (ptt.has_gongdou_by_other_mail()) {
                                                        pd::dynamic_data mdd;
                                                        mdd.add_name(gid2rolename_func_(role));
                                                        send_mail(ptt.gongdou_by_other_mail(), mdd, pd::event_array());
                                                }
                                                if (ptt.has_system_chat()) {
                                                        pd::dynamic_data dd;
                                                        dd.add_name(gid2rolename_func_(role_->gid()));
                                                        dd.add_name(gid2rolename_func_(data.other()));
                                                        ASSERT(send_system_chat_func_);
                                                        send_system_chat_func_(ptt.system_chat(), dd);
                                                }
                                                send_to_client(msg);

                                                ASSERT(gongdou_done_cb_);
                                                gongdou_done_cb_(role, role_->gid(), data.type(), pd::OK, role_->gongdou_effect(data.type()), event_res, call_helper, role_->league_gid());
                                        });
                        }

                        void gongdou_slander_vote_done(uint64_t from, uint64_t target, pd::gongdou_type type, pd::result result) {
                                async_call(
                                        [this, self = this->shared_from_this(), from, target, type, result] {
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_gongdou_vote_rsp::sgvr);
                                                notice->set_result(result);
                                                notice->set_type(type);
                                                notice->set_from(from);
                                                notice->set_target(target);
                                                send_to_client(msg);
                                        });
                        }

                        void gongdou_slander_stat_finish_done(const pd::gongdou_base_data& base_data, pd::result result) {
                                async_call(
                                        [this, self = this->shared_from_this(), base_data, result] {
                                                SPLAYER_DLOG << *this << " gongdou slander vote finish done " << pd::result_Name(result);
                                                ASSERT(check_state_playing());

                                                if (result == pd::OK) {
                                                        const auto& ptt = PTTS_GET(gongdou, base_data.type());
                                                        auto last_minutes = ptt.last_minutes();
                                                        if (role_->gongdou_has_effect(pd::GT_PRISON)) {
                                                                if (ptt.effect_type() == pd::GET_HARMFUL) {
                                                                        last_minutes *= 2;
                                                                }
                                                        }
                                                        auto record = role_->gongdou_add_effect(base_data.from(), base_data.type(), last_minutes * 60, base_data.slander_idx());
                                                        role_->change_gongdou_by_other_times(base_data.type(), 1);

                                                        pd::gongdou_data data;
                                                        data.set_type(base_data.type());
                                                        data.set_other(base_data.from());
                                                        data.set_slander_idx(base_data.slander_idx());

                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_gongdou_by_other::sgbo);
                                                        *notice->mutable_effect() = role_->gongdou_effect(base_data.type());
                                                        *notice->mutable_record() = record;
                                                        *notice->mutable_event_res() = gongdou_effect_event_process(data, *role_, pd::event_res());
                                                        send_to_client(msg);

                                                        if (ptt.has_gongdou_by_other_mail()) {
                                                                pd::dynamic_data mdd;
                                                                mdd.add_name(gid2rolename_func_(base_data.from()));
                                                                send_mail(ptt.gongdou_by_other_mail(), mdd, pd::event_array());
                                                        }

                                                        ASSERT(base_data.slander_idx() < ptt.slander_size());
                                                        pd::dynamic_data dd;
                                                        dd.add_name(gid2rolename_func_(base_data.target()));
                                                        dd.add_title(ptt.slander(base_data.slander_idx()).title());
                                                        ASSERT(send_system_chat_func_);
                                                        send_system_chat_func_(ptt.slander(base_data.slander_idx()).system_chat(), dd);

                                                } else {
                                                        role_->gongdou_slander_fail(base_data.from(), base_data.target(), result, false);
                                                }
                                        });
                        }

                        void gongdou_slander_finish_itself_effect_done(const pd::gongdou_base_data& base_data, pd::result result, bool has_debuff) {
                                async_call(
                                        [this, self = this->shared_from_this(), base_data, result, has_debuff] {
                                                SPLAYER_DLOG << *this << " gongdou slander itself effect, has_debuff = " << has_debuff;
                                                ASSERT(check_state_playing());

                                                auto record = role_->gongdou_slander_fail(base_data.from(), base_data.target(), result, has_debuff);
                                                if (has_debuff) {
                                                        const auto& ptt = PTTS_GET(gongdou, pd::GT_SLANDER);
                                                        pd::ce_env ce;
                                                        ce.set_origin(pd::CO_GONGDOU_BY_OTHER);

                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_gongdou_by_other::sgbo);
                                                        *notice->mutable_effect() = role_->gongdou_effect(base_data.type());
                                                        *notice->mutable_record() = record;
                                                        *notice->mutable_event_res() = event_process(ptt.slander_fail_events().events(), *role_, ce);
                                                        send_to_client(msg);

                                                        pd::dynamic_data dd;
                                                        dd.add_name(gid2rolename_func_(base_data.from()));
                                                        dd.add_name(gid2rolename_func_(base_data.target()));
                                                        ASSERT(send_system_chat_func_);
                                                        send_system_chat_func_(ptt.slander_fail_events().system_chat(), dd);
                                                }
                                        });
                        }

                        void gongdou_done(uint64_t role, pd::gongdou_type type, pd::result result, const pd::gongdou_effect& effect, const pd::event_res& event_res, uint64_t call_helper, uint64_t to_league = 0) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, type, result, effect, event_res, call_helper, to_league] {
                                                SPLAYER_DLOG << *this << " gongdou done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_gongdou_rsp::sgr);
                                                rsp->set_type(type);
                                                rsp->set_target(role);
                                                rsp->set_result(result);
                                                const auto& ptt = PTTS_GET(gongdou, type);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_effect() = effect;
                                                        *rsp->mutable_event_res() = event_res;
                                                        role_->change_gongdou_times(type, role);
                                                        role_->on_event(ECE_ROLE_GONGDOU, { to_league, role_->league_gid(), ptt.effect_type() });
                                                        db_log::instance().log_gongdou(type, role_->gid(), result, role, event_res, *role_);

                                                        if (call_helper != 0) {
                                                                league_gongdou_helped_by_other_func_(role_->gid(), type, role, call_helper);
                                                        }

                                                } else if (result == pd::GONGDOU_NOT_HITTED || result == pd::CONTINUE) {
                                                        *rsp->mutable_event_res() = event_res;
                                                        role_->change_gongdou_times(type, role);
                                                        role_->on_event(ECE_ROLE_GONGDOU, { to_league, role_->league_gid(),ptt.effect_type() });
                                                        db_log::instance().log_gongdou(type, role_->gid(), result, role, event_res, *role_);
                                                } else {
                                                        event_revert(event_res, *role_, pd::CO_GONGDOU_REVERT);
                                                }
                                                send_to_client(rsp_msg);

                                                if (result == pd::OK) {
                                                        role_->try_levelup();
                                                }

                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_GONGDOU) {
                                                                can->mutable_cache()->set_status(result == pd::OK ? pd::OK : pd::FAILED);
                                                        }
                                                }

                                        });
                        }

                        void gongdou_add_spouse_record(uint64_t role, pd::gongdou_type type, const pd::gongdou_record& record) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, type, record] {
                                                SPLAYER_DLOG << *this << " gongdou add spouse record " << role;

                                                ASSERT(check_state_playing());

                                                if (!role_->marriage_married() || role_->marriage_spouse() != role) {
                                                        return;
                                                }

                                                role_->gongdou_add_spouse_record(type, record);

                                        });
                        }

                        void marriage_update_spouse_vip_exp(uint64_t role, uint64_t vip_exp) {
                                async_call(
                                        [this, self = this->shared_from_this(), role, vip_exp] {
                                                SPLAYER_DLOG << *this << " marriage update spouse vip exp " << role;

                                                ASSERT(check_state_playing());

                                                if (!role_->marriage_married() || role_->marriage_spouse() != role) {
                                                        return;
                                                }

                                                role_->marriage_set_spouse_vip_exp(vip_exp);
                                        });
                        }

                        void marriage_refresh_questions(uint64_t spouse) {
                                async_call(
                                        [this, self = this->shared_from_this(), spouse] {
                                                SPLAYER_DLOG << *this << " marriage refresh questions " << spouse;

                                                ASSERT(check_state_playing());

                                                if (!role_->marriage_married() || role_->marriage_spouse() != spouse) {
                                                        return;
                                                }

                                                role_->marriage_refresh_questions(true);

                                        });
                        }

                        void marriage_spouse_sync_questions(uint64_t spouse, const vector<pd::marriage_question>& questions) {
                                async_call(
                                        [this, self = this->shared_from_this(), spouse, questions] {
                                                SPLAYER_DLOG << *this << " marriage spouse sync questions " << spouse;

                                                if (!role_->marriage_married() || role_->marriage_spouse() != spouse) {
                                                        return;
                                                }


                                                role_->marriage_spouse_sync_questions(questions);

                                        });
                        }

                        void get_ranking_list_done(uint32_t self_rank, const vector<pd::rank_entity>& entities, uint32_t count, pd::rank_type type) {
                                async_call(
                                        [this, self = this->shared_from_this(), self_rank, entities, count, type] {
                                                SPLAYER_DLOG << *this << " get ranking list done";
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_ranking_list_rsp::srlr);
                                                rsp->set_self_rank(self_rank);
                                                auto need_count = count < 50 ? count : 50;
                                                auto index = entities.size() < need_count ? entities.size() : need_count;
                                                for (uint i = 0; i < index; i++) {
                                                        *rsp->add_entities() = entities[i];
                                                }
                                                rsp->set_type(type);
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void get_ranking_self_rank_done(const pcs::ranking_self_ranks& ranks) {
                                async_call(
                                        [this, self = this->shared_from_this(), ranks] {
                                                SPLAYER_DLOG << *this << " get ranking self rank done";
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_ranking_get_self_rank_rsp::srgsrr);
                                                *rsp->mutable_ranks() = ranks;

                                                send_to_client(rsp_msg);
                                        });
                        }

                        void rank_give_role_a_like_done(pd::result result, uint64_t gid, pd::rank_type type, int like_count, const pd::event_res& event_res) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gid, type, like_count, event_res] {
                                                SPLAYER_DLOG << *this << " rank give role a like done";
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_rank_give_role_a_like_rsp::srgralr);
                                                rsp->set_result(result);
                                                rsp->set_role(gid);
                                                rsp->set_type(type);
                                                if (result == pd::OK) {
                                                        role_->on_event(ECE_GIVE_ROLE_A_LIKE, {});
                                                        rsp->set_like_count(like_count);
                                                        *rsp->mutable_event_res() = event_res;
                                                } else {
                                                        role_->rank_like_delete_role(gid, type);
                                                        event_revert(event_res, *role_, pd::CO_RANK_GIVE_A_LIKE_FILED);
                                                }
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void replay_battle_conflict_record_done(pd::result result, uint64_t gid, const pd::conflict_battle_record& record) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gid, record] {
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_replay_battle_conflict_record_rsp::srbcrr);

                                                if (result == pd::OK) {
                                                        auto b = make_unique<battle>();
                                                        auto result = b->init(record.pttid(), record.team_a(), record.team_b(), record.context());
                                                        if (result == pd::CONTINUE) {
                                                                if (record.battle_info().skip_battle()) {
                                                                        result = b->process_role_inputs(record.role(), record.battle_info().inputs(), true);
                                                                } else{
                                                                        result = b->process_role_inputs(record.role(), record.battle_info().inputs(), false);
                                                                }
                                                                b->check_battle_info(record.battle_info(), result);
                                                        }
                                                        *rsp->mutable_record() = record;
                                                }

                                                rsp->set_result(result);
                                                rsp->set_gid(gid);
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void guanpin_promote_done(pd::result result, int gpin, const pd::battle_damage_info& damage_info,
                                                  const pd::event_res& event_res, int idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, damage_info, event_res, idx] {
                                                SPLAYER_DLOG << *this << " guanpin promote done " << pd::result_Name(result);
                                                ASSERT(check_state_playing());

                                                if (result != pd::OK && result != pd::BATTLE_LOST) {
                                                        event_revert(event_res, *role_, pd::CO_GUANPIN_PROMOTE_REVERT);
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_promote_rsp::sgpr);
                                                rsp->set_gpin(gpin);
                                                rsp->set_result(result);
                                                rsp->set_battle_seed(role_->guanpin_battle_seed());
                                                pd::event_res events = event_res;
                                                if (result == pd::OK) {
                                                        if (role_->guanpin_gpin() != gpin && role_->guanpin_idx() != idx) {
                                                                const auto& ptt = PTTS_GET(guanpin, gpin);
                                                                pd::ce_env ce;
                                                                ce.set_origin(pd::CO_GUANPIN_PROMOTE);
                                                                *rsp->mutable_renzhi_event_res() = event_process(ptt.renzhi_events(), *role_, ce);
                                                        }

                                                        role_->guanpin_set(gpin, idx);
                                                        rsp->set_cur_gpin(gpin);
                                                        rsp->set_cur_idx(idx);
                                                        *rsp->mutable_damage_info() = damage_info;
                                                        *rsp->mutable_event_res() = event_res;

                                                        db_log::instance().log_guanpin_promote(gpin, idx, rsp->event_res(), *role_);
                                                }


                                                send_to_client(rsp_msg);

                                                if (!mirror_role()) {
                                                        pd::role_mirror_event event;
                                                        event.set_gid(gid());
                                                        event.set_challenge_gpin(gpin);
                                                        event.set_type(pd::RMET_GUANPIN_PROMOTE);
                                                        send_mirror_role_event_msg(event);
                                                        if (result == pd::OK) {
                                                                //bi::instance().guanpin_promote(username(), yci_, ip_, role_->gid(), role_->league_gid(), role_->level(), event_res, pd::CO_GUANPIN_PROMOTE);
                                                        }
                                                }

                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_GUANPIN_PROMOTE) {
                                                                can->mutable_cache()->set_status(pd::OK);
                                                        }
                                                }
                                        });
                        }

                        void guanpin_fetch_data_done(int gpin, int idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), gpin, idx] {
                                                SPLAYER_DLOG << *this << " guanpin fetch data done " << gpin << " " << idx;

                                                if (check_state_playing()) {
                                                        role_->guanpin_set(gpin, idx);


                                                        pcs::base msg;
                                                        auto *notice = msg.MutableExtension(pcs::s2c_guanpin_fetch_data_rsp::sgfdr);
                                                        notice->set_gpin(gpin);
                                                        notice->set_idx(idx);
                                                        send_to_client(msg);
                                                } else if (check_state_fetch_data()) {
                                                        role_->guanpin_set(gpin, idx);

                                                        fetch_one_data_done();
                                                }
                                        });

                        }

                        void guanpin_get_guans_done(pd::result result, uint32_t gpin, uint32_t page, uint32_t page_size, const list<pd::guan>& guans, uint32_t gpin_guan_count) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, page, page_size, guans, gpin_guan_count] {
                                                SPLAYER_DLOG << *this << " guanpin get guan done " << pd::result_Name(result) << " " << gpin << " " << page;
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_get_guans_rsp::sgggr);
                                                rsp->set_gpin(gpin);
                                                rsp->set_page(page);
                                                if (page_size != rsp->page_size()) {
                                                        rsp->set_page_size(page_size);
                                                }
                                                rsp->set_gpin_guan_count(gpin_guan_count);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        for (const auto& i : guans) {
                                                                *rsp->mutable_guans()->add_guans() = i;
                                                        }
                                                }
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void guanpin_collect_fenglu_done(pd::result result, const pd::event_array& events, uint32_t fenglu_factor) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, events, fenglu_factor] {
                                                SPLAYER_DLOG << *this << " guanpin collect fenglu done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_collect_fenglu_rsp::sgcfr);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        pd::ce_env ce;
                                                        ce.set_origin(pd::CO_GUANPIN_FENGLU);
                                                        ce.set_add_percent(fenglu_factor);
                                                        *rsp->mutable_event_res() = event_process(events, *role_, ce);
                                                        role_->on_event(ECE_GUANPIN_COLLECT_FENGLU, {});
                                                        role_->guanpin_set_collect_fenglu_day();

                                                        db_log::instance().log_guanpin_collect_fenglu(role_->guanpin_gpin(), role_->guanpin_idx(), rsp->event_res(), *role_);
                                                }

                                                send_to_client(rsp_msg);

                                        });
                        }

                        void guanpin_challenge_done(pd::result result, int target_gpin, int target_idx, uint64_t target, uint32_t battle_seed, const pd::battle_team& target_team, const pd::battle_team& self_team, const pd::battle_damage_info& damage_info, int gpin, int idx, const pd::event_res& event_res) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, target_gpin, target_idx, target, battle_seed, target_team, self_team, damage_info, gpin, idx, event_res = event_res] () mutable {
                                                ASSERT(check_state_playing());
                                                SPLAYER_DLOG << *this << " guanpin challenge done " << pd::result_Name(result);

                                                if (result != pd::OK && result != pd::BATTLE_LOST) {
                                                        event_revert(event_res, *role_, pd::CO_GUANPIN_CHALLENGE_REVERT);
                                                } else if (result == pd::OK){
                                                        role_->guanpin_set(gpin, idx);
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_challenge_rsp::sgcr);
                                                rsp->set_target_gpin(target_gpin);
                                                rsp->set_target_idx(target_idx);
                                                rsp->set_target(target);
                                                rsp->set_result(result);
                                                rsp->set_gpin(gpin);
                                                rsp->set_idx(idx);
                                                rsp->set_battle_seed(battle_seed);
                                                *rsp->mutable_target_team() = target_team;
                                                *rsp->mutable_damage_info() = damage_info;
                                                *rsp->mutable_self_team() = self_team;

                                                pd::ce_env ce;
                                                ce.set_origin(pd::CO_GUANPIN_CHALLENGE);
                                                const auto& ptt = PTTS_GET(guanpin, role_->guanpin_gpin());
                                                if (result == pd::OK) {
                                                        *rsp->mutable_event_res() = event_process(ptt.challenge().win_events(), *role_, ce, &event_res);
                                                } else {
                                                        *rsp->mutable_event_res() = event_process(ptt.challenge().lose_events(), *role_, ce, &event_res);
                                                }

                                                db_log::instance().log_guanpin_challenge(target_gpin, target_idx, rsp->event_res(), *role_);

                                                role_->on_event(ECE_GUANPIN_CHALLENGE, {});

                                                send_to_client(rsp_msg);


                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_GUANPIN_CHAOTANG_CHANLLENGE) {
                                                                can->mutable_cache()->set_status(pd::OK);
                                                        }
                                                }
                                        });
                        }

                        void guanpin_challenged_by_other(pd::result result, int gpin, int idx, const pd::guanpin_challenge_record& challenge_record, uint64_t from) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, idx, challenge_record, from] {
                                                SPLAYER_DLOG << *this << " guanpin challenged by other " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_guanpin_challenged_by_other::sgcbo);

                                                if (role_->guanpin_gpin() != gpin) {
                                                        role_->guanpin_changed_record(from, pd::RT_GUANPIN_CHANGED_DEMOTE, role_->guanpin_gpin(), gpin);
                                                }

                                                role_->guanpin_set(gpin, idx);
                                                notice->set_gpin(gpin);
                                                notice->set_idx(idx);
                                                *notice->mutable_challenge_record() = challenge_record;
                                                send_to_client(msg);

                                        });
                        }

                        void guanpin_dianshi_baoming_done(pd::result result, int gpin, const pd::event_res& event_res) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, event_res] {
                                                SPLAYER_DLOG << *this << " guanpin dianshi baoming done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                if (result != pd::OK) {
                                                        event_revert(event_res, *role_, pd::CO_GUANPIN_DIANSHI_BAOMING_REVERT);
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_dianshi_baoming_rsp::sgdbr);
                                                rsp->set_gpin(gpin);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_event_res() = event_res;
                                                        db_log::instance().log_guanpin_baoming(gpin, rsp->event_res(), *role_);
                                                }
                                                send_to_client(rsp_msg);


                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_GUANPIN_DIANSHI_BAOMING) {
                                                                can->mutable_cache()->set_status(result == pd::OK ? pd::OK : pd::FAILED);
                                                        }
                                                }

                                                if (!mirror_role()) {
                                                        pd::role_mirror_event event;
                                                        event.set_gid(gid());
                                                        event.set_challenge_gpin(gpin);
                                                        event.set_type(pd::RMET_GUANPIN_DIANSHI_BAOMING);
                                                        send_mirror_role_event_msg(event);
                                                }
                                        });
                        }

                        void guanpin_dianshi_demote(int gpin, int idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), gpin, idx] {
                                                SPLAYER_DLOG << *this << " guanpin dianshi demote " << gpin << " " << idx;
                                                ASSERT(check_state_playing());

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_guanpin_dianshi_demote::sgdd);

                                                if (role_->guanpin_gpin() != gpin && role_->guanpin_idx() != idx) {
                                                        const auto& ptt = PTTS_GET(guanpin, gpin);
                                                        pd::ce_env ce;
                                                        ce.set_origin(pd::CO_GUANPIN_DEMOTE);
                                                        *notice->mutable_event_res() = event_process(ptt.renzhi_events(), *role_, ce);

                                                        role_->guanpin_changed_record(role_->gid(), pd::RT_GUANPIN_CHANGED_DEMOTE, role_->guanpin_gpin(), gpin);
                                                }

                                                role_->guanpin_set(gpin, idx);

                                                notice->set_gpin(gpin);
                                                notice->set_idx(idx);

                                                db_log::instance().log_guanpin_dianshi_demote(gpin, idx, *role_);

                                                send_to_client(msg);

                                        });
                        }

                        void guanpin_dianshi_promote(int gpin, int idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), gpin, idx] {
                                                SPLAYER_DLOG << *this << " guanpin dianshi promote " << gpin << " " << idx;
                                                ASSERT(check_state_playing());

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_guanpin_dianshi_promote::sgdp);
                                                if (role_->guanpin_gpin() != gpin && role_->guanpin_idx() != idx) {
                                                        const auto& ptt = PTTS_GET(guanpin, gpin);
                                                        pd::ce_env ce;
                                                        ce.set_origin(pd::CO_GUANPIN_PROMOTE);
                                                        *notice->mutable_event_res() = event_process(ptt.renzhi_events(), *role_, ce);

                                                        role_->guanpin_changed_record(role_->gid(), pd::RT_GUANPIN_CHANGED_PROMOTE, role_->guanpin_gpin(), gpin);
                                                }

                                                role_->guanpin_set(gpin, idx);

                                                notice->set_gpin(gpin);
                                                notice->set_idx(idx);
                                                send_to_client(msg);

                                        });
                        }

                        void guanpin_gongdou_promote(int gpin, int idx, pd::result result, const pd::gongdou_data& gongdou_data, const pd::event_res& event_res) {
                                async_call(
                                        [this, self = this->shared_from_this(), gpin, idx, result, gongdou_data, event_res] {
                                                SPLAYER_DLOG << *this << " guanpin gongdou promote " << gpin << " " << idx;
                                                ASSERT(check_state_playing());

                                                if (result != pd::OK) {
                                                        if (role_->gid() == gongdou_data.other()) {
                                                                event_revert(event_res, *role_, pd::CO_GONGDOU_REVERT);
                                                        } else {
                                                                role_->change_gongdou_by_other_times(gongdou_data.type(), -1);
                                                        }
                                                }

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_guanpin_gongdou_promote::sggp);

                                                if (result == pd::OK) {
                                                        if (role_->guanpin_gpin() != gpin && role_->guanpin_idx() != idx) {
                                                                const auto& ptt = PTTS_GET(guanpin, gpin);
                                                                pd::ce_env ce;
                                                                ce.set_origin(pd::CO_GUANPIN_PROMOTE);
                                                                *notice->mutable_event_res() = event_process(ptt.renzhi_events(), *role_, ce);
                                                        }
                                                        role_->guanpin_set(gpin, idx);
                                                        notice->set_gpin(gpin);
                                                        notice->set_idx(idx);
                                                        send_to_client(msg);

                                                }
                                        });
                        }

                        void guanpin_dianshi_info_done(pd::result result, int gpin, const pd::guanpin_dianshi_gpin& data) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, data] {
                                                SPLAYER_DLOG << *this << " guanpin info done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_dianshi_info_rsp::sgdir);
                                                rsp->set_result(result);
                                                rsp->set_gpin(gpin);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_data() = data;
                                                }
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void guanpin_dianshi_replay_done(pd::result result, int gpin, int record_idx, const pd::guanpin_dianshi_record& data) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, record_idx, data] {
                                                SPLAYER_DLOG << *this << " guanpin dianshi replay done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_guanpin_dianshi_replay_rsp::sgdrr);
                                                rsp->set_result(result);
                                                rsp->set_gpin(gpin);
                                                rsp->set_record_idx(record_idx);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_record() = data;
                                                }
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void guanpin_fetch_baoming_info_done(const pd::guanpin_baoming_infos& baomings) {
                                async_call(
                                        [this, self = this->shared_from_this(), baomings] {
                                                SPLAYER_DLOG << *this << " guanpin fetch baoming info done";
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_fetch_gpin_baoming_info_rsp::sfgbir);
                                                *rsp->mutable_baomings() = baomings;
                                                send_to_client(rsp_msg);
                                        });
                        }

                        void guanpin_set_gpin_done(pd::result result, int gpin, int idx) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, gpin, idx] {
                                                SPLAYER_DLOG << *this << " guanpin set gpin done " << gpin << " " << idx;
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                if (result != pd::OK) {
                                                        return;
                                                }

                                                role_->guanpin_set(gpin, idx);

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_guanpin_set_gpin::sgsg);
                                                notice->set_gpin(gpin);
                                                notice->set_idx(idx);
                                                send_to_client(msg);
                                        });
                        }

                        void marriage_spouse_login(uint64_t spouse) {
                                async_call(
                                        [this, self = this->shared_from_this(), spouse] {
                                                SPLAYER_DLOG << *this << " marriage spouse login " << spouse;
                                                ASSERT(check_state_playing());

                                                if (!role_->marriage_married() || role_->marriage_spouse() != spouse) {
                                                        return;
                                                }

                                                role_->marriage_spouse_login();

                                                pcs::base msg;
                                                msg.MutableExtension(pcs::s2c_marriage_spouse_login::smsl);
                                                send_to_client(msg);
                                        });
                        }

                        void marriage_spouse_logout(uint64_t spouse) {
                                async_call(
                                        [this, self = this->shared_from_this(), spouse] {
                                                SPLAYER_DLOG << *this << " marriage spouse logout " << spouse;
                                                ASSERT(check_state_playing());

                                                if (!role_->marriage_married() || role_->marriage_spouse() != spouse) {
                                                        return;
                                                }

                                                role_->marriage_spouse_logout();

                                                pcs::base msg;
                                                msg.MutableExtension(pcs::s2c_marriage_spouse_logout::smsl);
                                                send_to_client(msg);
                                        });
                        }

                        void marriage_fetch_data_done(bool star_wishing, bool pking, uint32_t qiu_qian_actor, uint32_t pregnent_time) {
                                async_call(
                                        [this, self = this->shared_from_this(), star_wishing, pking, qiu_qian_actor, pregnent_time] {
                                                SPLAYER_DLOG << *this << " marriage fetch data done " << star_wishing << " " << pking << " " << qiu_qian_actor << " " << pregnent_time;
                                                if (!check_state_fetch_data()) {
                                                        return;
                                                }

                                                role_->marriage_set_star_wishing(star_wishing);
                                                role_->marriage_set_pking(pking);
                                                role_->marriage_set_qiu_qian_actor(qiu_qian_actor);
                                                role_->marriage_set_pregnent_time(pregnent_time);

                                                this->fetch_one_data_done();
                                        });
                        }

                        void marriage_start_star_wish_done(pd::result result, const string& declaration, const pd::voice& voice, const pd::event_res& event_res, uint32_t time) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, declaration, voice, event_res, time] {
                                                SPLAYER_DLOG << *this << " marriage start star wish done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                if (result != pd::OK) {
                                                        event_revert(event_res, *role_, pd::CO_MARRIAGE_START_STAR_WISH_REVERT);
                                                        if (result != pd::MARRIAGE_ALREADY_START_STAR_WISH) {
                                                                role_->marriage_set_star_wishing(false);
                                                        }
                                                } else {
                                                        auto ptt = PTTS_GET(marriage_logic, 1);
                                                        pd::dynamic_data dd;
                                                        dd.add_name(gid2rolename_func_(role_->gid()));
                                                        ASSERT(send_system_chat_func_);
                                                        send_system_chat_func_(ptt.star_wish_start_system_chat(), dd);
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_marriage_start_star_wish_rsp::smsswr);
                                                rsp->set_result(result);
                                                rsp->set_declaration(declaration);
                                                *rsp->mutable_voice() = voice;
                                                rsp->set_time(time);
                                                *rsp->mutable_event_res() = event_res;
                                                send_to_client(rsp_msg);
                                                db_log::instance().log_marriage_start_star_wish(declaration, event_res, *role_);


                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_MARRIAGE_START_STAR_WISH) {
                                                                can->mutable_cache()->set_status(result);
                                                                ai_.marriage_.star_wish_start_time_ = time;
                                                        }
                                                }
                                        });
                        }

                        void marriage_star_wish_list_done(pd::result result, const pd::marriage_star_wish_array& data) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, data] {
                                                SPLAYER_DLOG << *this << " marriage star wish list done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                for (const auto& i : fetch_marriage_star_wish_list_call_funcs_) {
                                                        i(result, data);
                                                }
                                                fetch_marriage_star_wish_list_call_funcs_.clear();
                                        });
                        }

                        void marriage_star_wish_send_gift_done(pd::result result, uint64_t target, uint32_t gift, uint32_t count, const pd::event_res& event_res, int star_wish, bool suiting) {
                                async_call(
                                        [this, self = this->shared_from_this(), result, target, count, gift, event_res, star_wish, suiting] {
                                                SPLAYER_DLOG << *this << " marriage star wish send gift done " << pd::result_Name(result);
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                if (result != pd::OK) {
                                                        event_revert(event_res, *role_, pd::CO_MARRIAGE_STAR_WISH_SEND_GIFT_REVERT);
                                                        if (!suiting) {
                                                                role_->marriage_set_star_wishing(false);
                                                        }
                                                }

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_marriage_star_wish_send_gift_rsp::smswsgr);
                                                rsp->set_role(target);
                                                rsp->set_gift(gift);
                                                rsp->set_count(count);
                                                rsp->set_result(result);
                                                if (result == pd::OK) {
                                                        *rsp->mutable_event_res() = event_res;
                                                        rsp->set_star_wish(star_wish);
                                                        db_log::instance().log_marriage_star_wish_send_gift(event_res, *role_);
                                                }
                                                send_to_client(rsp_msg);


                                                auto *can = bt_->cur_action_node();
                                                if (can) {
                                                        const auto& ptt = PTTS_GET(behavior_tree, can->pttid());
                                                        if (ptt.has_player_action() && ptt.player_action().type() == pd::PAT_MARRIAGE_STAR_WISH_SEND_GIFT) {
                                                                can->mutable_cache()->set_status(result == pd::OK ? pd::OK : pd::FAILED);
                                                                if (ai_.marriage_.star_wish_target_ == 0) {
                                                                        ai_.marriage_.star_wish_target_ = target;
                                                                }
                                                        }
                                                }
                                        });
                        }

                        void marriage_star_wish_receive_gift(uint64_t from, uint32_t gift, uint32_t count, int star_wish) {
                                async_call(
                                        [this, self = this->shared_from_this(), from, gift, count, star_wish] {
                                                SPLAYER_DLOG << *this << " marriage star wish receive gift";
                                                if (!check_state_playing()) {
                                                        return;
                                                }

                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_marriage_star_wish_receive_gift::smswrg);
                                                notice->set_role(from);
                                                notice->set_gift(gift);
                                                notice->set_count(count);
                                                notice->set_star_wish(star_wish);
                                                send_to_client(msg);

                                        });
                        }

                    
                        /*void role_mirror_event_mansion_leave_banquet() {
                                async_call(
                                        [this, self = this->shared_from_this()] {
                                                ASSERT(check_state_playing());

                                                ASSERT(mansion_leave_banquet_func_);
                                                mansion_leave_banquet_func_(role_->gid());
                                        });
                        }*/

                      
/*
                        pd::result ai_action_activity_qiandao(pd::behavior_tree_node& node) {
                                auto cur_month = refresh_month();
                                auto cur_mday = refresh_mday();
                                auto result = activity_check_qiandao(cur_month, cur_mday, *role_);
                                if (result != pd::OK) {
                                        return pd::FAILED;
                                }

                                pd::ce_env ce;
                                ce.set_origin(pd::CO_ACTIVITY_QIANDAO_REWARD);
                                const auto& qiandao_ptt = PTTS_GET(activity_qiandao, cur_month);
                                ASSERT(qiandao_ptt.rewards_size() > cur_mday - 1);
                                ASSERT(condevent_check(qiandao_ptt.rewards(cur_mday - 1).qiandao_events(), *role_, ce) == pd::OK);
                                condevent_process(qiandao_ptt.rewards(cur_mday - 1).qiandao_events(), *role_, ce);
                                role_->activity_add_qiandao(cur_month, cur_mday);
                                return pd::OK;
                        }
                        pd::result ai_action_activity_recharge(pd::behavior_tree_node& node) {
                                const auto& ptts = PTTS_GET_ALL(recharge);
                                auto iter = ptts.begin();
                                advance(iter, rand() % ptts.size());
                                auto recharge_pttid = iter->second.id();

                                const auto& ptt = PTTS_GET(recharge, recharge_pttid);
                                auto result = activity_check_recharge(recharge_pttid, *role_);
                                if (result != pd::OK) {
                                        return pd::FAILED;
                                }
                                auto order = role_->add_recharge_order(recharge_pttid);
                                activity_recharge(recharge_pttid, *role_);

                                auto opt_ptt = PTTS_GET_COPY(options, 1);
                                yunying_recharge(0, 0, order, "", "RMB", ptt.price(), system_clock::to_time_t(system_clock::now()), "", true);

                                return pd::OK;
                        }
                        pd::result ai_acition_activity_yueka_reward(pd::behavior_tree_node& node) {
                                set<uint32_t> yueka_set;
                                for (const auto& i : PTTS_GET_ALL(recharge)) {
                                        if (i.second.has_days()) {
                                                yueka_set.insert(i.second.id());
                                        }
                                }
                                if (yueka_set.empty()) {
                                        return pd::FAILED;
                                }

                                auto iter = yueka_set.begin();
                                advance(iter, rand() % yueka_set.size());
                                auto result = check_recharge_day(*iter, *role_);
                                if (result != pd::OK) {
                                        return pd::FAILED;
                                }

                                const auto& ptt = PTTS_GET(recharge, *iter);
                                role_->recharge_update_last_process_day(*iter, refresh_day());
                                pd::ce_env ce;
                                ce.set_origin(pd::CO_GET_RECHARGE);
                                event_process(ptt.day_events(), *role_, ce);

                                return pd::OK;
                        }

                        pd::result ai_action_adventure_challenge_new(pd::behavior_tree_node& node) {
                                auto passed_adventures = role_->adventure_passed_adventures();
                                set<uint64_t> adventures;
                                auto result = pd::OK;
                                const auto& ptts = PTTS_GET_ALL(adventure);
                                for (const auto& i : ptts) {
                                        if (passed_adventures.count(i.first) > 0) {
                                                continue;
                                        }
                                        result = adventure_check_challenge(i.second, *role_);
                                        if (result == pd::OK) {
                                                adventures.insert(i.first);
                                        }
                                }
                                if (adventures.empty()) {
                                        return pd::FAILED;
                                }

                                auto iter = adventures.begin();
                                advance(iter, rand() % adventures.size());
                                pd::pttid_array plots;
                                const auto& selections = get_plot_selection_array(*iter, plots);
                                if (selections.selections_size() > 1) {
                                        for (auto i : selections.selections()) {
                                                pd::plot_selection_array psa;
                                                if (selections.has_plot()) {
                                                        psa.set_plot(selections.plot());
                                                }
                                                psa.add_selections(i);
                                                result = challenge_adventure(*iter, psa, plots, pd::battle_info(), pd::huanzhuang_item_id_array());
                                                if (result != pd::OK) {
                                                        return pd::FAILED;
                                                }
                                        }
                                } else {
                                        result = challenge_adventure(*iter, selections, plots, pd::battle_info(), pd::huanzhuang_item_id_array());
                                }
                                return result == pd::OK ? pd::OK : pd::FAILED;
                        }
*/

                        void clear_timeout() {
                                async_call(
                                        [this, self = this->shared_from_this()] {
                                                if (this->check_state_playing() && !quiting_playing_) {
                                                        SPLAYER_DLOG << *this << " clear timeout";
                                                        this->sm_->reset_timeout(5min);
                                                }
                                        });
                        }

                        const shared_ptr<service_thread>& get_st() const {
                                return st_;
                        }

                        function<void(const Descriptor *, size_t)> send_msg_cb_;
                        function<pd::result(const shared_ptr<player<T>>&)> login_cb_;
                        function<bool(const shared_ptr<player<T>>&)> start_fetch_data_cb_;
                        function<bool(const shared_ptr<player<T>>&, const string&, const string&)> login_done_cb_;
                        function<void(const shared_ptr<player<T>>&)> quit_cb_;
                        function<void(const shared_ptr<player<T>>&)> start_playing_cb_;
                        function<bool(const shared_ptr<player<T>>&)> stop_playing_cb_;
                        function<uint64_t(const string&)> rolename2gid_func_;
                        function<string(uint64_t)> gid2rolename_func_;
                        function<string(uint32_t)> random_role_name_func_;
                        function<uint32_t()> server_open_day_func_;
                        function<void(bool)> activity_add_fund_func_;

                private:
                        bool check_state_playing() const {
                                return sm_->get_state() == state_playing::instance();
                        }
                        void set_quiting_playing() {
                                ai_stop();
                                stop_resource_grow_timer();
                                quiting_playing_ = true;
                        }
                        bool check_state_fetch_data() const {
                                return sm_->get_state() == state_fetch_data::instance();
                        }
                        bool check_state_init() const {
                                return sm_->get_state() == state_init::instance();
                        }
                        bool check_state_check_login() const {
                                return sm_->get_state() == state_check_login::instance();
                        }
                        bool check_state_client_create_role() const {
                                return sm_->get_state() == state_client_create_role::instance();
                        }
                        bool check_state_create_role() const {
                                return sm_->get_state() == state_create_role::instance();
                        }
                        bool check_state_find_role() const {
                                return sm_->get_state() == state_find_role::instance();
                        }
                        vector<string> translate_data(const vector<boost::any>& data) {
                                vector<string> str_data;
                                for (auto& i : data) {
                                        try {
                                                str_data.push_back(boost::any_cast<string>(i));
                                        } catch (const boost::bad_any_cast& e) {
                                                auto value = boost::any_cast<uint64_t>(i);
                                                str_data.push_back(to_string(value));
                                        }
                                }

                                return str_data;
                        }

                        bool init_role(const pd::role& data) {
                                role_ = make_shared<role>(st_, yci_, ip_);
                                role_->parse_from(data);


                                role_->gid2rolename_func_ = [this, self = this->shared_from_this()] (auto gid) {
                                        ASSERT(gid2rolename_func_);
                                        return gid2rolename_func_(gid);
                                };
                                role_->server_open_day_func_ = [this, self = this->shared_from_this()] () {
                                        ASSERT(server_open_day_func_);
                                        return server_open_day_func_();
                                };
                                role_->username_func_ = [this] {
                                        return username();
                                };
                                role_->actor_info_update_cb_ = [this, self = this->shared_from_this()] (auto role, const auto& actor) {
                                        ASSERT(actor_info_update_cb_);
                                        actor_info_update_cb_(role, actor);
                                };
                                role_->friend_intimacy_changed_cb_ = [this, self = this->shared_from_this()] (auto role, auto other, auto value) {
                                        ASSERT(change_friend_intimacy_func_);
                                        change_friend_intimacy_func_(other, role, value);
                                };
                                role_->quest_update_func_ = [this, self = this->shared_from_this()] (auto role, const auto& quest) {
                                        // role login before sending login rsp
                                        if (!this->check_state_playing()) {
                                                return;
                                        }
                                        pcs::base msg;
                                        auto *notice = msg.MutableExtension(pcs::s2c_quest_update_notice::squn);
                                        *notice->mutable_quest() = quest;
                                        this->send_to_client(msg);
                                };
                                role_->forever_quest_update_func_ = [this, self = this->shared_from_this()] (auto role, const auto& quest, const auto& event_res) {
                                        // role login before sending login rsp
                                        if (!this->check_state_playing()) {
                                                return;
                                        }
                                        pcs::base msg;
                                        auto *notice = msg.MutableExtension(pcs::s2c_quest_update_notice::squn);
                                        *notice->mutable_quest() = quest;
                                        *notice->mutable_event_res() = event_res;
                                        this->send_to_client(msg);
                                };
                                role_->quest_auto_commit_func_ = [this, self = this->shared_from_this()] (auto role, auto quest_pttid) {
                                        auto result = quest_check_commit(quest_pttid, *self->role_);
                                        if (result != pd::OK) {
                                                return;
                                        }
                                        this->async_call(
                                                [this, self, quest_pttid, role] {
                                                        SPLAYER_DLOG << "auto commit quset " << quest_pttid;
                                                        if (!this->check_state_playing()) {
                                                                return;
                                                        }

                                                        if (!role_->quest_has_accepted(quest_pttid)) {
                                                                return;
                                                        }

                                                        pcs::base rsp_msg;
                                                        auto *rsp = rsp_msg.MutableExtension(pcs::s2c_quest_commit_rsp::sqcr);
                                                        rsp->set_pttid(quest_pttid);
                                                        rsp->set_result(pd::OK);
                                                        const auto& ptt = PTTS_GET(quest, quest_pttid);
                                                        if (ptt.has__league_quest()) {
                                                                auto lea = role_->get_quest_cur_pass_league_events(quest_pttid);
                                                                league_quest_finish_notice_func_(role, quest_pttid, lea);
                                                                role_->on_event(ECE_LEAGUE_QUEST_FINISH_TIMES, {});
                                                        }
                                                        auto event_res = quest_commit(quest_pttid, *role_);
                                                        *rsp->mutable_event_res() = event_res;
                                                        db_log::instance().log_quest_commit(quest_pttid, event_res, *role_);
                                                        this->send_to_client(rsp_msg);
                                                });
                                };
                                role_->levelup_cb_ = [this, self = this->shared_from_this()] (auto role, auto old_level, auto cur_level, const auto& event_res, auto has_mansion) {
                                        if (this->check_state_playing()) {
                                                pcs::base msg;
                                                auto *notice = msg.MutableExtension(pcs::s2c_role_levelup_event_notice::srlen);
                                                *notice->mutable_event_res() = event_res;
                                                this->send_to_client(msg);
                                        }

                                        ASSERT(levelup_cb_);
                                        levelup_cb_(role, old_level, cur_level, has_mansion);

                                        for (auto i = old_level + 1; i <= cur_level; ++i) {
                                                const auto& levelup_ptt = PTTS_GET(role_levelup, i);
                                                if (levelup_ptt.has_mail()) {
                                                        this->send_mail(levelup_ptt.mail(), pd::dynamic_data(), pd::event_array());
                                                }
                                        }

                                        this->update_level_rank();
                                };
                                role_->update_db_func_ = [this, self = this->shared_from_this()] () {
                                        update_role_db();
                                };
                                role_->on_new_day_cb_ = [this, self = this->shared_from_this()] (const auto& event_res, const auto& battles) {
                                        pcs::base msg;
                                        auto *notice = msg.MutableExtension(pcs::s2c_role_on_new_day::srond);
                                        *notice->mutable_event_res() = event_res;
                                        for(const auto& i : battles) {
                                                auto *bt = notice->add_battles();
                                                bt->set_pttid(i.first);
                                                bt->set_battle_pttid(i.second);
                                        }
                                        this->send_to_client(msg);
                                };
                                role_->arena_update_role_func_ = [this, self = this->shared_from_this()] (auto role, const auto& team) {
                                        ASSERT(arena_update_role_func_);
                                        arena_update_role_func_(role, team);
                                };
                                role_->gongdou_sync_record_to_spouse_func_ = [this, self = this->shared_from_this()] (auto spouse, auto role, auto type, const auto& record) {
                                        ASSERT(gongdou_sync_record_to_spouse_func_);
                                        gongdou_sync_record_to_spouse_func_(spouse, role, type, record);
                                };
                                role_->vip_exp_sync_to_spouse_func_ = [this, self = this->shared_from_this()] (auto spouse, auto role, auto vip_exp) {
                                        ASSERT(vip_exp_sync_to_spouse_func_);
                                        vip_exp_sync_to_spouse_func_(spouse, role, vip_exp);
                                };
                                role_->gongdou_promote_guanpin_func_ = [this, self = this->shared_from_this()] (auto role, auto gpin, auto gongdou_data, const auto& event_res) {
                                        ASSERT(gongdou_promote_guanpin_func_);
                                        gongdou_promote_guanpin_func_(role, gpin, gongdou_data, event_res);
                                };
                                role_->marriage_spouse_refresh_questions_func_ = [this, self = this->shared_from_this()] (auto role, auto spouse) {
                                        ASSERT(marriage_spouse_refresh_questions_func_);
                                        marriage_spouse_refresh_questions_func_(role, spouse);
                                };
                                role_->marriage_sync_questions_func_ = [this, self = this->shared_from_this()] (const auto& questions) {
                                        pcs::base msg;
                                        auto *sync = msg.MutableExtension(pcs::s2c_marriage_update_questions::smuq);
                                        for (const auto& i : questions) {
                                                *sync->add_questions() = i;
                                        }
                                        this->send_to_client(msg);
                                };
                                role_->marriage_sync_questions_to_spouse_func_ = [this, self = this->shared_from_this()] (auto role, auto spouse, const auto& questions) {
                                        ASSERT(marriage_sync_questions_to_spouse_func_);
                                        marriage_sync_questions_to_spouse_func_(role, spouse, questions);
                                };
                                role_->mansion_add_hall_quest_func_ = [this, self = this->shared_from_this()] (auto role, const auto& quest, auto need_notice) {
                                        ASSERT(mansion_add_hall_quest_func_);
                                        mansion_add_hall_quest_func_(role, quest, need_notice);
                                };
                                role_->gongdou_effect_changed_notice_ = [this, self = this->shared_from_this()] (auto role, const auto& extra_battle_data) {
                                        ASSERT(gongdou_effect_changed_notice_);
                                        gongdou_effect_changed_notice_(role, extra_battle_data);
                                };
                                role_->guanpin_update_role_func_ = [this, self = this->shared_from_this()] (auto role, const auto& data) {
                                        ASSERT(guanpin_update_role_func_);
                                        guanpin_update_role_func_(role, data);
                                };
                                role_->guanpin_changed_cb_ = [this, self = this->shared_from_this()] (auto role, auto old_gpin, auto new_gpin) {
                                        ASSERT(guanpin_changed_cb_);
                                        guanpin_changed_cb_(role, old_gpin, new_gpin);
                                };
                                role_->role_add_new_record_notice_ = [this, self = this->shared_from_this()] (auto role, auto type, auto sub_type, const auto& record) {
                                        pcs::base msg;
                                        auto *notice = msg.MutableExtension(pcs::s2c_role_new_record_notice::srnrn);
                                        notice->set_type(type);
                                        notice->set_sub_type(sub_type);
                                        *notice->mutable_record() = record;
                                        this->send_to_client(msg);
                                };
                                role_->broadcast_system_chat_func_ = [this, self = this->shared_from_this()] (auto role, auto pttid) {
                                        pd::dynamic_data dd;
                                        dd.add_name(gid2rolename_func_(role));
                                        ASSERT(send_system_chat_func_);
                                        send_system_chat_func_(pttid, dd);
                                        this->mirror_broad_system_chat(pttid);
                                };
                                role_->league_update_role_func_ = [this, self = this->shared_from_this()] (auto role, auto battle_team) {
                                        ASSERT(league_update_role_func_);
                                        league_update_role_func_(role, battle_team);
                                };
                                role_->child_sync_actor_stars_func_ = [this, self = this->shared_from_this()] (auto role, const auto& data) {
                                        child_mgr::instance().sync_actor_stars(role, data);
                                };
                                role_->send_mail_func_ = [this, self = this->shared_from_this()] (auto mail, const auto& events) {
                                        this->send_mail(mail, pd::dynamic_data(), events);
                                };

                                role_->register_event(ECE_CHANGE_RESOURCE,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              ASSERT(args.size() == 2);
                                                              auto type = boost::any_cast<pd::resource_type>(args[0]);
                                                              if (type == pd::resource_type::EXP) {
                                                                      this->update_level_rank();
                                                              } else if (type == pd::resource_type::GIVE_GIFT) {
                                                                      this->update_give_gift_rank();
                                                              } else if (type == pd::resource_type::RECEIVE_GIFT) {
                                                                      this->update_receive_gift_rank();
                                                              } else if (type == pd::resource_type::TEA_PARTY_FAVOR) {
                                                                      this->update_tea_party_favor();
                                                              }
                                                      });
                                role_->register_event(ECE_ZHANLI_CHANGE,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              this->update_zhanli_rank();
                                                      });
                                role_->register_event(ECE_MANSION_UPDATE_FANCY,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              if (this->check_update_ranking_permit(pd::RT_MANSION_FANCY) == pd::OK) {
                                                                      ASSERT(update_mansion_fancy_rank_func_);
                                                                      update_mansion_fancy_rank_func_(role_->gid(), boost::any_cast<uint32_t>(args[0]));
                                                              }
                                                      });
                                role_->register_event(ECE_TOWER_NEW_HIGHEST_LEVEL,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              if (this->check_update_ranking_permit(pd::RT_TOWER) == pd::OK) {
                                                                      ASSERT(update_tower_rank_func_);
                                                                      update_tower_rank_func_(role_->gid(), boost::any_cast<uint32_t>(args[0]));
                                                              }
                                                      });
                                role_->register_event(ECE_CHAOTANG_RANK_CHANGE,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              if (this->check_update_ranking_permit(pd::RT_CHAOTANG) == pd::OK) {
                                                                      ASSERT(update_chaotang_rank_func_);
                                                                      update_chaotang_rank_func_(role_->gid(), boost::any_cast<int>(args[0]), boost::any_cast<int>(args[1]));
                                                              }
                                                      });
                                role_->register_event(ECE_RELATION_FRIEND_INTIMACY,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              if (this->check_update_ranking_permit(pd::RT_MARRIAGE_DEVOTION) == pd::OK) {
                                                                      if (role_->marriage_married() && role_->marriage_proposer()) {
                                                                              ASSERT(update_marriage_devotion_rank_func_);
                                                                              update_marriage_devotion_rank_func_(role_->gid(), role_->marriage_spouse(), boost::any_cast<int>(args[0]));
                                                                      }
                                                              }
                                                      });
                                role_->register_event(ECE_MARRIAGE_PROPOSE,
                                                      [this, self = this->shared_from_this()] (const auto& args) {
                                                              this->update_marriage_devotion_rank();
                                                      });
                                if (online()) {
                                        if (role_->ban_until_time() >= system_clock::to_time_t(system_clock::now())) {
                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                rsp->set_username(username_);
                                                rsp->set_result(pd::ROLE_IS_BANNED);
                                                send_to_client(msg);
                                                return false;
                                        }
                                }

                                if (mirror_role()) {
                                        role_->set_mirror_role();
                                        SPLAYER_DLOG << *this << " update mirror role " << mirror_role_data_->gid();
                                        ASSERT(mirror_role_data_);
                                        role_->mirror_update(*mirror_role_data_);
                                }

                                return true;
                        }

                        void fetch_data() {
                                if (online()) {
                                        ASSERT(start_fetch_data_cb_);
                                        if (!start_fetch_data_cb_(this->shared_from_this())) {
                                                pcs::base msg;
                                                auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                rsp->set_username(username_);
                                                rsp->set_result(pd::AGAIN);
                                                send_to_client(msg, true);

                                                quit();
                                                return;
                                        }

                                        fetching_data_count_ += 1;
                                        fetch_league_data_call_func(
                                                [this, self = this->shared_from_this()] (auto result, const auto& data) {
                                                        SPLAYER_DLOG << *this << " got league data";
                                                        if (result == pd::OK) {
                                                                login_league_data_ = make_unique<pd::league>();
                                                                *login_league_data_ = data.league_data();
                                                                role_->set_league_info(data.league_data());
                                                                for (auto i : data.bonus_cities()) {
                                                                        role_->add_bonus_city(i);
                                                                }

                                                                if (data.has_member_position()) {
                                                                        role_->set_league_member_position(data.member_position());
                                                                }
                                                                auto president_gid = league_president_gid(data.league_data());
                                                                if (role_->get_league_member_position() == pd::LMP_PRESIDENT && president_gid != role_->gid()) {
                                                                        role_->set_league_member_position(pd::LMP_MEMBER);
                                                                }

                                                                if (this->mirror_role()) {
                                                                        if (president_gid == role_->gid()) {
                                                                                this->mirror_role_league_transfer_president();
                                                                        }
                                                                }
                                                        }
                                                        else {
                                                                role_->clear_league_info();
                                                        }
                                                        this->fetch_one_data_done();
                                                });

                                        fetching_data_count_ += 1;
                                        ASSERT(fetch_fief_data_func_);
                                        fetch_fief_data_func_(role_->gid(), role_->ip());

                                        fetching_data_count_ += 1;
                                        ASSERT(fetch_mansion_data_func_);
                                        fetch_mansion_data_func_(role_->gid(), role_->level());

                                        fetching_data_count_ += 1;
                                        ASSERT(fetch_marriage_data_func_);
                                        fetch_marriage_data_func_(role_->gid(), role_->marriage_married() ? role_->marriage_spouse() : 0);

                                        fetching_data_count_ += 1;
                                        ASSERT(fetch_arena_rank_func_);
                                        fetch_arena_rank_func_(role_->gid());

                                        fetching_data_count_ += 1;
                                        huanzhuang_pvp::instance().fetch_data(role_->gid(), role_->gender());

                                        fetching_data_count_ += 1;
                                        ASSERT(guanpin_fetch_data_func_);
                                        guanpin_fetch_data_func_(role_->gid());

                                        fetching_data_count_ += 1;
                                        child_mgr::instance().fetch_children(role_->gid());

                                        fetching_data_count_ += 1;
                                        activity_mgr::instance().fetch_data(role_->gid());
                                } else {
                                        this->async_call(
                                                [this, self = this->shared_from_this()] {
                                                        event_fetch_data_done event;
                                                        sm_->on_event(&event);
                                                });
                                }
                        }

                        bool login_done() {
                                ASSERT(st_->check_in_thread());
                                ASSERT(login_done_cb_);
                                if (!login_done_cb_(this->shared_from_this(), role_->name(), username())) {
                                        pcs::base msg;
                                        auto *rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                        rsp->set_username(username_);
                                        rsp->set_result(pd::AGAIN);
                                        send_to_client(msg, true);
                                        return false;
                                }

                                if (online()) {
                                        role_->login();
                                        role_->try_levelup();
                                        const auto& ptts = PTTS_GET_ALL(resource);
                                        for (const auto& i : ptts) {
                                                if (i.second.grow_minutes() > 0) {
                                                        role_->grow_resource(i.second.type());
                                                        add_resource_grow_timer(i.second.type());
                                                }
                                        }
                                        if (!mirror_role()) {
                                                /*bi::instance().login(username(), yci_, ip_, role_->gid(), role_->league_gid(),
                                                                     role_->name(), role_->level(),
                                                                     role_->calc_max_zhanli(), role_->get_resource(pd::resource_type::EXP),
                                                                     role_->get_resource(pd::resource_type::DIAMOND),
                                                                     role_->get_resource(pd::resource_type::GOLD),
                                                                     pd::gender_Name(role_->gender()),
                                                                     role_->get_resource(pd::resource_type::VIP_EXP),
                                                                     role_->vip_level());*/
                                                db_log::instance().log_login(*role_, ip_);

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                                rsp->set_result(pd::OK);
                                                rsp->set_username(username_);
                                                role_->serialize_for_client(rsp->mutable_role());
                                                auto emperor_effect = gongdou_emperor_favour_effect_func_();
                                                if (system_clock::to_time_t(system_clock::now()) <= emperor_effect.start_time() + emperor_effect.last_time()) {
                                                        *rsp->mutable_role()->mutable_gongdou()->add_effects() = emperor_effect;
                                                }
                                                rsp->set_seconds_west(clock::instance().seconds_west());
                                                rsp->set_server_open_day(server_open_day_func_());
                                                for(const auto& i : get_images_announcements_func_()) {
                                                        *rsp->add_images_announcements() = i.first;
                                                }

                                                if (login_league_data_) {
                                                        *rsp->mutable_league() = *login_league_data_;
                                                        login_league_data_.reset();
                                                }
                                                if (login_mansion_data_) {
                                                        *rsp->mutable_mansion() = *login_mansion_data_;
                                                        login_mansion_data_.reset();
                                                }
                                                if (login_fief_data_) {
                                                        *rsp->mutable_fief() = *login_fief_data_;
                                                        login_fief_data_.reset();
                                                }
                                                if (login_arena_champion_) {
                                                        *rsp->mutable_arena_champion() = *login_arena_champion_;
                                                        login_arena_champion_.reset();
                                                }
                                                if (login_huanzhuang_pvp_) {
                                                        *rsp->mutable_huanzhuang_pvp() = *login_huanzhuang_pvp_;
                                                        login_huanzhuang_pvp_.reset();
                                                }
                                                if (login_children_) {
                                                        *rsp->mutable_children() = *login_children_;
                                                        login_children_.reset();
                                                }
                                                if (login_activity_mgr_) {
                                                        *rsp->mutable_activity_mgr() = *login_activity_mgr_;
                                                        login_activity_mgr_.reset();
                                                }
                                                auto options_ptt = PTTS_GET_COPY(options, 1);
                                                if (options_ptt.scene_info().testin()) {
                                                        rsp->set_testin(true);
                                                }
                                                rsp->set_skip_xinshou(options_ptt.scene_info().skip_xinshou());
                                                send_to_client(rsp_msg);

                                                auto db_msg = make_shared<db::message>("find_notice",
                                                                                       db::message::req_type::rt_select,
                                                                                       [this, self = this->shared_from_this()] (const auto& msg) {
                                                                                               this->handle_finished_db_msg(msg);
                                                                                       });
                                                db_msg->push_param(role_->gid());
                                                gamedb_notice_->push_message(db_msg, st_);
                                                SPLAYER_DLOG << *this << " find notice";

                                                recharge_day_bufa();
                                                update_all_ranking();
                                                behavior_tree_start();
                                                activity_daiyanren_check_finish();
                                                activity_seven_days_check_finish();

                                                pd::role_mirror_event event;
                                                serialize_mirror_data(rsp_msg, &event);
                                                send_mirror_role_event_msg(event);

                                                if (role_->marriage_married()) {
                                                        ASSERT(marriage_notice_spouse_login_func_);
                                                        marriage_notice_spouse_login_func_(role_->gid(), role_->marriage_spouse());
                                                }
                                        }
                                        else {
                                                update_all_ranking();
                                                if (!mirror_role_data_->other_data().league().has_league_gid()) {
                                                        role_mirror_event_try_join_league();
                                                }
                                                mirror_role_data_.reset();
                                        }

                                        update_role_db();
                                }

                                return true;
                        }

                        void serialize_mirror_data(const pcs::base& msg, pd::role_mirror_event *mirror_event) {
                                mirror_event->set_type(pd::RMET_LOGIN);
                                mirror_event->set_gid(gid());
                                role_->serialize(mirror_event->mutable_role());

                                const auto& rsp = msg.GetExtension(pcs::s2c_login_rsp::slr);
                                if (rsp.has_mansion()) {
                                        *mirror_event->mutable_mansion() = rsp.mansion();
                                }
                                if (rsp.has_fief()) {
                                        *mirror_event->mutable_fief_data() = rsp.fief();
                                }
                        }

                        bool can_send_mirror_role_event_msg() {
                                const auto& ptt = PTTS_GET(role_logic, 1);
                                auto result = condition_check(ptt.mirror_unlock_conditions(), *role_);
                                if (result == pd::OK) {
                                        return true;
                                } else {
                                        return false;
                                }
                        }

                        void send_mirror_role_event_msg(const pd::role_mirror_event& event) {
                                if (!role_) {
                                        return;
                                }
                                ASSERT(st_->check_in_thread());
                                if (!can_send_mirror_role_event_msg()) {
                                        return;
                                }

                                switch (event.type()) {
                                        case pd::RMET_LOGIN:
                                            mirrored_ = true;
                                            break;
                                        default:
                                            break;
                                }

                                if (!mirrored_) {
                                        return;
                                }

                                ps::base msg;
                                auto *notice = msg.MutableExtension(ps::sm_role_event::sre);
                                *notice->mutable_event() = event;
                                if (!event.has_gid()) {
                                        notice->mutable_event()->set_gid(gid());
                                }
                                mirror_mgr::instance().random_send_msg(msg);
                        }

                        void fetch_one_data_done() {
                                fetching_data_count_ -= 1;
                                if (fetching_data_count_ <= 0) {
                                        SPLAYER_DLOG << *this << " fetch data done";
                                        event_fetch_data_done event;
                                        sm_->on_event(&event);
                                }
                        }

                        void fetch_league_data_call_func(const function<void(pd::result, const pd::league_for_role&)>& func) {
                                if (fetch_league_data_call_funcs_.empty()) {
                                        fetch_league_data_call_funcs_.push_back(func);
                                        fetch_league_data_func_(role_->gid());
                                } else {
                                        fetch_league_data_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_league_list_call_func(const function<void(const list<pair<uint64_t, string>>&, const list<pair<uint64_t, string>>&)>& func) {
                                ASSERT(get_league_list_func_);
                                if (fetch_league_list_call_funcs_.empty()) {
                                        fetch_league_list_call_funcs_.push_back(func);
                                        get_league_list_func_(role_->gid());
                                } else {
                                        fetch_league_list_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_mansion_banquet_list_call_func(const function<void(const pd::mansion_banquets&)>& func) {
                                if (fetch_mansion_banquet_list_call_funcs_.empty()) {
                                        const auto& friends = role_->relation_friends();
                                        set<uint64_t> friends_set;
                                        for (const auto& i : friends) {
                                                friends_set.insert(i.first);
                                        }

                                        fetch_mansion_banquet_list_call_funcs_.push_back(func);
                                        mansion_fetch_banquet_list_func_(role_->gid(), friends_set);
                                } else {
                                        fetch_mansion_banquet_list_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_room_list_call_func(const function<void(pd::result, pd::room_type, const pd::room_array&)>& func, pd::room_type room_type) {
                                if (fetch_room_list_call_funcs_.empty()) {
                                        fetch_league_data_call_func(
                                                [this, func, room_type] (auto result, const auto& data) {
                                                        set<uint64_t> friends_and_league;
                                                        if (result == pd::OK) {
                                                                friends_and_league = league_members(data.league_data());
                                                        }
                                                        const auto& friends = role_->relation_friends();
                                                        for (auto i : friends) {
                                                                friends_and_league.insert(i.first);
                                                        }

                                                        fetch_room_list_call_funcs_.push_back(func);
                                                        room_mgr::instance().list_room(role_->gid(), room_type, friends_and_league);
                                                });
                                } else {
                                        fetch_room_list_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_room_info_call_func(const function<void(uint64_t, pd::result, const pd::room&)>& func, uint64_t room_keeper) {
                                if (fetch_room_info_call_funcs_.empty()) {
                                        fetch_room_info_call_funcs_.push_back(func);
                                        room_mgr::instance().get_room_info_by_role(role_->gid(), room_keeper);
                                } else {
                                        fetch_room_info_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_marriage_star_wish_list_call_func(const function<void(pd::result, const pd::marriage_star_wish_array&)>& func) {
                                if (fetch_marriage_star_wish_list_call_funcs_.empty()) {
                                        fetch_marriage_star_wish_list_call_funcs_.push_back(func);
                                        ASSERT(marriage_star_wish_list_func_);
                                        marriage_star_wish_list_func_(role_->gid());
                                } else {
                                        fetch_marriage_star_wish_list_call_funcs_.push_back(func);
                                }
                        }

                        void fetch_marriage_pk_list_call_func(const function<void(pd::result, const pd::marriage_pk_array&)>& func) {
                                if (fetch_marriage_pk_list_call_funcs_.empty()) {
                                        fetch_marriage_pk_list_call_funcs_.push_back(func);
                                        ASSERT(marriage_pk_list_func_);
                                        marriage_pk_list_func_(role_->gid());
                                } else {
                                        fetch_marriage_pk_list_call_funcs_.push_back(func);
                                }
                        }

                        void mirror_role_fief_action_list_call_func(uint64_t to_fief, const function<void()>& func) {
                                if (mirror_role_ai_.fief_.entered_fief_ == to_fief) {
                                        func();
                                } else if (mirror_role_fief_action_list_call_funcs_[to_fief].empty()) {
                                        mirror_role_ai_.fief_.action_count_ += 1;
                                        mirror_role_fief_action_list_call_funcs_[to_fief].push_back(func);
                                        ASSERT(enter_fief_func_);
                                        enter_fief_func_(role_->gid(), to_fief);
                                } else {
                                        mirror_role_ai_.fief_.action_count_ += 1;
                                        mirror_role_fief_action_list_call_funcs_[to_fief].push_back(func);
                                }
                        }

                        void mirror_role_mansion_action_list_call_func(uint64_t to_mansion, pd::mansion_building_type building_type, const function<void()>& func) {
                                if (mirror_role_ai_.mansion_.entered_mansion_ == to_mansion) {
                                        func();
                                } else if (mirror_role_mansion_action_list_call_funcs_[to_mansion].empty()) {
                                        mirror_role_ai_.mansion_.gid2action_count_[to_mansion] += 1;
                                        mirror_role_mansion_action_list_call_funcs_[to_mansion].push_back(func);
                                        ASSERT(enter_mansion_func_);
                                        auto is_friend = role_->gid() == to_mansion || role_->relation_has_friend(to_mansion);
                                        enter_mansion_func_(role_->gid(), to_mansion, building_type, is_friend, mansion_random_plot(to_mansion, building_type, *role_));
                                } else {
                                        mirror_role_ai_.mansion_.gid2action_count_[to_mansion] += 1;
                                        mirror_role_mansion_action_list_call_funcs_[to_mansion].push_back(func);
                                }
                        }

                        void login_no_role() {
                                if (!mirror_role()) {
                                        pcs::base msg;
                                        auto rsp = msg.MutableExtension(pcs::s2c_login_rsp::slr);
                                        rsp->set_username(username_);
                                        rsp->set_result(pd::NOT_FOUND);

                                        send_to_client(msg);
                                } else {
                                        ASSERT(mirror_role_data_);
                                        SPLAYER_DLOG << *this << " create mirror role " << mirror_role_data_->gid();

                                        ASSERT(random_role_name_func_);
                                        mirror_role_data_->mutable_data()->set_name(random_role_name_func_(MIRROR_NAME_POOL_START));

                                        create_role(*mirror_role_data_);
                                }
                        }

                        void update_role_db() {
                                pd::role data;
                                if (role_ && role_->serialize_changed_data(&data)) {
                                        role_->reset_data_changed();
                                        auto msg = dbcache::gen_update_role_req(data);
                                        dbcache::instance().send_to_dbcached(msg);
                                }
                        }

                        void notice_set_already_read(uint64_t gid) {
                                auto msg = make_shared<db::message>("notice_set_already_read",
                                                                    db::message::req_type::rt_update,
                                                                    [this, self = this->shared_from_this()] (const auto& msg) {
                                                                            this->handle_finished_db_msg(msg);
                                                                    });
                                msg->push_param(gid);
                                msg->push_param(role_->gid());
                                msg->push_param(1);
                                gamedb_notice_->push_message(msg, st_);
                        }

                        void notice_delete(uint64_t gid) {
                                auto msg = make_shared<db::message>("delete_notice",
                                                                    db::message::req_type::rt_delete);
                                msg->push_param(gid);
                                msg->push_param(role_->gid());
                                gamedb_notice_->push_message(msg);
                        }

                        void add_mirror_timer(uint32_t delay, const function<void()>& cb) {
                                ASSERT(st_->check_in_thread());
                                if (!check_state_playing()) {
                                        return;
                                }
                                auto timer = ADD_TIMER(st_,
                                        ([this, self = this->shared_from_this(), cb] (auto canceled, const auto& timer) {
                                                if (!canceled) {
                                                        mirror_timers_.erase(timer);
                                                        cb();
                                                }
                                        }),
                                        seconds(delay));
                                mirror_timers_.emplace(timer);
                        }

                        bool try_exec_chat_gm(string& content) {
                                if (content.find("\\gm ") == 0 && content.size() > 4) {
                                        content = content.substr(4);

                                        auto tokens = split_string(content);
                                        if (!tokens.empty()) {
                                                SPLAYER_DLOG << *this << " chat gm:";
                                                for (const auto& i : tokens) {
                                                        SPLAYER_DLOG << i;
                                                }

                                                static map<string, string> fief_event_type {
                                                        { "FET_ADD_RESOURCE", "ADD_RESOURCE" },
                                                        { "FET_DEC_RESOURCE", "DEC_RESOURCE" },
                                                        { "FET_INCIDENT_EXTRA_WEIGHT", "INCIDENT_EXTRA_WEIGHT" },
                                                };
                                                pd::event::event_type type;
                                                pd::league_event_type league_type;
                                                pd::fief_event::fief_event_type fief_type;
                                                if (pd::event::event_type_Parse(tokens[0], &type)) {
                                                        exec_chat_gm_events(tokens);
                                                } else if (pd::league_event_type_Parse(tokens[0], &league_type)) {
                                                        exec_chat_gm_league_events(tokens);
                                                } else if (fief_event_type.count(tokens[0]) > 0) {
                                                        tokens[0] = fief_event_type.at(tokens[0]);
                                                        if (pd::fief_event::fief_event_type_Parse(tokens[0], &fief_type)) {
                                                                exec_chat_gm_fief_events(tokens);
                                                        }
                                                } else {
                                                        exec_chat_gm_commands(tokens);
                                                }
                                        }
                                        return true;
                                }
                                return false;
                        }

                        void exec_chat_gm_league_events(const vector<string>& tokens) {
                                pd::league_event_type type;
                                pd::league_event_type_Parse(tokens[0], &type);

                                pd::league_event_array lea;
                                auto *e = lea.add_events();
                                e->set_type(type);
                                for (auto i = 1ul; i < tokens.size(); ++i) {
                                        e->add_arg(tokens[i]);
                                }
                                if (config::check_league_events(lea)) {
                                        league_gm_process_event_func_(role_->gid(), lea);
                                }
                        }

                        void exec_chat_gm_fief_events(const vector<string>& tokens) {
                                pd::fief_event::fief_event_type type;
                                pd::fief_event::fief_event_type_Parse(tokens[0], &type);

                                pd::fief_event_array fea;
                                auto *e = fea.add_events();
                                e->set_type(type);
                                for (auto i = 1ul; i < tokens.size(); ++i) {
                                        e->add_arg(tokens[i]);
                                }
                                if (config::check_fief_events(fea)) {
                                        fief_gm_process_event_func_(role_->gid(), fea);
                                }

                        }

                        void exec_chat_gm_events(const vector<string>& tokens) {
                                pd::event::event_type type;
                                pd::event::event_type_Parse(tokens[0], &type);
                                pd::event_array ea;
                                auto *e = ea.add_events();
                                e->set_type(type);
                                for (auto i = 1ul; i < tokens.size(); ++i) {
                                        e->add_arg(tokens[i]);
                                }
                                if (config::check_events(ea) && config::verify_events(ea)) {
                                        pcs::base rsp_msg;
                                        auto *rsp = rsp_msg.MutableExtension(pcs::s2c_gm_exec_event_rsp::sgeer);

                                        pd::ce_env ce;
                                        ce.set_origin(pd::CO_CHAT_GM);
                                        auto event_res = event_process(ea, *role_, ce);
                                        *rsp->mutable_res_array()->add_event_reses() = event_res;
                                        send_to_client(rsp_msg);

                                        db_log::instance().log_exec_gm(ea, rsp->res_array(), *role_, 0);

                                        role_->try_levelup();
                                }
                        }

                        void exec_chat_gm_commands(const vector<string>& tokens) {
                                auto result = pd::OK;
                                if (tokens[0] == "ADD_MIN") {
                                        if (tokens.size() == 2) {
                                                auto min = stoi(tokens[1]);
                                                if (min <= 0 || !add_min(min)) {
                                                        result = pd::INTERNAL_ERROR;
                                                }
                                        }
                                } else if (tokens[0] == "ADD_HOUR") {
                                        if (tokens.size() == 2) {
                                                auto hour = stoi(tokens[1]);
                                                if (hour <= 0 || !add_hour(hour)) {
                                                        result = pd::INTERNAL_ERROR;
                                                }
                                        }
                                } else if (tokens[0] == "SEND_MAIL") {
                                        if (tokens.size() == 2) {
                                                send_mail(stoul(tokens[1]), pd::dynamic_data(), pd::event_array());
                                        }
                                } else if (tokens[0] == "AI") {
                                        if (tokens.size() == 2) {
                                                auto token = stoi(tokens[1]);
                                                if (token == 0) {
                                                        ai_set_active(false);
                                                } else if (token == 1) {
                                                        ai_set_active(true);
                                                }
                                        }
                                } else if (tokens[0] == "PASS_XINSHOU_GROUP") {
                                        if (tokens.size() == 2) {
                                                auto group_pttid = stoul(tokens[1]);
                                                if (PTTS_HAS(xinshou_group, group_pttid)) {
                                                        const auto& ptts = PTTS_GET_ALL(xinshou_group);
                                                        for (const auto& i : ptts) {
                                                                if (i.first <= group_pttid) {
                                                                        role_->xinshou_add_passed_group(i.first);
                                                                }
                                                        }
                                                }
                                        }
                                } else if (tokens[0] == "EXEC_GM") {
                                        if (tokens.size() == 2) {
                                                ASSERT(PTTS_HAS(gm_data, stoul(tokens[1])));

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_gm_exec_event_rsp::sgeer);

                                                pd::ce_env ce;
                                                ce.set_origin(pd::CO_CHAT_GM);
                                                const auto& ptt = PTTS_GET(gm_data, stoul(tokens[1]));
                                                for (const auto& i : ptt.events().events()) {
                                                        pd::event_array ea;
                                                        *ea.add_events() = i;
                                                        auto event_res = event_process(ea, *role_, ce);
                                                        *rsp->mutable_res_array()->add_event_reses() = event_res;
                                                }
                                                send_to_client(rsp_msg);
                                                db_log::instance().log_exec_gm(pd::event_array(), rsp->res_array(), *role_, ptt.id());
                                        }
                                } else if (tokens[0] == "EXEC_GM_SEQ") {
                                        if (tokens.size() == 2) {
                                                ASSERT(PTTS_HAS(gm_sequence, stoul(tokens[1])));

                                                pcs::base rsp_msg;
                                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_gm_exec_event_rsp::sgeer);

                                                pd::ce_env ce;
                                                ce.set_origin(pd::CO_CHAT_GM);
                                                const auto& ptt = PTTS_GET(gm_sequence, stoul(tokens[1]));
                                                for (auto i : ptt.sequences()) {
                                                        const auto& gm_ptt = PTTS_GET(gm_data, i);
                                                        for (const auto& i : gm_ptt.events().events()) {
                                                                pd::event_array ea;
                                                                *ea.add_events() = i;
                                                                auto event_res = event_process(ea, *role_, ce);
                                                                *rsp->mutable_res_array()->add_event_reses() = event_res;
                                                        }
                                                }
                                                send_to_client(rsp_msg);
                                                db_log::instance().log_exec_gm(pd::event_array(), rsp->res_array(), *role_, ptt.id());
                                        }
                                } else if (tokens[0] == "ADD_FIEF_INCIDENT") {
                                        if (tokens.size() == 1) {
                                                fief_add_incident_func_(role_->gid(), 0u);
                                        } else if (tokens.size() == 2) {
                                                ASSERT(PTTS_HAS(fief_incident, stoul(tokens[1])));
                                                fief_add_incident_func_(role_->gid(), stoul(tokens[1]));
                                        }
                                } else if (tokens[0] == "SET_GPIN") {
                                        if (tokens.size() == 2) {
                                                auto battle_team = role_->battle_team(pd::BOT_GUANPIN, true);
                                                guanpin_set_gpin_func_(role_->gid(), battle_team, stoi(tokens[1]));
                                        }
                                } else if (tokens[0] == "SET_QUEST") {
                                        if (tokens.size() == 3) {
                                                ASSERT(PTTS_HAS(quest, stoul(tokens[1])));
                                                ASSERT(stoul(tokens[2]) > 0u);
                                                role_->exec_chat_gm_pass_quest(stoul(tokens[1]), stoul(tokens[2]));
                                        }
                                } else if (tokens[0] == "ADD_MANSION_HALL_QUEST") {
                                        if (tokens.size() == 2) {
                                                ASSERT(PTTS_HAS(mansion_hall_quest, stoul(tokens[1])));
                                                role_->mansion_add_hall_quest(stoul(tokens[1]));
                                        }
                                } else if (tokens[0] == "ADD_RANDOM_SEED") {
                                        if (tokens.size() >= 2) {
                                                for (size_t i = 1; i < tokens.size(); ++i) {
                                                        random_seeds::instance().push(stoul(tokens[i]));
                                                }
                                        }
                                } else if (tokens[0] == "CLEAR_RANDOM_SEED") {
                                        if (tokens.size() == 1) {
                                                random_seeds::instance().clear();
                                        }
                                } else if (tokens[0] == "SET_TOWER_HIGHEST_LEVEL") {
                                        if (tokens.size() == 2) {
                                                auto level = stoul(tokens[1]);
                                                role_->tower_set_highest_level(level);
                                                role_->on_event(ECE_TOWER_NEW_HIGHEST_LEVEL, { level });
                                        }
                                } else if (tokens[0] == "SET_TOWER_CUR_LEVEL") {
                                        if (tokens.size() == 2) {
                                                auto level = stoul(tokens[1]);
                                                role_->tower_set_cur_level(level);
                                                if (role_->tower_highest_level() < level) {
                                                        role_->tower_set_highest_level(level);
                                                        role_->on_event(ECE_TOWER_NEW_HIGHEST_LEVEL, { level });
                                                }
                                        }
                                } else if (tokens[0] == "RESET_FEIGE") {
                                        role_->reset_feige();
                                } else if (tokens[0] == "RECHARGE") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        auto order = role_->add_recharge_order(pttid);
                                        const auto& ptt = PTTS_GET(recharge, pttid);
                                        yunying_recharge(0, 0, order, "", "RMB", ptt.price(), system_clock::to_time_t(system_clock::now()), "", true);
                                } else if (tokens[0] == "REBATE") {
                                        ASSERT(tokens.size() >= 2);
                                        map<uint32_t, uint32_t> recharge2count;
                                        for (auto i = 1u; i < tokens.size(); ++i) {
                                                const auto& token = split_string(tokens[i], ',');
                                                ASSERT(token.size() == 2);
                                                if (recharge2count.count(stoul(token[0]) > 0)) {
                                                        recharge2count[stoul(token[0])] += stoul(token[1]);
                                                } else {
                                                        recharge2count[stoul(token[0])] = stoul(token[1]);
                                                }
                                        }

                                        pd::event_array events;
                                        for (const auto& i : recharge2count) {
                                                event_merge(events, rebate_events(i.first, i.second));
                                        }
                                        send_mail(2000, pd::dynamic_data(), events);
                                } else if (tokens[0] == "ACTIVITY_PRIZE_WHEEL_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_prize_wheel(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_DISCOUNT_GOODS_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_discount_goods(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_CONTINUE_RECHARGE_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_continue_recharge(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_LIMIT_PLAY_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_limit_play(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_LEIJI_RECHARGE_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_leiji_recharge(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_LEIJI_CONSUME_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_leiji_consume(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_FESTIVAL_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_festival(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_PRIZE_WHEEL_STOP") {
                                        ASSERT(tokens.size() == 1);
                                        activity_mgr::instance().stop_prize_wheel(0, 0);
                                } else if (tokens[0] == "ACTIVITY_DISCOUNT_GOODS_STOP") {
                                        ASSERT(tokens.size() == 1);
                                        activity_mgr::instance().stop_discount_goods(0, 0);
                                } else if (tokens[0] == "ACTIVITY_CONTINUE_RECHARGE_STOP") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        activity_mgr::instance().stop_continue_recharge(0, 0, pttid);
                                } else if (tokens[0] == "ACTIVITY_LIMIT_PLAY_STOP") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        activity_mgr::instance().stop_limit_play(0, 0, pttid);
                                } else if (tokens[0] == "ACTIVITY_LEIJI_RECHARGE_STOP") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        activity_mgr::instance().stop_leiji_recharge(0, 0, pttid);
                                } else if (tokens[0] == "ACTIVITY_LEIJI_CONSUME_STOP") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        activity_mgr::instance().stop_leiji_consume(0, 0, pttid);
                                } else if (tokens[0] == "ACTIVITY_TEA_PARTY_START") {
                                        ASSERT(tokens.size() == 3);
                                        auto pttid = stoul(tokens[1]);
                                        auto duration = stoul(tokens[2]);
                                        activity_mgr::instance().start_tea_party(0, 0, refresh_day(), duration, pttid);
                                } else if (tokens[0] == "ACTIVITY_TEA_PARTY_STOP") {
                                        ASSERT(tokens.size() == 1);
                                        activity_mgr::instance().stop_tea_party(0, 0);
                                } else if (tokens[0] == "ACTIVITY_FESTIVAL_STOP") {
                                        ASSERT(tokens.size() == 2);
                                        auto pttid = stoul(tokens[1]);
                                        activity_mgr::instance().stop_festival(0, 0, pttid);
                                } else if (tokens[0] == "ACTIVITY_RECHARGE_RANK_START") {
                                        ASSERT(tokens.size() == 2);
                                        auto duration = stoul(tokens[1]);
                                        activity_mgr::instance().start_recharge_rank(0, 0, refresh_day(), duration);
                                } else if (tokens[0] == "ACTIVITY_RECHARGE_RANK_STOP") {
                                        ASSERT(tokens.size() == 1);
                                        activity_mgr::instance().stop_recharge_rank(0, 0);
                                } else {
                                        result = pd::NOT_FOUND;
                                }

                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::s2c_gm_exec_command_rsp::sgecr);
                                rsp->set_result(result);
                                send_to_client(rsp_msg);
                        }

                        void update_all_ranking() {
                                update_level_rank();
                                update_give_gift_rank();
                                update_receive_gift_rank();
                                update_zhanli_rank();
                                update_chaotang_rank();
                                update_marriage_devotion_rank();
                                update_tea_party_favor();
                                update_tower_rank();
                        }

                        void update_level_rank() {
                                if (check_update_ranking_permit(pd::RT_LEVEL) == pd::OK) {
                                        update_level_rank_func_(role_->gid(), role_->level(), role_->get_resource(pd::resource_type::EXP));
                                }
                        }

                        void update_give_gift_rank() {
                                if (check_update_ranking_permit(pd::RT_GIVE_GIFT) == pd::OK) {
                                        update_give_gift_rank_func_(role_->gid(), role_->get_resource(pd::resource_type::GIVE_GIFT));
                                }
                        }

                        void update_receive_gift_rank() {
                                if (check_update_ranking_permit(pd::RT_RECEIVE_GIFT) == pd::OK) {
                                        update_receive_gift_rank_func_(role_->gid(), role_->get_resource(pd::resource_type::RECEIVE_GIFT));
                                }
                        }

                        void update_zhanli_rank() {
                                if (check_update_ranking_permit(pd::RT_ZHANLI) == pd::OK) {
                                        update_zhanli_rank_func_(role_->gid(), role_->calc_max_zhanli());
                                }
                        }

                        void update_tower_rank() {
                                if (check_update_ranking_permit(pd::RT_TOWER) == pd::OK) {
                                        update_tower_rank_func_(role_->gid(), role_->tower_highest_level());
                                }
                        }

                        void update_chaotang_rank() {
                                if (check_update_ranking_permit(pd::RT_CHAOTANG) == pd::OK) {
                                        if (role_->guanpin_gpin() != 0){
                                                update_chaotang_rank_func_(role_->gid(), role_->guanpin_gpin(), numeric_limits<int>::max() - role_->guanpin_idx());
                                        }
                                }
                        }

                        void update_marriage_devotion_rank() {
                                if (check_update_ranking_permit(pd::RT_MARRIAGE_DEVOTION) == pd::OK) {
                                        if (role_->marriage_married() && role_->marriage_proposer()) {
                                                update_marriage_devotion_rank_func_(role_->gid(), role_->marriage_spouse(), role_->relation_friend_intimacy(role_->marriage_spouse()));
                                        }
                                }
                        }

                        void update_tea_party_favor() {
                                if (check_update_ranking_permit(pd::RT_TEA_PARTY_FAVOR) == pd::OK) {
                                        update_tea_party_favor_func_(role_->gid(), role_->get_resource(pd::resource_type::TEA_PARTY_FAVOR));
                                }
                        }

                        void set_player_in_multiplayer_game(bool state) {
                                multiplayer_game_ = state;
                        }

                        bool check_player_in_multiplayer_game() {
                                if (multiplayer_game_) {
                                        return true;
                                }
                                return false;
                        }

                        void behavior_tree_start() {
                                if (ai_.robot_role_) {
                                        SPLAYER_TLOG << "set behavior_tree_root: " << ai_.behavior_children_node_;
                                        if (PTTS_HAS(behavior_tree, ai_.behavior_children_node_)) {
                                                set_cur_root(ai_.behavior_children_node_);
                                        }
                                        ai_set_active(true);
                                }
                        }

                        void reset_cur_mansion_game() {
                                cur_mansion_game_.type_ = pd::MGT_NONE;
                                cur_mansion_game_.free_extra_fishing_ = false;
                        }

                        string username_;
                        string origin_username_;
                        uint32_t server_id_ = 0;
                        shared_ptr<role> role_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<net::server<T>> nsv_;
                        shared_ptr<T> conn_;
                        shared_ptr<db::connector> gamedb_notice_;
                        shared_ptr<state_machine<player<T>>> sm_;
                        shared_ptr<behavior_tree<player<T>>> bt_;
                        string ip_;
                        pd::yunying_client_info yci_;

                        int fetching_data_count_ = 0;

                        atomic<int> async_call_count_{0};

                        map<pd::resource_type, shared_ptr<timer_type>> resource_grow_timer_;
                        set<shared_ptr<timer_type>> mirror_timers_;
                        shared_ptr<timer_type> rand_quit_timer_;
                        struct {
                                struct {
                                        queue<uint64_t> game_invitations_;
                                        pd::mansion_game_type game_ = pd::MGT_NONE;
                                        struct {
                                                uint32_t play_times_ = 0;
                                                bool took_bait_ = false;
                                        } fishing_;
                                        struct {
                                                struct {
                                                        uint32_t pttid_ = 0;
                                                        pd::pttid_array celebrities_;
                                                        pd::time_point open_time_;
                                                        bool allow_stranger_ = false;
                                                } host_info_;
                                                list<pd::mansion_banquet> hosted_;
                                                map<uint64_t, pd::mansion_banquet> reserved_;
                                                uint64_t mansion_ = 0;
                                                uint32_t pttid_ = 0;
                                                uint64_t spouse_ = 0;
                                                pd::mansion_building_type building_;
                                                bool dish_ = false;
                                                uint32_t riddle_ = 0;
                                                pd::mansion_building_type riddle_building_ = pd::MBT_NONE;
                                                map<pd::mansion_building_type, pd::mansion_banquet_riddle_box> riddle_boxes_;
                                                uint32_t question_ = 0;
                                                uint32_t thief_ = 0;
                                                pd::mansion_building_type thief_building_ = pd::MBT_NONE;
                                                map<uint32_t, pcs::mansion_banquet_celebrity> celebrities_;
                                                set<uint32_t> celebrities_challenged_;
                                                bool coins_ = false;
                                                uint32 coin_stage_level_ = 0;
                                                bool vote_bedroom_tool_ = false;
                                        } banquet_;
                                } mansion_;

                                struct {
                                        pd::fief_area_incident area_incident_;
                                        uint64_t enter_fief_role_ = 0;
                                } fief_incident_;

                                struct {
                                        pd::league_for_role data_;
                                        uint32_t refresh_time_ = 0;
                                } league_cache_;

                                struct {
                                        uint32_t update_day_  = 0;
                                } huanzhuang_pvp_;

                                system_clock::time_point last_chat_time_;

                                struct {
                                        map<uint32_t, set<uint32_t>> unlock_options_;
                                } plot_;

                                struct {
                                        bool started_qiu_qian_ = false;
                                        bool qiu_qian_started_by_spouse_ = false;
                                        bool qiu_qianing_ = false;
                                        uint32_t star_wish_start_time_ = 0;
                                        uint64_t star_wish_target_ = 0;
                                        uint32_t pk_start_time_ = 0;
                                        uint64_t pk_target_ = 0;
                                } marriage_;

                                bool robot_role_ = false;
                                uint32_t behavior_children_node_;
                        } ai_;

                        unique_ptr<pd::league> login_league_data_;
                        unique_ptr<pd::mansion> login_mansion_data_;
                        unique_ptr<pd::fief> login_fief_data_;
                        unique_ptr<pcs::gladiator> login_arena_champion_;
                        unique_ptr<pd::huanzhuang_pvp_data> login_huanzhuang_pvp_;
                        unique_ptr<pd::child_array> login_children_;
                        unique_ptr<pd::activity_mgr> login_activity_mgr_;

                        bool multiplayer_game_ = false;
                        bool mirrored_ = false;


                        bool quiting_playing_ = false;
                        bool mirror_role_ = false;
                        bool online_ = false;

                        struct {
                                pd::mansion_game_type type_ = pd::MGT_NONE;
                                bool free_extra_fishing_ = false;
                        } cur_mansion_game_;

                        unique_ptr<pd::role> mirror_role_data_;
                        struct {
                                struct {
                                        pd::role_mirror_event_type action_type_;
                                        uint64_t entered_mansion_ = 0;
                                        map<uint64_t, int> gid2action_count_;
                                        struct {
                                                map<uint64_t, pd::mansion_banquet> gid2banquets_;
                                                struct {
                                                        uint64_t mansion_gid_ = 0;
                                                        pd::mansion_banquet_stage current_stage_ = pd::MBS_NONE;
                                                        uint32_t question_ = 0;
                                                        map<pd::mansion_building_type, pd::mansion_banquet_riddle_box> riddle_boxes_;
                                                        int challenged_thief_idxs_ = 0;
                                                        pd::mansion_banquet_thieves thieves_;
                                                        map<uint32_t, pcs::mansion_banquet_celebrity> celebrities_;
                                                        pd::mansion_banquet_pintu_game pintu_games_;
                                                        vector<pd::mansion_banquet_weilie_game> weilie_game_;
                                                } live_;
                                        } banquet_;

                                        struct {
                                                int fetching_farm_count_ = 0;
                                                map<uint64_t, pd::mansion_farm> gid2farms_;
                                        } farm_;
                                } mansion_;

                                struct {
                                        uint64_t entered_fief_ = 0;
                                        int action_count_ = 0;
                                } fief_;

                        } mirror_role_ai_;

                        static map<pd::player_action_type, function<pd::result(player<T> *, pd::behavior_tree_node&)>> ai_action_handlers_;
                        static map<pd::player_condition_type, function<pd::result(player<T> *, pd::behavior_tree_node&)>> ai_condition_handlers_;

                        vector<function<void(pd::result, const pd::league_for_role&)>> fetch_league_data_call_funcs_;
                        vector<function<void(const list<pair<uint64_t, string>>&, const list<pair<uint64_t, string>>&)>> fetch_league_list_call_funcs_;
                        vector<function<void(const pd::mansion_banquets&)>> fetch_mansion_banquet_list_call_funcs_;
                        vector<function<void(pd::result, pd::room_type, const pd::room_array&)> > fetch_room_list_call_funcs_;
                        vector<function<void(uint64_t, pd::result, const pd::room&)>> fetch_room_info_call_funcs_;
                        vector<function<void(pd::result, const pd::marriage_star_wish_array&)>> fetch_marriage_star_wish_list_call_funcs_;
                        vector<function<void(pd::result, const pd::marriage_pk_array&)>> fetch_marriage_pk_list_call_funcs_;

                        map<uint64_t, vector<function<void()>>> mirror_role_fief_action_list_call_funcs_;
                        map<uint64_t, vector<function<void()>>> mirror_role_mansion_action_list_call_funcs_;
                };

                template <typename T>
                map<pd::player_action_type, function<pd::result(player<T> *, pd::behavior_tree_node&)>> player<T>::ai_action_handlers_;
                template <typename T>
                map<pd::player_condition_type, function<pd::result(player<T> *, pd::behavior_tree_node&)>> player<T>::ai_condition_handlers_;

        }
}
