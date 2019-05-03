#include "utils.hpp"
#include "loading_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                loading_ptts& loading_ptts_instance() {
                        static loading_ptts inst;
                        return inst;
                }

                void loading_ptts_set_funcs() {
                        loading_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << " check conditions failed";
                                }
                        };
                }

        }

}