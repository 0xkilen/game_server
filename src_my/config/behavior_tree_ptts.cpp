#include "utils.hpp"
#include "behavior_tree_ptts.hpp"
#include "mansion_ptts.hpp"

namespace nora {
        namespace config {

                namespace {

                        void check_player_action(const pd::player_action& pa) {
                                static map<pd::player_action_type, pair<int, int>> arg_size {
                                        { pd::PAT_MIRROR_ACTION, {0, 0} },
                                        { pd::PAT_ARENA_CHALLENGE, {0, 0} },
                                        { pd::PAT_ARENA_JOIN, {0, 0} },
                                        { pd::PAT_MANSION_CREATE, {0, 0} },
                                        { pd::PAT_MANSION_ACCEPT_QUEST, {1, 18} },
                                        { pd::PAT_MANSION_ENTER, {1, 1} },
                                        { pd::PAT_MANSION_ENTER_FRIEND, {1, 1} },
                                        { pd::PAT_MANSION_LEAVE, {0, 0} },
                                        { pd::PAT_MANSION_UPVOTE, {0, 0} },
                                        { pd::PAT_MANSION_HELP_QUEST, {1, 18} },
                                        { pd::PAT_MANSION_ACCEPT_GAME_INVITATION, {0, 0} },
                                        { pd::PAT_MANSION_PLAY_GAME, {0, 0} },
                                        { pd::PAT_MANSION_INVITE_GAME, {1, 1} },
                                        { pd::PAT_MANSION_GAME_WAIT_ACCEPT, {0, 0} },
                                        { pd::PAT_ADVENTURE_CHALLENGE_NEW, {0, 0} },
                                        { pd::PAT_ADVENTURE_CHALLENGE, {0, 0} },
                                        { pd::PAT_ADVENTURE_SWEEP, {0, 0} },
                                        { pd::PAT_HUANZHUANG_PVP_UPDATE_SELECTION, {0, 0} },
                                        { pd::PAT_HUANZHUANG_PVP_VOTE, {0, 0} },
                                        { pd::PAT_HUANZHUANG_PVP_UPVOTE, {0, 0} },
                                        { pd::PAT_ROLE_UPVOTE, {0, 0} },
                                        { pd::PAT_GONGDOU, {1, 1} },
                                        { pd::PAT_RELATION_SEND_GIFT_TO_FRIEND, {0, 0} },
                                        { pd::PAT_RELATION_SEND_GIFT_TO_SPOUSE, {0, 0} },
                                        { pd::PAT_RELATION_APPLY_FRIEND, {0, 0} },
                                        { pd::PAT_RELATION_APPROVE_APPLICATION, {0, 0} },
                                        { pd::PAT_FIEF_CREATE, {0, 0} },
                                        { pd::PAT_FIEF_ENTER_SELF, {0, 0} },
                                        { pd::PAT_FIEF_ENTER_OTHER, {0, 0} },
                                        { pd::PAT_FIEF_AREA_INICIDENT_OPEN, {0, 0} },
                                        { pd::PAT_FIEF_AREA_INCIDENT_INVITE, {0, 0} },
                                        { pd::PAT_FIEF_AREA_INCIDENT_JOIN, {0, 0} },
                                        { pd::PAT_FIEF_LEAVE, {0, 0} },
                                        { pd::PAT_LEAGUE_FIND_TO_JOIN, {0, 0} },
                                        { pd::PAT_LEAGUE_DONATION, {1, 1} },
                                        { pd::PAT_LEAGUE_QUEST_APPEAL, {0, 0} },
                                        { pd::PAT_LEAGUE_QUEST_HELP, {0, 0} },
                                        { pd::PAT_LEAGUE_LUCKY_BAG_ISSUE, {0, 0} },
                                        { pd::PAT_LEAGUE_LUCKY_BAG_OPEN, {0, 0} },
                                        { pd::PAT_LEAGUE_CREATE, {0, 0} },
                                        { pd::PAT_LEAGUE_APPROVE_JOINER, {0, 0} },
                                        { pd::PAT_LIEMING_ROOM_ENTER, {0, 0} },
                                        { pd::PAT_MULTI_LIEMING_READY, {0, 0} },
                                        { pd::PAT_MULTI_LIEMING_INPUT, {0, 0} },
                                        { pd::PAT_LIEMING_ROOM_ENTER_BY_INVITE, {0, 0} },
                                        { pd::PAT_CHAT_WORLD, {1, 1} },
                                        { pd::PAT_PLAYER_ON_HOOK, {0, 0} },
                                        { pd::PAT_GUANPIN_PROMOTE, {0, 0} },
                                        { pd::PAT_GUANPIN_DIANSHI_BAOMING, {0, 0} },
                                        { pd::PAT_GUANPIN_CHAOTANG_CHANLLENGE, {0, 0} },
                                        { pd::PAT_ACTIVITY_QIANDAO, {0, 0} },
                                        { pd::PAT_ACTIVITY_RECHARGE, {0, 0} },
                                        { pd::PAT_ACTIVITY_YUEKA_REWARD, {0, 0} },
                                        { pd::PAT_MANSION_FARM_PLANT_SOW, {0, 0} },
                                        { pd::PAT_MANSION_FARM_PLANT_WATER, {0, 0} },
                                        { pd::PAT_MANSION_FARM_PLANT_FERTILIZE, {0, 0} },
                                        { pd::PAT_MANSION_FARM_PLANT_DISINSECT, {0, 0} },
                                        { pd::PAT_MANSION_FARM_PLANT_HARVEST, {0, 0} },
                                        { pd::PAT_MARRIAGE_PROPOSE, {0, 0} },
                                        { pd::PAT_MARRIAGE_ACCEPT_PROPOSE, {0, 0} },
                                        { pd::PAT_MARRIAGE_DECLINE_PROPOSE, {0, 0} },
                                        { pd::PAT_MARRIAGE_START_STAR_WISH, {0, 0} },
                                        { pd::PAT_MARRIAGE_STAR_WISH_SEND_GIFT, {0, 0} },
                                        { pd::PAT_MARRIAGE_STAR_WISH_CANCEL, {0, 0} },
                                        { pd::PAT_MARRIAGE_STOP_STAR_WISH, {0, 0} },
                                        { pd::PAT_MARRIAGE_STAR_WISH_SELECT, {0, 0} },
                                        { pd::PAT_MARRIAGE_START_PK, {0, 0} },
                                        { pd::PAT_MARRIAGE_PK_CHALLENGE, {0, 0} },
                                        { pd::PAT_MARRIAGE_PK_CANCEL, {0, 0} },
                                        { pd::PAT_MARRIAGE_STOP_PK, {0, 0} },
                                        { pd::PAT_MARRIAGE_PK_SELECT, {0, 0} },
                                        { pd::PAT_MARRIAGE_START_QIU_QIAN, {0, 0} },
                                        { pd::PAT_MARRIAGE_JOIN_QIU_QIAN, {0, 0} },
                                        { pd::PAT_MARRIAGE_DECLINE_QIU_QIAN, {0, 0} },
                                        { pd::PAT_MARRIAGE_QIU_QIAN_CONFIRM, {0, 0} },
                                        { pd::PAT_MARRIAGE_DIVORCE, {0, 0} },
                                        { pd::PAT_MANSION_KITCHEN_COOK, {0, 0} },
                                        { pd::PAT_LEAGUE_WAR_ENROLL, {0, 0} },
                                        { pd::PAT_LEAGUE_UPGRADE_BUILDING, {0, 0} },
                                        { pd::PAT_LEAGUE_ACCELERATE_BUILDING_UPGRADING, {0, 0} },
                                        { pd::PAT_LEAGUE_TRADE_TO_CITY, {0, 0} },
                                        { pd::PAT_LEAGUE_ALIENATE_CITY, {0, 0} },
                                        { pd::PAT_LEAGUE_INVITE_CITY, {0, 0} },
                                        { pd::PAT_CHILD_RAISE, {0, 0} },
                                        { pd::PAT_CHILD_STUDY, {0, 0} },
                                        { pd::PAT_CHILD_LEARN_SKILL, {0, 0} },
                                };
                                ASSERT(arg_size.count(pa.type()) > 0);
                                if (arg_size.at(pa.type()).first > pa.arg_size() || arg_size.at(pa.type()).second < pa.arg_size()) {
                                        CONFIG_ELOG << pd::player_action_type_Name(pa.type()) << " wrong arg size " << pa.arg_size();
                                }
                        }

                        void check_player_condition(const pd::player_condition& pc) {
                                static map<pd::player_condition_type, pair<int, int>> arg_size {
                                        { pd::PCT_ARENA_JOINED, {0, 0} },
                                        { pd::PCT_MANSION_HAS_GAME_INVITATION, {0, 0} },
                                        { pd::PCT_FIEF_CREATED, {0, 0} },
                                        { pd::PCT_MANSION_CREATED, {0, 0} },
                                        { pd::PCT_MARRIAGE_STARTED_QIU_QIAN, {0, 0} },
                                        { pd::PCT_MARRIAGE_SPOUSE_STARTED_QIU_QIAN, {0, 0} },
                                        { pd::PCT_MARRIAGE_QIU_QIANING, {0, 0} },
                                        { pd::PCT_CHILD_HAS, {0, 0} },
                                        { pd::PCT_CHILD_DIVORCING, {0, 0} },
                                };
                                ASSERT(arg_size.count(pc.type()) > 0);
                                if (arg_size.at(pc.type()).first > pc.arg_size() || arg_size.at(pc.type()).second < pc.arg_size()) {
                                        CONFIG_ELOG << pd::player_condition_type_Name(pc.type()) << " wrong arg size " << pc.arg_size();
                                }
                        }

                        void check_behavior_tree_node(const pc::behavior_tree_node& node) {
                                switch (node.type()) {
                                case pc::BNT_ACTION:
                                        if (node.has_player_action()) {
                                                //check_player_action(node.player_action());
                                        }
                                        break;
                                case pc::BNT_CONDITION:
                                        if (node.has_player_condition()) {
                                                check_player_condition(node.player_condition());
                                        }
                                        if (node.has_conditions() && !check_conditions(node.conditions())) {
                                                CONFIG_ELOG << node.id() << " check condition node conditions failed";
                                        }
                                        break;
                                case pc::BNT_FALLBACK:
                                case pc::BNT_RANDOM_FALLBACK:
                                case pc::BNT_SEQUENCE:
                                        if (node.children_size() == 0) {
                                                CONFIG_ELOG << node.id() << " not leaf node no children";
                                        }
                                        break;
                                case pc::BNT_PARALLEL:
                                        if (node.children_size() < node.parallel().threshold()) {
                                                CONFIG_ELOG << node.id() << " parallel children less than threshold";
                                        }
                                        break;
                                }

                                if (node.children_size() == 0) {
                                        if (node.type() != pc::BNT_ACTION && node.type() != pc::BNT_CONDITION) {
                                                CONFIG_ELOG << node.id() << " leaf node is not action or condition";
                                        }
                                }
                        }

                        void verify_player_action(const pd::player_action& pa) {
                                switch (pa.type()) {
                                        case pd::PAT_MANSION_HELP_QUEST:
                                        case pd::PAT_MANSION_ACCEPT_QUEST: {
                                                for (auto i : pa.arg()) {
                                                        if (!PTTS_HAS(mansion_quest, std::stoul(i))) {
                                                                CONFIG_ELOG << pd::player_action_type_Name(pa.type()) << " wrong arg " << i << ", quest not exist";
                                                        }
                                                }
                                                break;
                                        }
                                        default:{
                                                break;
                                        }
                                }
                        }

                        void verify_behavior_tree_node(const pc::behavior_tree_node& node) {
                                switch (node.type()) {
                                case pc::BNT_ACTION:
                                        if (node.has_player_action()) {
                                                verify_player_action(node.player_action());
                                        }
                                        break;
                                case pc::BNT_CONDITION:
                                        if (node.has_conditions() && !verify_conditions(node.conditions())) {
                                                CONFIG_ELOG << node.id() << " verify conditions failed";
                                        }
                                        break;
                                case pc::BNT_FALLBACK:
                                case pc::BNT_RANDOM_FALLBACK:
                                case pc::BNT_SEQUENCE:
                                case pc::BNT_PARALLEL:
                                        break;
                                }
                        }

                }

                behavior_tree_ptts& behavior_tree_ptts_instance() {
                        static behavior_tree_ptts inst;
                        return inst;
                }

                void behavior_tree_ptts_set_funcs() {
                        behavior_tree_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_behavior_tree_node(ptt);
                        };
                        behavior_tree_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_behavior_tree_node(ptt);
                        };
                }

                behavior_tree_root_ptts& behavior_tree_root_ptts_instance() {
                        static behavior_tree_root_ptts inst;
                        return inst;
                }

                void behavior_tree_root_ptts_set_funcs() {
                        behavior_tree_root_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(behavior_tree, ptt.node())) {
                                        CONFIG_ELOG << pc::behavior_tree_root_type_Name(ptt.type()) << " node not exist " << ptt.node();
                                }
                        };
                }

        }
}
