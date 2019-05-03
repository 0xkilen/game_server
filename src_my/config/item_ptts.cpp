#include "utils.hpp"
#include "item_ptts.hpp"
#include "origin_ptts.hpp"

namespace nora {
        namespace config {

                item_ptts& item_ptts_instance() {
                        static item_ptts inst;
                        return inst;
                }

                void item_ptts_set_funcs() {
                        item_ptts_instance().check_func_ = [] (auto& ptt) {
                                if (!check_conditions(ptt.use_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check use conditions failed";
                                }
                                if (!check_events(ptt.use_events())) {
                                        CONFIG_ELOG << ptt.id() << " check use events failed";
                                }
                                if (!check_events(ptt.target_events())) {
                                        CONFIG_ELOG << ptt.id() << " check target events failed";
                                }
                                if (!check_events(ptt.sell_events())) {
                                        CONFIG_ELOG << ptt.id() << " check sell events failed";
                                }
                        };
                        item_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.use_conditions(), *ptt.mutable_use_events());
                        };
                        item_ptts_instance().verify_func_ = [] (auto& ptt) {
                                for (auto i : ptt.origins()) {
                                        if (!PTTS_HAS(origin, i)) {
                                                CONFIG_ELOG << ptt.id() << " origin not exist " << i;
                                        }
                                }
                                if (!verify_conditions(ptt.use_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify use conditions failed";
                                }
                                if (!verify_events(ptt.use_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify use events failed";
                                }
                                if (!verify_events(ptt.target_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify target events failed";
                                }
                                if (!verify_events(ptt.sell_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify sell events failed";
                                }
                        };
                }

                item_xinwu_exchange_ptts& item_xinwu_exchange_ptts_instance() {
                        static item_xinwu_exchange_ptts inst;
                        return inst;
                }

                void item_xinwu_exchange_ptts_set_funcs() {
                        item_xinwu_exchange_ptts_instance().check_func_ = [] (auto& ptt) {
                                if (!check_conditions(ptt.compose_conditions())) {
                                        CONFIG_ELOG << ptt.xinwu() << " check compose conditions failed";
                                }
                                if (!check_events(ptt.compose_events())) {
                                        CONFIG_ELOG << ptt.xinwu() << " check compose events failed";
                                }
                                if (!check_conditions(ptt.decompose_conditions())) {
                                        CONFIG_ELOG << ptt.xinwu() << " check decompose conditions failed";
                                }
                                if (!check_events(ptt.decompose_events())) {
                                        CONFIG_ELOG << ptt.xinwu() << " check decompose events failed";
                                }
                        };
                        item_xinwu_exchange_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.compose_conditions(), *ptt.mutable_compose_events());
                                modify_events_by_conditions(ptt.decompose_conditions(), *ptt.mutable_decompose_events());
                        };
                        item_xinwu_exchange_ptts_instance().verify_func_ = [] (auto& ptt) {
                                if (!verify_conditions(ptt.compose_conditions())) {
                                        CONFIG_ELOG << ptt.xinwu() << " verify compose conditions failed";
                                }
                                if (!verify_events(ptt.compose_events())) {
                                        CONFIG_ELOG << ptt.xinwu() << " verify compose events failed";
                                }
                                if (!verify_conditions(ptt.decompose_conditions())) {
                                        CONFIG_ELOG << ptt.xinwu() << " verify decompose conditions failed";
                                }
                                if (!verify_events(ptt.decompose_events())) {
                                        CONFIG_ELOG << ptt.xinwu() << " verify decompose events failed";
                                }
                        };
                }

        }
}
