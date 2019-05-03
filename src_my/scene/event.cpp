#include "event.hpp"
#include "proto/data_resource.pb.h"
#include "proto/data_item.pb.h"
#include "proto/data_spine.pb.h"
#include "proto/data_huanzhuang.pb.h"
#include "proto/data_log.pb.h"
#include "scene/player/role.hpp"
#include "scene/actor/actor.hpp"
#include "scene/actor/actor_logic.hpp"
#include "scene/item.hpp"
#include "scene/condition.hpp"
#include "scene/equipment/equipment.hpp"
#include "scene/activity/activity_mgr.hpp"
#include "scene/spine.hpp"
#include "scene/huanzhuang/huanzhuang.hpp"
#include "scene/drop.hpp"
#include "scene/yunying/bi.hpp"
#include "scene/db_log.hpp"
#include "config/adventure_ptts.hpp"
#include "config/skill_ptts.hpp"
#include "config/drop_ptts.hpp"
#include "config/item_ptts.hpp"
#include "config/resource_ptts.hpp"
#include "config/options_ptts.hpp"
#include "config/activity_ptts.hpp"
#include "utils/yunying.hpp"
#include <limits>

#define EVENT_TLOG __DLOG << setw(20) << "[EVENT] "
#define EVENT_DLOG __DLOG << setw(20) << "[EVENT] "
#define EVENT_ILOG __ILOG << setw(20) << "[EVENT] "
#define EVENT_ELOG __ELOG << setw(20) << "[EVENT] "

namespace pc = proto::config;

namespace nora {
        namespace scene {

                namespace {

                        set<pd::resource_type> bi_record_resource_types {
                                pd::GOLD,
                                        pd::DIAMOND,
                                        pd::EXP,
                                        pd::VIP_EXP,
                                        pd::POWER,
                                        pd::VIGOR,
                                        pd::BANQUET_PRESTIGE,
                                        pd::GIVE_GIFT,
                                        pd::RECEIVE_GIFT,
                                        pd::MANSION_FURNITURE_COIN,
                                        pd::LEAGUE_DONATION,
                                        pd::ARENA_REPUTATION,
                                        pd::TIAN_SHU_MO,
                                        pd::COUPON_A,
                                        pd::COUPON_B
                                        };

                }

                namespace {
                        pd::event_res_passed_adventure pass_adventure(const pc::adventure& ptt, nora::scene::role& role, uint32_t level) {
                                pd::event_res_passed_adventure passed_adventure;
                                passed_adventure.set_pttid(ptt.id());
                                if (ptt.has_battle_pttid()) {
                                        role.adventure_add_battle_passed(ptt.id());
                                        passed_adventure.set_latest_day(role.adventure_battle_passed_latest_day(ptt.id()));
                                        passed_adventure.set_times(role.adventure_battle_passed_times(ptt.id()));
                                } else if (ptt.has_plot_pttid()) {
                                        role.adventure_add_plot_passed(ptt.id());
                                } else {
                                        role.adventure_add_huanzhuang_passed(ptt.id(), level);
                                        passed_adventure.set_level(role.adventure_huanzhuang_passed_level(ptt.id()));
                                }
                                return passed_adventure;
                        }
                }

                pd::event_array drop_to_event(const pd::event_array& a) {
                        pd::event_array ret;
                        for (const auto& i : a.events()) {
                                switch (i.type()) {
                                case pd::event::DROP:{
                                        ASSERT(i.arg_size() == 2);
                                        const auto& ptt = PTTS_GET(drop, stoul(i.arg(0)));
                                        auto times = stoi(i.arg(1));
                                        for (auto n = 0; n < times; ++n) {
                                                auto events = drop_process(ptt);
                                                for (const auto& j : events.events()) {
                                                        *ret.add_events() = j;
                                                }
                                        }
                                        break;
                                }
                                default:
                                        *ret.add_events() = i;
                                        break;
                                }
                        }
                        return ret;
                }

                void event_merge(pd::event_array& a, const pd::event_array& b) {
                        vector<pd::event> append;
                        for (const auto& i : b.events()) {
                                auto merged = false;
                                switch (i.type()) {
                                case pd::event::ADD_RESOURCE:
                                        for (auto& j : *a.mutable_events()) {
                                                if (j.type() == i.type() && j.arg(0) == i.arg(0)) {
                                                        j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        break;
                                case pd::event::ADD_ITEM:
                                        for (auto& j : *a.mutable_events()) {
                                                if (j.type() == i.type() && j.arg(0) == i.arg(0)) {
                                                        j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        break;
                                case pd::event::CHANGE_INTIMACY:
                                        for (auto& j : *a.mutable_events()) {
                                                if (j.type() == i.type()) {
                                                        j.set_arg(0, to_string(stoi(i.arg(0)) + stoi(j.arg(0))));
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        break;
                                default:
                                        break;
                                }
                                if (!merged) {
                                        append.push_back(i);
                                }
                        }

                        for (const auto& i : append) {
                                *a.add_events() = i;
                        }
                }

                void event_adopt_env(pd::event_array& ea, const pd::ce_env& ce) {
                        for (auto& i : *ea.mutable_events()) {
                                switch (i.type()) {
                                case pd::event::DEC_RESOURCE: {
                                        ASSERT(i.arg_size() == 2);
                                        if (ce.has_cost_percent()) {
                                                auto cost = stoi(i.arg(1)) * ce.cost_percent() / 1000;
                                                i.set_arg(1, to_string(cost));
                                        }
                                        break;
                                }
                                case pd::event::REMOVE_ITEM: {
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_item());
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.item()));
                                                i.add_arg(arg);
                                        }
                                        ASSERT(i.arg_size() == 2);
                                        if (ce.has_cost_percent()) {
                                                auto cost = stoi(i.arg(1)) * ce.cost_percent() / 1000;
                                                i.set_arg(1, to_string(cost));
                                        }
                                        break;
                                }
                                case pd::event::ADD_RESOURCE:
                                case pd::event::ADD_RESOURCE_IGNORE_MAX: {
                                        ASSERT(i.arg_size() == 2);
                                        if (ce.has_add_percent()) {
                                                auto add = stoi(i.arg(1)) * ce.add_percent() / 1000;
                                                i.set_arg(1, to_string(add));
                                        }
                                        break;
                                }
                                case pd::event::ADD_ITEM: {
                                        ASSERT(i.arg_size() == 2);
                                        if (ce.has_add_percent()) {
                                                auto add = stoi(i.arg(1)) * ce.add_percent() / 1000;
                                                i.set_arg(1, to_string(add));
                                        }
                                        break;
                                }
                                case pd::event::CHANGE_ACTOR_INTIMACY: {
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.actors_size() == 2);
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.add_arg(to_string(ce.actors(1)));
                                                i.add_arg(arg);
                                        } else if (i.arg_size() == 2) {
                                                ASSERT(ce.actors_size() == 1);
                                                auto arg_a = i.arg(0);
                                                auto arg_b = i.arg(1);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.set_arg(1, arg_a);
                                                i.add_arg(arg_b);
                                        } else if (i.arg_size() == 3) {

                                        } else {
                                                ASSERT(false);
                                        }
                                        if (ce.has_intimacy_gain_percent() || ce.has_add_percent()) {
                                                auto value = stoul(i.arg(2));
                                                value = value * (ce.intimacy_gain_percent() + ce.add_percent()) / 1000;
                                                i.set_arg(2, to_string(value));
                                        }
                                        break;
                                }
                                case pd::event::CHANGE_INTIMACY: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_other());
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.other()));
                                                i.add_arg(arg);
                                        }
                                        if (ce.has_intimacy_gain_percent() || ce.has_add_percent()) {
                                                auto value = stoul(i.arg(1));
                                                value = value * (ce.intimacy_gain_percent() + ce.add_percent()) / 1000;
                                                i.set_arg(1, to_string(value));
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_PIN: {
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.actors_size() == 1);
                                                i.add_arg(to_string(ce.actors(0)));
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_STAR: {
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.actors_size() == 1);
                                                i.add_arg(to_string(ce.actors(0)));
                                        }
                                        break;
                                }
                                case pd::event::LEVELUP_ACTOR_SKILL:
                                case pd::event::ACTOR_UNLOCK_SKILL: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.actors_size() == 1);
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.add_arg(arg);
                                        }
                                        break;
                                }
                                case pd::event::DEC_GONGDOU_EFFECT: {
                                        if (ce.has_cost_percent()) {
                                                if (i.arg_size() == 1) {
                                                        auto cost = stoi(i.arg(0)) * ce.cost_percent() / 1000;
                                                        i.set_arg(0, to_string(cost));
                                                } else if (i.arg_size() == 2) {
                                                        auto cost = stoi(i.arg(1)) * ce.cost_percent() / 1000;
                                                        i.set_arg(1, to_string(cost));
                                                } else {
                                                        ASSERT(false);
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ADD_EQUIPMENT_EXP: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_equipment());
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.equipment()));
                                                i.add_arg(arg);
                                        }
                                        break;
                                }
                                case pd::event::EQUIPMENT_ADD_QUALITY: {
                                        ASSERT(i.arg_size() == 0 || i.arg_size() == 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_equipment());
                                                i.add_arg(to_string(ce.equipment()));
                                        }
                                        break;
                                }
                                case pd::event::EQUIPMENT_CHANGE_RAND_ATTR: {
                                        ASSERT(i.arg_size() == 0);
                                        ASSERT(ce.has_equipment() && ce.idx().idx_size() == 1);
                                        i.add_arg(to_string(ce.equipment()));
                                        i.add_arg(to_string(ce.idx().idx(0)));
                                        break;
                                }
                                case pd::event::EQUIPMENT_XILIAN: {
                                        ASSERT(i.arg_size() == 0);
                                        ASSERT(ce.has_equipment() && ce.has_idx());
                                        i.add_arg(to_string(ce.equipment()));
                                        for (auto j : ce.idx().idx()) {
                                                i.add_arg(to_string(j));
                                        }
                                        break;
                                }
                                case pd::event::EQUIPMENT_XILIAN_CONFIRM: {
                                        ASSERT(i.arg_size() == 0);
                                        ASSERT(ce.has_equipment());
                                        i.add_arg(to_string(ce.equipment()));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                pd::event_res event_process(const pd::event_array& ea, role& role, const pd::ce_env& ce, const pd::event_res *merge_res, bool send_bi) {
                        auto events = ea;

                        auto options_ptt = PTTS_GET_COPY(options, 1);
                        auto cost_free = options_ptt.scene_info().cost_free();
                        auto gain_times = options_ptt.scene_info().gain_times();
                        auto mutable_ce = ce;
                        if (cost_free) {
                                mutable_ce.set_cost_percent(0);
                        }
                        if (gain_times > 1) {
                                if (ce.has_add_percent()) {
                                        mutable_ce.set_add_percent(gain_times * ce.add_percent());
                                } else {
                                        mutable_ce.set_add_percent(gain_times * 1000);
                                }
                                if (ce.has_intimacy_gain_percent()) {
                                        mutable_ce.set_intimacy_gain_percent(gain_times * ce.intimacy_gain_percent());
                                } else {
                                        mutable_ce.set_intimacy_gain_percent(gain_times * 1000);
                                }
                        }
                        events = drop_to_event(events);
                        event_adopt_env(events, mutable_ce);

                        EVENT_TLOG << "process events ce\n" << ce.DebugString();
                        EVENT_TLOG << "process events\n" << events.DebugString();

                        pd::event_res res;
                        if (merge_res) {
                                res = *merge_res;
                        }

                        for (const auto& i : events.events()) {
                                switch (i.type()) {
                                case pd::event::ADD_RESOURCE:
                                case pd::event::ADD_RESOURCE_IGNORE_MAX: {
                                        ASSERT(i.arg_size() == 2);
                                        auto merged = false;
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        auto old_count = role.get_resource(type);
                                        auto add_count = stol(i.arg(1));

                                        if (i.type() == pd::event::ADD_RESOURCE) {
                                                const auto& ptts = PTTS_GET_ALL(resource);
                                                for (const auto& i : ptts) {
                                                        if (i.second.type() == type) {
                                                                if (i.second.has_max_value()) {
                                                                        add_count = min(add_count, i.second.max_value() - old_count);
                                                                }
                                                        }
                                                }
                                        }
                                        if (add_count <= 0) {
                                                continue;
                                        }
                                        auto cur_count = role.change_resource(type, stoi(i.arg(1)));
                                        for (auto i = 0; i < res.change_resources_size(); ++i) {
                                                auto *r = res.mutable_change_resources(i);
                                                if (r->type() == type) {
                                                        r->set_cur_count(cur_count);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *r = res.add_change_resources();
                                                r->set_type(type);
                                                r->set_old_count(old_count);
                                                r->set_cur_count(cur_count);
                                        }
                                        break;
                                }
                                case pd::event::DEC_RESOURCE: {
                                        ASSERT(i.arg_size() == 2);
                                        auto cost = stoi(i.arg(1));
                                        if (cost <= 0) {
                                                break;
                                        }
                                        bool merged = false;
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        auto old_count = role.get_resource(type);
                                        auto cur_count = role.change_resource(type, -cost);
                                        for (auto i = 0; i < res.change_resources_size(); ++i) {
                                                auto *r = res.mutable_change_resources(i);
                                                if (r->type() == type) {
                                                        r->set_cur_count(cur_count);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *r = res.add_change_resources();
                                                r->set_type(type);
                                                r->set_old_count(old_count);
                                                r->set_cur_count(cur_count);
                                        }
                                        break;
                                }
                                case pd::event::RESET_RESOURCE: {
                                        ASSERT(i.arg_size() == 2);
                                        auto reset_count = stoi(i.arg(1));
                                        ASSERT(reset_count > 0);
                                        bool merged = false;
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        auto old_count = role.get_resource(type);
                                        auto cur_count = role.change_resource(type, reset_count - old_count);
                                        for (auto i = 0; i < res.change_resources_size(); ++i) {
                                                auto *r = res.mutable_change_resources(i);
                                                if (r->type() == type) {
                                                        r->set_cur_count(cur_count);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *r = res.add_change_resources();
                                                r->set_type(type);
                                                r->set_old_count(old_count);
                                                r->set_cur_count(cur_count);
                                        }
                                        break;
                                }
                                case pd::event::ADD_ACTOR: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        const auto& ptt = PTTS_GET(actor, pttid);
                                        ASSERT(ptt.type() != pd::actor::ZHU);
                                        if (!role.has_actor(pttid)) {
                                                auto actor = role.add_actor(pttid);
                                                auto *a =  res.add_add_actors();
                                                actor->serialize(a);
                                        } else {
                                                ASSERT(ptt.has_xinwu());
                                                auto item_pttid = ptt.xinwu();
                                                ASSERT(item_pttid > 0);
                                                const auto& logic_ptt = PTTS_GET(actor_logic, 1);
                                                auto old_count = role.item_count(item_pttid);
                                                auto add_count = logic_ptt.exchange_xinwu_count();
                                                ASSERT(add_count >= 0);
                                                if (add_count == 0) {
                                                        break;
                                                }
                                                auto item = role.add_item(item_pttid, add_count);
                                                bool merged = false;
                                                for (auto i = 0; i < res.add_items_size(); ++i) {
                                                        auto *pi = res.mutable_add_items(i);
                                                        if (pi->pttid() == item->pttid()) {
                                                                pi->set_cur_count(item->count());
                                                                merged = true;
                                                                break;
                                                        }
                                                }
                                                if (!merged) {
                                                        auto *i = res.add_add_items();
                                                        i->set_pttid(item->pttid());
                                                        i->set_old_count(old_count);
                                                        i->set_cur_count(item->count());
                                                        i->set_from_actor(pttid);
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ADD_ITEM: {
                                        ASSERT(i.arg_size() >= 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto old_count = role.item_count(pttid);
                                        auto add_count = stoi(i.arg(1));
                                        ASSERT(add_count >= 0);
                                        if (add_count == 0) {
                                                break;
                                        }
                                        const auto& ptt = PTTS_GET(item, pttid);
                                        if (ptt._xinwu_actor() && !role.has_actor(ptt._xinwu_actor())) {
                                                ASSERT(PTTS_HAS(actor, ptt._xinwu_actor()));
                                                const auto& actor_ptt = PTTS_GET(actor, ptt._xinwu_actor());
                                                ASSERT(PTTS_HAS(actor_add_star, actor_ptt.add_star_pttid()));
                                                const auto& actor_add_star_ptt = PTTS_GET(actor_add_star, actor_ptt.add_star_pttid());
                                                ASSERT(actor_add_star_ptt.stars_size() > 0);
                                                auto cost_item_conditions = condition_find(actor_add_star_ptt.stars(0).conditions(), pd::condition::COST_ITEM);
                                                ASSERT(cost_item_conditions.size() == 1);
                                                auto count = stoi(cost_item_conditions[0].arg(0));
                                                if (add_count >= count) {
                                                        add_count -= count;
                                                        auto actor = role.add_actor(pttid);
                                                        auto *a =  res.add_add_actors();
                                                        actor->serialize(a);
                                                }
                                        }
                                        if (add_count == 0) {
                                                break;
                                        }
                                        auto item = role.add_item(pttid, add_count);
                                        bool merged = false;
                                        for (auto i = 0; i < res.add_items_size(); ++i) {
                                                auto *pi = res.mutable_add_items(i);
                                                if (pi->pttid() == item->pttid()) {
                                                        pi->set_cur_count(item->count());
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *i = res.add_add_items();
                                                i->set_pttid(item->pttid());
                                                i->set_old_count(old_count);
                                                i->set_cur_count(item->count());
                                        }
                                        break;
                                }
                                case pd::event::ADD_EQUIPMENT: {
                                        ASSERT(i.arg_size() == 2);
                                        const auto& equipment = role.add_equipment(stoul(i.arg(0)), stoul(i.arg(1)));
                                        auto *ae = res.add_add_equipments();
                                        equipment->serialize(ae);
                                        break;
                                }
                                case pd::event::ADD_JADE: {
                                        ASSERT(i.arg_size() == 2);
                                        auto count = stoi(i.arg(1));
                                        for (auto j = 0; j < count; ++j) {
                                                auto jade = role.add_jade(stoul(i.arg(0)));
                                                *res.add_add_jades() = jade;
                                        }
                                        break;
                                }
                                case pd::event::ADD_DAIYANREN_POINTS: {
                                        ASSERT(i.arg_size() == 1);
                                        activity_mgr::instance().daiyanren_add_points(stoul(i.arg(0)));
                                        res.set_daiyanren_point(activity_mgr::instance().daiyanren_points());
                                        break;
                                }
                                case pd::event::REMOVE_JADE: {
                                        ASSERT(i.arg_size() == 1);
                                        auto gid = stoul(i.arg(0));
                                        ASSERT(role.has_jade(gid));
                                        role.remove_jade(gid);
                                        res.add_remove_jades(gid);
                                        break;
                                }
                                case pd::event::REMOVE_ITEM: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto old_count = role.item_count(pttid);
                                        auto item = role.remove_item(pttid, stoi(i.arg(1)));
                                        ASSERT(item);
                                        bool merged = false;
                                        for (auto i = 0; i < res.dec_items_size(); ++i) {
                                                auto *pi = res.mutable_dec_items(i);
                                                if (pi->pttid() == item->pttid()) {
                                                        if (item) {
                                                                pi->set_cur_count(item->count());
                                                        } else {
                                                                pi->set_cur_count(0);
                                                        }
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *i = res.add_dec_items();
                                                i->set_pttid(pttid);
                                                i->set_old_count(old_count);
                                                if (item) {
                                                        i->set_cur_count(item->count());
                                                } else {
                                                        i->set_cur_count(0);
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ACCEPT_QUEST: {
                                        ASSERT(i.arg_size() == 1);
                                        uint32_t pttid = stoul(i.arg(0));
                                        if (role.quest_has_accepted(pttid)) {
                                                break;
                                        }
                                        role.quest_accept(pttid);
                                        role.serialize_accepted_quest(pttid, res.add_accept_quests());
                                        break;
                                }
                                case pd::event::REMOVE_QUEST: {
                                        ASSERT(i.arg_size() == 1);
                                        uint32_t pttid = stoul(i.arg(0));
                                        if (!role.quest_has_accepted(pttid)) {
                                                break;
                                        }
                                        role.quest_remove(pttid);
                                        res.add_remove_quests(pttid);
                                        break;
                                }
                                case pd::event::ADD_BUY_GOOD_TIMES: {
                                        ASSERT(i.arg_size() == 2);
                                        bool merged = false;
                                        auto pttid = stoul(i.arg(0));
                                        auto good_id = stoul(i.arg(1));
                                        const auto& record = role.shop_add_buy_times(pttid, good_id);
                                        for (auto i = 0; i < res.shop_records_size(); ++i) {
                                                auto *sr = res.mutable_shop_records(i);
                                                if (sr->shop_pttid() == pttid && sr->good_id() == good_id) {
                                                        sr->set_shop_day(record.shop_day_);
                                                        sr->set_times(record.times_);
                                                        sr->set_all_times(record.all_times_);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *sr = res.add_shop_records();
                                                sr->set_shop_pttid(pttid);
                                                sr->set_shop_day(record.shop_day_);
                                                sr->set_good_id(good_id);
                                                sr->set_times(record.times_);
                                                sr->set_all_times(record.all_times_);
                                        }
                                        break;
                                }
                                case pd::event::REMOVE_BUY_GOOD_TIMES: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto good_id = stoul(i.arg(1));
                                        role.reset_shop_records_times(pttid, good_id);
                                        auto* rsr = res.add_remove_shop_records();
                                        rsr->set_shop(pttid);
                                        rsr->set_good(good_id);
                                        break;
                                }
                                case pd::event::PASS_ADVENTURE: {
                                        ASSERT(i.arg_size() != 0);
                                        auto level = numeric_limits<uint32_t>::max();
                                        if (i.arg_size() == 2) {
                                                level = stoul(i.arg(1));
                                        }
                                        const auto& ptt = PTTS_GET(adventure, stoul(i.arg(0)));
                                        *res.add_passed_adventures() = pass_adventure(ptt, role, level);
                                        break;
                                }
                                case pd::event::PASS_ADVENTURE_CHAPTER: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pass_score = 0u;
                                        const auto& chapter = PTTS_GET(adventure_chapter, stoul(i.arg(0)));
                                        for (auto j : chapter.adventures()) {
                                                const auto& ptt = PTTS_GET(adventure, j);
                                                *res.add_passed_adventures() = pass_adventure(ptt, role, pass_score);
                                        }
                                        break;
                                }
                                case pd::event::PASS_ADVENTURE_ALL: {
                                        ASSERT(i.arg_size() == 0);
                                        auto pass_score = 0u;
                                        const auto& ptts = PTTS_GET_ALL(adventure);
                                        for (const auto& j : ptts) {
                                                *res.add_passed_adventures() = pass_adventure(j.second, role, pass_score);
                                        }
                                        break;
                                }
                                case pd::event::PASS_LIEMING: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.lieming_passed(pttid)) {
                                                role.lieming_add_passed(pttid);
                                                res.set_passed_lieming(pttid);
                                        }
                                        break;
                                }
                                case pd::event::LEVELUP_ACTOR_SKILL: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        ASSERT(role.has_actor(pttid));
                                        auto skill_pttid = stoul(i.arg(1));
                                        auto new_skill_pttid = role.levelup_actor_skill(pttid, skill_pttid);
                                        auto *als = res.add_actor_levelup_skill();
                                        als->set_pttid(pttid);
                                        als->set_old_skill_pttid(skill_pttid);
                                        als->set_new_skill_pttid(new_skill_pttid);
                                        break;
                                }
                                case pd::event::ACTOR_UNLOCK_SKILL: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto skill_pttid = stoul(i.arg(1));
                                        ASSERT(role.has_actor(pttid));
                                        role.unlock_actor_skill(pttid, skill_pttid);
                                        auto *aus = res.add_actor_unlock_skills();
                                        aus->set_pttid(pttid);
                                        aus->set_skill_pttid(skill_pttid);
                                        break;
                                }
                                case pd::event::ADD_SPINE_COLLECTION: {
                                        ASSERT(i.arg_size() <= 4);
                                        auto part_arg = 0;
                                        auto pttid_arg = 1;

                                        if (i.arg_size() == 4 && role.gender() == pd::MALE) {
                                                part_arg = 2;
                                                pttid_arg = 3;
                                        }
                                        pd::spine_part part;
                                        ASSERT(pd::spine_part_Parse(i.arg(part_arg), &part));
                                        auto pttid = stoul(i.arg(pttid_arg));
                                        if (!role.spine_has_collection(part, pttid)) {
                                                role.spine_add_collection(part, pttid);
                                                auto *sc = res.add_spine_collections();
                                                sc->set_part(part);
                                                sc->set_pttid(pttid);
                                        }
                                        break;
                                }
                                case pd::event::ADD_HUANZHUANG_COLLECTION: {
                                        ASSERT(i.arg_size() <= 4);
                                        auto part_arg = 0;
                                        auto pttid_arg = 1;
                                        if (i.arg_size() == 4 && role.gender() == pd::MALE) {
                                                part_arg = 2;
                                                pttid_arg = 3;
                                        }
                                        pd::huanzhuang_part part;
                                        ASSERT(pd::huanzhuang_part_Parse(i.arg(part_arg), &part));
                                        auto pttid = stoul(i.arg(pttid_arg));
                                        if (!role.huanzhuang_has_collection(part, pttid)) {
                                                role.huanzhuang_add_collection(part, pttid);
                                                auto *sc = res.add_huanzhuang_collections();
                                                sc->set_part(part);
                                                sc->set_pttid(pttid);
                                                sc->set_level(1);
                                                sc->set_exp(0);
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_LEVELUP: {
                                        ASSERT(i.arg_size() == 1);
                                        bool merged = false;
                                        auto pttid = stoul(i.arg(0));
                                        ASSERT(role.has_actor(pttid));
                                        const auto& actor = role.get_actor(stoul(i.arg(0)));
                                        auto old = actor->level();
                                        role.actor_levelup(pttid);
                                        auto cur = actor->level();
                                        for (auto i = 0; i < res.actor_levelup_size(); ++i) {
                                                auto *al = res.mutable_actor_levelup(i);
                                                if (al->pttid() == pttid) {
                                                        al->set_cur_level(cur);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *a = res.add_actor_levelup();
                                                a->set_pttid(pttid);
                                                a->set_old_level(old);
                                                a->set_cur_level(cur);
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_STAR: {
                                        ASSERT(i.arg_size() == 1);
                                        bool merged = false;
                                        auto pttid = stoul(i.arg(0));
                                        if (role.has_actor(pttid)) {
                                                const auto& actor = role.get_actor(pttid);
                                                auto old = actor->star();
                                                role.actor_add_star(pttid);
                                                auto cur = actor->star();
                                                for (auto i = 0; i < res.actor_add_star_size(); ++i) {
                                                        auto *aas = res.mutable_actor_add_star(i);
                                                        if (aas->pttid() == pttid) {
                                                                aas->set_cur_star(cur);
                                                                merged = true;
                                                                break;
                                                        }
                                                }
                                                if (!merged) {
                                                        auto *a = res.add_actor_add_star();
                                                        a->set_pttid(pttid);
                                                        a->set_old_star(old);
                                                        a->set_cur_star(cur);
                                                }
                                        } else {
                                                auto actor = role.add_actor(pttid);
                                                auto *a =  res.add_add_actors();
                                                actor->serialize(a);
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_STEP: {
                                        ASSERT(i.arg_size() == 1);
                                        bool merged = false;
                                        auto pttid = stoul(i.arg(0));
                                        const auto& actor = role.get_actor(pttid);
                                        auto old = actor->step();
                                        role.actor_add_step(pttid);
                                        auto cur = actor->step();
                                        for (auto i = 0; i < res.actor_add_step_size(); ++i) {
                                                auto *aas = res.mutable_actor_add_step(i);
                                                if (aas->pttid() == pttid) {
                                                        aas->set_cur_step(cur);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *a = res.add_actor_add_step();
                                                a->set_pttid(pttid);
                                                a->set_old_step(old);
                                                a->set_cur_step(cur);
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_PIN: {
                                        ASSERT(i.arg_size() == 1);
                                        auto merged = false;
                                        auto pttid = stoul(i.arg(0));
                                        ASSERT(role.has_actor(pttid));
                                        const auto& actor = role.get_actor(pttid);
                                        auto old = actor->pin();
                                        role.actor_add_pin(pttid);
                                        auto cur = actor->pin();
                                        for (auto i = 0; i < res.actor_add_pin_size(); ++i) {
                                                auto *aas = res.mutable_actor_add_pin(i);
                                                if (aas->pttid() == pttid) {
                                                        aas->set_cur_pin(cur);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *a = res.add_actor_add_pin();
                                                a->set_pttid(pttid);
                                                a->set_old_pin(old);
                                                a->set_cur_pin(cur);
                                        }
                                        break;
                                }
                                case pd::event::EQUIPMENT_LEVELUP: {
                                        ASSERT(i.arg_size() == 1);
                                        bool merged = false;
                                        auto gid = stoull(i.arg(0));
                                        auto equipment = role.get_equipment(gid);
                                        auto old_level = equipment->level();
                                        role.equipment_levelup(gid);
                                        auto cur_level = equipment->level();
                                        for (auto i = 0; i < res.equipment_levelup_size(); ++i) {
                                                auto *el = res.mutable_equipment_levelup(i);
                                                if (el->gid() == gid) {
                                                        el->set_cur_level(cur_level);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *a = res.add_equipment_levelup();
                                                a->set_gid(gid);
                                                a->set_old_level(old_level);
                                                a->set_cur_level(cur_level);
                                        }
                                        break;
                                }
                                case pd::event::EQUIPMENT_ADD_QUALITY: {
                                        ASSERT(i.arg_size() == 1);
                                        auto gid = stoull(i.arg(0));
                                        auto equipment = role.get_equipment(gid);
                                        role.equipment_add_quality(gid);
                                        auto *eaq = res.add_equipment_add_quality();
                                        equipment->serialize(eaq);
                                        break;
                                }
                                case pd::event::EQUIPMENT_CHANGE_RAND_ATTR: {
                                        ASSERT(i.arg_size() == 2);
                                        auto gid = stoull(i.arg(0));
                                        auto idx = stoul(i.arg(1));
                                        role.equipment_change_rand_attr(gid, idx);
                                        auto equipment = role.get_equipment(gid);
                                        auto *ecra = res.add_equipment_change_rand_attr();
                                        equipment->serialize(ecra);
                                        break;
                                }
                                case pd::event::EQUIPMENT_XILIAN: {
                                        ASSERT(i.arg_size() >= 1);
                                        auto gid = stoull(i.arg(0));
                                        pd::idx_array idx;
                                        for (auto j = 1; j < i.arg_size(); ++j) {
                                                idx.add_idx(stoi(i.arg(j)));
                                        }
                                        role.equipment_xilian(gid, idx);
                                        auto equipment = role.get_equipment(gid);
                                        auto *ex = res.add_equipment_xilian();
                                        equipment->serialize(ex);
                                        break;
                                }
                                case pd::event::EQUIPMENT_XILIAN_CONFIRM: {
                                        ASSERT(i.arg_size() >= 1);
                                        auto gid = stoull(i.arg(0));
                                        role.equipment_xilian_confirm(gid);
                                        auto equipment = role.get_equipment(gid);
                                        auto *exc = res.add_equipment_xilian_confirm();
                                        equipment->serialize(exc);
                                        break;
                                }
                                case pd::event::LEVELUP: {
                                        ASSERT(i.arg_size() == 0);
                                        bool merged = false;
                                        auto old_level = role.level();
                                        role.levelup();
                                        if (res.has_role_levelup()) {
                                                res.mutable_role_levelup()->set_cur_level(role.level());
                                                merged = true;
                                        }
                                        if (!merged) {
                                                res.mutable_role_levelup()->set_old_level(old_level);
                                                res.mutable_role_levelup()->set_cur_level(role.level());
                                                res.mutable_role_levelup()->set_levelup_time(role.last_levelup_time());
                                        }
                                        break;
                                }
                                case pd::event::ROLE_ADD_FATE: {
                                        ASSERT(i.arg_size() == 0);
                                        bool merged = false;
                                        auto old_fate = role.fate();
                                        role.add_fate();
                                        if (res.has_role_add_fate()) {
                                                res.mutable_role_add_fate()->set_cur_fate(role.fate());
                                                merged = true;
                                        }
                                        if (!merged) {
                                                res.mutable_role_add_fate()->set_old_fate(old_fate);
                                                res.mutable_role_add_fate()->set_cur_fate(role.fate());
                                        }
                                        break;
                                }
                                case pd::event::CHANGE_ACTOR_INTIMACY: {
                                        ASSERT(i.arg_size() == 3);
                                        bool merged = false;
                                        auto actor_a = stoul(i.arg(0));
                                        auto actor_b = stoul(i.arg(1));
                                        ASSERT(actor_a != actor_b);
                                        auto old_intimacy = role.actor_intimacy(actor_a, actor_b);
                                        auto cur_intimacy = role.actor_change_intimacy(actor_a, actor_b, stoi(i.arg(2)));
                                        auto role_actor = role.get_role_actor();
                                        ASSERT(role_actor->pttid() != actor_a && role_actor->pttid() != actor_b);
                                        for (auto i = 0; i < res.change_actor_intimacy_size(); ++i) {
                                                auto *aaf = res.mutable_change_actor_intimacy(i);
                                                if ((aaf->actors(0) == actor_a && aaf->actors(1) == actor_b) ||
                                                    (aaf->actors(1) == actor_a && aaf->actors(0) == actor_b)) {
                                                        aaf->set_cur_intimacy(cur_intimacy);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *a = res.add_change_actor_intimacy();
                                                a->add_actors(actor_a);
                                                a->add_actors(actor_b);
                                                a->set_old_intimacy(old_intimacy);
                                                a->set_cur_intimacy(cur_intimacy);
                                        }
                                        break;
                                }
                                case pd::event::REMOVE_EQUIPMENT: {
                                        ASSERT(i.arg_size() == 1);
                                        auto gid = stoull(i.arg(0));
                                        const auto& equipment = role.get_equipment(gid);

                                        role.remove_equipment(gid);
                                        auto *remove_equipments = res.add_remove_equipments();
                                        remove_equipments->set_pttid(equipment->pttid());
                                        remove_equipments->set_gid(gid);
                                        break;
                                }
                                case pd::event::ADD_EQUIPMENT_EXP: {
                                        ASSERT(i.arg_size() == 2);
                                        bool merged = false;
                                        auto gid = stoull(i.arg(0));
                                        const auto& equipment = role.get_equipment(gid);
                                        auto old_exp = equipment->exp();
                                        role.equipment_add_exp(gid, stoi(i.arg(1)));
                                        auto cur_exp = equipment->exp();
                                        for (auto i = 0; i < res.add_equipment_exp_size(); ++i) {
                                                auto *aee = res.mutable_add_equipment_exp(i);
                                                if (aee->gid() == gid) {
                                                        aee->set_cur_exp(cur_exp);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *aee = res.add_add_equipment_exp();
                                                aee->set_gid(gid);
                                                aee->set_old_exp(old_exp);
                                                aee->set_cur_exp(cur_exp);
                                        }
                                        break;
                                }
                                case pd::event::ADD_HUANZHUANG_EXP: {
                                        ASSERT(i.arg_size() == 3);
                                        auto merged = false;
                                        pd::huanzhuang_part part;
                                        pd::huanzhuang_part_Parse(i.arg(0), &part);
                                        uint32_t pttid = stoul(i.arg(1));
                                        const auto& huanzhuang = role.huanzhuang_get_collection(part, pttid);
                                        auto old_exp = huanzhuang->exp();
                                        role.huanzhuang_add_exp(part, pttid, stoi(i.arg(2)));
                                        auto cur_exp = huanzhuang->exp();
                                        for (auto i = 0; i < res.change_huanzhuang_exp_size(); ++i) {
                                                auto *che = res.mutable_change_huanzhuang_exp(i);
                                                if (che->part() == part && che->pttid() == pttid) {
                                                        che->set_cur_exp(cur_exp);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *che = res.add_change_huanzhuang_exp();
                                                che->set_part(part);
                                                che->set_pttid(pttid);
                                                che->set_old_exp(old_exp);
                                                che->set_cur_exp(cur_exp);
                                        }
                                        break;
                                }
                                case pd::event::RESET_HUANZHUANG: {
                                        ASSERT(i.arg_size() == 2);
                                        pd::huanzhuang_part part;
                                        pd::huanzhuang_part_Parse(i.arg(0), &part);
                                        uint32_t pttid = stoul(i.arg(1));
                                        role.huanzhuang_reset(part, pttid);
                                        auto *hr = res.add_huanzhuang_reset();
                                        hr->set_part(part);
                                        hr->set_pttid(pttid);
                                        break;
                                }
                                case pd::event::HUANZHUANG_LEVELUP: {
                                        ASSERT(i.arg_size() == 2);
                                        auto merged = false;
                                        pd::huanzhuang_part part;
                                        pd::huanzhuang_part_Parse(i.arg(0), &part);
                                        uint32_t pttid = stoul(i.arg(1));
                                        const auto& huanzhuang = role.huanzhuang_get_collection(part, pttid);
                                        auto old_level = huanzhuang->level();
                                        role.huanzhuang_levelup(part, pttid);
                                        auto cur_level = huanzhuang->level();
                                        for (auto i = 0; i < res.huanzhuang_levelup_size(); ++i) {
                                                auto *hl = res.mutable_huanzhuang_levelup(i);
                                                if (hl->part() == part && hl->pttid() == pttid) {
                                                        hl->set_cur_level(cur_level);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *hl = res.add_huanzhuang_levelup();
                                                hl->set_part(part);
                                                hl->set_pttid(pttid);
                                                hl->set_old_level(old_level);
                                                hl->set_cur_level(cur_level);
                                        }
                                        break;
                                }
                                case pd::event::BROADCAST_SYSTEM_CHAT: {
                                        ASSERT(i.arg_size() == 1);
                                        uint32_t pttid = stoul(i.arg(0));
                                        role.broadcast_system_chat(pttid);
                                        break;
                                }
                                case pd::event::DROP: {
                                        ASSERT(false);
                                        break;
                                }
                                case pd::event::CHANGE_INTIMACY: {
                                        ASSERT(i.arg_size() == 2);
                                        bool merged = false;
                                        auto other = stoul(i.arg(0));
                                        if (!role.relation_has_friend(other)) {
                                                break;
                                        }
                                        auto old_value = role.relation_friend_intimacy(other);
                                        auto cur_value = role.relation_friend_change_intimacy(other, stoi(i.arg(1)));
                                        for (auto& i : *res.mutable_friend_intimacy()) {
                                                if (i.other() == other) {
                                                        i.set_cur_value(cur_value);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *aee = res.add_friend_intimacy();
                                                aee->set_other(other);
                                                aee->set_old_value(old_value);
                                                aee->set_cur_value(cur_value);
                                        }
                                        break;
                                }
                                case pd::event::ADD_TITLE: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        role.add_title(pttid);
                                        *res.add_add_titles() = role.get_title(pttid);
                                        break;
                                }
                                case pd::event::ADD_PRESENT: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto old_count = role.present_count(pttid);
                                        role.add_present(pttid, stoi(i.arg(1)));
                                        bool merged = false;
                                        for (auto i = 0; i < res.change_presents_size(); ++i) {
                                                auto *cp = res.mutable_change_presents(i);
                                                if (cp->pttid() == pttid) {
                                                        cp->set_cur_count(role.present_count(pttid));
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *cp = res.add_change_presents();
                                                cp->set_pttid(pttid);
                                                cp->set_old_count(old_count);
                                                cp->set_cur_count(role.present_count(pttid));
                                        }
                                        break;
                                }
                                case pd::event::DEC_PRESENT: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto old_count = role.present_count(pttid);
                                        role.dec_present(pttid, stoul(i.arg(1)));
                                        bool merged = false;
                                        for (auto i = 0; i < res.change_presents_size(); ++i) {
                                                auto *dp = res.mutable_change_presents(i);
                                                if (dp->pttid() == pttid) {
                                                        dp->set_cur_count(role.present_count(pttid));
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *dp = res.add_change_presents();
                                                dp->set_pttid(pttid);
                                                dp->set_old_count(old_count);
                                                dp->set_cur_count(role.present_count(pttid));
                                        }
                                        break;
                                }
                                case pd::event::MANSION_ADD_HALL_QUEST: {
                                        ASSERT(i.arg_size() == 0);
                                        *res.add_mansion_add_hall_quest() = role.mansion_add_hall_quest();
                                        break;
                                }
                                case pd::event::PLOT_LOCK_OPTION: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto option_idx = stoi(i.arg(1));
                                        if (!role.plot_option_locked(pttid, option_idx)) {
                                                role.plot_lock_option(pttid, option_idx);
                                                auto *plo = res.add_plot_lock_options();
                                                plo->set_pttid(pttid);
                                                plo->set_option_idx(option_idx);
                                        }
                                        break;
                                }
                                case pd::event::PLOT_UNLOCK_OPTION: {
                                        ASSERT(i.arg_size() == 2);
                                        auto pttid = stoul(i.arg(0));
                                        auto option_idx = stoi(i.arg(1));
                                        if (role.plot_option_locked(pttid, option_idx)) {
                                                role.plot_unlock_option(pttid, option_idx);
                                                auto *puo = res.add_plot_unlock_options();
                                                puo->set_pttid(pttid);
                                                puo->set_option_idx(option_idx);
                                        }
                                        break;
                                }
                                case pd::event::TRIGGER_FEIGE: {
                                        uint32_t pttid = 0;
                                        if (i.arg_size() == 1) {
                                                pttid = stoul(i.arg(0));
                                        } else {
                                                ASSERT(i.arg_size() == 2);
                                                pttid = role.gender() == pd::FEMALE ? stoul(i.arg(0)) : stoul(i.arg(1));
                                        }
                                        ASSERT(pttid != 0);
                                        if (!role.feige_finished(pttid) && !role.feige_ongoing(pttid)) {
                                                role.feige_trigger(pttid);
                                                *res.add_trigger_feige() = role.feige_get_ongoing(pttid);
                                        }
                                        break;
                                }
                                case pd::event::ADD_FEIGE_CHUANWEN: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.feige_has_chuanwen(pttid)) {
                                                role.feige_add_chuanwen(pttid);
                                                auto *chuanwen = res.add_add_feige_chuanwen();
                                                chuanwen->set_pttid(pttid);
                                                chuanwen->set_read(false);
                                        }
                                        break;
                                }
                                case pd::event::ADD_FEIGE_CG: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.feige_has_cg(pttid)) {
                                                role.feige_add_cg(pttid, pd::CG);
                                                auto *cg = res.add_add_feige_cg();
                                                cg->set_pttid(pttid);
                                                cg->set_type(pd::CG);
                                                cg->set_read(false);
                                        }
                                        break;
                                }
                                case pd::event::ADD_FEIGE_MINGYUN: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.feige_has_cg(pttid)) {
                                                role.feige_add_cg(pttid, pd::MINGYUN);
                                                auto *cg = res.add_add_feige_cg();
                                                cg->set_pttid(pttid);
                                                cg->set_type(pd::MINGYUN);
                                                cg->set_read(false);
                                        }
                                        break;
                                }
                                case pd::event::MANSION_COOK_ADD_RECIPE: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.mansion_cook_has_recipe(pttid)) {
                                                role.mansion_cook_add_recipe(pttid);
                                                res.add_mansion_cook_add_recipe(pttid);
                                        }
                                        break;
                                }
                                case pd::event::DEC_GONGDOU_EFFECT: {
                                        ASSERT(i.arg_size() == 2);

                                        pd::gongdou_type type;
                                        pd::gongdou_type_Parse(i.arg(0), &type);
                                        if (!role.gongdou_has_effect(type)) {
                                                continue;
                                        }
                                        const auto& gongdou = role.gongdou_effect(type);
                                        auto old_time = gongdou.last_time();
                                        auto dec_time = stoul(i.arg(1));
                                        auto cur_time = old_time > dec_time ?  old_time - dec_time : 0;
                                        role.change_gongdou_effect_time(type, -dec_time);

                                        bool merged = false;
                                        for (auto j = 0; j < res.change_gongdou_effects_size(); ++j) {
                                                auto *r = res.mutable_change_gongdou_effects(j);
                                                if (r->type() == type) {
                                                        r->set_cur_time(cur_time);
                                                        merged = true;
                                                        break;
                                                }
                                        }
                                        if (!merged) {
                                                auto *r = res.add_change_gongdou_effects();
                                                r->set_type(type);
                                                r->set_old_time(old_time);
                                                r->set_cur_time(cur_time);
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_SKIN: {
                                        ASSERT(i.arg_size() >= 1);
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.actor_has_skin(pttid)) {
                                                role.actor_add_skin(pttid);
                                                res.add_actor_add_skin(pttid);
                                        }
                                        break;
                                }
                                case pd::event::TRIGGER_MAJOR_CITY_BUBBLE: {
                                        ASSERT(i.arg_size() == 1);
                                        auto pttid = stoul(i.arg(0));
                                        role.major_city_add_bubble(pttid);
                                        const auto& bubble_ptt = PTTS_GET(major_city_bubble, pttid);
                                        *res.add_city_bubble() = role.get_major_city_bubble(bubble_ptt.pos());
                                        break;
                                }
                                case pd::event::DROP_RECORD: {
                                        ASSERT(i.arg_size() == 2);
                                        auto *dr = res.add_drop_records();
                                        dr->set_pttid(stoul(i.arg(0)));
                                        dr->set_idx(std::stoi(i.arg(1)));
                                }
                                case pd::event::NONE: {
                                        break;
                                }
                                }
                        }

                        if (!role.mirror_role() && send_bi) {
                                send_event_res_to_bi(role, res, ce.origin());
                        }

                        EVENT_TLOG << "process res\n" << res.DebugString();
                        return res;
                }

                void send_event_res_to_bi(const role& role, const pd::event_res& res, pd::ce_origin origin) {
                        if (origin != pd::CO_ROLE_GROW_RESOURCE) {
                                for (const auto& i : res.change_resources()) {
                                        /*if (bi_record_resource_types.count(i.type()) > 0) {
                                                bi::instance().change_resource(role.username(),
                                                                               role.yci(),
                                                                               role.ip(),
                                                                               role.gid(),
                                                                               i.type(),
                                                                               i.old_count(),
                                                                               i.cur_count(),
                                                                               role.league_gid(),
                                                                               origin);
                                        }*/
                                        pd::currency_change_type change_type;
                                        if (i.cur_count() - i.old_count() < 0) {
                                                change_type = pd::DECREASE_CURRENCY;
                                        } else {
                                                change_type = pd::ADD_CURRENCY;
                                        }
                                        db_log::instance().log_currency_record(role, i.type(), origin, abs(i.cur_count() - i.old_count()), change_type);
                                }
                        }
                        for (const auto& i : res.add_items()) {
                                /*bi::instance().stuff_stream(role.username(),
                                                            role.yci(),
                                                            role.ip(),
                                                            role.gid(),
                                                            role.league_gid(),
                                                            origin,
                                                            yunying_serialize_item(i.pttid()),
                                                            i.old_count(),
                                                            i.cur_count());
                                */
                                const auto& ptt = PTTS_GET(item, i.pttid());
                                db_log::instance().log_item_change(i.pttid(),
                                                                   ptt.name(),
                                                                   i.cur_count() - i.old_count(),
                                                                   origin,
                                                                   role,
                                                                   pd::ADD_ITEM);
                        }
                        map<uint32_t, int> add_equipment_count;
                        for (const auto& i : res.add_equipments()) {
                                add_equipment_count[i.pttid()] += 1;
                        }
                        for (const auto& i : add_equipment_count) {
                                /*bi::instance().stuff_stream(role.username(),
                                                            role.yci(),
                                                            role.ip(),
                                                            role.gid(),
                                                            role.league_gid(),
                                                            origin,
                                                            yunying_serialize_equipment(i.first),
                                                            0,
                                                            1);*/
                        }
                        for (const auto& i : res.huanzhuang_collections()) {
                                /*bi::instance().stuff_stream(role.username(),
                                                            role.yci(),
                                                            role.ip(),
                                                            role.gid(),
                                                            role.league_gid(),
                                                            origin,
                                                            yunying_serialize_huanzhuang(i.part(), i.pttid()),
                                                            0,
                                                            1);*/
                        }
                        for (const auto& i : res.spine_collections()) {
                                /*bi::instance().stuff_stream(role.username(),
                                                            role.yci(),
                                                            role.ip(),
                                                            role.gid(),
                                                            role.league_gid(),
                                                            origin,
                                                            yunying_serialize_spine(i.part(), i.pttid()),
                                                            0,
                                                            1);*/
                        }
                        auto adventure = 0u;
                        auto passed_adventures = role.adventure_passed_adventures();
                        for (auto i : passed_adventures) {
                                const auto& ptt = PTTS_GET(adventure, i);
                                if (ptt._type() == pd::AT_ZHUXIAN) {
                                        adventure = max(adventure, i);
                                        break;
                                }
                        }

                        auto quest = 0u;
                        const auto& accepted_quests = role.quest_accepted_quests();
                        for (const auto& i : accepted_quests) {
                                const auto& ptt = PTTS_GET(quest, i.first);
                                if (ptt._everyday_quest() == 0 &&
                                    ptt._achievement_quest() == 0 &&
                                    !ptt._activity_seven_days_quest() &&
                                    !ptt._activity_seven_days_fuli_quest()) {
                                        quest = i.first;
                                        break;
                                }
                        }

                        for (const auto& i : res.dec_items()) {
                                /*bi::instance().stuff_stream(role.username(),
                                                            role.yci(),
                                                            role.ip(),
                                                            role.gid(),
                                                            role.league_gid(),
                                                            origin,
                                                            yunying_serialize_item(i.pttid()),
                                                            i.old_count(),
                                                            i.cur_count());*/
                                const auto& ptt = PTTS_GET(item, i.pttid());
                                db_log::instance().log_item_change(i.pttid(),
                                                                   ptt.name(),
                                                                   i.old_count() - i.cur_count(),
                                                                   origin,
                                                                   role,
                                                                   pd::DECREASE_ITEM);
                        }
                        map<uint32_t, int> remove_equipments;
                        for (const auto& i : res.remove_equipments()) {
                                remove_equipments[i.pttid()] += 1;
                        }
                        for (const auto& i : remove_equipments) {
                                /*bi::instance().dec_stuff(role.username(),
                                                         role.yci(),
                                                         role.ip(),
                                                         role.gid(),
                                                         role.league_gid(),
                                                         origin,
                                                         yunying_serialize_equipment(i.first),
                                                         i.second,
                                                         adventure,
                                                         quest);*/
                        }
                        for (const auto& i : res.actor_levelup()) {
                                /*bi::instance().actor_levelup(role.username(),
                                                             role.yci(),
                                                             role.ip(),
                                                             role.gid(),
                                                             role.level(),
                                                             origin,
                                                             i);*/
                        }
                }

                void event_revert(const pd::event_res& er, role& role, pd::ce_origin ce_type) {
                        EVENT_TLOG << "revert events\n" << er.DebugString();

                        for (const auto& i : er.change_resources()) {
                                role.change_resource(i.type(), i.old_count() - i.cur_count(), false, true);
                        }
                        for (const auto& i : er.dec_items()) {
                                role.add_item(i.pttid(), i.old_count() - i.cur_count());
                        }
                        for (const auto& i : er.change_gongdou_effects()) {
                                role.change_gongdou_effect_time(i.type(), i.old_time() - i.cur_time());
                        }

                        if (!role.mirror_role()) {
                                for (const auto& i : er.change_resources()) {
                                        if (bi_record_resource_types.count(i.type()) > 0) {
                                                /*bi::instance().change_resource(role.username(),
                                                                               role.yci(),
                                                                               role.ip(),
                                                                               role.gid(),
                                                                               i.type(),
                                                                               i.old_count(),
                                                                               i.cur_count(),
                                                                               role.league_gid(),
                                                                               ce_type);*/
                                        }
                                }
                                for (const auto& i : er.add_items()) {
                                        /*bi::instance().stuff_stream(role.username(),
                                                                    role.yci(),
                                                                    role.ip(),
                                                                    role.gid(),
                                                                    role.league_gid(),
                                                                    ce_type,
                                                                    yunying_serialize_item(i.pttid()),
                                                                    i.old_count(),
                                                                    i.cur_count());*/
                                        const auto& ptt = PTTS_GET(item, i.pttid());
                                        db_log::instance().log_item_change(i.pttid(),
                                                                           ptt.name(),
                                                                           i.cur_count() - i.old_count(),
                                                                           ce_type,
                                                                           role,
                                                                           pd::ADD_ITEM);
                                }
                        }
                }

                void event_revert(const pd::event_array& events, role& role, pd::ce_origin ce_type) {
                        EVENT_TLOG << "revert events\n" << events.DebugString();

                        for (const auto& i : events.events()) {
                                switch (i.type()) {
                                case pd::event::REMOVE_ITEM:
                                        ASSERT(i.arg_size() == 2);
                                        role.add_item(stoul(i.arg(0)), stoul(i.arg(1)));
                                        break;
                                case pd::event::DEC_RESOURCE:
                                        ASSERT(i.arg_size() == 2);
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        role.change_resource(type, stoi(i.arg(1)));
                                        break;
                                default:
                                        break;
                                }
                        }
                        if (!role.mirror_role()) {
                                for (const auto& i : events.events()) {
                                        if (i.type() == pd::event::DEC_RESOURCE) {
                                                pd::resource_type type;
                                                pd::resource_type_Parse(i.arg(0), &type);
                                                if (bi_record_resource_types.count(type) > 0) {
                                                        /*bi::instance().change_resource(role.username(),
                                                                                       role.yci(),
                                                                                       role.ip(),
                                                                                       role.gid(),
                                                                                       type,
                                                                                       role.get_resource(type) + stoi(i.arg(1)),
                                                                                       role.get_resource(type),
                                                                                       role.league_gid(),
                                                                                       ce_type);*/
                                                }
                                        }
                                        if (i.type() == pd::event::REMOVE_ITEM) {
                                                /*bi::instance().add_stuff(role.username(),
                                                                         role.yci(),
                                                                         role.ip(),
                                                                         role.gid(),
                                                                         role.league_gid(),
                                                                         ce_type,
                                                                         yunying_serialize_item(stoul(i.arg(0))),
                                                                         stoul(i.arg(1)),
                                                                         pd::GOLD,
                                                                         0);)*/

                                                const auto& ptt = PTTS_GET(item, stoul(i.arg(0)));
                                                db_log::instance().log_item_change(stoul(i.arg(0)),
                                                                                   ptt.name(),
                                                                                   stoul(i.arg(1)),
                                                                                   ce_type,
                                                                                   role,
                                                                                   pd::ADD_ITEM);
                                        }
                                }
                        }
                }

                pd::event_array event_revert_events(const pd::event_array& events) {
                        EVENT_TLOG << "get revert events\n" << events.DebugString();

                        pd::event_array ret;
                        for (const auto& i : events.events()) {
                                switch (i.type()) {
                                case pd::event::REMOVE_ITEM: {
                                        ASSERT(i.arg_size() == 2);
                                        auto *e = ret.add_events();
                                        e->set_type(pd::event::ADD_ITEM);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                case pd::event::DEC_RESOURCE: {
                                        ASSERT(i.arg_size() == 2);
                                        auto *e = ret.add_events();
                                        e->set_type(pd::event::ADD_RESOURCE);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                int event_add_resource_count(const pd::event_array& events, pd::resource_type type) {
                        int ret = 0;
                        auto type_str = pd::resource_type_Name(type);
                        for (const auto& i : events.events()) {
                                if (i.type() == pd::event::ADD_RESOURCE) {
                                        ASSERT(i.arg_size() == 2);
                                        if (i.arg(0) == type_str) {
                                                ret += stoi(i.arg(1));
                                        }
                                }
                        }
                        return ret;
                }

                void fief_event_merge(pd::fief_event_array& a, const pd::fief_event_array& b) {
                        for (const auto& i : b.events()) {
                                bool merged = false;
                                for (auto& j : *a.mutable_events()) {
                                        if ((i.type() == pd::fief_event::ADD_RESOURCE || i.type() == pd::fief_event::DEC_RESOURCE) &&
                                            i.type() == j.type() && i.arg(0) == j.arg(0)) {
                                                j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                merged = true;
                                                break;
                                        }
                                }
                                if (!merged) {
                                        *a.add_events() = i;
                                }
                        }
                }

                void plot_event_merge(pd::plot_event_array& a, const pd::plot_event_array& b) {
                        for (const auto& i : b.events()) {
                                *a.add_events() = i;
                        }
                }

                void league_event_merge(pd::league_event_array& a, const pd::league_event_array& b) {
                        for (const auto& i : b.events()) {
                                *a.add_events() = i;
                        }
                }

        }
}
