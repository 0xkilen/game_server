#include "utils.hpp"
#include "title_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                title_ptts& title_ptts_instance() {
                        static title_ptts inst;
                        return inst;
                }

                void title_ptts_set_funcs() {
                }
        }
}
