#include "utils.hpp"
#include "resource_ptts.hpp"

namespace nora {
        namespace config {

                resource_ptts& resource_ptts_instance() {
                        static resource_ptts inst;
                        return inst;
                }

                void resource_ptts_set_funcs() {
                        resource_ptts_instance().check_func_ = [] (const auto& ptt) {};
                        resource_ptts_instance().modify_func_ = [] (const auto& ptt) {};
                        resource_ptts_instance().verify_func_ = [] (const auto& ptt) {};
                }
        }
}
