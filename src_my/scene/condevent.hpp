#pragma once

#include "condition.hpp"
#include "event.hpp"
#include "scene/player/role.hpp"
#include "proto/data_condevent.pb.h"
#include <set>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                class role;

                pd::result condevent_check(const pd::condevent_array& cea, const role& role, const pd::ce_env& ce = pd::ce_env());
                pd::result condevent_check(const pd::cond_event_array& cea, const role& role, const pd::ce_env& ce = pd::ce_env());
                pd::event_res condevent_process(const pd::condevent_array& cea, role& role, const pd::ce_env& ce, pd::event_res *merge_er = nullptr, bool send_bi = true);
                pd::event_array condevent_get_events(const pd::cond_event_array& cea, role& role, const pd::ce_env& ce);
        }
}
