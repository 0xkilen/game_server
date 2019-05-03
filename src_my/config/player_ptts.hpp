#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_player.pb.h"
#include "proto/config_robot.pb.h"
#include "proto/config_gm_data.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using create_role_ptts = ptts<pc::create_role>;
                create_role_ptts& create_role_ptts_instance();
                void create_role_ptts_set_funcs();

                using role_levelup_ptts = ptts<pc::role_levelup>;
                role_levelup_ptts& role_levelup_ptts_instance();
                void role_levelup_ptts_set_funcs();

                using role_add_fate_ptts = ptts<pc::role_add_fate>;
                role_add_fate_ptts& role_add_fate_ptts_instance();
                void role_add_fate_ptts_set_funcs();

                using role_logic_ptts = ptts<pc::role_logic>;
                role_logic_ptts& role_logic_ptts_instance();
                void role_logic_ptts_set_funcs();

                using role_present_ptts = ptts<pc::role_present>;
                role_present_ptts& role_present_ptts_instance();
                void role_present_ptts_set_funcs();

                using robot_ptts = ptts<pc::robot>;
                robot_ptts& robot_ptts_instance();
                void robot_ptts_set_funcs();

                using gm_data_ptts = ptts<pc::gm_data>;
                gm_data_ptts& gm_data_ptts_instance();
                void gm_data_ptts_set_funcs();

                using gm_sequence_ptts = ptts<pc::gm_sequence>;
                gm_sequence_ptts& gm_sequence_ptts_instance();
                void gm_sequence_ptts_set_funcs();

        }
}
