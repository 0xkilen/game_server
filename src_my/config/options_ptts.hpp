#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using options_ptts = ptts<pc::options>;
                options_ptts& options_ptts_instance();
                void options_ptts_set_funcs();
        }
}
