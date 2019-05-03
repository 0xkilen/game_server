#pragma once

#include "utils/ptts.hpp"
#include "proto/config_area_ip.pb.h"
#include "proto/config_week_calendar.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using area_ip_ptts = ptts<pc::area_ip>;
                area_ip_ptts& area_ip_ptts_instance();
                void area_ip_ptts_set_funcs();

                using week_calendar_ptts = ptts<pc::week_calendar>;
                week_calendar_ptts& week_calendar_ptts_instance();
                void week_calendar_ptts_set_funcs();

        }
}
