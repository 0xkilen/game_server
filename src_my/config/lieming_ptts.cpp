#include "utils.hpp"
#include "lieming_ptts.hpp"
#include "battle_ptts.hpp"
#include "plot_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                lieming_ptts& lieming_ptts_instance() {
                        static lieming_ptts inst;
                        return inst;
                }

                void lieming_ptts_set_funcs() {
                        lieming_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *we = ptt.mutable_win_events();
                                auto *event = we->add_events();
                                event->set_type(pd::event::PASS_LIEMING);
                                event->add_arg(to_string(ptt.id()));

                                for (auto i = 0; i < ptt.challenge_conditions().condevents_size(); i++) {
                                        auto *condevent = ptt.mutable_challenge_conditions()->mutable_condevents(i);
                                        auto *de = condevent->mutable__events();
                                        modify_events_by_conditions(condevent->conditions(), *de);
                                }
                        };
                }

                lieming_logic_ptts& lieming_logic_ptts_instance() {
                        static lieming_logic_ptts inst;
                        return inst;
                }

                void lieming_logic_ptts_set_funcs() {
                        lieming_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(plot, ptt.open_plot())) {
                                        CONFIG_ELOG << "open plot not exist " << ptt.open_plot();
                                }
                        };
                }

        }
}
