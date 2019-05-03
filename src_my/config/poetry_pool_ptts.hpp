#pragma once

#include "utils/ptts.hpp"
#include "proto/config_poetry.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using poetry_pool_ptts = ptts<pc::poetry_pool>;
                poetry_pool_ptts& poetry_pool_ptts_instance();
                void poetry_pool_ptts_set_funcs();

        }
}
