#pragma once

#include "utils/ptts.hpp"
#include "proto/config_jade.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using jade_ptts = ptts<pc::jade>;
                jade_ptts& jade_ptts_instance();
                void jade_ptts_set_funcs();

        }
}
