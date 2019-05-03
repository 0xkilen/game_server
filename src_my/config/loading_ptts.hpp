#pragma once

#include "utils/ptts.hpp"
#include "proto/config_loading.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using loading_ptts = ptts<pc::loading>;
                loading_ptts& loading_ptts_instance();
                void loading_ptts_set_funcs();
        }
}