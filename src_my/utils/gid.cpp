#include "gid.hpp"
#include "time_utils.hpp"
#include "assert.hpp"

namespace nora {

        void gid::set_server_id(int server_id) {
                server_id_ = server_id;
        }

        uint64_t gid::new_gid(gid_type type) {
                lock_guard<mutex> lock(mutex_);

                uint64_t ret;
                uint32_t now_time_t = system_clock::to_time_t(system_clock::now());
                auto *tem = (gid_template *)&ret;
                tem->type_ = static_cast<uint8_t>(type);
                tem->server_id_ = server_id_;
                if (now_time_t != serial_time_) {
                        for (auto& i : serial_) {
                                i.second = 0;
                        }
                        serial_time_ = now_time_t;
                }
                ASSERT(serial_[type] <= numeric_limits<uint16_t>::max());
                tem->serial_ = serial_[type]++;
                tem->time_ = serial_time_;
                return ret;
        }

        gid_type gid::get_type(uint64_t gid) {
                return static_cast<gid_type>((reinterpret_cast<gid_template *>(&gid)->type_));
        }

        uint64_t gid::change_type(uint64_t gid, gid_type type) {
                reinterpret_cast<gid_template *>(&gid)->type_ = static_cast<uint64_t>(type);
                return gid;
        }

        int gid::get_server_id(uint64_t gid) {
                return static_cast<int>((reinterpret_cast<gid_template *>(&gid)->server_id_));
        }

}
