#include "utils.hpp"
#include "area_ip_ptts.hpp"

namespace nora {
        namespace config {

                area_ip_ptts& area_ip_ptts_instance() {
                        static area_ip_ptts inst;
                        return inst;
                }

                void area_ip_ptts_set_funcs() {
                        area_ip_ptts_instance().check_func_ = [] (const auto& ptt) {

                        };

                        area_ip_ptts_instance().modify_func_ = [] (const auto& ptt) {

                        };

                        area_ip_ptts_instance().verify_func_ = [] (const auto& ptt) {

                        };
                }

                week_calendar_ptts& week_calendar_ptts_instance() {
                        static week_calendar_ptts inst;
                        return inst;
                }

                void week_calendar_ptts_set_funcs() {

                }
        }
}
