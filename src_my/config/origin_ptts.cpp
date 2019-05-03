#include "utils.hpp"
#include "origin_ptts.hpp"

namespace pd = proto::data;
namespace pcs = proto::config;

namespace nora {
        namespace config {

                origin_ptts& origin_ptts_instance() {
                        static origin_ptts inst;
                        return inst;
                }

                void origin_ptts_set_funcs() {

                }

        }
}
