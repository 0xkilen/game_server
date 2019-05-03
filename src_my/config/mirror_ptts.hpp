#pragma once

#include "utils/ptts.hpp"
#include "proto/config_mirror.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using mirror_ptts = ptts<pc::mirror>;
                mirror_ptts& mirror_ptts_instance();
                void mirror_ptts_set_funcs();

        }
}
