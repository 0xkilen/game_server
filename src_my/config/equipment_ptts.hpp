#pragma once

#include "utils/ptts.hpp"
#include "proto/config_equipment.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using equipment_ptts = ptts<pc::equipment>;
                equipment_ptts& equipment_ptts_instance();
                void equipment_ptts_set_funcs();

                using equipment_levelup_ptts = ptts<pc::equipment_levelup>;
                equipment_levelup_ptts& equipment_levelup_ptts_instance();
                void equipment_levelup_ptts_set_funcs();

                using equipment_suite_ptts = ptts<pc::equipment_suite>;
                equipment_suite_ptts& equipment_suite_ptts_instance();
                void equipment_suite_ptts_set_funcs();

                using equipment_rand_attr_pool_ptts = ptts<pc::equipment_rand_attr_pool>;
                equipment_rand_attr_pool_ptts& equipment_rand_attr_pool_ptts_instance();
                void equipment_rand_attr_pool_ptts_set_funcs();

                using equipment_rand_attr_quality_ptts = ptts<pc::equipment_rand_attr_quality>;
                equipment_rand_attr_quality_ptts& equipment_rand_attr_quality_ptts_instance();
                void equipment_rand_attr_quality_ptts_set_funcs();

                using equipment_add_quality_ptts = ptts<pc::equipment_add_quality>;
                equipment_add_quality_ptts& equipment_add_quality_ptts_instance();
                void equipment_add_quality_ptts_set_funcs();

                using equipment_decompose_ptts = ptts<pc::equipment_decompose>;
                equipment_decompose_ptts& equipment_decompose_ptts_instance();
                void equipment_decompose_ptts_set_funcs();

                using equipment_compose_ptts = ptts<pc::equipment_compose>;
                equipment_compose_ptts& equipment_compose_ptts_instance();
                void equipment_compose_ptts_set_funcs();

                using equipment_logic_ptts = ptts<pc::equipment_logic>;
                equipment_logic_ptts& equipment_logic_ptts_instance();
                void equipment_logic_ptts_set_funcs();

        }
}
