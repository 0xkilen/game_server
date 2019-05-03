#include "quest_ptts.hpp"
#include "item_ptts.hpp"
#include "adventure_ptts.hpp"
#include "resource_ptts.hpp"
#include "utils.hpp"
#include "proto/data_event.pb.h"
#include "proto/data_quest.pb.h"
#include "proto/data_mansion.pb.h"
#include "utils/assert.hpp"
#include <map>

namespace pd = proto::data;
namespace pcs = proto::config;

namespace nora {
        namespace config {

                quest_ptts& quest_ptts_instance() {
                        static quest_ptts inst;
                        return inst;
                }

                void quest_ptts_set_funcs() {
                        quest_ptts_instance().check_func_ = [] (const auto& ptt) {
                                map<pd::quest_target_type, set<int>> targets_arg_size {
                                        { pd::GET_ACTOR_QUALITY_GE_TIMES, {1}},
                                        { pd::LEVELUP_EQUIPMENT_TIMES, {0}},
                                        { pd::FORMATION_GRIDS_ACTOR_COUNT, {0}},
                                        { pd::WORLD_CHAT_TIMES, {0}},
                                        { pd::PRIVATE_CHAT_TIMES, {0} },
                                        { pd::CHALLENGE_ARENA_TIMES, {0}},
                                        { pd::PASS_ADVENTURE, {1}},
                                        { pd::LEVELUP_ACTOR_TIMES, {0}},
                                        { pd::LEAGUE_DONATE_TIMES, {0}},
                                        { pd::LEVELUP_REACH, {0}},
                                        { pd::TURN_IN_ITEM, {1}},
                                        { pd::TURN_IN_RESOURCE, {1}},
                                        { pd::ZHANLI_REACH, {0}},
                                        { pd::PLAYER_SKILL_UNLOCK_COUNT, {0}},
                                        { pd::PLAYER_SKILL_LEVEL_LIMIT, {1}},
                                        { pd::COST_USE_ITEM_COUNT, {1}},
                                        { pd::ANY_EQUIPMENT_LEVEL_REACH, {0}},
                                        { pd::COLLECT_SUIT_COUNT, {0}},
                                        { pd::SUIT_ALL_PART_REACH_LEVEL, {1}},
                                        { pd::PASS_CHAPTER_ADVENTURE, {1}},
                                        { pd::FORMATION_ACTOR_QUALITY_GE_COUNT, {1}},
                                        { pd::ADD_FATE, {0}},
                                        { pd::ADD_FATE_TIMES, {0}},
                                        { pd::FIEF_BOSS_TOTAL_DAMAGE, {0}},
                                        { pd::LEAGUE_BOSS_TOTAL_DAMAGE, {0}},
                                        { pd::SINGLE_BATTLE_VALUE_REACH, {0}},
                                        { pd::HUANZHUANG_ADVENTURE_HIGHEST_LEVEL, {0}},
                                        { pd::HUANZHUANG_COMPOSE_TIMES_GE, {0}},
                                        { pd::HUANZHUANG_LEVELUP_TIMES_GE, {0}},
                                        { pd::HUANZHUANG_INTENSIFY_TIMES_GE, {0}},
                                        { pd::ARENA_RANK_REACH, {1}},
                                        { pd::ARENA_WIN_TIMES, {0}},
                                        { pd::PASS_LIEMING, {1}},
                                        { pd::JOIN_LIEMING_BATTLE_TIMES, {0}},
                                        { pd::DRAW_LOTTERY_GOLD_TEN_TIMES, {1}},
                                        { pd::DRAW_LOTTERY_DIAMOND_TEN_TIMES, {1}},
                                        { pd::FRIEND_NUMBER_GE, {0}},
                                        { pd::LEAGUE_LEVEL_REACH, {0}},
                                        { pd::LEAGUE_KILL_BOSS_TIMES, {0}},
                                        { pd::LEAGUE_SEND_LUCKY_BAG_GE_COUNT, {0}},
                                        { pd::LEAGUE_GET_LUCKY_BAG_KING_GE_TIMES, {0}},
                                        { pd::LEAGUE_GET_LUCKY_BAG_DIAMAND, {0}},
                                        { pd::LEAGUE_QUEST_FINISH_TIMES, {0}},
                                        { pd::LEAGUE_HELP_OTHER_TIMES, {0}},
                                        { pd::MANSION_FISHING_GET, {0}},
                                        { pd::MANSION_VISIT_TIMES, {0}},
                                        { pd::MANSION_POISON_BANQUET_TIMES, {0}},
                                        { pd::MANSION_FURNITURE_GE, {0}},
                                        { pd::MANSION_FANCY_GE, {0}},
                                        { pd::MANSION_HELP_QUEST_TIMES, {0}},
                                        { pd::MANSION_UPVOTED_TIMES, {0}},
                                        { pd::MANSION_BANQUET_PRESTIGE_LEVEL_GE, {0}},
                                        { pd::MANSION_BANQUET_ATMOSPHERE_GE, {0}},
                                        { pd::MANSION_BANQUET_ACTIVITY_GE, {0}},
                                        { pd::MANSION_BANQUET_ANSWER_QUESTION_COUNT_GE, {0}},
                                        { pd::MANSION_BANQUET_EAT_DISH_LEVEL_GE, {0}},
                                        { pd::MANSION_BANQUET_USE_FIREWORK_COUNT_GE, {0}},
                                        { pd::ACTOR_STAR_GE, {1}},
                                        { pd::ACTOR_ACTOR_STAR_GE, {0}},
                                        { pd::FIEF_DEFEAT_THIEF_TIMES, {0}},
                                        { pd::FIEF_DEFEAT_BOSS_TIMES, {0}},
                                        { pd::HUANZHUANG_ZHUANG_XIA_GE, {0}},
                                        { pd::HUANZHUANG_YI_CHU_GE, {0}},
                                        { pd::CHALLENGE_HUANZHUANG_ADVENTURE_TIMES, {0}},
                                        { pd::FIEF_TRIGGER_EVENT_TIMES, {0}},
                                        { pd::FIEF_CHALLENGE_BOSS_TIMES, {0}},
                                        { pd::ACTOR_ADD_STEP_TIMES, {0}},
                                        { pd::MANSION_COMMIT_HALL_QUEST_TIMES, {0}},
                                        { pd::MANSION_COMMIT_QUEST_TIMES, {0}},
                                        { pd::MANSION_ENTER_BANQUET_TIMES, {0}},
                                        { pd::LEAGUE_SEND_LUCKY_BAG_TIMES, {0}},
                                        { pd::LEAGUE_CAMPAIGN_TIMES, {0}},
                                        { pd::LEAGUE_TROUBLE_TIMES, {0}},
                                        { pd::LEAGUE_TRADE_TIMES, {0}},
                                        { pd::CUR_DAY_RECHARGE_GE, {0}},
                                        { pd::RECHARGE_GE, {0}},
                                        { pd::GET_ACTOR_PIN_GE_COUNT, {1} },
                                        { pd::GET_ACTOR_LEVEL_GE_COUNT, {1} },
                                        { pd::LEIJI_CONSUME_RESOURCES_GE, {1} },
                                        { pd::COST_RESOURCES_GE, {1} },
                                        { pd::OWN_RESOURCES_GE, {1} },
                                        { pd::SEND_GIFT_TO_FRIEND_GE_COUNT, {0} },
                                        { pd::SEND_GIFT_TO_SELF_LEAGUE_TIMES, {0} },
                                        { pd::PASS_LIEZHUAN_CHAPTER_COUNT_GE, {0} },
                                        { pd::PASS_LIEMING_SPECIAL_BATTLE_TIMES, {0} },
                                        { pd::HUANZHUANG_PVP_UPDATE_SELECTION, {0} },
                                        { pd::HUANZHUANG_PVP_VOTE, {0} },
                                        { pd::HUANZHUANG_PVP_RANK_GE, {1} },
                                        { pd::HUANZHUANG_PVP_NEW_HIGH_RANK_REACH, {0} },
                                        { pd::HUANZHUANG_COLLECTION, {1} },
                                        { pd::HUANZHUANG_PVP_GUESS_VOTE_TIMES_GE, {0} },
                                        { pd::HUANZHUANG_PART_ITEM_COUNT_GE, {3} },
                                        { pd::MANSION_ACCEPT_QUEST_TIMES, {0} },
                                        { pd::MANSION_ACCEPT_OTHER_QUEST_TIMES, {0} },
                                        { pd::MANSION_ADD_FURNITURE_GE, {1} },
                                        { pd::MANSION_BUILDING_LEVEL_GE, {1} },
                                        { pd::MASNION_GAME_FISHING_TIMES, {0} },
                                        { pd::MANSION_TEAM_GAME_FISHING_TIMES, {0} },
                                        { pd::RELATION_FRIEND_INTIMACY_GE, {0} },
                                        { pd::GONGDOU_TIMES, {0} },
                                        { pd::GONGDOU_TO_SELF_LEAGUE_TIMES, {0} },
                                        { pd::SHOP_BUY_GE, {1} },
                                        { pd::ACTOR_LEVEL_GE, {1} },
                                        { pd::ACTOR_PIN_GE, {2} },
                                        { pd::ACTOR_ADD_PIN_TIMES, {0} },
                                        { pd::ACTOR_STEP_GE, {2} },
                                        { pd::ACTOR_ADD_STAR_TIMES_GE, {0} },
                                        { pd::ACTOR_STAR_COUNT_GE, {1} },
                                        { pd::FIEF_JOINED_AREA_INCIDENT_COUNT_GE, {0} },
                                        { pd::FIEF_OPEN_AREA_INCIDENT_COUNT_GE, {0} },
                                        { pd::FIEF_CHALLENGED_BOSS_LEVEL_GE, {0} },
                                        { pd::FIEF_SET_BUILDING_MANAGER, {1} },
                                        { pd::FIEF_SET_BUILDING_ASSISTANTS, {1} },
                                        { pd::FIEF_BUILDING_MANAGER_CRAFTS_GE, {1} },
                                        { pd::FIEF_BUILDING_LEVEL_GE, {1} },
                                        { pd::FIEF_LEVEL_GE, {1} },
                                        { pd::PASS_ADVENTURE_TIMES_GE, {1} },
                                        { pd::PASS_EPISODE_ADVENTURE_COUNT_GE, {1} },
                                        { pd::ACTOR_ALL_STAR_GE, {0} },
                                        { pd::EQUIPMENT_COMPOSE_TIMES, {0} },
                                        { pd::MYS_SHOP_REFRESH_TIMES, {0} },
                                        { pd::EQUIPMENT_HIGHEST_SCORE_GE, {0} },
                                        { pd::SPINE_ADD_COLLECTION_GE, {0} },
                                        { pd::DRAW_LOTTERY_TIMES, {1, 2} },
                                        { pd::GUANPIN_GE, {1} },
                                        { pd::GUANPIN_CHALLENGE_TIMES, {0} },
                                        { pd::GUANPIN_COLLECT_FENGLU_TIMES, {0} },
                                        { pd::ACTIVITY_FESTIVAL_RECHARGE_MONEY_GE, {0} },
                                        { pd::TOWER_CHALLENGE_TIMES, {0} },
                                        { pd::TOWER_HIGHEST_LEVEL_GE, {0}},
                                        { pd::PASS_FEIGE, {1} },
                                        { pd::MANSION_FARM_PLANT_TIMES_GE, {0} },
                                        { pd::MANSION_KITCHEN_COOK_TIMES_GE, {0} },
                                        { pd::EQUIPMENT_MERGE_TIMES_GE, {0} },
                                        { pd::EQUIPMENT_XILIAN_TIMES_GE, {0} },
                                        { pd::MARRIAGE_BECOME_PROPOSE, {0} },
                                        { pd::MARRIAGE_ANSWER_QUESTION_TIMES_GE, {0} },
                                        { pd::MARRIAGE_GUESTURE_NEXT_ROUND_TIMES_GE, {0} },
                                        { pd::MARRIAGE_GUESTURE_NEXT_ROUND_TIMES_GE, {0} },
                                        { pd::EACH_SHOP_BUG_TIMES_GE, {1} },
                                        { pd::GIVE_ROLE_A_LIKE_TIMES_GE, {0}},
                                };

                                for (const auto& i : ptt.targets()) {
                                        if (ptt.forever_quest()) {
                                                if (i.params_size() != 1) {
                                                        CONFIG_ELOG << ptt.id() << " wrong target params size for forever quest";
                                                }
                                        }
                                        if (i.params_size() == 0) {
                                                CONFIG_ELOG << ptt.id() << " wrong target params size cannot empty";
                                        }
                                        if (targets_arg_size.count(i.type()) == 0) {
                                                CONFIG_ELOG << ptt.id() << " " << pd::quest_target_type_Name(i.type()) << " wrong type";
                                        }
                                        for (auto j = 0; j < i.params_size() - 1; j++) {
                                                if (i.params(j).arg_size() != i.params(j + 1).arg_size()) {
                                                        CONFIG_ELOG << ptt.id() << " " << pd::quest_target_type_Name(i.type()) << " arg size diff between params";
                                                        break;
                                                }
                                        }
                                        for (const auto& j : i.params()) {
                                                if (targets_arg_size.at(i.type()).count(j.arg_size()) <= 0) {
                                                        CONFIG_ELOG << ptt.id() << " " << pd::quest_target_type_Name(i.type()) << " wrong arg size " << j.arg_size();
                                                }
                                        }
                                }

                        };
                        quest_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.targets()) {
                                        for (const auto& j : i.params()) {
                                                switch (i.type()) {
                                                case pd::PASS_CHAPTER_ADVENTURE: {
                                                        if (j.arg_size() == 1) {
                                                                if (!PTTS_HAS(adventure_chapter, std::stoul(j.arg(0)))) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " wrong arg, adventure_chapter table not exist " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::PASS_ADVENTURE: {
                                                        if (j.arg_size() == 1) {
                                                                if (!PTTS_HAS(adventure, std::stoul(j.arg(0)))) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " wrong arg, adventure table not exist " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::FIEF_SET_BUILDING_MANAGER:
                                                case pd::FIEF_SET_BUILDING_ASSISTANTS:
                                                case pd::FIEF_BUILDING_MANAGER_CRAFTS_GE:
                                                case pd::FIEF_BUILDING_LEVEL_GE: {
                                                        if (j.arg_size() == 1) {
                                                                pd::fief_building_type building_type;
                                                                if (!pd::fief_building_type_Parse(j.arg(0), &building_type)) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " has invalid fief_building_type " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::PASS_EPISODE_ADVENTURE_COUNT_GE: {
                                                        if (j.arg_size() == 1) {
                                                                if (!PTTS_HAS(adventure_episode, std::stoul(j.arg(0)))) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " wrong arg, adventure_chapter not exist " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::MANSION_BUILDING_LEVEL_GE: {
                                                        if (j.arg_size() == 1) {
                                                                pd::mansion_building_type building_type;
                                                                if (!pd::mansion_building_type_Parse(j.arg(0), &building_type)) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " has invalid mansion building type " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::GONGDOU_TIMES: {
                                                        if (j.arg_size() == 1) {
                                                                pd::gongdou_type type;
                                                                if (!pd::gongdou_type_Parse(j.arg(0), &type)) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " has invalid gongdou type " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::HUANZHUANG_PART_ITEM_COUNT_GE: {
                                                        ASSERT(j.arg_size() == 3);
                                                        pd::huanzhuang_part part;
                                                        if (!pd::huanzhuang_part_Parse(j.arg(0), &part)) {
                                                                CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " has invalid huanzhuang part " << j.arg(0);
                                                        }
                                                        break;
                                                }
                                                case pd::TURN_IN_ITEM: {
                                                        if (j.arg_size() == 1) {
                                                                if (!PTTS_HAS(item, std::stoul(j.arg(0)))) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " item not exist " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                case pd::TURN_IN_RESOURCE: {
                                                        if (j.arg_size() == 1) {
                                                                pd::resource_type type;
                                                                if (!pd::resource_type_Parse(j.arg(0), &type)) {
                                                                        CONFIG_ELOG << pd::quest_target_type_Name(i.type()) << " resource not exist " << j.arg(0);
                                                                }
                                                        }
                                                        break;
                                                }
                                                default:
                                                break;
                                                }
                                        }
                                }
                        };
                }

                everyday_quest_ptts& everyday_quest_ptts_instance() {
                        static everyday_quest_ptts inst;
                        return inst;
                }

                void everyday_quest_ptts_set_funcs() {
                        everyday_quest_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(quest, ptt.quest())) {
                                        CONFIG_ELOG << ptt.id() << " quest not exist " << ptt.quest();
                                        return;
                                }
                                auto& quest_ptt = PTTS_GET(quest, ptt.quest());
                                quest_ptt.set__everyday_quest(ptt.id());
                        };
                }

                everyday_quest_reward_ptts& everyday_quest_reward_ptts_instance() {
                        static everyday_quest_reward_ptts inst;
                        return inst;
                }

                void everyday_quest_reward_ptts_set_funcs() {
                        everyday_quest_reward_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed";
                                }
                        };
                        everyday_quest_reward_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable_events());
                        };
                        everyday_quest_reward_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                        };
                }

                ui_quest_ptts& ui_quest_ptts_instance() {
                        static ui_quest_ptts inst;
                        return inst;
                }

                void ui_quest_ptts_set_funcs() {
                        ui_quest_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << " ui quest:" << pcs::ui_quest_module_type_Name(ptt.type()) << "quest not exist " << i;
                                        } else {
                                                const auto& quest_ptt = PTTS_GET(quest, i);
                                                if (quest_ptt.targets_size() != 1) {
                                                        CONFIG_ELOG << " ui quest:" << pcs::ui_quest_module_type_Name(ptt.type()) << "quest targets more than one, quest id:" << i;
                                                }
                                        }
                                }

                                for (auto i : ptt.accept_record_quests()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << " ui quest:" << pcs::ui_quest_module_type_Name(ptt.type()) << "quest not exist " << i;
                                        }
                                }
                        };
                }

        }
}
