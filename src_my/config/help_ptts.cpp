#include "utils.hpp"
#include "help_ptts.hpp"

namespace nora {
        namespace config {

                help_ptts& help_ptts_instance() {
                        static help_ptts inst;
                        return inst;
                }

                void help_ptts_set_funcs() {
                }

                help_tips_ptts& help_tips_ptts_instance() {
                        static help_tips_ptts inst;
                        return inst;
                }

                void help_tips_ptts_set_funcs() {
                }

        }
}
