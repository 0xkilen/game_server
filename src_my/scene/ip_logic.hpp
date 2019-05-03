#pragma once

#include "config/area_ip_ptts.hpp"
#include <string>

namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                uint64_t parse_ip_to_int(const string& ip);
                const pc::area_ip& fetch_area_ip_ptt(const string& ip);

        }
}
