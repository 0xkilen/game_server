#pragma once

#include "utils/ptts.hpp"
#include "proto/config_help.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using help_ptts = ptts<pc::help>;
                help_ptts& help_ptts_instance();
                void help_ptts_set_funcs();

                using help_tips_ptts = ptts<pc::help_tips>;
                help_tips_ptts& help_tips_ptts_instance();
                void help_tips_ptts_set_funcs();
        }
}
