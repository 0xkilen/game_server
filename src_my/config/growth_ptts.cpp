#include "utils.hpp"
#include "utils/assert.hpp"
#include "growth_ptts.hpp"
#include "actor_ptts.hpp"

namespace nora {
        namespace config {

                growth_ptts& growth_ptts_instance() {
                        static growth_ptts inst;
                        return inst;
                }

                void growth_ptts_set_funcs() {
                }

                not_portional_growth_ptts& not_portional_growth_ptts_instance() {
                        static not_portional_growth_ptts inst;
                        return inst;
                }

                void not_portional_growth_ptts_set_funcs() {
                }

                craft_growth_ptts& craft_growth_ptts_instance() {
                        static craft_growth_ptts inst;
                        return inst;
                }

                void craft_growth_ptts_set_funcs() {
                }

        }
}
