#include "condevent.hpp"

#include "condition.hpp"
#include "event.hpp"
#include "scene/player/role.hpp"

namespace nora {
        namespace scene {

                pd::result condevent_check(const pd::condevent_array& cea, const role& role, const pd::ce_env& ce) {
                        auto result = pd::NOT_FOUND;
                        for (const auto& i : cea.condevents()) {
                                result = condition_check(i.conditions(), role, ce);
                                if (result == pd::OK) {
                                        return result;
                                }
                        }
                        return result;
                }

                pd::result condevent_check(const pd::cond_event_array& cea, const role& role, const pd::ce_env& ce) {
                        auto result = pd::NOT_FOUND;
                        for (const auto& i : cea.condevents()) {
                                result = condition_check(i.conditions(), role, ce);
                                if (result == pd::OK) {
                                        return result;
                                }
                        }
                        return result;
                }

                pd::event_res condevent_process(const pd::condevent_array& cea, role& role, const pd::ce_env& ce, pd::event_res *merge_er, bool send_bi) {
                        for (const auto& i : cea.condevents()) {
                                if (condition_check(i.conditions(), role, ce) == pd::OK) {
                                        return event_process(i.events(), role, ce, merge_er, send_bi);
                                }
                        }
                        ASSERT(false);
                        return pd::event_res();
                }

                pd::event_array condevent_get_events(const pd::cond_event_array& cea, role& role, const pd::ce_env& ce) {
                        pd::event_array events;
                        auto result = pd::OK;
                        for (const auto& i: cea.condevents()) {
                                result = condition_check(i.conditions(), role, ce);
                                if (result == pd::OK) {
                                        for (const auto& j: i._events().events()) {
                                                *events.add_events() = j;
                                        }
                                        return events;
                                }
                        }
                        return events;
                }
        }
}
