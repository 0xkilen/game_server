#pragma once

#include "proto/data_base.pb.h"
#include "proto/config_recharge.pb.h"
#include <vector>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                class role;
                pd::result recharge_check(uint64_t order, bool check_price, uint32_t price, const role& role);
                pd::result check_recharge_day(uint32_t pttid, const role& role);
                pd::event_res recharge_process(uint64_t order, uint32_t price, role& role);
                pd::result recharge_check_get_vip_box(uint32_t vip_level, const role& role);
                pd::event_res recharge_get_vip_box(uint32_t vip_level, role& role);
                pd::result recharge_check_get_vip_buy_box(uint32_t vip_level, const role& role);
                pd::event_res recharge_get_vip_buy_box(uint32_t vip_level, role& role);
                uint32_t vip_exp_to_level(uint32_t exp);
        }
}
