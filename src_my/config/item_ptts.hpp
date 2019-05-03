#pragma once

#include "utils/ptts.hpp"
#include "proto/config_item.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using item_ptts = ptts<pc::item>;
                item_ptts& item_ptts_instance();
                void item_ptts_set_funcs();

                using item_xinwu_exchange_ptts = ptts<pc::item_xinwu_exchange>;
                item_xinwu_exchange_ptts& item_xinwu_exchange_ptts_instance();
                void item_xinwu_exchange_ptts_set_funcs();

        }
}
