#include "drop.hpp"
#include "scene/player/role.hpp"
#include "scene/event.hpp"

namespace nora {
        namespace scene {

                pd::event_array drop_process(const pc::drop& ptt) {
                        ASSERT(ptt.has__use_odds());
                        if (!ptt._use_odds()) {
                                vector<drop_item<pd::event_array>> dis;
                                for (const auto& i : ptt.events()) {
                                        dis.emplace_back(i.events(), i.weight());
                                }
                                auto dropped = drop_by_weight(dis);
                                ASSERT(dropped.size() == 1);
                                return dropped[0];
                        } else {
                                pd::event_array events;
                                for (const auto& i : ptt.events()) {
                                        if (rand() % 1000 < i.odds()) {
                                                event_merge(events, i.events());
                                        }
                                }
                                return events;
                        }
                }

                pd::league_event_array drop_process(const pc::league_drop& ptt) {
                        ASSERT(ptt.has__use_odds());
                        if (!ptt._use_odds()) {
                                vector<drop_item<pd::league_event_array>> dils;
                                for (const auto& i : ptt.events()) {
                                        dils.emplace_back(i.events(), i.weight());
                                }
                                auto dropped = drop_by_weight(dils);
                                ASSERT(dropped.size() == 1);
                                return dropped[0];
                        } else {
                                pd::league_event_array events;
                                for (const auto& i : ptt.events()) {
                                        if (rand() % 1000 < i.odds()) {
                                                league_event_merge(events, i.events());
                                        }
                                }
                                return events;
                        }
                }

                pd::child_event_array drop_process(const pc::child_drop& ptt) {
                        vector<drop_item<pd::child_event_array>> dis;
                        for (const auto& i : ptt.events()) {
                                dis.emplace_back(i.events(), i.weight());
                        }
                        auto dropped = drop_by_weight(dis);
                        ASSERT(dropped.size() == 1);
                        return dropped[0];
                }

        }
}
