#pragma once

#include "utils/ptts.hpp"
#include "proto/config_tower.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using tower_ptts = ptts<pc::tower>;
                tower_ptts& tower_ptts_instance();
                void tower_ptts_set_funcs();

                using tower_logic_ptts = ptts<pc::tower_logic>;
                tower_logic_ptts& tower_logic_ptts_instance();
                void tower_logic_ptts_set_funcs();

        }
}
