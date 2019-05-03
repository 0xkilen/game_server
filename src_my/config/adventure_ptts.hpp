#pragma once

#include "utils/ptts.hpp"
#include "proto/config_adventure.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using adventure_ptts = ptts<pc::adventure>;
                adventure_ptts& adventure_ptts_instance();
                void adventure_ptts_set_funcs();

                using adventure_episode_ptts = ptts<pc::adventure_episode>;
                adventure_episode_ptts& adventure_episode_ptts_instance();
                void adventure_episode_ptts_set_funcs();

                using adventure_chapter_ptts = ptts<pc::adventure_chapter>;
                adventure_chapter_ptts& adventure_chapter_ptts_instance();
                void adventure_chapter_ptts_set_funcs();

                using adventure_logic_ptts = ptts<pc::adventure_logic>;
                adventure_logic_ptts& adventure_logic_ptts_instance();
                void adventure_logic_ptts_set_funcs();

        }
}
