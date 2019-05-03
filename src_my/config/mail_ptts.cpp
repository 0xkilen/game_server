#include "utils.hpp"
#include "mail_ptts.hpp"

namespace nora {
        namespace config {

                mail_ptts& mail_ptts_instance() {
                        static mail_ptts inst;
                        return inst;
                }

                void mail_ptts_set_funcs() {}

        }
}
