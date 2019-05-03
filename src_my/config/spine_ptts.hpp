#pragma once

#include "utils/ptts.hpp"
#include "proto/config_spine.pb.h"

using namespace std;
namespace pc = proto::config;
namespace pd = proto::data;

namespace nora {
        namespace config {

                using spine_limit_ptts = ptts<pc::spine_limit>;
                spine_limit_ptts& spine_limit_ptts_instance();
                void spine_limit_ptts_set_funcs();

                using hongmo_ptts = ptts<pc::spine_item>;
                hongmo_ptts& hongmo_ptts_instance();
                void hongmo_ptts_set_funcs();

                using yanjing_ptts = ptts<pc::spine_item>;
                yanjing_ptts& yanjing_ptts_instance();
                void yanjing_ptts_set_funcs();

                using yanshenguang_ptts = ptts<pc::spine_item>;
                yanshenguang_ptts& yanshenguang_ptts_instance();
                void yanshenguang_ptts_set_funcs();

                using saihong_ptts = ptts<pc::spine_item>;
                saihong_ptts& saihong_ptts_instance();
                void saihong_ptts_set_funcs();

                using meimao_ptts = ptts<pc::spine_item>;
                meimao_ptts& meimao_ptts_instance();
                void meimao_ptts_set_funcs();

                using yanzhuang_ptts = ptts<pc::spine_item>;
                yanzhuang_ptts& yanzhuang_ptts_instance();
                void yanzhuang_ptts_set_funcs();

                using chuncai_ptts = ptts<pc::spine_item>;
                chuncai_ptts& chuncai_ptts_instance();
                void chuncai_ptts_set_funcs();

                using zui_ptts = ptts<pc::spine_item>;
                zui_ptts& zui_ptts_instance();
                void zui_ptts_set_funcs();

                using huzi_ptts = ptts<pc::spine_item>;
                huzi_ptts& huzi_ptts_instance();
                void huzi_ptts_set_funcs();

                using lian_ptts = ptts<pc::spine_item>;
                lian_ptts& lian_ptts_instance();
                void lian_ptts_set_funcs();

                using tiehua_ptts = ptts<pc::spine_item>;
                tiehua_ptts& tiehua_ptts_instance();
                void tiehua_ptts_set_funcs();

                using lianxing_ptts = ptts<pc::spine_item>;
                lianxing_ptts& lianxing_ptts_instance();
                void lianxing_ptts_set_funcs();

                using spine_deform_face_ptts = ptts<pc::spine_item>;
                spine_deform_face_ptts& spine_deform_face_ptts_instance();
                void spine_deform_face_ptts_set_funcs();

                using spine_deform_head_ptts = ptts<pc::spine_item>;
                spine_deform_head_ptts& spine_deform_head_ptts_instance();
                void spine_deform_head_ptts_set_funcs();

                using spine_pool_ptts = ptts<pc::spine_pool>;
                spine_pool_ptts& spine_pool_ptts_instance();
                void spine_pool_ptts_set_funcs();

                using spine_model_ptts = ptts<pc::spine_model>;
                spine_model_ptts& spine_model_ptts_instance();
                void spine_model_ptts_set_funcs();

        }
}
