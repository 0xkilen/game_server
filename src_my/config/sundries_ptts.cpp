#include "utils.hpp"
#include "sundries_ptts.hpp"

namespace nora {
        namespace config {

                sundries_ptts& sundries_ptts_instance() {
                        static sundries_ptts inst;
                        return inst;
                }

                void sundries_ptts_set_funcs() {
                }
        }
}
