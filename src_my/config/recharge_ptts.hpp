#pragma once

#include "utils/ptts.hpp"
#include "proto/config_recharge.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using recharge_ptts = ptts<pc::recharge>;
                recharge_ptts& recharge_ptts_instance();
                void recharge_ptts_set_funcs();

                using vip_ptts = ptts<pc::vip>;
                vip_ptts& vip_ptts_instance();
                void vip_ptts_set_funcs();

        }
}
