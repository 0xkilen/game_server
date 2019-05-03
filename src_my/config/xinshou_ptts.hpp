#pragma once

#include "utils/ptts.hpp"
#include "proto/config_xinshou.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using xinshou_ptts = ptts<pc::xinshou>;
                xinshou_ptts& xinshou_ptts_instance();
                void xinshou_ptts_set_funcs();

                using xinshou_group_ptts = ptts<pc::xinshou_group>;
                xinshou_group_ptts& xinshou_group_ptts_instance();
                void xinshou_group_ptts_set_funcs();

                using xinshou_guide_ptts = ptts<pc::xinshou_guide>;
                xinshou_guide_ptts& xinshou_guide_ptts_instance();
                void xinshou_guide_ptts_set_funcs();

                using module_introduce_ptts = ptts<pc::module_introduce>;
                module_introduce_ptts& module_introduce_ptts_instance();
                void module_introduce_ptts_set_funcs();

        }
}
