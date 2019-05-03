#include "utils.hpp"
#include "tower_ptts.hpp"
#include "battle_ptts.hpp"
#include "actor_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                tower_ptts& tower_ptts_instance() {
                        static tower_ptts inst;
                        return inst;
                }

                void tower_ptts_set_funcs() {
                        tower_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.level() << " check conditions failed";
                                }
                                if (!check_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.level() << " check win events failed";
                                }
                                if (!check_events(ptt.first_win_events())) {
                                        CONFIG_ELOG << ptt.level() << " check first win events failed";
                                }
                        };
                        tower_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.level() > 1) {
                                        auto *c = ptt.mutable_conditions()->add_conditions();
                                        c->set_type(pd::condition::TOWER_CUR_LEVEL_E);
                                        c->add_arg(to_string(ptt.level() - 1));
                                }

                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable_win_events());
                        };
                        tower_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.level() << " verify conditions failed";
                                }
                                if (!verify_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.level() << " verify win events failed";
                                }
                                if (!verify_events(ptt.first_win_events())) {
                                        CONFIG_ELOG << ptt.level() << " verify first win events failed";
                                }
                                for (auto i : ptt.recommend_actors()) {
                                        if (!PTTS_HAS(actor, i)) {
                                                CONFIG_ELOG << ptt.level() << " recommend actors not exist " << i;
                                        }
                                }
                        };
                }

                tower_logic_ptts& tower_logic_ptts_instance() {
                        static tower_logic_ptts inst;
                        return inst;
                }

                void tower_logic_ptts_set_funcs() {
                        tower_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.refresh_conditions())) {
                                        CONFIG_ELOG << "check refresh conditions failed";
                                }
                                if (!check_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "check unlock conditions failed";
                                }
                        };
                        tower_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.refresh_conditions(), *ptt.mutable__refresh_events());
                        };
                        tower_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.refresh_conditions())) {
                                        CONFIG_ELOG << "verify refresh conditions failed";
                                }
                                if (!verify_events(ptt._refresh_events())) {
                                        CONFIG_ELOG << "verify refresh events failed";
                                }
                                if (!verify_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "verify unlock conditions failed";
                                }
                        };
                }

        }
}
