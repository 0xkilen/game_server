#pragma once

#include "utils/ptts.hpp"
#include "recharge_ptts.hpp"
#include "proto/config_activity.pb.h"
#include "proto/config_recharge.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using activity_login_days_ptts = ptts<pc::activity_login_days>;
                activity_login_days_ptts& activity_login_days_ptts_instance();
                void activity_login_days_ptts_set_funcs();

                using activity_online_time_ptts = ptts<pc::activity_online_time>;
                activity_online_time_ptts& activity_online_time_ptts_instance();
                void activity_online_time_ptts_set_funcs();

                using activity_seven_days_ptts = ptts<pc::activity_seven_days>;
                activity_seven_days_ptts& activity_seven_days_ptts_instance();
                void activity_seven_days_ptts_set_funcs();

                using activity_qiandao_ptts = ptts<pc::activity_qiandao>;
                activity_qiandao_ptts& activity_qiandao_ptts_instance();
                void activity_qiandao_ptts_set_funcs();

                using activity_chaozhi_libao_ptts = ptts<pc::activity_chaozhi_libao>;
                activity_chaozhi_libao_ptts& activity_chaozhi_libao_ptts_instance();
                void activity_chaozhi_libao_ptts_set_funcs();

                using activity_everyday_libao_ptts = ptts<pc::activity_everyday_libao>;
                activity_everyday_libao_ptts& activity_everyday_libao_ptts_instance();
                void activity_everyday_libao_ptts_set_funcs();

                using activity_first_recharge_ptts = ptts<pc::activity_first_recharge>;
                activity_first_recharge_ptts& activity_first_recharge_ptts_instance();
                void activity_first_recharge_ptts_set_funcs();

                using activity_vip_qianzhuang_ptts = ptts<pc::activity_vip_qianzhuang>;
                activity_vip_qianzhuang_ptts& activity_vip_qianzhuang_ptts_instance();
                void activity_vip_qianzhuang_ptts_set_funcs();

                using activity_vip_qianzhuang_logic_ptts = ptts<pc::activity_vip_qianzhuang_logic>;
                activity_vip_qianzhuang_logic_ptts& activity_vip_qianzhuang_logic_ptts_instance();
                void activity_vip_qianzhuang_logic_ptts_set_funcs();

                using activity_emperor_qianzhuang_ptts = ptts<pc::activity_emperor_qianzhuang>;
                activity_emperor_qianzhuang_ptts& activity_emperor_qianzhuang_ptts_instance();
                void activity_emperor_qianzhuang_ptts_set_funcs();

                using activity_emperor_qianzhuang_logic_ptts = ptts<pc::activity_emperor_qianzhuang_logic>;
                activity_emperor_qianzhuang_logic_ptts& activity_emperor_qianzhuang_logic_ptts_instance();
                void activity_emperor_qianzhuang_logic_ptts_set_funcs();

                using activity_fund_ptts = ptts<pc::activity_fund>;
                activity_fund_ptts& activity_fund_ptts_instance();
                void activity_fund_ptts_set_funcs();

                using activity_limit_libao_ptts = ptts<pc::activity_limit_libao>;
                activity_limit_libao_ptts& activity_limit_libao_ptts_instance();
                void activity_limit_libao_ptts_set_funcs();

                using activity_festival_ptts = ptts<pc::activity_festival>;
                activity_festival_ptts& activity_festival_ptts_instance();
                void activity_festival_ptts_set_funcs();

                using activity_seventh_day_rank_award_ptts = ptts<pc::activity_seventh_day_rank_award>;
                activity_seventh_day_rank_award_ptts& activity_seventh_day_rank_award_ptts_instance();
                void activity_seventh_day_rank_award_ptts_set_funcs();

                using activity_prize_wheel_ptts = ptts<pc::activity_prize_wheel>;
                activity_prize_wheel_ptts& activity_prize_wheel_ptts_instance();
                void activity_prize_wheel_ptts_set_funcs();

                using activity_discount_goods_ptts = ptts<pc::activity_discount_goods>;
                activity_discount_goods_ptts& activity_discount_goods_ptts_instance();
                void activity_discount_goods_ptts_set_funcs();

                using activity_shop_recharge_ptts = ptts<pc::activity_shop_recharge>;
                activity_shop_recharge_ptts& activity_shop_recharge_ptts_instance();
                void activity_shop_recharge_ptts_set_funcs();

                using activity_continue_recharge_ptts = ptts<pc::activity_continue_recharge>;
                activity_continue_recharge_ptts& activity_continue_recharge_ptts_instance();
                void activity_continue_recharge_ptts_set_funcs();

                using activity_daiyanren_ptts = ptts<pc::activity_daiyanren>;
                activity_daiyanren_ptts& activity_daiyanren_ptts_instance();
                void activity_daiyanren_ptts_set_funcs();

                using activity_limit_play_ptts = ptts<pc::activity_limit_play>;
                activity_limit_play_ptts& activity_limit_play_ptts_instance();
                void activity_limit_play_ptts_set_funcs();

                using activity_leiji_recharge_ptts = ptts<pc::activity_leiji_recharge>;
                activity_leiji_recharge_ptts& activity_leiji_recharge_ptts_instance();
                void activity_leiji_recharge_ptts_set_funcs();

                using activity_leiji_consume_ptts = ptts<pc::activity_leiji_consume>;
                activity_leiji_consume_ptts& activity_leiji_consume_ptts_instance();
                void activity_leiji_consume_ptts_set_funcs();

                using activity_tea_party_ptts = ptts<pc::activity_tea_party>;
                activity_tea_party_ptts& activity_tea_party_ptts_instance();
                void activity_tea_party_ptts_set_funcs();

                using activity_logic_ptts = ptts<pc::activity_logic>;
                activity_logic_ptts& activity_logic_ptts_instance();
                void activity_logic_ptts_set_funcs();
        }
}
