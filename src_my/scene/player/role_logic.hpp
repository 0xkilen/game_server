#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_player.pb.h"
#include "proto/data_rank.pb.h"
#include <string>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;

                pd::result role_check_eat(const role& role);
                pd::event_res role_eat(role& role);
                pd::result role_check_add_fate(const role& role);
                pd::event_res role_add_fate(role& role);
                pd::result role_check_comment(const string& content, const role& role);
                pd::result role_comment_by_other(uint64_t from, uint64_t reply_to, const string& content, role& role, uint64_t& gid);
                pd::result role_check_buy_present(uint32_t present, const role& role);
                pd::event_res role_buy_present(uint32_t present, role& role);
                pd::result role_check_upvote_by_other(uint64_t other, const role& role);
                pd::event_res role_upvote_by_other(uint64_t other, role& role, uint32_t& present);
                pd::result role_check_share(const role& role);
                pd::event_res role_share(role& role);
                pd::result role_check_change_icon(int icon, const role& role);
                pd::result role_check_change_custom_icon(const pd::custom_icon& custom_icon, const role& role);
                pd::result role_rank_give_like_check(const role& role, uint64_t gid, pd::rank_type type);
                pd::event_res role_rank_give_a_like(role& role);
                pd::result role_arena_give_like_check(const role& role, uint64_t gid);
        }

}
