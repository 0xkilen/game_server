#include "quest.hpp"

namespace nora {
        namespace scene {

                pd::result quest_check_commit(uint32_t pttid, const role& role) {
                        const auto& ptt = PTTS_GET(quest, pttid);
                        if (!role.quest_has_accepted(ptt.id())) {
                                return pd::QUEST_NOT_ACCEPTED;
                        }
                        return role.quest_check_commit(ptt.id());
                }

                pd::event_array quest_commit_to_events(uint32_t pttid, role& role) {
                        ASSERT(role.quest_has_accepted(pttid));

                        auto& quest = role.get_accepted_quest(pttid);
                        pd::event_array events;
                        for (const auto& i : quest.counters_) {
                                if (i.type_ == pd::TURN_IN_ITEM) {
                                        ASSERT(i.param_.arg_size() == 1);
                                        auto need = i.param_.need();
                                        if (need > 0) {
                                                auto *e = events.add_events();
                                                e->set_type(pd::event::REMOVE_ITEM);
                                                e->add_arg(i.param_.arg(0));
                                                e->add_arg(to_string(need));
                                        }
                                } else if (i.type_ == pd::TURN_IN_RESOURCE) {
                                        ASSERT(i.param_.arg_size() == 1);
                                        auto need = i.param_.need();
                                        if (need > 0) {
                                                auto *e = events.add_events();
                                                e->set_type(pd::event::DEC_RESOURCE);
                                                e->add_arg(i.param_.arg(0));
                                                e->add_arg(to_string(need));
                                        }
                                }
                        }
                        event_merge(events, role.get_quest_cur_pass_events(pttid));

                        role.quest_move_to_next_param(pttid);
                        role.quest_commit(pttid);

                        return events;
                }

                pd::event_res quest_commit(uint32_t pttid, role& role) {
                        auto events = quest_commit_to_events(pttid, role);

                        const auto& ptt = PTTS_GET(quest, pttid);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_QUEST_COMMIT);
                        ce.add_actors(role.get_role_actor()->pttid());
                        if (ptt.has__everyday_quest()) {
                                const auto& eq_ptt = PTTS_GET(everyday_quest, ptt._everyday_quest());
                                ce.set_origin(pd::CO_QUEST_COMMIT_EVERYDAY_QUEST);
                                event_merge(events, eq_ptt.events());
                        } else if (ptt._activity_seven_days_fuli_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_SEVEN_DAYS_FULI);
                        } else if (ptt._activity_seven_days_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_SEVEN_DAYS_QUEST);
                        } else if (ptt.has__activity_leiji_recharge_quest()) {
                                 ce.set_origin(pd::CO_ACTIVITY_LEIJI_RECHARGE);
                        } else if (ptt.has__activity_leiji_consume_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_LEIJI_CONSUME);
                        } else if (ptt.has__activity_festival_recharge_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_FESTIVAL_RECHARGE);
                        } else if (ptt.has__activity_festival_fuli_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_FESTIVAL_FULI);
                        } else if (ptt.has__activity_continue_recharge_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_CONTINUE_RECAHRGE);
                        } else if (ptt.has__activity_limit_play_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_LIMIT_PLAY);
                        } else if (ptt.has__activity_daiyanren_duihuan_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_DAIYANREN_DUIHUAN_REWARD);
                        } else if (ptt.has__activity_daiyanren_fuli_quest()) {
                                ce.set_origin(pd::CO_ACTIVITY_DAIYANREN_FULI_REWARD);
                        }

                        return event_process(events, role, ce);
                }

                void set_quest_to_events(uint32_t pttid, pd::event_array& events) {
                        auto *e = events.add_events();
                        e->set_type(pd::event::ACCEPT_QUEST);
                        e->add_arg(to_string(pttid));
                }

                pd::result quest_check_everyday_reward(uint32_t pttid, const role& role) {
                        const auto& ptt = PTTS_GET(everyday_quest_reward, pttid);
                        if (role.got_everyday_quest_reward(pttid)) {
                                return pd::QUEST_ALREADY_GOT_THIS_REWARD;
                        }
                        return condition_check(ptt.conditions(), role);
                }

                pd::event_res quest_everyday_reward(uint32_t pttid, role& role) {
                        const auto& ptt = PTTS_GET(everyday_quest_reward, pttid);
                        role.add_got_everyday_quest_reward(pttid);

                        pd::ce_env ce;
                        ce.set_origin(pd::CO_QUEST_EVERYDAY_QUEST_REWARD);
                        return event_process(ptt.events(), role, ce);
                }

        }
}
