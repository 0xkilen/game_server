#include "skill_ptts.hpp"
#include "buff_ptts.hpp"
#include "utils.hpp"
#include "utils/exception.hpp"

namespace nora {
        namespace config {

                skill_ptts& skill_ptts_instance() {
                        static skill_ptts inst;
                        return inst;
                }

                void skill_ptts_set_funcs() {
                        skill_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.name().empty()) {
                                        CONFIG_ELOG << "skill " << ptt.id() << " name is empty";
                                }
                        };
                        skill_ptts_instance().modify_func_ = [] (auto& ptt) {
                                pd::condition_array levelup_conditions;
                                auto *cond = levelup_conditions.add_conditions();
                                cond->set_type(pd::condition::ACTOR_LEVEL_GE);
                                cond->add_arg(to_string(ptt.level() * 10));
                                for (const auto& i : ptt.levelup_conditions().conditions()) {
                                        *levelup_conditions.add_conditions() = i;
                                }
                                *ptt.mutable_levelup_conditions() = levelup_conditions;

                                pd::event_array levelup_events = ptt._levelup_events();
                                auto *event = levelup_events.add_events();
                                event->set_type(pd::event::LEVELUP_ACTOR_SKILL);
                                event->add_arg(to_string(ptt.id()));
                                for (const auto& i : ptt._levelup_events().events()) {
                                        *levelup_events.add_events() = i;
                                }
                                *ptt.mutable__levelup_events() = levelup_events;

                                modify_events_by_conditions(ptt.levelup_conditions(), *(ptt.mutable__levelup_events()));
                        };
                        skill_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.name().empty()) {
                                        CONFIG_ELOG << "skill " << ptt.id() << " name is empty";
                                }
                                if(ptt.on_hit_size() > 0) {
                                        for (auto i : ptt.on_hit()) {
                                                if (!PTTS_HAS(buff, i.pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " on_hit buff not exist " << i.pttid();
                                                }
                                        }
                                }
                                if (ptt.has_unite_skill()) {
                                        if (!PTTS_HAS(skill, ptt.unite_skill().dps())) {
                                                CONFIG_ELOG << ptt.id() << " unite skill dps not exist " << ptt.unite_skill().dps();
                                        }
                                        if (!PTTS_HAS(skill, ptt.unite_skill().tank())) {
                                                CONFIG_ELOG << ptt.id() << " unite skill tank not exist " << ptt.unite_skill().tank();
                                        }
                                        if (!PTTS_HAS(skill, ptt.unite_skill().assist())) {
                                                CONFIG_ELOG << ptt.id() << " unite skill assist not exist " << ptt.unite_skill().assist();
                                        }
                                }
                        };
                }

        }
}
