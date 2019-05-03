#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_record.pb.h"
#include <memory>

namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;

                pd::result relation_check_send_gift(role& role, uint32_t gift, uint64_t to_gid, uint32_t count);
                pd::result relation_check_receive_gift(role& role, uint64_t to_gid);
                pd::record_entity relation_receive_gift(role& role, uint64_t from, uint32_t gift, uint32_t count, uint64_t& comment_gid);
                pd::result relation_check_apply_friend(role& role, uint64_t gid);
                pd::result relation_check_remove_friend(role& role, uint64_t gid);
                pd::result relation_check_blacklist(role& role, uint64_t gid);
                pd::result relation_check_application(role& role, uint64_t gid);
                pd::result relation_check_follow(role& role, uint64_t gid);
        }
}
