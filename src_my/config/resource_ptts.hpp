#pragma once

#include "utils/ptts.hpp"
#include "proto/config_resource.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using resource_ptts = ptts<pc::resource>;
                resource_ptts& resource_ptts_instance();
                void resource_ptts_set_funcs();

        }
}
