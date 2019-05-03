#pragma once

#include "utils/ptts.hpp"
#include "proto/config_behavior_tree.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using behavior_tree_ptts = ptts<pc::behavior_tree_node>;
                behavior_tree_ptts& behavior_tree_ptts_instance();
                void behavior_tree_ptts_set_funcs();

                using behavior_tree_root_ptts = ptts<pc::behavior_tree_root>;
                behavior_tree_root_ptts& behavior_tree_root_ptts_instance();
                void behavior_tree_root_ptts_set_funcs();

        }
}
