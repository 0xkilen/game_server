#pragma once

#include "utils/ptts.hpp"
#include "proto/config_fief.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using fief_ptts = ptts<pc::fief>;
                fief_ptts& fief_ptts_instance();
                void fief_ptts_set_funcs();

                using fief_thief_ptts = ptts<pc::fief_thief>;
                fief_thief_ptts& fief_thief_ptts_instance();
                void fief_thief_ptts_set_funcs();

                using fief_boss_ptts = ptts<pc::fief_boss>;
                fief_boss_ptts& fief_boss_ptts_instance();
                void fief_boss_ptts_set_funcs();

                using fief_product_ptts = ptts<pc::fief_product>;
                fief_product_ptts& fief_product_ptts_instance();
                void fief_product_ptts_set_funcs();

                using fief_hospital_ptts = ptts<pc::fief_building>;
                fief_hospital_ptts& fief_hospital_ptts_instance();
                void fief_hospital_ptts_set_funcs();

                using fief_bar_ptts = ptts<pc::fief_building>;
                fief_bar_ptts& fief_bar_ptts_instance();
                void fief_bar_ptts_set_funcs();

                using fief_cofc_ptts = ptts<pc::fief_building>;
                fief_cofc_ptts& fief_cofc_ptts_instance();
                void fief_cofc_ptts_set_funcs();

                using fief_guard_ptts = ptts<pc::fief_building>;
                fief_guard_ptts& fief_guard_ptts_instance();
                void fief_guard_ptts_set_funcs();

                using fief_cloth_ptts = ptts<pc::fief_building>;
                fief_cloth_ptts& fief_cloth_ptts_instance();
                void fief_cloth_ptts_set_funcs();

                using fief_mine_ptts = ptts<pc::fief_building>;
                fief_mine_ptts& fief_mine_ptts_instance();
                void fief_mine_ptts_set_funcs();

                using fief_incident_ptts = ptts<pc::fief_incident>;
                fief_incident_ptts& fief_incident_ptts_instance();
                void fief_incident_ptts_set_funcs();

                using fief_logic_ptts = ptts<pc::fief_logic>;
                fief_logic_ptts& fief_logic_ptts_instance();
                void fief_logic_ptts_set_funcs();

                using fief_area_incident_pool_ptts = ptts<pc::fief_area_incident_pool>;
                fief_area_incident_pool_ptts& fief_area_incident_pool_ptts_instance();
                void fief_area_incident_pool_ptts_set_funcs();

                using fief_area_incident_ptts = ptts<pc::fief_area_incident>;
                fief_area_incident_ptts& fief_area_incident_ptts_instance();
                void fief_area_incident_ptts_set_funcs();

                using fief_area_incident_reward_ptts = ptts<pc::fief_area_incident_reward>;
                fief_area_incident_reward_ptts& fief_area_incident_reward_ptts_instance();
                void fief_area_incident_reward_ptts_set_funcs();

                using fief_bubble_ptts = ptts<pc::fief_bubble>;
                fief_bubble_ptts& fief_bubble_ptts_instance();
                void fief_bubble_ptts_set_funcs();

        }
}
