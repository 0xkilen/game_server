#pragma once

#include "utils/ptts.hpp"
#include "proto/config_arena.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using arena_npc_pool_ptts = ptts<pc::arena_npc_pool>;
                arena_npc_pool_ptts& arena_npc_pool_ptts_instance();
                void arena_npc_pool_ptts_set_funcs();

                using arena_reward_ptts = ptts<pc::arena_reward>;
                arena_reward_ptts& arena_reward_ptts_instance();
                void arena_reward_ptts_set_funcs();

                using arena_ptts = ptts<pc::arena>;
                arena_ptts& arena_ptts_instance();
                void arena_ptts_set_funcs();

                using public_arena_group_ptts = ptts<pc::public_arena_group>;
                public_arena_group_ptts& public_arena_group_ptts_instance();
                void public_arena_group_ptts_set_funcs();

        }
}
