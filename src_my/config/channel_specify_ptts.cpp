#include "utils.hpp"
#include "channel_specify_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                channel_specify_ptts& channel_specify_ptts_instance() {
                        static channel_specify_ptts inst;
                        return inst;
                }

                void channel_specify_ptts_set_funcs() {
                }
        }
}
