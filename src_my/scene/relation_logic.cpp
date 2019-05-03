#include "relation_logic.hpp"
#include "scene/player/role.hpp"

namespace nora {
        namespace scene {

                pd::result relation_check_send_gift(role& role, uint32_t gift, uint64_t to_gid, uint32_t count) {
                        if (!PTTS_HAS(relation_gift, gift)) {
                                return pd::RELATION_INVAILID_GIFT;
                        }
                        const auto& ptt = PTTS_GET(relation_gift, gift);
                        if (role.item_count(ptt.item()) < static_cast<int>(count)) {
                                return pd::ROLE_ITEM_NOT_ENOUGH;
                        }
                        if (role.relation_in_blacklist(to_gid)) {
                                return pd::RELATION_IN_BLACKLIST;
                        }
                        return pd::OK;
                }

                pd::result relation_check_receive_gift(role& role, uint64_t to_gid) {
                        if (role.relation_in_blacklist(to_gid)) {
                                return pd::RELATION_IN_BLACKLIST;
                        }
                        return pd::OK;
                }

                pd::record_entity relation_receive_gift(role& role, uint64_t from, uint32_t gift, uint32_t count, uint64_t& comment_gid) {
                        role.add_comment(from, gift, count, comment_gid);
                        return role.relation_receive_gift(from, gift, count);
                }

                pd::result relation_check_apply_friend(role& role, uint64_t gid) {
                        if (gid == role.gid()) {
                                return pd::RELATION_OTHER_IS_SELF;
                        }
                        if (role.relation_in_blacklist(gid)) {
                                return pd::RELATION_IN_BLACKLIST;
                        }
                        if (role.relation_has_friend(gid)) {
                                return pd::RELATION_ALREADY_FRIEND;
                        }
                        if (role.relation_has_application(gid)) {
                                return pd::RELATION_ALREADY_APPLIED;
                        }
                        return pd::OK;
                }

                pd::result relation_check_remove_friend(role& role, uint64_t gid) {
                        if (!role.relation_has_friend(gid)) {
                                return pd::RELATION_NO_SUCH_FRIEND;
                        }
                        if (role.marriage_married() && role.marriage_spouse() == gid) {
                                return pd::RELATION_OTHER_IS_SPOUSE;
                        }
                        return pd::OK;
                }

                pd::result relation_check_blacklist(role& role, uint64_t gid) {
                        if (gid == role.gid()) {
                                return pd::RELATION_OTHER_IS_SELF;
                        }
                        if (role.relation_in_blacklist(gid)) {
                                return pd::RELATION_IN_BLACKLIST;
                        }
                        if (role.marriage_married() && role.marriage_spouse() == gid) {
                                return pd::RELATION_OTHER_IS_SPOUSE;
                        }
                        return pd::OK;
                }

                pd::result relation_check_application(role& role, uint64_t gid) {
                        if (gid == role.gid()) {
                                return pd::RELATION_OTHER_IS_SELF;
                        }
                        if (role.relation_friend_full()) {
                                return pd::RELATION_FRIENDLIST_FULL;
                        }
                        if (!role.relation_has_application(gid)) {
                                return pd::RELATION_NO_SUCH_APPLICATION;
                        }
                        return pd::OK;
                }

                pd::result relation_check_follow(role& role, uint64_t gid) {
                        if (gid == role.gid()) {
                                return pd::RELATION_OTHER_IS_SELF;
                        }
                        if (role.relation_in_blacklist(gid)) {
                                return pd::RELATION_IN_BLACKLIST;
                        }
                        if (role.relation_in_followlist(gid)) {
                                return pd::RELATION_ALREADY_IN_FOLLOWLIST;
                        }
                        if (role.relation_followlist_full()) {
                                return pd::RELATION_FOLLOWLIST_FULL;
                        }
                        return pd::OK;
                }
        }
}
