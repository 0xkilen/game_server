#pragma once

#include "utils/ptts.hpp"
#include "proto/config_title.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using title_ptts = ptts<pc::title>;
                title_ptts& title_ptts_instance();
                void title_ptts_set_funcs();

        }
}
