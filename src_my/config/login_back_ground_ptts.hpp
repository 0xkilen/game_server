#pragma once

#include "utils/ptts.hpp"
#include "proto/config_login_back_ground.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {
                using login_back_ground_ptts = ptts<pc::login_back_ground>;
                login_back_ground_ptts& login_back_ground_ptts_instance();
                void login_back_ground_ptts_set_funcs();
        }
}
