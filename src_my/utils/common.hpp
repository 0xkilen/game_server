#pragma once

#include <boost/asio/system_timer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdint>

using namespace std;
namespace ba = boost::asio;

using timer_type = ba::steady_timer;

#define TO_TIME_T steady_clock_to_time_t

namespace nora {

        const size_t KB = (1024);
        const size_t MB = (1024 * KB);
        const size_t GB = (1024 * MB);

}
