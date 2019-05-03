#pragma once

#include "utils/ptts.hpp"
#include "proto/config_common.pb.h"

using namespace std;

namespace nora {
        namespace config {

                using system_preview_ptts = ptts<proto::config::system_preview>;
                system_preview_ptts& system_preview_ptts_instance();
                void system_preview_ptts_set_funcs();

                using name_pool_ptts = ptts<pc::name_pool>;
                name_pool_ptts& name_pool_ptts_instance();
                void name_pool_ptts_set_funcs();

                using common_ptts = ptts<pc::common>;
                common_ptts& common_ptts_instance();
                void common_ptts_set_funcs();

        }
}
