#pragma once

#include "utils/ptts.hpp"
#include "proto/config_growth.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using growth_ptts = ptts<pc::growth>;
                growth_ptts& growth_ptts_instance();
                void growth_ptts_set_funcs();

                using not_portional_growth_ptts = ptts<pc::not_portional_growth>;
                not_portional_growth_ptts& not_portional_growth_ptts_instance();
                void not_portional_growth_ptts_set_funcs();

                using craft_growth_ptts = ptts<pc::craft_growth>;
                craft_growth_ptts& craft_growth_ptts_instance();
                void craft_growth_ptts_set_funcs();

        }
}
