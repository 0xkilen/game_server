#include "utils.hpp"
#include "common_ptts.hpp"
#include "actor_ptts.hpp"

namespace nora {
        namespace config {

                system_preview_ptts& system_preview_ptts_instance() {
                        static system_preview_ptts inst;
                        return inst;
                }

                void system_preview_ptts_set_funcs() {
                        system_preview_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << ptt.level() << " actor not exist " << ptt.actor();
                                }
                        };
                }

                name_pool_ptts& name_pool_ptts_instance() {
                        static name_pool_ptts inst;
                        return inst;
                }

                void name_pool_ptts_set_funcs() {
                }

                common_ptts& common_ptts_instance() {
                        static common_ptts inst;
                        return inst;
                }

                void common_ptts_set_funcs() {
                        common_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.forbid_name_actors()) {
                                        if (!PTTS_HAS(actor, i)) {
                                                CONFIG_ELOG <<  " forbid name actor not exist " << i;
                                        }
                                }
                        };
                }

        }
}
