#pragma once

#include "utils/ptts.hpp"
#include "proto/config_huanzhuang.pb.h"

using namespace std;
namespace pc = proto::config;
namespace pd = proto::data;

namespace nora {
        namespace config {

                using toufa_ptts = ptts<pc::huanzhuang_item>;
                toufa_ptts& toufa_ptts_instance();
                void toufa_ptts_set_funcs();

                using jiezhi_ptts = ptts<pc::huanzhuang_item>;
                jiezhi_ptts& jiezhi_ptts_instance();
                void jiezhi_ptts_set_funcs();

                using edai_ptts = ptts<pc::huanzhuang_item>;
                edai_ptts& edai_ptts_instance();
                void edai_ptts_set_funcs();

                using fazan_ptts = ptts<pc::huanzhuang_item>;
                fazan_ptts& fazan_ptts_instance();
                void fazan_ptts_set_funcs();

                using guanshi_ptts = ptts<pc::huanzhuang_item>;
                guanshi_ptts& guanshi_ptts_instance();
                void guanshi_ptts_set_funcs();

                using toujin_ptts = ptts<pc::huanzhuang_item>;
                toujin_ptts& toujin_ptts_instance();
                void toujin_ptts_set_funcs();

                using erhuan_ptts = ptts<pc::huanzhuang_item>;
                erhuan_ptts& erhuan_ptts_instance();
                void erhuan_ptts_set_funcs();

                using xianglian_ptts = ptts<pc::huanzhuang_item>;
                xianglian_ptts& xianglian_ptts_instance();
                void xianglian_ptts_set_funcs();

                using beishi_ptts = ptts<pc::huanzhuang_item>;
                beishi_ptts& beishi_ptts_instance();
                void beishi_ptts_set_funcs();

                using guanghuan_ptts = ptts<pc::huanzhuang_item>;
                guanghuan_ptts& guanghuan_ptts_instance();
                void guanghuan_ptts_set_funcs();

                using shouchi_ptts = ptts<pc::huanzhuang_item>;
                shouchi_ptts& shouchi_ptts_instance();
                void shouchi_ptts_set_funcs();

                using yifu_ptts = ptts<pc::huanzhuang_item>;
                yifu_ptts& yifu_ptts_instance();
                void yifu_ptts_set_funcs();

                using huanzhuang_deform_face_ptts = ptts<pc::huanzhuang_item>;
                huanzhuang_deform_face_ptts& huanzhuang_deform_face_ptts_instance();
                void huanzhuang_deform_face_ptts_set_funcs();

                using huanzhuang_deform_head_ptts = ptts<pc::huanzhuang_item>;
                huanzhuang_deform_head_ptts& huanzhuang_deform_head_ptts_instance();
                void huanzhuang_deform_head_ptts_set_funcs();

                using huanzhuang_taozhuang_ptts = ptts<pc::huanzhuang_taozhuang>;
                huanzhuang_taozhuang_ptts& huanzhuang_taozhuang_ptts_instance();
                void huanzhuang_taozhuang_ptts_set_funcs();

                using huanzhuang_tujian_ptts = ptts<pc::huanzhuang_tujian>;
                huanzhuang_tujian_ptts& huanzhuang_tujian_ptts_instance();
                void huanzhuang_tujian_ptts_set_funcs();

                using huanzhuang_pool_ptts = ptts<pc::huanzhuang_pool>;
                huanzhuang_pool_ptts& huanzhuang_pool_ptts_instance();
                void huanzhuang_pool_ptts_set_funcs();

                using huanzhuang_ptts = ptts<pc::huanzhuang>;
                huanzhuang_ptts& huanzhuang_ptts_instance();
                void huanzhuang_ptts_set_funcs();

                using huanzhuang_tag_ptts = ptts<pc::huanzhuang_tag>;
                huanzhuang_tag_ptts& huanzhuang_tag_ptts_instance();
                void huanzhuang_tag_ptts_set_funcs();

                using huanzhuang_pvp_ptts = ptts<pc::huanzhuang_pvp>;
                huanzhuang_pvp_ptts& huanzhuang_pvp_ptts_instance();
                void huanzhuang_pvp_ptts_set_funcs();

                using huanzhuang_pvp_round_ptts = ptts<pc::huanzhuang_pvp_round>;
                huanzhuang_pvp_round_ptts& huanzhuang_pvp_round_ptts_instance();
                void huanzhuang_pvp_round_ptts_set_funcs();

                using huanzhuang_logic_ptts = ptts<pc::huanzhuang_logic>;
                huanzhuang_logic_ptts& huanzhuang_logic_ptts_instance();
                void huanzhuang_logic_ptts_set_funcs();

                using huanzhuang_compose_ptts = ptts<pc::huanzhuang_compose>;
                huanzhuang_compose_ptts& huanzhuang_compose_ptts_instance();
                void huanzhuang_compose_ptts_set_funcs();

                using huanzhuang_levelup_ptts = ptts<pc::huanzhuang_levelup>;
                huanzhuang_levelup_ptts& huanzhuang_levelup_ptts_instance();
                void huanzhuang_levelup_ptts_set_funcs();

        }
}
