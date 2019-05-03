#pragma once

#include <boost/any.hpp>

namespace nora {

        using event_handler = function<void(const vector<boost::any>&)>;
        enum ec_event {
                ECE_ADD_ACTOR,
                ECE_ACTOR_ADD_PIN,
                ECE_ACTOR_ADD_STEP,
                ECE_ACTOR_STAR_CHANGED,
                ECE_ACTOR_ACTOR_STAR_CHANGED,
                ECE_ADD_ITEM,
                ECE_REMOVE_ITEM,
                ECE_LEVELUP_EQUIPMENT,
                ECE_EQUIPMENT_ADD_QUALITY,
                ECE_SET_FORMATION,
                ECE_WORLD_CHAT,
                ECE_PRIVATE_CHAT,
                ECE_LEVELUP_ACTOR,
                ECE_DONATE_LEAGUE,
                ECE_PASS_ADVENTURE,
                ECE_LEVELUP,
                ECE_CHALLENGE_ARENA,
                ECE_HUANZHUANG_ADVENTURE_HIGHEST_LEVEL,
                ECE_HUANZHUANG_COMPOSE,
                ECE_HUANZHUANG_LEVELUP,
                ECE_HUANZHUANG_INTENSIFY,
                ECE_UNLOCK_SKILL,
                ECE_LEVELUP_ACTOR_SKILL,
                ECE_COST_ITEM,
                ECE_ARENA_CHANGE,
                ECE_ZHANLI_CHANGE,
                ECE_CHALLENGE_FIEF_BOSS_DAMAGE,
                ECE_CHALLENGE_LEAGUE_BOSS_DAMAGE,
                ECE_SINGLE_BATTLE_HEAL,
                ECE_CHANGE_RESOURCE,
                ECE_PASS_LIEMING,
                ECE_DRAW_TEN_TIMES_LOTTERY,
                ECE_ADD_FRIEND,
                ECE_REMOVE_FRIEND,
                ECE_MANSION_FISHING_GET,
                ECE_MANSION_VISIT,
                ECE_MANSION_POISON_BANQUET,
                ECE_MANSION_HELP_QUEST,
                ECE_MANSION_UPVOTED_BY_OTHER,
                ECE_MANSION_UPDATE_FANCY,
                ECE_MANSION_UPDATE_FURNITURE_COUNT,
                ECE_MANSION_FURNITURE,
                ECE_MANSION_COMMIT_HALL_QUEST,
                ECE_MANSION_COMMIT_QUEST,
                ECE_MANSION_ENTER_BANQUET,
                ECE_MANSION_ADD_FURNITURE,
                ECE_MANSION_BANQUET_ATMOSPHERE_UPDATE,
                ECE_MANSION_BANQUET_ACTIVITY_UPDATE,
                ECE_MANSION_BANQUET_USE_FIREWORK,
                ECE_MANSION_BANQUET_ANSWER_QUESTION,
                ECE_MANSION_BANQUET_EAT_DISH,
                ECE_ACTOR_PUT_ON_EQUIPMENT,
                ECE_FIEF_DEFEATED_BOSS,
                ECE_FIEF_DEFEATED_THIEF,
                ECE_FIEF_TRIGGER_EVENT,
                ECE_FIEF_CHALLENGE_BOSS,
                ECE_LEAGUE_SEND_LUCKY_BAG,
                ECE_LEAGUE_GET_LUCKY_BAG_DIAMAND,
                ECE_LEAGUE_GET_LUCKY_BAG_KING,
                ECE_LEAGUE_LEVEL_REACH,
                ECE_LEAGUE_BOSS_KILL,
                ECE_LEAGUE_QUEST_FINISH_TIMES,
                ECE_LEAGUE_HELP_OTHER_TIMES,
                ECE_LEAGUE_SEND_LUCKY_BAG_TIMES,
                ECE_LEAGUE_CAMPAIGN_TIMES,
                ECE_LEAGUE_TROUBLE_TIMES,
                ECE_LEAGUE_TRADE_TIMES,
                ECE_ADD_FATE,
                ECE_EQUIPMENT_CHANGE,
                ECE_HUANZHUANG_ADDED_COLLECTION,
                ECE_HUANZHUANG_PVP_GUESS_VOTE,
                ECE_CHALLENGE_LIEMING,
                ECE_RECHARGE,
                ECE_ROLE_GONGDOU,
                ECE_ROLE_SEND_GIFT,
                ECE_SPECIAL_CHALLENGE_LIEMING,
                ECE_HUANZHUANG_PVP_UPDATE_SELECTION,
                ECE_HUANZHUANG_PVP_VOTE,
                ECE_HUANZHUANG_PVP_NEW_HIGH_RANK,
                ECE_MANSION_ACCEPT_QUEST,
                ECE_MANSION_ACCEPT_OTHER_QUEST,
                ECE_MANSION_BUILDING_LEVEL,
                ECE_MANSION_GAME_FISHING,
                ECE_MANSION_TEAM_GAME_FISHING,
                ECE_RELATION_FRIEND_INTIMACY,
                ECE_SHOP_BUY,
                ECE_ACTOR_ADD_STAR,
                ECE_FIEF_JOIN_AREA_INCIDENT,
                ECE_FIEF_OPEN_AREA_INCIDENT,
                ECE_FIEF_SET_BUILDING_MANAGER,
                ECE_FIEF_SET_BUILDING_ASSISTANTS,
                ECE_FIEF_UPDATE_BUILDING_MANAGER_CRAFTS,
                ECE_FIEF_BUILDING_LEVELUP,
                ECE_FIEF_LEVELUP,
                ECE_EQUIPMENT_COMPOSE,
                ECE_MYS_SHOP_REFRESH,
                ECE_SPINE_ADD_COLLECTION,
                ECE_DRAW_LOTTERY,
                ECE_GUANPIN_CHANGE,
                ECE_GUANPIN_CHALLENGE,
                ECE_GUANPIN_COLLECT_FENGLU,
                ECE_TOWER_NEW_HIGHEST_LEVEL,
                ECE_TOWER_CHALLENGE,
                ECE_MANSION_FARM_PLANT,
                ECE_MANSION_KITCHEN_COOK,
                ECE_PASS_FEIGE,
                ECE_CHAOTANG_RANK_CHANGE,
                ECE_MARRIAGE_PROPOSE,
                ECE_MARRIAGE_ANSWER_QUESTION,
                ECE_MARRIAGE_GUESTURE_NEXT_ROUND,
                ECE_EQUIPMENT_MERGE,
                ECE_EQUIPMENT_XILIAN,
                ECE_GIVE_ROLE_A_LIKE,

                ECE_ROLE_RESOURCE_CHANGE,
        };
        class event_center {
        public:
                list<event_handler>::iterator register_event(ec_event event, const event_handler& func);
                void on_event(ec_event event, const vector<boost::any>& args);
                void unregister_event(ec_event event, list<event_handler>::iterator iter);
                void unregister_event_by_type(ec_event event);
        private:
                map<ec_event, list<event_handler>> event_handlers_;
        };

        inline list<event_handler>::iterator event_center::register_event(ec_event event, const event_handler& handler) {
                event_handlers_[event].push_back(handler);
                return --event_handlers_[event].end();
        }

        inline void event_center::on_event(ec_event event, const vector<boost::any>& args) {
                if (event_handlers_.count(event) == 0) {
                        return;
                }
                auto funcs = event_handlers_.at(event);
                for (const auto& i : funcs) {
                        i(args);
                }
        }

        inline void event_center::unregister_event(ec_event event, list<event_handler>::iterator iter) {
                assert(event_handlers_.count(event) > 0);
                event_handlers_.at(event).erase(iter);
        }

        inline void event_center::unregister_event_by_type(ec_event event) {
                if (event_handlers_.count(event) <= 0) {
                        return;
                }
                event_handlers_.erase(event);
        }

}
