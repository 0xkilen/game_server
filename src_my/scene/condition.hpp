#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_condition.pb.h"
#include "proto/data_player.pb.h"
#include <set>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;

                void condition_merge(pd::condition_array& a, const pd::condition_array& b);
                pd::result condition_check(const pd::condition_array& ca, const pd::role_info& ri);
                pd::result condition_check(const pd::condition_array& ca, const role& role, const pd::ce_env& env = pd::ce_env());
                vector<pd::condition> condition_find(const pd::condition_array& ca, pd::condition::condition_type type);

        }
}
