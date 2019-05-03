#pragma once

#include "utils/ptts.hpp"
#include "proto/config_lottery.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using lottery_ptts = ptts<pc::lottery>;
                lottery_ptts& lottery_ptts_instance();
                void lottery_ptts_set_funcs();

                using lottery_show_ptts = ptts<pc::lottery_show>;
                lottery_show_ptts& lottery_show_ptts_instance();
                void lottery_show_ptts_set_funcs();

        }
}
