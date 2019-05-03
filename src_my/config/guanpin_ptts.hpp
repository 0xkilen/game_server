#pragma once

#include "utils/ptts.hpp"
#include "proto/config_guanpin.pb.h"

using namespace std;
namespace pc = proto::config;
namespace pd = proto::data;

namespace nora {
        namespace config {

                using guanpin_ptts = ptts<pc::guanpin>;
                guanpin_ptts& guanpin_ptts_instance();
                void guanpin_ptts_set_funcs();

                using guanpin_logic_ptts = ptts<pc::guanpin_logic>;
                guanpin_logic_ptts& guanpin_logic_ptts_instance();
                void guanpin_logic_ptts_set_funcs();

                using guanpin_fenghao_ptts = ptts<pc::guanpin_fenghao>;
                guanpin_fenghao_ptts& guanpin_fenghao_ptts_instance();
                void guanpin_fenghao_ptts_set_funcs();

        }
}
