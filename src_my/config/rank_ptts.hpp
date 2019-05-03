#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_rank.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using rank_reward_ptts = ptts<pc::rank_reward>;
                rank_reward_ptts& rank_reward_ptts_instance();
                void rank_reward_ptts_set_funcs();

                using rank_logic_ptts = ptts<pc::rank_logic>;
                rank_logic_ptts& rank_logic_ptts_instance();
                void rank_logic_ptts_set_funcs();

        }
}
