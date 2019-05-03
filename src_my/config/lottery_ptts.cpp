#include "utils.hpp"
#include "lottery_ptts.hpp"
#include "utils/exception.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                lottery_ptts& lottery_ptts_instance() {
                        static lottery_ptts inst;
                        return inst;
                }

                void lottery_ptts_set_funcs() {
                        lottery_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.one_time_conditions().condevents()) {
                                        if(!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check one_time_conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.ten_time_conditions().condevents()) {
                                        if(!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check ten_time_conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.lottery_events()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check lottery_events failed";
                                        }
                                }
                        };
                        lottery_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto i = 0 ;i < ptt.one_time_conditions().condevents_size(); i++) {
                                        auto *condevent = ptt.mutable_one_time_conditions()->mutable_condevents(i);
                                        auto *de = condevent->mutable__events();
                                        modify_events_by_conditions(condevent->conditions(), *de);
                                }
                                for (auto i = 0 ;i < ptt.ten_time_conditions().condevents_size(); i++) {
                                        auto *condevent = ptt.mutable_ten_time_conditions()->mutable_condevents(i);
                                        auto *de = condevent->mutable__events();
                                        modify_events_by_conditions(condevent->conditions(), *de);
                                }
                        };
                        lottery_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.one_time_conditions().condevents()) {
                                        if(!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify one_time_conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.one_time_conditions().condevents()) {
                                        if(!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify ten_time_conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.lottery_events()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify lottery_events failed";
                                        }
                                }
                                if (ptt.lottery_pttids_size() > 0) {
                                        for (auto i : ptt.lottery_pttids()) {
                                                if (!PTTS_HAS(lottery, i)) {
                                                        CONFIG_ELOG << ptt.id() << " lottery not exist " << i;
                                                } else {
                                                        auto& lottery_ptt = PTTS_GET(lottery, i);
                                                        lottery_ptt.set__has_parent(true);
                                                        if (ptt.has_common_events()) {
                                                                for (auto& j : *lottery_ptt.mutable_lottery_events()) {
                                                                        for (const auto& k : lottery_ptt.common_events().events()) {
                                                                                *j.mutable_events()->add_events() = k;
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                } else if (!ptt.has_loop_back_draw_times()) {
                                        CONFIG_ELOG << ptt.id() << " need loop back draw times";
                                }
                        };
                }

                lottery_show_ptts& lottery_show_ptts_instance() {
                        static lottery_show_ptts inst;
                        return inst;
                }

                void lottery_show_ptts_set_funcs() {
                        lottery_show_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_events(ptt.events());
                        };
                }

        }
}
