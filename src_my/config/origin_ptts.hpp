#pragma once

#include "utils/ptts.hpp"
#include "proto/config_origin.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using origin_ptts = ptts<pc::origin>;
                origin_ptts& origin_ptts_instance();
                void origin_ptts_set_funcs();

        }
}
