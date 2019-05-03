#include "utils.hpp"
#include "login_back_ground_ptts.hpp"

using namespace std;

namespace pc = proto::config;

namespace nora {
        namespace config {

                login_back_ground_ptts& login_back_ground_ptts_instance() {
                        static login_back_ground_ptts inst;
                        return inst;
                }

                void login_back_ground_ptts_set_funcs() {
                }
        }
}
