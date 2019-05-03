#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_quest.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using quest_ptts = ptts<pc::quest>;
                quest_ptts& quest_ptts_instance();
                void quest_ptts_set_funcs();

                using everyday_quest_ptts = ptts<pc::everyday_quest>;
                everyday_quest_ptts& everyday_quest_ptts_instance();
                void everyday_quest_ptts_set_funcs();

                using everyday_quest_reward_ptts = ptts<pc::everyday_quest_reward>;
                everyday_quest_reward_ptts& everyday_quest_reward_ptts_instance();
                void everyday_quest_reward_ptts_set_funcs();

                using ui_quest_ptts = ptts<pc::ui_quest>;
                ui_quest_ptts& ui_quest_ptts_instance();
                void ui_quest_ptts_set_funcs();

        }
}
