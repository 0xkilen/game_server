#pragma once

#include "utils/ptts.hpp"
#include "proto/config_mansion.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using mansion_furniture_group_ptts = ptts<pc::mansion_furniture_group>;
                mansion_furniture_group_ptts& mansion_furniture_group_ptts_instance();
                void mansion_furniture_group_ptts_set_funcs();

                using mansion_furniture_type_ptts = ptts<pc::mansion_furniture_type>;
                mansion_furniture_type_ptts& mansion_furniture_type_ptts_instance();
                void mansion_furniture_type_ptts_set_funcs();

                using mansion_furniture_collision_group_ptts = ptts<pc::mansion_furniture_collision_group>;
                mansion_furniture_collision_group_ptts& mansion_furniture_collision_group_ptts_instance();
                void mansion_furniture_collision_group_ptts_set_funcs();

                using mansion_area_ptts = ptts<pc::mansion_area>;
                mansion_area_ptts& mansion_area_ptts_instance();
                void mansion_area_ptts_set_funcs();

                using mansion_furniture_ptts = ptts<pc::mansion_furniture>;
                mansion_furniture_ptts& mansion_furniture_ptts_instance();
                void mansion_furniture_ptts_set_funcs();

                using mansion_hall_ptts = ptts<pc::mansion_hall>;
                mansion_hall_ptts& mansion_hall_ptts_instance();
                void mansion_hall_ptts_set_funcs();

                using mansion_hall_quest_ptts = ptts<pc::mansion_hall_quest>;
                mansion_hall_quest_ptts& mansion_hall_quest_ptts_instance();
                void mansion_hall_quest_ptts_set_funcs();

                using mansion_quest_ptts = ptts<pc::mansion_quest>;
                mansion_quest_ptts& mansion_quest_ptts_instance();
                void mansion_quest_ptts_set_funcs();

                using mansion_incident_ptts = ptts<pc::mansion_incident>;
                mansion_incident_ptts& mansion_incident_ptts_instance();
                void mansion_incident_ptts_set_funcs();

                using mansion_craft_type_summary_ptts = ptts<pc::mansion_craft_type_summary>;
                mansion_craft_type_summary_ptts& mansion_craft_type_summary_ptts_instance();
                void mansion_craft_type_summary_ptts_set_funcs();

                using mansion_pond_ptts = ptts<pc::mansion_pond>;
                mansion_pond_ptts& mansion_pond_ptts_instance();
                void mansion_pond_ptts_set_funcs();

                using mansion_kitchen_ptts = ptts<pc::mansion_kitchen>;
                mansion_kitchen_ptts& mansion_kitchen_ptts_instance();
                void mansion_kitchen_ptts_set_funcs();

                using mansion_bedroom_ptts = ptts<pc::mansion_bedroom>;
                mansion_bedroom_ptts& mansion_bedroom_ptts_instance();
                void mansion_bedroom_ptts_set_funcs();

                using mansion_vegetable_plot_ptts = ptts<pc::mansion_vegetable_plot>;
                mansion_vegetable_plot_ptts& mansion_vegetable_plot_ptts_instance();
                void mansion_vegetable_plot_ptts_set_funcs();

                using mansion_bedroom_tool_ptts = ptts<pc::mansion_bedroom_tool>;
                mansion_bedroom_tool_ptts& mansion_bedroom_tool_ptts_instance();
                void mansion_bedroom_tool_ptts_set_funcs();

                using mansion_logic_ptts = ptts<pc::mansion_logic>;
                mansion_logic_ptts& mansion_logic_ptts_instance();
                void mansion_logic_ptts_set_funcs();

                using mansion_banquet_poison_ptts = ptts<pc::mansion_banquet_poison>;
                mansion_banquet_poison_ptts& mansion_banquet_poison_ptts_instance();
                void mansion_banquet_poison_ptts_set_funcs();

                using mansion_banquet_firework_ptts = ptts<pc::mansion_banquet_firework>;
                mansion_banquet_firework_ptts& mansion_banquet_firework_ptts_instance();
                void mansion_banquet_firework_ptts_set_funcs();

                using mansion_banquet_dish_ptts = ptts<pc::mansion_banquet_dish>;
                mansion_banquet_dish_ptts& mansion_banquet_dish_ptts_instance();
                void mansion_banquet_dish_ptts_set_funcs();

                using mansion_banquet_celebrity_ptts = ptts<pc::mansion_banquet_celebrity>;
                mansion_banquet_celebrity_ptts& mansion_banquet_celebrity_ptts_instance();
                void mansion_banquet_celebrity_ptts_set_funcs();

                using mansion_banquet_celebrity_group_ptts = ptts<pc::mansion_banquet_celebrity_group>;
                mansion_banquet_celebrity_group_ptts& mansion_banquet_celebrity_group_ptts_instance();
                void mansion_banquet_celebrity_group_ptts_set_funcs();

                using mansion_banquet_prestige_ptts = ptts<pc::mansion_banquet_prestige>;
                mansion_banquet_prestige_ptts& mansion_banquet_prestige_ptts_instance();
                void mansion_banquet_prestige_ptts_set_funcs();

                using mansion_banquet_riddle_ptts = ptts<pc::mansion_banquet_riddle>;
                mansion_banquet_riddle_ptts& mansion_banquet_riddle_ptts_instance();
                void mansion_banquet_riddle_ptts_set_funcs();

                using mansion_banquet_question_ptts = ptts<pc::mansion_banquet_question>;
                mansion_banquet_question_ptts& mansion_banquet_question_ptts_instance();
                void mansion_banquet_question_ptts_set_funcs();

                using mansion_banquet_question_group_ptts = ptts<pc::mansion_banquet_question_group>;
                mansion_banquet_question_group_ptts& mansion_banquet_question_group_ptts_instance();
                void mansion_banquet_question_group_ptts_set_funcs();

                using mansion_banquet_thief_ptts = ptts<pc::mansion_banquet_thief>;
                mansion_banquet_thief_ptts& mansion_banquet_thief_ptts_instance();
                void mansion_banquet_thief_ptts_set_funcs();

                using mansion_banquet_pintu_game_ptts = ptts<pc::mansion_banquet_pintu_game>;
                mansion_banquet_pintu_game_ptts& mansion_banquet_pintu_game_ptts_instance();
                void mansion_banquet_pintu_game_ptts_set_funcs();

                using mansion_banquet_weilie_game_ptts = ptts<pc::mansion_banquet_weilie_game>;
                mansion_banquet_weilie_game_ptts& mansion_banquet_weilie_game_ptts_instance();
                void mansion_banquet_weilie_game_ptts_set_funcs();

                using mansion_banquet_link_game_ptts = ptts<pc::mansion_banquet_link_game>;
                mansion_banquet_link_game_ptts& mansion_banquet_link_game_ptts_instance();
                void mansion_banquet_link_game_ptts_set_funcs();

                using mansion_banquet_rhythm_master_game_ptts = ptts<pc::mansion_banquet_rhythm_master_game>;
                mansion_banquet_rhythm_master_game_ptts& mansion_banquet_rhythm_master_game_ptts_instance();
                void mansion_banquet_rhythm_master_game_ptts_set_funcs();

                using mansion_banquet_coin_ptts = ptts<pc::mansion_banquet_coin>;
                mansion_banquet_coin_ptts& mansion_banquet_coin_ptts_instance();
                void mansion_banquet_coin_ptts_set_funcs();

                using mansion_banquet_coin_stage_ptts = ptts<pc::mansion_banquet_coin_stage>;
                mansion_banquet_coin_stage_ptts& mansion_banquet_coin_stage_ptts_instance();
                void mansion_banquet_coin_stage_ptts_set_funcs();

                using mansion_banquet_ptts = ptts<pc::mansion_banquet>;
                mansion_banquet_ptts& mansion_banquet_ptts_instance();
                void mansion_banquet_ptts_set_funcs();

                using mansion_farm_plant_ptts = ptts<pc::mansion_farm_plant>;
                mansion_farm_plant_ptts& mansion_farm_plant_ptts_instance();
                void mansion_farm_plant_ptts_set_funcs();

                using mansion_cook_recipe_ptts = ptts<pc::mansion_cook_recipe>;
                mansion_cook_recipe_ptts& mansion_cook_recipe_ptts_instance();
                void mansion_cook_recipe_ptts_set_funcs();

                using mansion_game_ptts = ptts<pc::mansion_game>;
                mansion_game_ptts& mansion_game_ptts_instance();
                void mansion_game_ptts_set_funcs();

                using mansion_exchange_ptts = ptts<pc::mansion_exchange>;
                mansion_exchange_ptts& mansion_exchange_ptts_instance();
                void mansion_exchange_ptts_set_funcs();

                using mansion_game_fish_capper_ptts = ptts<pc::mansion_game_fish_capper>;
                mansion_game_fish_capper_ptts& mansion_game_fish_capper_ptts_instance();
                void mansion_game_fish_capper_ptts_set_funcs();

                using mansion_banquet_wedding_ball_ptts = ptts<pc::mansion_banquet_wedding_ball>;
                mansion_banquet_wedding_ball_ptts& mansion_banquet_wedding_ball_ptts_instance();
                void mansion_banquet_wedding_ball_ptts_set_funcs();

                using mansion_banquet_wedding_candy_ptts = ptts<pc::mansion_banquet_wedding_candy>;
                mansion_banquet_wedding_candy_ptts& mansion_banquet_wedding_candy_ptts_instance();
                void mansion_banquet_wedding_candy_ptts_set_funcs();

        }
}
