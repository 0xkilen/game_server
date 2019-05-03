#pragma once

#include "utils/time_utils.hpp"
#include "utils/exception.hpp"
#include <chrono>
#include <mutex>

using namespace std;
using namespace chrono;

namespace nora {

        enum class gid_type {
                ROLE,
                NOTICE,
                MAIL,
                LEAGUE,
                GLADIATOR,
                ARENA_CHALLENGE_RECORD,
                RECHARGE_ORDER,
                MANSION_FURNITURE,
                MANSION_QUEST,
                CHILD,
                CONFLICT_BATTLE_RECORD,
                GUANPIN_CHANGED_RECORD,
                MIRROR_ROLE,
        };

#pragma pack(push)
#pragma pack(1)
        struct gid_template {
                uint8_t type_: 5;
                uint16_t server_id_: 11;
                uint32_t time_;
                uint16_t serial_;
        };
#pragma pack(pop)

        class gid {
        public:
                static gid& instance() {
                        static gid inst;
                        return inst;
                }
                void set_server_id(int server_id);
                uint64_t new_gid(gid_type type);
                gid_type get_type(uint64_t gid);
                uint64_t change_type(uint64_t gid, gid_type type);
                int get_server_id(uint64_t gid);
        private:
                mutable mutex mutex_;
                uint32_t serial_time_;
                map<gid_type, uint32_t> serial_;
                int server_id_;
        };

}
