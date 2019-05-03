#pragma once

#include "scene/player/role.hpp"
#include "scene/event.hpp"
#include "scene/condition.hpp"

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                pd::result quest_check_commit(uint32_t pttid, const role& role);
                pd::event_res quest_commit(uint32_t pttid, role& role);
                pd::event_array quest_commit_to_events(uint32_t pttid, role& role);
                void set_quest_to_events(uint32_t pttid, pd::event_array& events);

                pd::result quest_check_everyday_reward(uint32_t pttid, const role& role);
                pd::event_res quest_everyday_reward(uint32_t pttid, role& role);

        }
}
