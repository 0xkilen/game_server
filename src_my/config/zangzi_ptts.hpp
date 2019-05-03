#pragma once

#include "utils/ptts.hpp"
#include "proto/config_zangzi.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using zangzi_ptts = ptts<pc::zangzi>;
                zangzi_ptts& zangzi_ptts_instance();
                void zangzi_ptts_set_funcs();

        }
}
