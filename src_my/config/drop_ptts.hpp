#pragma once

#include "utils/ptts.hpp"
#include "proto/config_drop.pb.h"

using namespace std;

namespace nora {
        namespace config {

                using drop_ptts = ptts<proto::config::drop>;
                drop_ptts& drop_ptts_instance();
                void drop_ptts_set_funcs();

        }
}
