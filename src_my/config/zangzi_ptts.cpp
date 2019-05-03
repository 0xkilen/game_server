#include "utils/log.hpp"
#include "zangzi_ptts.hpp"

namespace nora {
namespace config {
        zangzi_ptts& zangzi_ptts_instance() {
                static zangzi_ptts inst;
                return inst;
        }

        void zangzi_ptts_set_funcs() {
                zangzi_ptts_instance().check_func_ = [] (const auto& ptt) {};
                zangzi_ptts_instance().modify_func_ = [] (const auto& ptt) {};
                zangzi_ptts_instance().verify_func_ = [] (const auto& ptt) {};
        }
}
}
