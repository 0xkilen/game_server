#include "condition.hpp"
#include "scene/player/role.hpp"
#include "scene/player/player_mgr.hpp"
#include "scene/condition.hpp"
#include "scene/item.hpp"
#include "scene/actor/actor.hpp"
#include "scene/huanzhuang/huanzhuang.hpp"
#include "config/resource_ptts.hpp"
#include "config/options_ptts.hpp"
#include "utils/string_utils.hpp"

#define CONDITION_TLOG __TLOG << setw(20) << "[CONDITION] "
#define CONDITION_DLOG __DLOG << setw(20) << "[CONDITION] "
#define CONDITION_ILOG __ILOG << setw(20) << "[CONDITION] "
#define CONDITION_ELOG __ELOG << setw(20) << "[CONDITION] "

namespace pc = proto::config;

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                void condition_merge(pd::condition_array& a, const pd::condition_array& b) {
                        for (const auto& i : b.conditions()) {
                                auto merge = false;
                                for (auto& j : *a.mutable_conditions()) {
                                        if (i.type() == j.type()) {
                                                switch (i.type()) {
                                                case pd::condition::COST_ITEM:
                                                        if (stoul(i.arg(0)) == stoul(j.arg(0))) {
                                                                j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                                merge = true;
                                                        }
                                                        break;
                                                case pd::condition::COST_RESOURCE:
                                                        pd::resource_type a_type, b_type;
                                                        pd::resource_type_Parse(i.arg(0), &a_type);
                                                        pd::resource_type_Parse(j.arg(0), &b_type);
                                                        if (a_type == b_type) {
                                                                j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                                merge = true;
                                                        }
                                                        break;
                                                case pd::condition::COST_PRESENT:
                                                        if (stoi(i.arg(0)) == stoi(j.arg(0))) {
                                                                j.set_arg(1, to_string(stoi(i.arg(1)) + stoi(j.arg(1))));
                                                                merge = true;
                                                        }
                                                        break;
                                                default :
                                                        break;
                                                }
                                                break;
                                        }
                                }
                                if (!merge) {
                                        *a.add_conditions() = i;
                                }
                        }
                }

                void condition_adopt_env(pd::condition_array& ca, const pd::ce_env& ce) {
                        auto options_ptt = PTTS_GET_COPY(options, 1);
                        auto cost_free = options_ptt.scene_info().cost_free();
                        auto mutable_ce = ce;
                        if (cost_free) {
                                mutable_ce.set_cost_percent(0);
                        }

                        for (auto& i : *ca.mutable_conditions()) {
                                switch (i.type()) {
                                case pd::condition::PASS_ADVENTURE:
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_adventure());
                                                i.add_arg(to_string(ce.adventure()));
                                        }
                                        break;
                                case pd::condition::ADVENTURE_NO_CHANCES:
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_adventure());
                                                i.add_arg(to_string(ce.adventure()));
                                        }
                                        break;
                                case pd::condition::NOT_PASS_ADVENTURE:
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_adventure());
                                                i.add_arg(to_string(ce.adventure()));
                                        }
                                        break;
                                case pd::condition::PASS_LIEMING:
                                        ASSERT(i.arg_size() <= 1);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_lieming());
                                                i.add_arg(to_string(ce.lieming()));
                                        }
                                        break;
                                case pd::condition::COST_ITEM: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_item());
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.item()));
                                                i.add_arg(arg);
                                        }
                                        if (mutable_ce.has_cost_percent()) {
                                                auto cost = stoi(i.arg(1)) * mutable_ce.cost_percent() / 1000;
                                                i.set_arg(1, to_string(cost));
                                        }
                                        break;
                                }
                                case pd::condition::ITEM_COUNT_LE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_item());
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.item()));
                                                i.add_arg(arg);
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_INTIMACY_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.actors_size() >= 2);
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.add_arg(to_string(ce.actors(1)));
                                                i.add_arg(arg);
                                        } else {
                                                ASSERT(ce.actors_size() >= 1);
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.add_arg(arg);
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_LEVEL_GE:
                                case pd::condition::ACTOR_STAR_GE:
                                case pd::condition::ACTOR_STEP_GE:
                                case pd::condition::ACTOR_QUALITY_GE:
                                case pd::condition::ACTOR_PIN_LE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.actors_size() >= 1);
                                                auto arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.add_arg(arg);
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_CRAFT_GE:
                                case pd::condition::ACTOR_CRAFT_E: {
                                        ASSERT(i.arg_size() == 2 || i.arg_size() == 3);
                                        if (i.arg_size() == 2) {
                                                ASSERT(ce.actors_size() >= 1);
                                                auto arg0 = i.arg(0);
                                                auto arg1 = i.arg(1);
                                                i.set_arg(0, to_string(ce.actors(0)));
                                                i.set_arg(1, arg0);
                                                i.add_arg(arg1);
                                        }
                                        break;
                                }
                                case pd::condition::NO_ACTOR:
                                case pd::condition::HAS_ACTOR:
                                case pd::condition::LEVEL_GREATER_THAN_ACTOR_LEVEL: {
                                        ASSERT(i.arg_size() < 2);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.actors_size() >= 1);
                                                i.add_arg(to_string(ce.actors(0)));
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_EXP_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_equipment());
                                                auto first_arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.equipment()));
                                                i.add_arg(first_arg);
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_QUALITY_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_equipment());
                                                auto first_arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.equipment()));
                                                i.add_arg(first_arg);
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_LEVEL_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_equipment());
                                                auto first_arg = i.arg(0);
                                                i.set_arg(0, to_string(ce.equipment()));
                                                i.add_arg(first_arg);
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_EQUIPMENT_LEVEL: {
                                        ASSERT(i.arg_size() < 2);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_equipment());
                                                i.add_arg(to_string(ce.equipment()));
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_HUANZHUANG_LEVEL: {
                                        ASSERT(i.arg_size() < 2);
                                        if (i.arg_size() == 0) {
                                                ASSERT(ce.has_huanzhuang_part());
                                                ASSERT(ce.has_huanzhuang_pttid());
                                                i.add_arg(ce.huanzhuang_part());
                                                i.add_arg(to_string(ce.huanzhuang_pttid()));
                                        }
                                        break;
                                }
                                case pd::condition::COST_RESOURCE: {
                                        ASSERT(i.arg_size() == 2);
                                        if (mutable_ce.has_cost_percent()) {
                                                i.set_arg(1, to_string(stoi(i.arg(1)) * mutable_ce.cost_percent() / 1000));
                                        }
                                        break;
                                }
                                case pd::condition::BUY_GOOD_TIMES_LESS:
                                case pd::condition::FORMATION_GRID_NO_ACTOR: {
                                        ASSERT(i.arg_size() == 3);
                                        break;
                                }
                                case pd::condition::INTIMACY_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 2);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_other());
                                                auto arg0 = i.arg(0);
                                                i.set_arg(0, to_string(ce.other()));
                                                i.add_arg(arg0);
                                        }
                                        break;
                                }
                                case pd::condition::CHILD_PHASE_IN: {
                                        ASSERT(i.arg_size() == 2);
                                        ASSERT(ce.has_child());
                                        auto arg0 = i.arg(0);
                                        auto arg1 = i.arg(1);
                                        i.set_arg(0, to_string(ce.child()));
                                        i.set_arg(1, arg0);
                                        i.add_arg(arg1);
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_CHILD_LEVEL: {
                                        ASSERT(i.arg_size() == 2);
                                        ASSERT(ce.has_child());
                                        auto arg0 = i.arg(0);
                                        auto arg1 = i.arg(1);
                                        i.set_arg(0, to_string(ce.child()));
                                        i.set_arg(1, arg0);
                                        i.add_arg(arg1);
                                        break;
                                }
                                case pd::condition::PASS_PLOT:
                                case pd::condition::LEVEL_GE:
                                case pd::condition::LEVEL_LE:
                                case pd::condition::GENDER:
                                case pd::condition::VIP_LEVEL_GE:
                                case pd::condition::VIP_LEVEL_LE:
                                case pd::condition::SPOUSE_VIP_LEVEL_GE:
                                case pd::condition::SPOUSE_VIP_LEVEL_LE:
                                case pd::condition::FAIL:
                                case pd::condition::RANDOM:
                                case pd::condition::FRIEND_COUNT_LE:
                                case pd::condition::MANSION_FANCY_GE:
                                case pd::condition::ROLE_PIN_LE:
                                case pd::condition::ROLE_PIN_E:
                                case pd::condition::RESOURCE_NOT_REACH_MAX:
                                case pd::condition::MANSION_BANQUET_PRESTIGE_LEVEL_GE:
                                case pd::condition::MAX_GPIN_GE:
                                case pd::condition::PASS_XINSHOU_GROUP:
                                case pd::condition::HAS_TITLE:
                                case pd::condition::ROLE_NOT_EQUIPED_EQUIPMENT_PART:
                                case pd::condition::ROLE_HAS_NOT_EQUIPED_EQUIPMENT:
                                case pd::condition::TOWER_CUR_LEVEL_E:
                                case pd::condition::TOWER_HIGHEST_LEVEL_GE:
                                case pd::condition::QUEST_COMMITTED:
                                case pd::condition::LOTTERY_FREE:
                                case pd::condition::FIRST_LOGIN_DAY_E:
                                case pd::condition::FEIGE_ONGOING:
                                case pd::condition::FEIGE_FINISHED:
                                case pd::condition::MANSION_COOK_HAS_RECIPE:
                                case pd::condition::MANSION_FARM_PLOT_STATE:
                                case pd::condition::ARENA_BEST_RANK_GE:
                                case pd::condition::LEAGUE_SHOP_LEVEL_IS:
                                case pd::condition::LEAGUE_SHOP_LEVEL_GE:
                                case pd::condition::HAS_BONUS_CITY:
                                case pd::condition::MANSION_HALL_QUEST_COUNT_GE:
                                case pd::condition::QUEST_ACCEPTED:
                                case pd::condition::ZHANLI_GE: {
                                        ASSERT(i.arg_size() == 1);
                                        break;
                                }
                                case pd::condition::GPIN_LIMIT: {
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_guanpin());
                                                i.add_arg(to_string(ce.guanpin()));
                                        } else if (i.arg_size() == 2) {

                                        } else {
                                                ASSERT(false);
                                        }
                                        break;
                                }
                                case pd::condition::SERVER_OPEN_DAY_IN:
                                case pd::condition::NO_HUANZHUANG_COLLECTION:
                                case pd::condition::NO_SPINE_COLLECTION:
                                case pd::condition::RESOURCE_GE:
                                case pd::condition::COST_PRESENT:
                                case pd::condition::PLOT_OPTION_NOT_LOCKED:
                                case pd::condition::PLOT_OPTION_SELECTED:
                                case pd::condition::FORMATION_NO_ACTOR:
                                case pd::condition::MANSION_FURNITURE_COUNT_GE:
                                case pd::condition::ROLE_HAS_EQUIPMENT_PART_COUNT_GE: {
                                        ASSERT(i.arg_size() == 2);
                                        break;
                                }
                                case pd::condition::LEAGUE_JOINED:
                                case pd::condition::MANSION_CREATED:
                                case pd::condition::NOT_IN_LEAGUE:
                                case pd::condition::MARRIED:
                                case pd::condition::NOT_MARRIED:
                                case pd::condition::EVER_MARRIED: {
                                        ASSERT(i.arg_size() == 0);
                                        break;
                                }
                                case pd::condition::HUANZHUANG_EXP_GE: {
                                        ASSERT(i.arg_size() == 1 || i.arg_size() == 3);
                                        if (i.arg_size() == 1) {
                                                ASSERT(ce.has_huanzhuang_part());
                                                ASSERT(ce.has_huanzhuang_pttid());
                                                auto first_arg = i.arg(0);
                                                i.set_arg(0, ce.huanzhuang_part());
                                                i.add_arg(to_string(ce.huanzhuang_pttid()));
                                                i.add_arg(first_arg);
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                pd::result condition_check(const pd::condition_array& ca, const role& role, const pd::ce_env& ce) {
                        auto result = pd::OK;
                        auto conditions = ca;
                        condition_adopt_env(conditions, ce);

                        CONDITION_TLOG << "check conditions ce\n" << ce.DebugString();
                        CONDITION_TLOG << "check conditions\n" << ca.DebugString();

                        for (const auto& i : conditions.conditions()) {
                                switch (i.type()) {
                                case pd::condition::PASS_ADVENTURE:
                                        if (!role.adventure_passed(stoul(i.arg(0)))) {
                                                result = pd::ADVENTURE_NOT_PASSED;
                                        }
                                        break;
                                case pd::condition::ADVENTURE_NO_CHANCES: {
                                        auto pttid = stoul(i.arg(0));
                                        const auto& ptt = PTTS_GET(adventure, pttid);
                                        ASSERT(ptt.has_day_times_limit());
                                        if (role.adventure_battle_passed_times(pttid) < ptt.day_times_limit()) {
                                                result = pd::ADVENTURE_HAS_CHANCES;
                                        }
                                        break;
                                }
                                case pd::condition::NOT_PASS_ADVENTURE:
                                        if (role.adventure_passed(stoul(i.arg(0)))) {
                                                result = pd::ADVENTURE_PASSED;
                                        }
                                        break;
                                case pd::condition::PASS_LIEMING:
                                        if (!role.lieming_passed(stoul(i.arg(0)))) {
                                                result = pd::LIEMING_NOT_PASSED;
                                        }
                                        break;
                                case pd::condition::COST_RESOURCE: {
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        if (role.get_resource(type) < stoi(i.arg(1))) {
                                                result = pd::ROLE_NOT_ENOUGH_RESOURCE;
                                        }
                                        break;
                                }
                                case pd::condition::RESOURCE_GE: {
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        if (role.get_resource(type) < stoi(i.arg(1))) {
                                                result = pd::ROLE_NOT_ENOUGH_RESOURCE;
                                        }
                                        break;
                                }
                                case pd::condition::COST_ITEM: {
                                        auto it = role.get_item(stoul(i.arg(0)));
                                        if (!it || it->count() < stoi(i.arg(1))) {
                                                result = pd::ROLE_NOT_ENOUGH_ITEM;
                                        }
                                        break;
                                }
                                case pd::condition::ITEM_COUNT_LE: {
                                        auto it = role.get_item(stoul(i.arg(0)));
                                        if (it && it->count() > stoi(i.arg(1))) {
                                                result = pd::ROLE_TOO_MANY_ITEM;
                                        }
                                        break;
                                }
                                case pd::condition::BUY_GOOD_TIMES_LESS: {
                                        auto buy_times = role.shop_buy_times(stoul(i.arg(0)), stoul(i.arg(1)));
                                        if (buy_times >= stoul(i.arg(2))) {
                                                result = pd::SHOP_BUY_GOOD_TIMES_REACH_MAX;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_ACTOR_LEVEL: {
                                        ASSERT(role.has_actor(stoul(i.arg(0))));
                                        const auto& actor = role.get_actor(stoul(i.arg(0)));
                                        if (actor->level() >= role.level()) {
                                                result = pd::ACTOR_LEVEL_NOT_LESS_THAN_LEVEL;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_EQUIPMENT_LEVEL: {
                                        ASSERT(role.has_equipment(stoul(i.arg(0))));
                                        const auto& equipment = role.get_equipment(stoul(i.arg(0)));
                                        if (equipment->level() >= role.level()) {
                                                result = pd::EQUIPMENT_LEVEL_NOT_LESS_THAN_LEVEL;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_HUANZHUANG_LEVEL: {
                                        pd::huanzhuang_part part;
                                        pd::huanzhuang_part_Parse(i.arg(0), &part);
                                        uint32_t pttid = stoul(i.arg(1));
                                        ASSERT(role.huanzhuang_has_collection(part, pttid));
                                        const auto& hz = role.huanzhuang_get_collection(part, pttid);
                                        if (hz->level() >= role.level()) {
                                                result = pd::HUANZHUANG_LEVEL_NOT_LESS_THAN_LEVEL;
                                        }
                                        break;
                                }
                                case pd::condition::HAS_ACTOR: {
                                        if (!role.has_actor(stoul(i.arg(0)))) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                        }
                                        break;
                                }
                                case pd::condition::NO_ACTOR: {
                                        if (role.has_actor(stoul(i.arg(0)))) {
                                                result = pd::ROLE_HAS_SUCH_ACTOR;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GE: {
                                        if (role.level() < stoul(i.arg(0))) {
                                                result = pd::ROLE_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_LE: {
                                        if (role.level() > stoul(i.arg(0))) {
                                                result = pd::ROLE_LEVEL_TOO_HIGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_INTIMACY_GE: {
                                        if (role.actor_intimacy(stoul(i.arg(0)), stoul(i.arg(1))) < stoi(i.arg(2))) {
                                                result = pd::ACTOR_INTIMACY_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_LEVEL_GE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.has_actor(pttid)) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                                break;
                                        }
                                        const auto& actor = role.get_actor(pttid);
                                        if (actor->level() < stoul(i.arg(1))) {
                                                result = pd::ACTOR_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_STAR_GE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.has_actor(pttid)) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                                break;
                                        }
                                        const auto& actor = role.get_actor(pttid);
                                        if (actor->star() < stoul(i.arg(1))) {
                                                result = pd::ACTOR_STAR_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_STEP_GE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.has_actor(pttid)) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                                break;
                                        }
                                        const auto& actor = role.get_actor(pttid);
                                        if (actor->step() < stoul(i.arg(1))) {
                                                result = pd::ACTOR_STEP_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_QUALITY_GE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.has_actor(pttid)) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                                break;
                                        }
                                        const auto& actor = role.get_actor(pttid);
                                        if (actor->quality() < stoul(i.arg(1))) {
                                                result = pd::ACTOR_QUALITY_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_PIN_LE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.has_actor(pttid)) {
                                                result = pd::ROLE_NO_SUCH_ACTOR;
                                                break;
                                        }
                                        const auto& actor = role.get_actor(pttid);
                                        if (actor->pin() > stoul(i.arg(1))) {
                                                result = pd::ACTOR_PIN_GREATER_THAN_NEED;
                                        }
                                        break;
                                }
                                case pd::condition::SERVER_OPEN_DAY_IN: {
                                        auto begin_day = stoul(i.arg(0));
                                        auto end_day = stoul(i.arg(1));
                                        auto day = refresh_day() - player_mgr_class::instance().server_open_day();
                                        if (day < begin_day || day > end_day) {
                                                result = pd::SERVER_OPEN_DAY_MISMATCH;
                                        }
                                        break;
                                }
                                case pd::condition::FIRST_LOGIN_DAY_E: {
                                        auto day = stoul(i.arg(0));
                                        auto first_login_day = refresh_day_from_time_t(role.first_login_time());
                                        if (day != refresh_day() - first_login_day) {
                                                result = pd::ROLE_FIRST_LOGIN_DAY_MISMATCH;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_FARM_PLOT_STATE: {
                                        ASSERT(false);
                                        break;
                                }
                                case pd::condition::VIP_LEVEL_GE: {
                                        if (role.vip_level() < stoul(i.arg(0))) {
                                                result = pd::ROLE_VIP_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::SPOUSE_VIP_LEVEL_GE: {
                                        if (!role.marriage_married()) {
                                                break;
                                        }
                                        if (role.spouse_vip_level() < stoul(i.arg(0))) {
                                                result = pd::ROLE_SPOUSE_VIP_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::VIP_LEVEL_LE: {
                                        if (role.vip_level() > stoul(i.arg(0))) {
                                                result = pd::ROLE_VIP_LEVEL_TOO_HIGH;
                                        }
                                        break;
                                }
                                case pd::condition::SPOUSE_VIP_LEVEL_LE: {
                                        if (!role.marriage_married()) {
                                                break;
                                        }
                                        if (role.spouse_vip_level() > stoul(i.arg(0))) {
                                                result = pd::ROLE_SPOUSE_VIP_LEVEL_TOO_HIGH;
                                        }
                                        break;
                                }
                                case pd::condition::GENDER: {
                                        pd::gender gender;
                                        pd::gender_Parse(i.arg(0), &gender);
                                        if (role.gender() != gender) {
                                                result = pd::ROLE_WRONG_GENDER;
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_EXP_GE: {
                                        const auto& eq = role.get_equipment(stoul(i.arg(0)));
                                        if (eq->exp() < stol(i.arg(1))) {
                                                result = pd::EQUIPMENT_EXP_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_QUALITY_GE: {
                                        const auto& eq = role.get_equipment(stoul(i.arg(0)));
                                        if (eq->quality() < stoul(i.arg(1))) {
                                                result = pd::EQUIPMENT_QUALITY_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::EQUIPMENT_LEVEL_GE: {
                                        const auto& eq = role.get_equipment(stoul(i.arg(0)));
                                        if (eq->level() < stoul(i.arg(1))) {
                                                result = pd::EQUIPMENT_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_CRAFT_GE: {
                                        ASSERT(i.arg_size() == 3);
                                        const auto& actor = role.get_actor(stoul(i.arg(0)));
                                        pd::actor::craft_type type;
                                        pd::actor::craft_type_Parse(i.arg(1), &type);
                                        if (actor->craft(type) < stoul(i.arg(2))) {
                                                result = pd::ACTOR_CRAFT_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ACTOR_CRAFT_E: {
                                        ASSERT(i.arg_size() == 3);
                                        const auto& actor = role.get_actor(stoul(i.arg(0)));
                                        pd::actor::craft_type type;
                                        pd::actor::craft_type_Parse(i.arg(1), &type);
                                        if (actor->craft(type) != stoul(i.arg(2))) {
                                                result = pd::ACTOR_CRAFT_NOT_EQUAL;
                                        }
                                        break;
                                }
                                case pd::condition::PASS_PLOT: {
                                        if (!role.plot_passed(stoul(i.arg(0)))) {
                                                result = pd::PLOT_NOT_PASSED;
                                        }
                                        break;
                                }
                                case pd::condition::INTIMACY_GE: {
                                        auto other = stoul(i.arg(0));
                                        if (!role.relation_has_friend(other)) {
                                                result = pd::RELATION_NO_SUCH_FRIEND;
                                        } else if (role.relation_friend_intimacy(other) < stoi(i.arg(1))) {
                                                result = pd::RELATION_INTIMACY_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::FAIL: {
                                        ASSERT(i.arg_size() == 1);
                                        pd::result_Parse(i.arg(0), &result);
                                        break;
                                }
                                case pd::condition::NO_HUANZHUANG_COLLECTION: {
                                        pd::huanzhuang_part part;
                                        ASSERT(pd::huanzhuang_part_Parse(i.arg(0), &part));
                                        if (role.huanzhuang_has_collection(part, stoul(i.arg(1)))) {
                                                result = pd::HUANZHUANG_HAS_SUCH_COLLECTION;
                                        }
                                        break;
                                }
                                case pd::condition::HUANZHUANG_EXP_GE: {
                                        pd::huanzhuang_part part;
                                        pd::huanzhuang_part_Parse(i.arg(0), &part);
                                        uint32_t pttid = stoul(i.arg(1));
                                        const auto& huanzhuang = role.huanzhuang_get_collection(part, pttid);
                                        if (huanzhuang->exp() < stol(i.arg(2))) {
                                                result = pd::HUANZHUANG_EXP_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::NO_SPINE_COLLECTION: {
                                        pd::spine_part part;
                                        ASSERT(pd::spine_part_Parse(i.arg(0), &part));
                                        if (role.spine_has_collection(part, stoul(i.arg(1)))) {
                                                result = pd::SPINE_HAS_SUCH_COLLECTION;
                                        }
                                        break;
                                }
                                case pd::condition::COST_PRESENT: {
                                        auto count = role.present_count(stoul(i.arg(0)));
                                        if (count < stoi(i.arg(1))) {
                                                result = pd::ROLE_NOT_ENOUGH_PRESENT;
                                        }
                                        break;
                                }
                                case pd::condition::RANDOM: {
                                        if ((rand() % 1000) >= stoi(i.arg(0))) {
                                                result = pd::RANDOM_FAILED;
                                        }
                                        break;
                                }
                                case pd::condition::FRIEND_COUNT_LE: {
                                        if (role.relation_friend_count() > stoul(i.arg(0))) {
                                                result = pd::RELATION_TOO_MANY_FRIENDS;
                                        }
                                        break;
                                }
                                case pd::condition::LEAGUE_JOINED: {
                                        if (!role.has_league()) {
                                                result = pd::LEAGUE_ROLE_NO_LEAGUE;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_CREATED: {
                                        if (!role.mansion_exist()) {
                                                result = pd::MANSION_NO_MANSION;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_FANCY_GE: {
                                        if (role.mansion_fancy() < stoul(i.arg(0))) {
                                                result = pd::MANSION_FANCY_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::ROLE_PIN_LE: {
                                        const auto& ra = role.get_role_actor();
                                        if (ra->pin() > stoul(i.arg(0))) {
                                                result = pd::ROLE_PIN_GREATER_THAN_NEED;
                                        }
                                        break;
                                }
                                case pd::condition::ROLE_PIN_E: {
                                        const auto& ra = role.get_role_actor();
                                        if (ra->pin() != stoul(i.arg(0))) {
                                                result = pd::ROLE_PIN_AND_NEED_NOT_EQUAL;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_BANQUET_PRESTIGE_LEVEL_GE: {
                                        if (role.mansion_banquet_prestige_level() < stoul(i.arg(0))) {
                                                result = pd::MANSION_BANQUET_NEED_HIGHER_PRESTIGE_LEVEL;
                                        }
                                        break;
                                }
                                case pd::condition::RESOURCE_NOT_REACH_MAX: {
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        const auto& ptt = PTTS_GET(resource, type);
                                        if (ptt.has_max_value() && role.get_resource(type) >= ptt.max_value()) {
                                                result = pd::ROLE_RESOURCE_REACH_MAX;
                                        }
                                        break;
                                }
                                case pd::condition::PLOT_OPTION_NOT_LOCKED: {
                                        if (role.plot_option_locked(stoul(i.arg(0)), stoi(i.arg(1)))) {
                                                result = pd::PLOT_OPTION_LOCKED;
                                        }
                                        break;
                                }
                                case pd::condition::TOWER_CUR_LEVEL_E: {
                                        if (role.tower_cur_level() != stoul(i.arg(0))) {
                                                result = pd::TOWER_CUR_LEVEL_NOT_MATCH;
                                        }
                                        break;
                                }
                                case pd::condition::TOWER_HIGHEST_LEVEL_GE: {
                                        if (role.tower_highest_level() < stoul(i.arg(0))) {
                                                result = pd::TOWER_HIGHEST_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::PLOT_OPTION_SELECTED: {
                                        if (role.plot_option_selected(stoul(i.arg(0)), stoi(i.arg(1)))) {
                                                result = pd::PLOT_OPTION_NOT_SELECTED;
                                        }
                                        break;
                                }
                                case pd::condition::MAX_GPIN_GE: {
                                        if (role.guanpin_max_gpin() < stoi(i.arg(0))) {
                                                result = pd::GUANPIN_MAX_GPIN_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::GPIN_LIMIT: {
                                        ASSERT(i.arg_size() == 2);
                                        if (!str_compare_value(i.arg(0), role.guanpin_gpin(), stoi(i.arg(1)))) {
                                                if (i.arg(0) == "<" || i.arg(0) == "<=") {
                                                        result = pd::GUANPIN_GPIN_TOO_HIGH;
                                                } else if (i.arg(0) == ">" || i.arg(0) == ">=") {
                                                        result = pd::GUANPIN_GPIN_TOO_LOW;
                                                } else {
                                                        result = pd::GUANPIN_GPIN_LIMIT_MISMATCH;
                                                }
                                        }
                                        break;
                                }
                                case pd::condition::PASS_XINSHOU_GROUP: {
                                        if (!role.xinshou_group_passed(stoul(i.arg(0)))) {
                                                result = pd::XINSHOU_GROUP_NOT_PASSED;
                                        }
                                        break;
                                }
                                case pd::condition::HAS_TITLE: {
                                        if (!role.has_title(stoul(i.arg(0)))) {
                                                result = pd::ROLE_NO_SUCH_TITLE;
                                        }
                                        break;
                                }
                                case pd::condition::ROLE_NOT_EQUIPED_EQUIPMENT_PART: {
                                        pd::equipment_part part;
                                        pd::equipment_part_Parse(i.arg(0), &part);
                                        const auto& equipments = role.get_role_actor()->equipments();
                                        if (equipments.count(part) > 0) {
                                                result = pd::ROLE_EQUIPED_EQUIPMENT_PART;
                                        }
                                        break;
                                }
                                case pd::condition::ROLE_HAS_NOT_EQUIPED_EQUIPMENT: {
                                        auto pttid = stoul(i.arg(0));
                                        auto found = false;
                                        const auto& equipments = role.equipments();
                                        for (auto i : equipments) {
                                                const auto& e = role.get_equipment(i);
                                                if (e->pttid() == pttid && !role.equipment_on_actor(i)) {
                                                        found = true;
                                                        break;
                                                }
                                        }
                                        if (!found) {
                                                result = pd::ROLE_DOES_NOT_HAVE_NOT_EQUIPED_EQUIPMENT;
                                        }
                                        break;
                                }
                                case pd::condition::NOT_IN_LEAGUE: {
                                        if (role.has_league()) {
                                                result = pd::LEAGUE_ROLE_IN_LEAGUE;
                                        }
                                        break;
                                }
                                case pd::condition::LEAGUE_SHOP_LEVEL_IS: {
                                        if (!role.has_league()) {
                                                result = pd::LEAGUE_ROLE_IN_LEAGUE;
                                                break;
                                        }
                                        if (role.get_league_shop_level() != stoul(i.arg(0))) {
                                                result = pd::LEAGUE_SHOP_LEVEL_DIFF;
                                        }
                                        break;
                                }
                                case pd::condition::LEAGUE_SHOP_LEVEL_GE: {
                                        if (!role.has_league()) {
                                                result = pd::LEAGUE_ROLE_IN_LEAGUE;
                                                break;
                                        }
                                        if (role.get_league_shop_level() < stoul(i.arg(0))) {
                                                result = pd::LEAGUE_SHOP_LEVEL_NOT_GE;
                                        }
                                        break;
                                }
                                case pd::condition::HAS_BONUS_CITY: {
                                        if (!role.has_league()) {
                                                result = pd::LEAGUE_ROLE_IN_LEAGUE;
                                                break;
                                        }
                                        if (!role.has_bonus_city(stoul(i.arg(0)))) {
                                                result = pd::LEAGUE_HAS_NOT_GIVEN_BONUS_CITY;
                                        }
                                        break;
                                }
                                case pd::condition::QUEST_COMMITTED: {
                                        if (role.has_league()) {
                                                result = pd::LEAGUE_ROLE_IN_LEAGUE;
                                        }
                                        break;
                                }
                                case pd::condition::FORMATION_NO_ACTOR: {
                                        break;
                                }
                                case pd::condition::FORMATION_GRID_NO_ACTOR: {
                                        break;
                                }
                                case pd::condition::LOTTERY_FREE: {
                                        break;
                                }
                                case pd::condition::ROLE_HAS_EQUIPMENT_PART_COUNT_GE: {
                                        break;
                                }
                                case pd::condition::MARRIED: {
                                        if (!role.marriage_married()) {
                                                result = pd::MARRIAGE_NOT_MARRIED;
                                        }
                                        break;
                                }
                                case pd::condition::NOT_MARRIED: {
                                        if (role.marriage_married()) {
                                                result = pd::MARRIAGE_ALREADY_MARRIED;
                                        }
                                        break;
                                }
                                case pd::condition::EVER_MARRIED: {
                                        if (!role.marriage_ever_married()) {
                                                result = pd::MARRIAGE_NEVER_MARRIED;
                                        }
                                        break;
                                }
                                case pd::condition::FEIGE_ONGOING: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.feige_ongoing(pttid)) {
                                                result = pd::FEIGE_NOT_ONGOING;
                                        }
                                        break;
                                }
                                case pd::condition::FEIGE_FINISHED: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.feige_finished(pttid)) {
                                                result = pd::FEIGE_NOT_FINISHED;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_COOK_HAS_RECIPE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!role.mansion_cook_has_recipe(pttid)) {
                                                return pd::MANSION_COOK_ROLE_NO_RECIPE;
                                        }
                                        break;
                                }
                                case pd::condition::ARENA_BEST_RANK_GE: {
                                        auto rank = stoul(i.arg(0));
                                        if (role.arena_best_rank() > rank) {
                                                return pd::ARENA_BEST_RANK_NOT_REACH;
                                        }
                                        break;
                                }
                                case pd::condition::QUEST_ACCEPTED: {
                                        uint32_t pttid = stoul(i.arg(0));
                                        if (!role.quest_has_accepted(pttid)) {
                                                return pd::QUEST_NOT_ACCEPTED;
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_HALL_QUEST_COUNT_GE: {
                                        ASSERT(false);
                                        break;
                                }
                                case pd::condition::MANSION_FURNITURE_COUNT_GE: {
                                        if (!role.mansion_exist()) {
                                                result = pd::MANSION_NO_MANSION;
                                                break;
                                        }
                                        uint32_t pttid = stoul(i.arg(0));
                                        auto need = stoul(i.arg(1));
                                        if (role.mansion_furniture_count(pttid) < need) {
                                                result = pd::MANSION_FURNITURE_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::CHILD_PHASE_IN: {
                                        auto gid = stoul(i.arg(0));
                                        auto child = role.get_child(gid);
                                        ASSERT(child);
                                        auto phase = child->phase();
                                        pd::child_phase min_phase;
                                        pd::child_phase_Parse(i.arg(1), &min_phase);
                                        pd::child_phase max_phase;
                                        pd::child_phase_Parse(i.arg(1), &max_phase);
                                        if (phase < min_phase || phase > max_phase) {
                                                result = pd::CHILD_WRONG_PHASE;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GREATER_THAN_CHILD_LEVEL: {
                                        auto gid = stoul(i.arg(0));
                                        auto child = role.get_child(gid);
                                        ASSERT(child);
                                        if (child->level() >= role.level()) {
                                                result = pd::CHILD_LEVEL_NOT_LESS_THAN_LEVEL;
                                        }
                                        break;
                                }
                                case pd::condition::ZHANLI_GE: {
                                        auto zhanli = stoul(i.arg(0));
                                        if (role.calc_max_zhanli() < zhanli) {
                                                result = pd::ROLE_ZHANLI_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                }
                                if (result != pd::OK) {
                                        break;
                                }
                        }

                        CONDITION_DLOG << "check result " << pd::result_Name(result);
                        return result;
                }

                pd::result condition_check(const pd::condition_array& ca, const pd::role_info& ri) {
                        auto result = pd::OK;
                        auto conditions = ca;
                        auto role = ri;

                        CONDITION_TLOG << "check conditions\n" << ca.DebugString();
                        for (const auto& i : conditions.conditions()) {
                                switch (i.type()) {
                                case pd::condition::COST_RESOURCE: {
                                        pd::resource_type type;
                                        pd::resource_type_Parse(i.arg(0), &type);
                                        auto res = role.resources();
                                        for (auto& r : res.values()) {
                                                if (r.type() == type && r.value() < stoi(i.arg(1))) {
                                                        result = pd::ROLE_NOT_ENOUGH_RESOURCE;
                                                        break;
                                                }
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_GE: {
                                        if (ri.simple_info().level() < stoul(i.arg(0))) {
                                                result = pd::ROLE_LEVEL_NOT_ENOUGH;
                                        }
                                        break;
                                }
                                case pd::condition::LEVEL_LE: {
                                        if (ri.simple_info().level() > stoul(i.arg(0))) {
                                                result = pd::ROLE_LEVEL_TOO_HIGH;
                                        }
                                        break;
                                }
                                default: {
                                        break;
                                }
                                }

                                if (result != pd::OK) {
                                        break;
                                }
                        }

                        CONDITION_DLOG << "check result " << pd::result_Name(result);
                        return result;
                }

                vector<pd::condition> condition_find(const pd::condition_array& ca, pd::condition::condition_type type) {
                        vector<pd::condition> ret;
                        for (const auto& i : ca.conditions()) {
                                if (i.type() == type) {
                                        ret.push_back(i);
                                }
                        }

                        return ret;
                }

        }
}
