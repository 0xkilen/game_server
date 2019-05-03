#include "utils.hpp"
#include "spine_ptts.hpp"

namespace nora {
        namespace config {

                spine_limit_ptts& spine_limit_ptts_instance() {
                        static spine_limit_ptts inst;
                        return inst;
                }

                void spine_limit_ptts_set_funcs() {
                }

                hongmo_ptts& hongmo_ptts_instance() {
                        static hongmo_ptts inst;
                        return inst;
                }

                void hongmo_ptts_set_funcs() {
                }

                yanjing_ptts& yanjing_ptts_instance() {
                        static yanjing_ptts inst;
                        return inst;
                }

                void yanjing_ptts_set_funcs() {
                }

                yanshenguang_ptts& yanshenguang_ptts_instance() {
                        static yanshenguang_ptts inst;
                        return inst;
                }

                void yanshenguang_ptts_set_funcs() {
                }

                saihong_ptts& saihong_ptts_instance() {
                        static saihong_ptts inst;
                        return inst;
                }

                void saihong_ptts_set_funcs() {
                }

                meimao_ptts& meimao_ptts_instance() {
                        static meimao_ptts inst;
                        return inst;
                }

                void meimao_ptts_set_funcs() {
                }

                yanzhuang_ptts& yanzhuang_ptts_instance() {
                        static yanzhuang_ptts inst;
                        return inst;
                }

                void yanzhuang_ptts_set_funcs() {
                }

                chuncai_ptts& chuncai_ptts_instance() {
                        static chuncai_ptts inst;
                        return inst;
                }

                void chuncai_ptts_set_funcs() {
                }

                zui_ptts& zui_ptts_instance() {
                        static zui_ptts inst;
                        return inst;
                }

                void zui_ptts_set_funcs() {
                }

                huzi_ptts& huzi_ptts_instance() {
                        static huzi_ptts inst;
                        return inst;
                }

                void huzi_ptts_set_funcs() {
                }

                lian_ptts& lian_ptts_instance() {
                        static lian_ptts inst;
                        return inst;
                }

                void lian_ptts_set_funcs() {
                }

                tiehua_ptts& tiehua_ptts_instance() {
                        static tiehua_ptts inst;
                        return inst;
                }

                void tiehua_ptts_set_funcs() {
                }

                lianxing_ptts& lianxing_ptts_instance() {
                        static lianxing_ptts inst;
                        return inst;
                }

                void lianxing_ptts_set_funcs() {
                }

                spine_deform_face_ptts& spine_deform_face_ptts_instance() {
                        static spine_deform_face_ptts inst;
                        return inst;
                }

                void spine_deform_face_ptts_set_funcs() {
                }

                spine_deform_head_ptts& spine_deform_head_ptts_instance() {
                        static spine_deform_head_ptts inst;
                        return inst;
                }

                void spine_deform_head_ptts_set_funcs() {
                }

                spine_pool_ptts& spine_pool_ptts_instance() {
                        static spine_pool_ptts inst;
                        return inst;
                }

                void spine_pool_ptts_set_funcs() {
                        spine_pool_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i = 0; i < ptt.npc_spines_size(); ++i) {
                                        const auto& spines = ptt.npc_spines(i);
                                        for (const auto& j : spines.selection()) {
                                                if (!SPINE_PTTS_HAS(j.part(),j.pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " spine not exist " << pd::spine_part_Name(j.part()) << " " << j.pttid();
                                                }
                                        }
                                }
                        };
                }

                spine_model_ptts& spine_model_ptts_instance() {
                        static spine_model_ptts inst;
                        return inst;
                }

                void spine_model_ptts_set_funcs() {
                        spine_model_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.selection()) {
                                        if (!SPINE_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " " << pd::spine_part_Name(i.part()) << " not exist " << i.pttid();
                                        }
                                }
                        };
                }

        }
}
