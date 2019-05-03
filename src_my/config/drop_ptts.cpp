#include "utils.hpp"
#include "drop_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                drop_ptts& drop_ptts_instance() {
                        static drop_ptts inst;
                        return inst;
                }

                void drop_ptts_set_funcs() {
                        drop_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check events failed";
                                        }

                                }
                        };
                        drop_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (i.has_odds()) {
                                                if (ptt.has__use_odds()) {
                                                        if (!ptt._use_odds()) {
                                                                CONFIG_ELOG << ptt.id() << " use odds and weight conflict";
                                                        }
                                                } else {
                                                        ptt.set__use_odds(true);
                                                }
                                        } else if (i.has_weight()) {
                                                if (ptt.has__use_odds()) {
                                                        if (ptt._use_odds()) {
                                                                CONFIG_ELOG << ptt.id() << " use odds and weight conflict";
                                                        }
                                                } else {
                                                        ptt.set__use_odds(false);
                                                }
                                        } else {
                                                CONFIG_ELOG << ptt.id() << " no odds and no weight";
                                        }
                                }
                                for (auto i = 0; i < ptt.events_size(); ++i) {
                                        auto *e = ptt.mutable_events(i)->mutable_events()->add_events();
                                        e->set_type(pd::event::DROP_RECORD);
                                        e->add_arg(to_string(ptt.id()));
                                        e->add_arg(to_string(i));
                                }
                        };
                        drop_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify events failed";
                                        }
                                }
                        };
                }

        }
}
