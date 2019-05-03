#pragma once

#include "utils/ptts.hpp"
#include "proto/config_gongdou.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using gongdou_ptts = ptts<pc::gongdou>;
                gongdou_ptts& gongdou_ptts_instance();
                void gongdou_ptts_set_funcs();

        }
}
