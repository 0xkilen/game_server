#pragma once

#include "utils/ptts.hpp"
#include "proto/config_league.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using league_ptts = ptts<pc::league>;
                league_ptts& league_ptts_instance();
                void league_ptts_set_funcs();

                
                using league_record_list_limit_ptts = ptts<pc::league_record_list_limit>;
                league_record_list_limit_ptts& league_record_list_limit_ptts_instance();
                void league_record_list_limit_ptts_set_funcs();

                using league_levelup_ptts = ptts<pc::league_levelup>;
                league_levelup_ptts& league_levelup_ptts_instance();
                void league_levelup_ptts_set_funcs();

                using league_resource_ptts = ptts<pc::league_resource>;
                league_resource_ptts& league_resource_ptts_instance();
                void league_resource_ptts_set_funcs();

                using league_campaign_ptts = ptts<pc::league_campaign>;
                league_campaign_ptts& league_campaign_ptts_instance();
                void league_campaign_ptts_set_funcs();

                using league_drop_ptts = ptts<pc::league_drop>;
                league_drop_ptts& league_drop_ptts_instance();
                void league_drop_ptts_set_funcs();

                using league_apply_item_ptts = ptts<pc::league_apply_item>;
                league_apply_item_ptts& league_apply_item_ptts_instance();
                void league_apply_item_ptts_set_funcs();

                using league_creating_ptts = ptts<pc::league_creating>;
                league_creating_ptts& league_creating_ptts_instance();
                void league_creating_ptts_set_funcs();

                using league_building_ptts = ptts<pc::league_building>;
                league_building_ptts& league_building_ptts_instance();
                void league_building_ptts_set_funcs();

                using league_urban_management_level_ptts = ptts<pc::league_urban_management_level>;
                league_urban_management_level_ptts& league_urban_management_level_ptts_instance();
                void league_urban_management_level_ptts_set_funcs();

                using league_daily_quest_level_ptts = ptts<pc::league_daily_quest_level>;
                league_daily_quest_level_ptts& league_daily_quest_level_ptts_instance();
                void league_daily_quest_level_ptts_set_funcs();

                using league_government_level_ptts = ptts<pc::league_government_level>;
                league_government_level_ptts& league_government_level_ptts_instance();
                void league_government_level_ptts_set_funcs();

                using city_ptts = ptts<pc::city>;
                city_ptts& city_ptts_instance();
                void city_ptts_set_funcs();

                using system_league_ptts = ptts<pc::system_league>;
                system_league_ptts& system_league_ptts_instance();
                void system_league_ptts_set_funcs();

                using city_battle_logic_ptts = ptts<pc::city_battle_logic>;
                city_battle_logic_ptts& city_battle_logic_ptts_instance();
                void city_battle_logic_ptts_set_funcs();

                using city_battle_treasure_box_defender_ptts = ptts<pc::city_battle_treasure_box>;
                city_battle_treasure_box_defender_ptts& city_battle_treasure_box_defender_ptts_instance();
                void city_battle_treasure_box_defender_ptts_set_funcs();

                using city_battle_treasure_box_offensive_ptts = ptts<pc::city_battle_treasure_box>;
                city_battle_treasure_box_offensive_ptts& city_battle_treasure_box_offensive_ptts_instance();
                void city_battle_treasure_box_offensive_ptts_set_funcs();

                using city_battle_ptts = ptts<pc::city_battle>;
                city_battle_ptts& city_battle_ptts_instance();
                void city_battle_ptts_set_funcs();

                using league_war_logic_ptts = ptts<pc::league_war_logic>;
                league_war_logic_ptts& league_war_logic_ptts_instance();
                void league_war_logic_ptts_set_funcs();

                using league_permission_ptts = ptts<pc::league_permission>;
                league_permission_ptts& league_permission_ptts_instance();
                void league_permission_ptts_set_funcs();

                using league_permission_function_ptts = ptts<pc::league_permission_function>;
                league_permission_function_ptts& league_permission_function_ptts_instance();
                void league_permission_function_ptts_set_funcs();
        }
}
