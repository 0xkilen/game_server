#include "utils.hpp"
#include "chat_ptts.hpp"

namespace nora {
        namespace config {

                chat_ptts& chat_ptts_instance() {
                        static chat_ptts inst;
                        return inst;
                }

                void chat_ptts_set_funcs() {
                        chat_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::chat_type_Name(ptt.type()) << " check conditions failed";
                                }
                        };
                        chat_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__events());
                        };
                        chat_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::chat_type_Name(ptt.type()) << " verify conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << pd::chat_type_Name(ptt.type()) << " verify _events failed";
                                }
                        };
                }

                system_chat_ptts& system_chat_ptts_instance() {
                        static system_chat_ptts inst;
                        return inst;
                }

                void system_chat_ptts_set_funcs() {
                        system_chat_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.times() == 0) {
                                        CONFIG_ELOG << ptt.id() << " times is 0";
                                }
                                if (ptt.content().empty()) {
                                        CONFIG_ELOG << ptt.id() << " content size is 0";
                                }
                        };
                }
        }
}
