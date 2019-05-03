#include "utils.hpp"
#include "common_ptts.hpp"
#include "player_ptts.hpp"
#include "utils/dirty_word_filter.hpp"
#include "child_ptts.hpp"
#include <map>

namespace nora {
        namespace config {

                bool check_conditions(const pd::condition_array& ca) {
                        static const map<pd::condition::condition_type, set<int>> condition_arg_size {
                                { pd::condition::PASS_ADVENTURE, {1}},
                                { pd::condition::ADVENTURE_NO_CHANCES, {0}},
                                { pd::condition::NOT_PASS_ADVENTURE, {1}},
                                { pd::condition::COST_RESOURCE, {2}},
                                { pd::condition::COST_ITEM, {1, 2}},
                                { pd::condition::RESOURCE_GE, {2}},
                                { pd::condition::LEVEL_GREATER_THAN_ACTOR_LEVEL, {0}},
                                { pd::condition::BUY_GOOD_TIMES_LESS, {3}},
                                { pd::condition::HAS_ACTOR, {1}},
                                { pd::condition::NO_ACTOR, {1}},
                                { pd::condition::LEVEL_GE, {1}},
                                { pd::condition::LEVEL_LE, {1}},
                                { pd::condition::LEVEL_GREATER_THAN_EQUIPMENT_LEVEL, {0}},
                                { pd::condition::LEVEL_GREATER_THAN_HUANZHUANG_LEVEL, {0}},
                                { pd::condition::ACTOR_INTIMACY_GE, {1, 2}},
                                { pd::condition::ACTOR_LEVEL_GE, {1, 2}},
                                { pd::condition::PASS_LIEMING, {1}},
                                { pd::condition::VIP_LEVEL_GE, {1}},
                                { pd::condition::GENDER, {1}},
                                { pd::condition::EQUIPMENT_EXP_GE, {1}},
                                { pd::condition::EQUIPMENT_QUALITY_GE, {1}},
                                { pd::condition::EQUIPMENT_LEVEL_GE, {1}},
                                { pd::condition::ACTOR_STAR_GE, {1, 2}},
                                { pd::condition::ACTOR_STEP_GE, {1}},
                                { pd::condition::ACTOR_QUALITY_GE, {1}},
                                { pd::condition::ACTOR_PIN_LE, {1, 2}},
                                { pd::condition::ACTOR_CRAFT_GE, {2}},
                                { pd::condition::ACTOR_CRAFT_E, {2}},
                                { pd::condition::PASS_PLOT, {1}},
                                { pd::condition::INTIMACY_GE, {1}},
                                { pd::condition::FAIL, {1}},
                                { pd::condition::NO_HUANZHUANG_COLLECTION, {2}},
                                { pd::condition::HUANZHUANG_EXP_GE, {1}},
                                { pd::condition::NO_SPINE_COLLECTION, {2}},
                                { pd::condition::ITEM_COUNT_LE, {1, 2}},
                                { pd::condition::RANDOM, {1}},
                                { pd::condition::FRIEND_COUNT_LE, {1}},
                                { pd::condition::LEAGUE_JOINED, {0}},
                                { pd::condition::MANSION_CREATED, {0}},
                                { pd::condition::MANSION_FANCY_GE, {1}},
                                { pd::condition::ROLE_PIN_LE, {1}},
                                { pd::condition::MANSION_BANQUET_PRESTIGE_LEVEL_GE, {1}},
                                { pd::condition::ROLE_PIN_E, {1}},
                                { pd::condition::RESOURCE_NOT_REACH_MAX, {1}},
                                { pd::condition::PLOT_OPTION_NOT_LOCKED, {2}},
                                { pd::condition::TOWER_CUR_LEVEL_E, {1}},
                                { pd::condition::TOWER_HIGHEST_LEVEL_GE, {1}},
                                { pd::condition::PLOT_OPTION_SELECTED, {2}},
                                { pd::condition::MAX_GPIN_GE, {1}},
                                { pd::condition::GPIN_LIMIT, {1, 2}},
                                { pd::condition::PASS_XINSHOU_GROUP, {1}},
                                { pd::condition::HAS_TITLE, {1}},
                                { pd::condition::ROLE_NOT_EQUIPED_EQUIPMENT_PART, {1}},
                                { pd::condition::ROLE_HAS_NOT_EQUIPED_EQUIPMENT, {1}},
                                { pd::condition::NOT_IN_LEAGUE, {0}},
                                { pd::condition::QUEST_COMMITTED, {1}},
                                { pd::condition::FORMATION_NO_ACTOR, {2}},
                                { pd::condition::FORMATION_GRID_NO_ACTOR, {3}},
                                { pd::condition::LOTTERY_FREE, {1}},
                                { pd::condition::MARRIED, {0}},
                                { pd::condition::NOT_MARRIED, {0}},
                                { pd::condition::EVER_MARRIED, {0}},
                                { pd::condition::SERVER_OPEN_DAY_IN, {2}},
                                { pd::condition::FIRST_LOGIN_DAY_E, {1}},
                                { pd::condition::FEIGE_ONGOING, {1}},
                                { pd::condition::FEIGE_FINISHED, {1}},
                                { pd::condition::MANSION_COOK_HAS_RECIPE, {1}},
                                { pd::condition::MANSION_FARM_PLOT_STATE, {1}},
                                { pd::condition::ARENA_BEST_RANK_GE, {1}},
                                { pd::condition::QUEST_ACCEPTED, {1}},
                                { pd::condition::LEAGUE_SHOP_LEVEL_IS, {1}},
                                { pd::condition::LEAGUE_SHOP_LEVEL_GE, {1}},
                                { pd::condition::HAS_BONUS_CITY, {1}},
                                { pd::condition::MANSION_HALL_QUEST_COUNT_GE, {1}},
                                { pd::condition::MANSION_FURNITURE_COUNT_GE, {2}},
                                { pd::condition::CHILD_PHASE_IN, {2}},
                                { pd::condition::LEVEL_GREATER_THAN_CHILD_LEVEL, {}},
                                { pd::condition::VIP_LEVEL_LE, {1}},
                                { pd::condition::SPOUSE_VIP_LEVEL_GE, {1}},
                                { pd::condition::SPOUSE_VIP_LEVEL_LE, {1}},
                                { pd::condition::ROLE_HAS_EQUIPMENT_PART_COUNT_GE, {2}}
                        };
                        auto ret = true;
                        static const set<string> opers{"<", ">", "<=", ">=", "!=", "=", "=="};
                        for (const auto& i : ca.conditions()) {
                                ASSERT(condition_arg_size.count(i.type()) > 0);
                                if (condition_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        ret = false;
                                }
                                switch (i.type()) {
                                case pd::condition::NO_HUANZHUANG_COLLECTION: {
                                        pd::huanzhuang_part part;
                                        if (!pd::huanzhuang_part_Parse(i.arg(0), &part)) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " wrong huanzhuang part " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::condition::NO_SPINE_COLLECTION: {
                                        pd::spine_part part;
                                        if (!pd::spine_part_Parse(i.arg(0), &part)) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " wrong spine part " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::condition::GPIN_LIMIT: {
                                        if (i.arg_size() >= 1) {
                                                if (opers.count(i.arg(0)) <= 0) {
                                                        CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " invalid arg " << i.arg(0);
                                                        ret = false;
                                                }
                                        }
                                        break;
                                }
                                case pd::condition::FORMATION_NO_ACTOR: {
                                        if (!PTTS_HAS(actor, stoul(i.arg(1)))) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " actor not exist " << i.arg(1);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::condition::QUEST_COMMITTED:
                                case pd::condition::QUEST_ACCEPTED: {
                                        if (!PTTS_HAS(quest, stoul(i.arg(0)))) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " quest not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                bool verify_conditions(const pd::condition_array& ca) {
                        auto ret = true;
                        for (const auto& i : ca.conditions()) {
                                switch (i.type()) {
                                case pd::condition::NO_HUANZHUANG_COLLECTION: {
                                        pd::huanzhuang_part part;
                                        if (!pd::huanzhuang_part_Parse(i.arg(0), &part)) {
                                                if (!HUANZHUANG_PTTS_HAS(part, stoul(i.arg(1)))) {
                                                        CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " huanzhuang not exist " << i.arg(0) << " " << i.arg(1);
                                                        ret = false;
                                                }
                                        }
                                        break;
                                }
                                case pd::condition::NO_SPINE_COLLECTION: {
                                        pd::spine_part part;
                                        if (!pd::spine_part_Parse(i.arg(0), &part)) {
                                                if (!SPINE_PTTS_HAS(part, stoul(i.arg(1)))) {
                                                        CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " spine not exist " << i.arg(0) << " " << i.arg(1);
                                                        ret = false;
                                                }
                                        }
                                        break;
                                }
                                case pd::condition::MANSION_FURNITURE_COUNT_GE: {
                                        uint32_t pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(mansion_furniture, pttid)) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " mansion furniture not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::condition::CHILD_PHASE_IN: {
                                        pd::child_phase phase;
                                        if (!pd::child_phase_Parse(i.arg(0), &phase)) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " invalid child phase " << i.arg(0);
                                                ret = false;
                                        }
                                        if (!pd::child_phase_Parse(i.arg(1), &phase)) {
                                                CONFIG_ELOG << pd::condition::condition_type_Name(i.type()) << " invalid child phase " << i.arg(1);
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                bool check_events(const pd::event_array& ea) {
                        static const map<pd::event::event_type, set<int>> event_arg_size {
                                { pd::event::ADD_RESOURCE, {2}},
                                { pd::event::ADD_RESOURCE_IGNORE_MAX, {2}},
                                { pd::event::DEC_RESOURCE, {2}},
                                { pd::event::RESET_RESOURCE, {2}},
                                { pd::event::ADD_ACTOR, {1}},
                                { pd::event::ADD_ITEM, {2}},
                                { pd::event::REMOVE_ITEM, {2}},
                                { pd::event::ADD_BUY_GOOD_TIMES, {2}},
                                { pd::event::PASS_ADVENTURE, {1}},
                                { pd::event::LEVELUP_ACTOR_SKILL, {1, 2}},
                                { pd::event::ADD_SPINE_COLLECTION, {2, 4}},
                                { pd::event::ADD_HUANZHUANG_COLLECTION, {2, 4}},
                                { pd::event::ACCEPT_QUEST, {1}},
                                { pd::event::REMOVE_QUEST, {1}},
                                { pd::event::ACTOR_LEVELUP, {0, 1}},
                                { pd::event::ADD_EQUIPMENT, {2}},
                                { pd::event::EQUIPMENT_LEVELUP, {0}},
                                { pd::event::EQUIPMENT_ADD_QUALITY, {0}},
                                { pd::event::EQUIPMENT_CHANGE_RAND_ATTR, {0}},
                                { pd::event::EQUIPMENT_XILIAN, {0}},
                                { pd::event::EQUIPMENT_XILIAN_CONFIRM, {0}},
                                { pd::event::LEVELUP, {0}},
                                { pd::event::CHANGE_ACTOR_INTIMACY, {1, 2, 3}},
                                { pd::event::ACTOR_ADD_STEP, {0, 1}},
                                { pd::event::ACTOR_ADD_STAR, {0}},
                                { pd::event::ACTOR_ADD_PIN, {0, 1}},
                                { pd::event::ROLE_ADD_FATE, {0}},
                                { pd::event::PASS_LIEMING, {1}},
                                { pd::event::REMOVE_EQUIPMENT, {0}},
                                { pd::event::ADD_EQUIPMENT_EXP, {1}},
                                { pd::event::DROP, {2}},
                                { pd::event::ACTOR_UNLOCK_SKILL, {1, 2}},
                                { pd::event::CHANGE_INTIMACY, {1}},
                                { pd::event::NONE, {0}},
                                { pd::event::ADD_TITLE, {1}},
                                { pd::event::ADD_PRESENT, {2}},
                                { pd::event::DEC_PRESENT, {2}},
                                { pd::event::MANSION_ADD_HALL_QUEST, {1}},
                                { pd::event::PASS_ADVENTURE_ALL, {0}},
                                { pd::event::PASS_ADVENTURE_CHAPTER, {1}},
                                { pd::event::PLOT_LOCK_OPTION, {2}},
                                { pd::event::PLOT_UNLOCK_OPTION, {2}},
                                { pd::event::TRIGGER_FEIGE, {1, 2}},
                                { pd::event::ADD_FEIGE_CHUANWEN, {1}},
                                { pd::event::ADD_FEIGE_CG, {1}},
                                { pd::event::ADD_FEIGE_MINGYUN, {1}},
                                { pd::event::MANSION_COOK_ADD_RECIPE, {1}},
                                { pd::event::DEC_GONGDOU_EFFECT, {1, 2}},
                                { pd::event::ACTOR_ADD_SKIN, {1}},
                                { pd::event::TRIGGER_MAJOR_CITY_BUBBLE, {1}},
                                { pd::event::ADD_HUANZHUANG_EXP, {1}},
                                { pd::event::RESET_HUANZHUANG, {1}},
                                { pd::event::HUANZHUANG_LEVELUP, {0}},
                                { pd::event::BROADCAST_SYSTEM_CHAT, {1}},
                                { pd::event::ADD_JADE, {2}},
                                { pd::event::REMOVE_JADE, {1}},
                                { pd::event::DROP_RECORD, {2}},
                                { pd::event::ADD_DAIYANREN_POINTS, {1}},
                        };
                        auto ret = true;
                        for (const auto& i : ea.events()) {
                                ASSERT(event_arg_size.count(i.type()) > 0);
                                if (event_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        return false;
                                }
                        }

                        for (const auto& i : ea.events()) {
                                switch (i.type()) {
                                case pd::event::ADD_RESOURCE:
                                case pd::event::ADD_RESOURCE_IGNORE_MAX: {
                                        pd::resource_type type;
                                        if (!pd::resource_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong resource type " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        if (stoi(i.arg(1)) <= 0) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::DEC_RESOURCE: {
                                        pd::resource_type type;
                                        if (!pd::resource_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong resource type " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        if (stoi(i.arg(1)) <= 0) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::RESET_RESOURCE: {
                                        pd::resource_type type;
                                        if (!pd::resource_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong resource type " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        if (stoi(i.arg(1)) <= 0) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::CHANGE_ACTOR_INTIMACY: {
                                        for (auto j = 0; j < i.arg_size() - 1; ++j) {
                                                auto pttid = stoul(i.arg(j));
                                                if (!PTTS_HAS(actor, pttid)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " actor not exist " << pttid;
                                                        ret = false;
                                                        continue;
                                                }
                                                const auto& actor_ptt = PTTS_GET(actor, pttid);
                                                if (actor_ptt.type() == pd::actor::ZHU) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " actor is zhu " << pttid;
                                                        ret = false;
                                                }
                                        }
                                        if (stoi(i.arg(i.arg_size() - 1)) <= 0) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(i.arg_size() - 1);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::DEC_GONGDOU_EFFECT: {
                                        if (i.arg_size() == 1) {
                                                if (stoi(i.arg(0)) <= 0) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(0);
                                                        ret = false;
                                                        continue;
                                                }

                                        } else if (i.arg_size() == 2) {
                                                pd::gongdou_type type;
                                                if (!pd::gongdou_type_Parse(i.arg(0), &type)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong resource type " << i.arg(0);
                                                        ret = false;
                                                        continue;
                                                }
                                                if (stoi(i.arg(1)) <= 0) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid count " << i.arg(1);
                                                        ret = false;
                                                        continue;
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_ADD_SKIN: {
                                        if (!PTTS_HAS(actor_skin, stoul(i.arg(0)))) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " skin not exist " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::ACCEPT_QUEST:
                                case pd::event::REMOVE_QUEST: {
                                        if (!PTTS_HAS(quest, stoul(i.arg(0)))) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " quest not exist " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }

                        return ret;
                }

                bool verify_events(const pd::event_array& ea) {
                        auto ret = true;
                        for (const auto& i : ea.events()) {
                                switch (i.type()) {
                                case pd::event::ADD_ITEM: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(item, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " item not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::ADD_EQUIPMENT: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(equipment, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " equipment not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::ADD_JADE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(jade, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " jade not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::event::ADD_SPINE_COLLECTION: {
                                        pd::spine_part part;
                                        if (!pd::spine_part_Parse(i.arg(0), &part)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong spine part " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        auto pttid = stoul(i.arg(1));
                                        if (!SPINE_PTTS_HAS(part, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " spine not exist " << pd::spine_part_Name(part) << " " << pttid;
                                                ret = false;
                                                continue;
                                        }

                                        if (i.arg_size() == 4) {
                                                if (!pd::spine_part_Parse(i.arg(2), &part)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong spine part " << i.arg(2);
                                                        ret = false;
                                                        continue;
                                                }
                                                pttid = stoul(i.arg(3));
                                                if (!SPINE_PTTS_HAS(part, pttid)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " spine not exist " << pd::spine_part_Name(part) << " " << pttid;
                                                        ret = false;
                                                        continue;
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ADD_HUANZHUANG_COLLECTION:{
                                        pd::huanzhuang_part part;
                                        if (!pd::huanzhuang_part_Parse(i.arg(0), &part)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong huanzhuang part " << i.arg(0);
                                                ret = false;
                                                continue;
                                        }
                                        auto pttid = stoul(i.arg(1));
                                        if (!HUANZHUANG_PTTS_HAS(part, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " huanzhuang not exist " << pd::huanzhuang_part_Name(part) << " " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        if (i.arg_size() == 4) {
                                                if (!pd::huanzhuang_part_Parse(i.arg(2), &part)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " wrong huanzhuang part " << i.arg(2);
                                                        ret = false;
                                                        continue;
                                                }
                                                pttid = stoul(i.arg(3));
                                                if (!HUANZHUANG_PTTS_HAS(part, pttid)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " huanzhuang not exist " << pd::huanzhuang_part_Name(part) << " " << pttid;
                                                        ret = false;
                                                        continue;
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ACTOR_UNLOCK_SKILL: {
                                        if (i.arg_size() == 1) {
                                                if (!PTTS_HAS(skill, stoul(i.arg(0)))) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " skill not exist " << i.arg(0);
                                                        ret = false;
                                                        continue;
                                                }
                                        } else if (i.arg_size() == 2) {
                                                if (!PTTS_HAS(actor, stoul(i.arg(0)))) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " actor not exist " << i.arg(0);
                                                        ret = false;
                                                        continue;
                                                }
                                                const auto& actor_ptt = PTTS_GET(actor, stoul(i.arg(0)));
                                                auto found = false;
                                                for (auto j : actor_ptt.lock_skill()) {
                                                        if (j == stoul(i.arg(1))) {
                                                                found = true;
                                                                break;
                                                        }
                                                }
                                                if (!found) {
                                                        CONFIG_ELOG << i.arg(0) << " " << pd::event::event_type_Name(i.type()) << " skill not in role lock skill " << i.arg(1);
                                                        ret = false;
                                                        continue;
                                                }
                                                if (!PTTS_HAS(skill, stoul(i.arg(1)))) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " skill not exist " << i.arg(1);
                                                        ret = false;
                                                        continue;
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ADD_TITLE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(title, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " title not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::PLOT_LOCK_OPTION:
                                case pd::event::PLOT_UNLOCK_OPTION: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(plot_options, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " plot options not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        const auto& ptt = PTTS_GET(plot_options, pttid);
                                        auto idx = stoi(i.arg(1));
                                        if (idx < 0 || idx >= ptt.options_size()) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " invalid option: " << idx << " options size: " << ptt.options_size();
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::TRIGGER_FEIGE: {
                                        for (auto j = 0; j < i.arg_size(); ++j) {
                                                auto pttid = stoul(i.arg(j));
                                                if (!PTTS_HAS(feige, pttid)) {
                                                        CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " feige not exist " << i.arg(0);
                                                        ret = false;
                                                        break;
                                                }
                                        }
                                        break;
                                }
                                case pd::event::ADD_FEIGE_CHUANWEN: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(feige_chuanwen, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " feige chuanwen not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::ADD_FEIGE_MINGYUN:
                                case pd::event::ADD_FEIGE_CG: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(feige_cg, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " feige cg not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::MANSION_COOK_ADD_RECIPE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(mansion_cook_recipe, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " mansion cook recipe not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::TRIGGER_MAJOR_CITY_BUBBLE: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(major_city_bubble, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " major city bubble not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::event::BROADCAST_SYSTEM_CHAT: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(system_chat, pttid)) {
                                                CONFIG_ELOG << pd::event::event_type_Name(i.type()) << " system chat not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }

                        return ret;
                }

                bool check_condevents(const pd::condevent_array& cea) {
                        bool ret = true;
                        for (const auto& i : cea.condevents()) {
                                if (!check_conditions(i.conditions())) {
                                        CONFIG_ELOG << "check condevents conditions failed";
                                        ret = false;
                                }

                                if (!check_events(i.events())) {
                                        CONFIG_ELOG << "check condevents events failed";
                                        ret = false;
                                }
                        }
                        return ret;
                }

                bool verify_condevents(const pd::condevent_array& cea) {
                        bool ret = true;
                        for (const auto& i : cea.condevents()) {
                                if (!verify_conditions(i.conditions())) {
                                        CONFIG_ELOG << "verify condevents conditions failed";
                                        ret = false;
                                }

                                if (!verify_events(i.events())) {
                                        CONFIG_ELOG << "verify condevents events failed";
                                        ret = false;
                                }
                        }
                        return ret;
                }

                void modify_events_by_conditions(const pd::condition_array& ca, pd::event_array& ea) {
                        for (const auto& i : ca.conditions()) {
                                switch (i.type()) {
                                case pd::condition::COST_RESOURCE: {
                                        auto *e = ea.add_events();
                                        e->set_type(pd::event::DEC_RESOURCE);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                case pd::condition::COST_ITEM: {
                                        auto *e = ea.add_events();
                                        e->set_type(pd::event::REMOVE_ITEM);
                                        for (auto j = 0; j < i.arg_size(); ++j) {
                                                e->add_arg(i.arg(j));
                                        }
                                        break;
                                }
                                case pd::condition::COST_PRESENT: {
                                        auto *e = ea.add_events();
                                        e->set_type(pd::event::DEC_PRESENT);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                void modify_conditions_by_events(const pd::event_array& ea, pd::condition_array& ca) {
                        for (const auto& i : ea.events()) {
                                switch (i.type()) {
                                case pd::event::ADD_RESOURCE: {
                                        auto *c = ca.add_conditions();
                                        c->set_type(pd::condition::RESOURCE_NOT_REACH_MAX);
                                        c->add_arg(i.arg(0));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                bool check_fief_events(pd::fief_event_array& fea, bool silent) {
                        static map<pd::fief_event::fief_event_type, set<int>> event_arg_size {
                                { pd::fief_event::ADD_RESOURCE, {2}},
                                { pd::fief_event::DEC_RESOURCE, {2}},
                                { pd::fief_event::INCIDENT_EXTRA_WEIGHT, {2}},
                        };
                        auto ret = true;
                        for (const auto& i : fea.events()) {
                                ASSERT(event_arg_size.count(i.type()) > 0);
                                if (event_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        ret = false;
                                        if (!silent) {
                                                CONFIG_ELOG << pd::fief_event::fief_event_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        }
                                        continue;
                                }
                                switch (i.type()) {
                                case pd::fief_event::ADD_RESOURCE: {
                                        pd::fief_resource_type type;
                                        if (!pd::fief_resource_type_Parse(i.arg(0), &type)) {
                                                if (!silent) {
                                                        CONFIG_ELOG <<"ADD_RESOURCE has invalid resource type \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        if (stoul(i.arg(1)) <= 0) {
                                                if (!silent) {
                                                        CONFIG_ELOG << "add_resource count <= 0 \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::fief_event::DEC_RESOURCE: {
                                        pd::fief_resource_type type;
                                        if (!pd::fief_resource_type_Parse(i.arg(0), &type)) {
                                                if (!silent) {
                                                        CONFIG_ELOG <<"DEC_RESOURCE has invalid resource type \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        if (stoul(i.arg(1)) <= 0) {
                                                if (!silent) {
                                                        CONFIG_ELOG << "dec_resource count <= 0 \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::fief_event::INCIDENT_EXTRA_WEIGHT: {
                                        pd::fief_resource_type type;
                                        if (!pd::fief_resource_type_Parse(i.arg(0), &type)) {
                                                if (!silent) {
                                                        CONFIG_ELOG <<"INCIDENT_EXTRA_WEIGHT has invalid resource type \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        if (stoul(i.arg(1)) <= 0) {
                                                if (!silent) {
                                                        CONFIG_ELOG << "include_extra_weight count <= 0 \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        break;
                                }
                                }
                        }
                        return ret;
                }

                void modify_fief_events_by_fief_conditions(const pd::fief_condition_array& fca, pd::fief_event_array& fea) {
                        for (const auto& i : fca.conditions()) {
                                switch (i.type()) {
                                case pd::fief_condition::COST_RESOURCE: {
                                        auto *e = fea.add_events();
                                        e->set_type(pd::fief_event::DEC_RESOURCE);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                bool check_league_conditions(const pd::league_condition_array& lca) {
                        static map<pd::league_condition_type, set<int>> condition_arg_size {
                                { pd::LCT_COST_RESOURCE, {2}},
                                { pd::LCT_LEVEL_GE, {1}},
                                { pd::LCT_PASS_CAMPAIGN, {1}},
                                { pd::LCT_FAIL, {1}},
                                { pd::LCT_MEMBER_COUNT_GE, {1}},
                                        };
                        auto ret = true;
                        for (const auto& i : lca.conditions()) {
                                ASSERT(condition_arg_size.count(i.type()) > 0);
                                if (condition_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::league_condition_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        ret = false;
                                }
                        }
                        return ret;
                }

                bool check_league_events(const pd::league_event_array& lea, bool silent) {
                        static map<pd::league_event_type, set<int>> event_arg_size {
                                { pd::LET_ADD_RESOURCE, {2}},
                                { pd::LET_DEC_RESOURCE, {2}},
                                { pd::LET_LEVEL_UP, {0}},
                                { pd::LET_ADD_ALLOCATABLE_ITEM, {2}},
                                { pd::LET_DEC_ALLOCATABLE_ITEM, {2}},
                                { pd::LET_DROP, {1}},
                                { pd::LET_PASS_CAMPAIGN, {1}},
                                { pd::LET_PASS_ALL_CAMPAIGN, {0}},
                        };
                        auto ret = true;
                        for (const auto& i : lea.events()) {
                                ASSERT(event_arg_size.count(i.type()) > 0);
                                if (event_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::league_event_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        ret = false;
                                }
                        }

                        for (const auto& i : lea.events()) {
                                switch (i.type()) {
                                case pd::LET_ADD_RESOURCE: {
                                        pd::league_resource_type type;
                                        if (!pd::league_resource_type_Parse(i.arg(0), &type)) {
                                                if (!silent) {
                                                        CONFIG_ELOG <<"LET_ADD_RESOURCE has invalid resource type \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        if (stoul(i.arg(1)) <= 0) {
                                                if (!silent) {
                                                        CONFIG_ELOG << "add_resource count <= 0 \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::LET_DEC_RESOURCE: {
                                        pd::league_resource_type type;
                                        if (!pd::league_resource_type_Parse(i.arg(0), &type)) {
                                                if (!silent) {
                                                        CONFIG_ELOG <<"LET_DEC_RESOURCE has invalid resource type \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        if (stoul(i.arg(1)) <= 0) {
                                                if (!silent) {
                                                        CONFIG_ELOG << "dec_resource count <= 0 \n" << i.DebugString();
                                                }
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }

                        return ret;
                }

                bool verify_league_events(const pd::league_event_array& lea) {
                        auto ret = true;
                        for (const auto& i : lea.events()) {
                                switch (i.type()) {
                                case pd::LET_ADD_ALLOCATABLE_ITEM: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(item, pttid)) {
                                                CONFIG_ELOG << pd::league_event_type_Name(i.type()) << " item not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::LET_DEC_ALLOCATABLE_ITEM: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(item, pttid)) {
                                                CONFIG_ELOG << pd::league_event_type_Name(i.type()) << " item not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                case pd::LET_DROP: {
                                        auto pttid = stoul(i.arg(0));
                                        if (!PTTS_HAS(league_drop, pttid)) {
                                                CONFIG_ELOG << pd::league_event_type_Name(i.type()) << " league drop not exist " << pttid;
                                                ret = false;
                                                continue;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                void modify_league_events_by_league_conditions(const pd::league_condition_array& lca, pd::league_event_array& lea) {
                        for (const auto& i : lca.conditions()) {
                                switch (i.type()) {
                                case pd::LCT_COST_RESOURCE: {
                                        auto *e = lea.add_events();
                                        e->set_type(pd::LET_DEC_RESOURCE);
                                        e->add_arg(i.arg(0));
                                        e->add_arg(i.arg(1));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                bool check_child_conditions(const pd::child_condition_array& cca) {
                        static map<pd::child_condition_type, set<int>> condition_arg_size {
                                { pd::CCT_LEVEL_GE, {1}},
                                { pd::CCT_VALUE_NOT_FULL, {1}},
                                { pd::CCT_PHASE_IN, {2}},
                                { pd::CCT_VALUE_GE, {2}},
                                { pd::CCT_VALUE_LE, {2}},
                                { pd::CCT_SKILL_SLOT_EMPTY, {1}},
                                { pd::CCT_STUDY_LEVEL_IN, {3}},
                                { pd::CCT_ACTOR_STAR_GE, {2}},
                                { pd::CCT_FAIL, {1}},
                                        };
                        auto ret = true;
                        for (const auto& i : cca.conditions()) {
                                ASSERT(condition_arg_size.count(i.type()) > 0);
                                if (condition_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        ret = false;
                                        continue;
                                }

                                switch (i.type()) {
                                case pd::CCT_VALUE_NOT_FULL:
                                case pd::CCT_VALUE_LE:
                                case pd::CCT_VALUE_GE: {
                                        pd::child_value_type type;
                                        if (!pd::child_value_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " invalid child value type " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::CCT_PHASE_IN: {
                                        pd::child_phase phase;
                                        if (!pd::child_phase_Parse(i.arg(0), &phase)) {
                                                CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " invalid phase " << i.arg(0);
                                                ret = false;
                                        }
                                        if (!pd::child_phase_Parse(i.arg(1), &phase)) {
                                                CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " invalid phase " << i.arg(1);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::CCT_STUDY_LEVEL_IN: {
                                        pd::actor::attr_type type;
                                        if (!pd::actor::attr_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " invalid attr type " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::CCT_ACTOR_STAR_GE: {
                                        auto actor = stoul(i.arg(0));
                                        if (!PTTS_HAS(actor, actor)) {
                                                CONFIG_ELOG << pd::child_condition_type_Name(i.type()) << " actor not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                bool check_child_events(const pd::child_event_array& cea) {
                        static map<pd::child_event_type, set<int>> event_arg_size {
                                { pd::CET_CHANGE_PHASE, {0}},
                                { pd::CET_CHANGE_VALUE, {2}},
                                { pd::CET_LEVELUP, {0}},
                                { pd::CET_LEARN_SKILL, {1}},
                                { pd::CET_LEVELUP_SKILL, {1}},
                                { pd::CET_STUDY_LEVELUP, {1}},
                                { pd::CET_DROP, {1}},
                                { pd::CET_CHANGE_CONTRIBUTION, {1}},
                                { pd::CET_NONE, {0}},
                        };
                        auto ret = true;
                        for (const auto& i : cea.events()) {
                                ASSERT(event_arg_size.count(i.type()) > 0);
                                if (event_arg_size.at(i.type()).count(i.arg_size()) <= 0) {
                                        CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " wrong arg size " << i.arg_size();
                                        ret = false;
                                        continue;
                                }

                                switch (i.type()) {
                                case pd::CET_CHANGE_VALUE: {
                                        pd::child_value_type type;
                                        if (!pd::child_value_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " invalid child value type " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::CET_STUDY_LEVELUP: {
                                        pd::actor::attr_type type;
                                        if (!pd::actor::attr_type_Parse(i.arg(0), &type)) {
                                                CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " invalid actor attr type " << i.arg(0);
                                                ret = false;
                                                break;
                                        }
                                        if (type != pd::actor::HP && type != pd::actor::ATTACK && type != pd::actor::PHYSICAL_DEFENCE && type != pd::actor::MAGICAL_DEFENCE) {
                                                CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " wrong actor attr type " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                case pd::CET_LEARN_SKILL:
                                case pd::CET_LEVELUP_SKILL: {
                                        auto skill = stoul(i.arg(0));
                                        if (!PTTS_HAS(child_skill, skill)) {
                                                CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " child skill not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                bool verify_child_conditions(const pd::child_condition_array& cca) {
                        return true;
                }

                bool verify_child_events(const pd::child_event_array& cea) {
                        auto ret = true;
                        for (const auto& i : cea.events()) {
                                switch (i.type()) {
                                case pd::CET_DROP:
                                        if (i.arg_size() > 0 && !PTTS_HAS(child_drop, stoul(i.arg(0)))) {
                                                CONFIG_ELOG << pd::child_event_type_Name(i.type()) << " child drop not exist " << i.arg(0);
                                                ret = false;
                                        }
                                        break;
                                default:
                                        break;
                                }
                        }
                        return ret;
                }

                void modify_child_events_by_child_conditions(const pd::child_condition_array& cca, pd::child_event_array& cea) {
                }

                pc::spine_item& spine_ptts_get(pd::spine_part part, uint32_t pttid) {
                        switch (part) {
                        case pd::SP_HONGMO:
                                return PTTS_GET(hongmo, pttid);
                        case pd::SP_YANJING:
                                return PTTS_GET(yanjing, pttid);
                        case pd::SP_YANSHENGUANG:
                                return PTTS_GET(yanshenguang, pttid);
                        case pd::SP_SAIHONG:
                                return PTTS_GET(saihong, pttid);
                        case pd::SP_MEIMAO:
                                return PTTS_GET(meimao, pttid);
                        case pd::SP_YANZHUANG:
                                return PTTS_GET(yanzhuang, pttid);
                        case pd::SP_CHUNCAI:
                                return PTTS_GET(chuncai, pttid);
                        case pd::SP_TIEHUA:
                                return PTTS_GET(tiehua, pttid);
                        case pd::SP_LIANXING:
                                return PTTS_GET(lianxing, pttid);
                        case pd::SP_DEFORM_FACE:
                                return PTTS_GET(spine_deform_face, pttid);
                        case pd::SP_DEFORM_HEAD:
                                return PTTS_GET(spine_deform_head, pttid);
                        case pd::SP_ZUI:
                                return PTTS_GET(zui, pttid);
                        case pd::SP_HUZI:
                                return PTTS_GET(huzi, pttid);
                        case pd::SP_LIAN:
                                return PTTS_GET(lian, pttid);
                        default:
                                break;
                        }
                        THROW(not_found);
                }

                bool spine_ptts_has(pd::spine_part part, uint32_t pttid) {
                        switch (part) {
                        case pd::SP_HONGMO:
                                return PTTS_HAS(hongmo, pttid);
                        case pd::SP_YANJING:
                                return PTTS_HAS(yanjing, pttid);
                        case pd::SP_YANSHENGUANG:
                                return PTTS_HAS(yanshenguang, pttid);
                        case pd::SP_SAIHONG:
                                return PTTS_HAS(saihong, pttid);
                        case pd::SP_MEIMAO:
                                return PTTS_HAS(meimao, pttid);
                        case pd::SP_YANZHUANG:
                                return PTTS_HAS(yanzhuang, pttid);
                        case pd::SP_CHUNCAI:
                                return PTTS_HAS(chuncai, pttid);
                        case pd::SP_TIEHUA:
                                return PTTS_HAS(tiehua, pttid);
                        case pd::SP_LIANXING:
                                return PTTS_HAS(lianxing, pttid);
                        case pd::SP_DEFORM_FACE:
                                return PTTS_HAS(spine_deform_face, pttid);
                        case pd::SP_DEFORM_HEAD:
                                return PTTS_HAS(spine_deform_head, pttid);
                        case pd::SP_ZUI:
                                return PTTS_HAS(zui, pttid);
                        case pd::SP_HUZI:
                                return PTTS_HAS(huzi, pttid);
                        case pd::SP_LIAN:
                                return PTTS_HAS(lian, pttid);
                        default:
                                break;
                        }
                        THROW(not_found);
                }

                pc::huanzhuang_item& huanzhuang_ptts_get(pd::huanzhuang_part part, uint32_t pttid) {
                        switch (part) {
                        case pd::HP_YIFU:
                                return PTTS_GET(yifu, pttid);
                        case pd::HP_TOUFA:
                                return PTTS_GET(toufa, pttid);
                        case pd::HP_JIEZHI:
                                return PTTS_GET(jiezhi, pttid);
                        case pd::HP_EDAI:
                                return PTTS_GET(edai, pttid);
                        case pd::HP_FAZAN:
                                return PTTS_GET(fazan, pttid);
                        case pd::HP_GUANSHI:
                                return PTTS_GET(guanshi, pttid);
                        case pd::HP_TOUJIN:
                                return PTTS_GET(toujin, pttid);
                        case pd::HP_ERHUAN:
                                return PTTS_GET(erhuan, pttid);
                        case pd::HP_XIANGLIAN:
                                return PTTS_GET(xianglian, pttid);
                        case pd::HP_BEISHI:
                                return PTTS_GET(beishi, pttid);
                        case pd::HP_GUANGHUAN:
                                return PTTS_GET(guanghuan, pttid);
                        case pd::HP_SHOUCHI:
                                return PTTS_GET(shouchi, pttid);
                        case pd::HP_DEFORM_FACE:
                                return PTTS_GET(huanzhuang_deform_face, pttid);
                        case pd::HP_DEFORM_HEAD:
                                return PTTS_GET(huanzhuang_deform_head, pttid);
                        default:
                                break;
                        }
                        THROW(not_found);
                }

                bool huanzhuang_ptts_has(pd::huanzhuang_part part, uint32_t pttid) {
                        switch (part) {
                        case pd::HP_YIFU:
                                return PTTS_HAS(yifu, pttid);
                        case pd::HP_TOUFA:
                                return PTTS_HAS(toufa, pttid);
                        case pd::HP_JIEZHI:
                                return PTTS_HAS(jiezhi, pttid);
                        case pd::HP_EDAI:
                                return PTTS_HAS(edai, pttid);
                        case pd::HP_FAZAN:
                                return PTTS_HAS(fazan, pttid);
                        case pd::HP_GUANSHI:
                                return PTTS_HAS(guanshi, pttid);
                        case pd::HP_TOUJIN:
                                return PTTS_HAS(toujin, pttid);
                        case pd::HP_ERHUAN:
                                return PTTS_HAS(erhuan, pttid);
                        case pd::HP_XIANGLIAN:
                                return PTTS_HAS(xianglian, pttid);
                        case pd::HP_BEISHI:
                                return PTTS_HAS(beishi, pttid);
                        case pd::HP_GUANGHUAN:
                                return PTTS_HAS(guanghuan, pttid);
                        case pd::HP_SHOUCHI:
                                return PTTS_HAS(shouchi, pttid);
                        case pd::HP_DEFORM_FACE:
                                return PTTS_HAS(huanzhuang_deform_face, pttid);
                        case pd::HP_DEFORM_HEAD:
                                return PTTS_HAS(huanzhuang_deform_head, pttid);
                        default:
                                break;
                        }
                        THROW(not_found);
                }

                void load_dirty_word() {
                        const auto& ptts = PTTS_GET_ALL(zangzi);
                        for (const auto& i : ptts) {
                                for (const auto& j : i.second.words()) {
                                        dirty_word_filter::instance().insert(j);
                                }
                        }
                }

                set<uint32_t> quest_to_all_quests(uint32_t quest_pttid) {
                        set<uint32_t> ret;
                        ASSERT(PTTS_HAS(quest, quest_pttid));
                        ret.insert(quest_pttid);

                        const auto& ptt = PTTS_GET(quest, quest_pttid);
                        for (const auto& i : ptt.targets()) {
                                for (const auto& j : i.params()) {
                                        for (const auto& k : j.pass_events().events()) {
                                                if (k.type() == pd::event::ACCEPT_QUEST) {
                                                        ASSERT(k.arg_size() == 1);
                                                        auto quests = quest_to_all_quests(stoul(k.arg(0)));
                                                        ret.insert(quests.begin(), quests.end());
                                                }
                                        }
                                }
                        }

                        return ret;
                }

                void load_config() {
                        PTTS_SET_FUNCS(options);

                        PTTS_LOAD(options);
                }

                void check_config() {
                        PTTS_MVP(options);
                }

                void write_all() {
                        PTTS_WRITE_ALL(options);
                }

        }
}
