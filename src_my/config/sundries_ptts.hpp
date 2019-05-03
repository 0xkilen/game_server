#pragma once

#include "utils/ptts.hpp"
#include "proto/config_sundries.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {
                using sundries_ptts = ptts<pc::sundries>;
                sundries_ptts& sundries_ptts_instance();
                void sundries_ptts_set_funcs();
        }
}
