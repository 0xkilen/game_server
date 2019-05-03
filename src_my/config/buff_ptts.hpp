#pragma once

#include "utils/ptts.hpp"
#include "proto/config_buff.pb.h"
#include "utils.hpp"
#include <string>

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using buff_ptts = ptts<pc::buff>;
                buff_ptts& buff_ptts_instance();
                void buff_ptts_set_funcs();
                void check_buff_change_value_type(string arg);
                void check_buff_effect_condition_size(const pc::buff& ptt, const pc::buff_effect& bf);
                void check_buff_effect_effect_size(const pc::buff& ptt, const pc::buff_effect& bf);
                void verify_buff_effect_effect(const pc::buff& ptt, const pc::buff_effect& bf);
                void verify_buff_actor_effect_effect(const pc::buff& ptt, const pc::buff_effect& bf);

        }
}
