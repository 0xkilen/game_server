#pragma once

#include "utils/ptts.hpp"
#include "proto/config_battle.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using battle_ptts = ptts<pc::battle>;
                battle_ptts& battle_ptts_instance();
                void battle_ptts_set_funcs();

                using formation_pool_ptts = ptts<pc::formation_pool>;
                formation_pool_ptts& formation_pool_ptts_instance();
                void formation_pool_ptts_set_funcs();

                using battle_ai_pool_ptts = ptts<pc::battle_ai_pool>;
                battle_ai_pool_ptts& battle_ai_pool_ptts_instance();
                void battle_ai_pool_ptts_set_funcs();
        }
}
