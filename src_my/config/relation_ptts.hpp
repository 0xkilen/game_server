#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_relation.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using relation_logic_ptts = ptts<pc::relation_logic>;
                relation_logic_ptts& relation_logic_ptts_instance();
                void relation_logic_ptts_set_funcs();

                using relation_suggestion_ptts = ptts<pc::relation_suggestion>;
                relation_suggestion_ptts& relation_suggestion_ptts_instance();
                void relation_suggestion_ptts_set_funcs();

                using relation_gift_ptts = ptts<pc::relation_gift>;
                relation_gift_ptts& relation_gift_ptts_instance();
                void relation_gift_ptts_set_funcs();
}
}
