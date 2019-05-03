#pragma once

#include "utils/ptts.hpp"
#include "proto/config_lieming.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using lieming_ptts = ptts<pc::lieming>;
                lieming_ptts& lieming_ptts_instance();
                void lieming_ptts_set_funcs();

                using lieming_logic_ptts = ptts<pc::lieming_logic>;
                lieming_logic_ptts& lieming_logic_ptts_instance();
                void lieming_logic_ptts_set_funcs();

        }
}
