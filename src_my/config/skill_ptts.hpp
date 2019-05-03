#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_skill.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using skill_ptts = ptts<pc::skill>;
                skill_ptts& skill_ptts_instance();
                void skill_ptts_set_funcs();

        }
}
