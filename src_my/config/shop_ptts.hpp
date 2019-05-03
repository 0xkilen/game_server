#pragma once

#include "utils/ptts.hpp"
#include "proto/config_shop.pb.h"

using namespace std;

namespace nora {
        namespace config {

                using shop_ptts = ptts<proto::config::shop>;
                shop_ptts& shop_ptts_instance();
                void shop_ptts_set_funcs();

                using mys_shop_ptts = ptts<proto::config::mys_shop>;
                mys_shop_ptts& mys_shop_ptts_instance();
                void mys_shop_ptts_set_funcs();

        }
}
