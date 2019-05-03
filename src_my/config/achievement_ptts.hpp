#pragma once

#include "utils/ptts.hpp"
#include "proto/config_achievement.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using achievement_ptts = ptts<pc::achievement>;
                achievement_ptts& achievement_ptts_instance();
                void achievement_ptts_set_funcs();

        }
}
