#include "config/utils.hpp"
#include "utils/assert.hpp"
#include "ip_logic.hpp"
#include "log.hpp"

using namespace std;

namespace nora {
        namespace scene {

                uint64_t parse_ip_to_int(const string& ip) {
                        string str;
                        for (auto& i : split_string(ip, '.')) {
                                if (atoi(i.c_str()) < 10) {
                                        str += "00" + i;
                                } else if (atoi(i.c_str()) < 100) {
                                        str += "0" + i;
                                } else {
                                        str += i;
                                }
                        }
			str = '0' + str;
                        return stoul(str);
                }

                const pc::area_ip& fetch_area_ip_ptt(const string& ip) {
                        const auto& ptts = PTTS_GET_ALL(area_ip);
                        auto low = 1;
                        auto high = static_cast<int>(ptts.size());
                        auto mid = 1;
                        auto ip_area = parse_ip_to_int(ip);
                        while (low <= high) {
                                mid = (low + high) / 2;
                                const auto& ptt = ptts.at(mid);
                                auto start = parse_ip_to_int(ptt.ips().origin());
                                auto end = parse_ip_to_int(ptt.ips().end_point());
                                if (start <= ip_area && ip_area <= end) {
                                        return PTTS_GET(area_ip, ptt.id());
                                }
                                if (start < ip_area) {
                                        low = mid + 1;
                                } else {
                                        high = mid - 1;
                                }
                        }
                        return PTTS_GET(area_ip, static_cast<uint32_t>(ptts.size()));
                }

        }
}
