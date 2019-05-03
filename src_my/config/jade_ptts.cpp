#include "utils.hpp"
#include "jade_ptts.hpp"
#include "origin_ptts.hpp"

namespace nora {
        namespace config {

                jade_ptts& jade_ptts_instance() {
                        static jade_ptts inst;
                        return inst;
                }

                void jade_ptts_set_funcs() {
                        jade_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.origins()) {
                                        if (!PTTS_HAS(origin, i)) {
                                                CONFIG_ELOG << ptt.id() << " origin not exist " << i;
                                        }
                                }
                                if (ptt.has_levelup_jade() && !PTTS_HAS(jade, ptt.levelup_jade())) {
                                        CONFIG_ELOG << ptt.id() << " levelup jade not exist " << ptt.levelup_jade();
                                }
                        };
                }

        }
}
