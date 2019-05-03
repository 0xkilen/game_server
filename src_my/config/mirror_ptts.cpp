#include "utils.hpp"
#include "mirror_ptts.hpp"
#include "growth_ptts.hpp"

namespace nora {
        namespace config {

                mirror_ptts& mirror_ptts_instance() {
                        static mirror_ptts inst;
                        return inst;
                }

                void mirror_ptts_set_funcs() {
                }

        }
}
