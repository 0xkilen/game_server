#include "utils.hpp"
#include "poetry_pool_ptts.hpp"

namespace nora {
        namespace config {

                poetry_pool_ptts& poetry_pool_ptts_instance() {
                        static poetry_pool_ptts inst;
                        return inst;
                }

                void poetry_pool_ptts_set_funcs() {
                }

        }
}
