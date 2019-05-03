#pragma once

#include "utils/ptts.hpp"
#include "proto/config_channel_specify.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using channel_specify_ptts = ptts<pc::channel_specify>;
                channel_specify_ptts& channel_specify_ptts_instance();
                void channel_specify_ptts_set_funcs();

        }
}
