#include "utils.hpp"
#include "huanzhuang_ptts.hpp"
#include "plot_ptts.hpp"
#include "rank_ptts.hpp"
#include "mail_ptts.hpp"
#include "chat_ptts.hpp"
#include "growth_ptts.hpp"
#include "origin_ptts.hpp"
#include "item_ptts.hpp"

namespace nora {
        namespace config {

                void verify_huanzhuang(const pc::huanzhuang_item& ptt) {
                        if (ptt.has_fix_attr()) {
                                if (!PTTS_HAS(not_portional_growth, ptt.fix_attr().levelup_growth())) {
                                        CONFIG_ELOG << ptt.id() << " levelup growth not exist " << ptt.fix_attr().levelup_growth();
                                }
                        }

                        if (!PTTS_HAS(huanzhuang_levelup, ptt.levelup_pttid())) {
                                CONFIG_ELOG << ptt.id() << " levelup pttid not exist " << ptt.levelup_pttid();
                        }
                        for (auto i : ptt.origins()) {
                                if (!PTTS_HAS(origin, i)) {
                                        CONFIG_ELOG << ptt.id() << " origin not exist " << i;
                                }
                        }
                }

                yifu_ptts& yifu_ptts_instance() {
                        static yifu_ptts inst;
                        return inst;
                }

                void yifu_ptts_set_funcs() {
                        yifu_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                toufa_ptts& toufa_ptts_instance() {
                        static toufa_ptts inst;
                        return inst;
                }

                void toufa_ptts_set_funcs() {
                        toufa_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                jiezhi_ptts& jiezhi_ptts_instance() {
                        static jiezhi_ptts inst;
                        return inst;
                }

                void jiezhi_ptts_set_funcs() {
                        jiezhi_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                edai_ptts& edai_ptts_instance() {
                        static edai_ptts inst;
                        return inst;
                }

                void edai_ptts_set_funcs() {
                        edai_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                fazan_ptts& fazan_ptts_instance() {
                        static fazan_ptts inst;
                        return inst;
                }

                void fazan_ptts_set_funcs() {
                        fazan_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                guanshi_ptts& guanshi_ptts_instance() {
                        static guanshi_ptts inst;
                        return inst;
                }

                void guanshi_ptts_set_funcs() {
                        guanshi_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                toujin_ptts& toujin_ptts_instance() {
                        static toujin_ptts inst;
                        return inst;
                }

                void toujin_ptts_set_funcs() {
                        toujin_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                erhuan_ptts& erhuan_ptts_instance() {
                        static erhuan_ptts inst;
                        return inst;
                }

                void erhuan_ptts_set_funcs() {
                        erhuan_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                beishi_ptts& beishi_ptts_instance() {
                        static beishi_ptts inst;
                        return inst;
                }

                void beishi_ptts_set_funcs() {
                        beishi_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                guanghuan_ptts& guanghuan_ptts_instance() {
                        static guanghuan_ptts inst;
                        return inst;
                }

                void guanghuan_ptts_set_funcs() {
                        guanghuan_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                shouchi_ptts& shouchi_ptts_instance() {
                        static shouchi_ptts inst;
                        return inst;
                }

                void shouchi_ptts_set_funcs() {
                        shouchi_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                xianglian_ptts& xianglian_ptts_instance() {
                        static xianglian_ptts inst;
                        return inst;
                }

                void xianglian_ptts_set_funcs() {
                        xianglian_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                huanzhuang_deform_face_ptts& huanzhuang_deform_face_ptts_instance() {
                        static huanzhuang_deform_face_ptts inst;
                        return inst;
                }

                void huanzhuang_deform_face_ptts_set_funcs() {
                        huanzhuang_deform_face_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                huanzhuang_deform_head_ptts& huanzhuang_deform_head_ptts_instance() {
                        static huanzhuang_deform_head_ptts inst;
                        return inst;
                }

                void huanzhuang_deform_head_ptts_set_funcs() {
                        huanzhuang_deform_head_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_huanzhuang(ptt);
                        };
                }

                huanzhuang_taozhuang_ptts& huanzhuang_taozhuang_ptts_instance() {
                        static huanzhuang_taozhuang_ptts inst;
                        return inst;
                }

                void huanzhuang_taozhuang_ptts_set_funcs() {
                        huanzhuang_taozhuang_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.reward_events())) {
                                        CONFIG_ELOG << "check reward events failed";
                                }
                        };
                        huanzhuang_taozhuang_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.reward_events())) {
                                        CONFIG_ELOG << "verify reward events failed";
                                }
                                for (const auto& i : ptt.item()) {
                                        if (!HUANZHUANG_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " " << pd::huanzhuang_part_Name(i.part()) << " not exist " << i.pttid();
                                                return;
                                        }
                                        for (const auto& j : ptt.item()) {
                                                auto& huanzhuang_ptt = HUANZHUANG_PTTS_GET(j.part(), j.pttid());
                                                if (huanzhuang_ptt.has__taozhuang() && huanzhuang_ptt._taozhuang() != ptt.id()) {
                                                        CONFIG_ELOG << ptt.id() << " " << pd::huanzhuang_part_Name(i.part()) << " " << i.pttid() << " also belong to " << huanzhuang_ptt._taozhuang();
                                                }
                                                huanzhuang_ptt.set__taozhuang(ptt.id());
                                        }
                                }
                        };
                }

                huanzhuang_tujian_ptts& huanzhuang_tujian_ptts_instance() {
                        static huanzhuang_tujian_ptts inst;
                        return inst;
                }

                void huanzhuang_tujian_ptts_set_funcs() {
                        huanzhuang_tujian_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.taozhuang()) {
                                        if (!PTTS_HAS(huanzhuang_taozhuang, i)) {
                                                CONFIG_ELOG << ptt.id() << " taozhuang " << i  << " not exist";
                                        }
                                }
                        };
                }

                huanzhuang_pool_ptts& huanzhuang_pool_ptts_instance() {
                        static huanzhuang_pool_ptts inst;
                        return inst;
                }

                void huanzhuang_pool_ptts_set_funcs() {
                        huanzhuang_pool_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i = 0; i < ptt.npc_huanzhuangs_size(); ++i) {
                                        const auto& huanzhuangs = ptt.npc_huanzhuangs(i);
                                        for (auto j = 0; j < huanzhuangs.selection_size(); ++j) {
                                                const auto& parts = huanzhuangs.selection(j);
                                                if (!HUANZHUANG_PTTS_HAS(parts.part(), parts.pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " " << pd::huanzhuang_part_Name(parts.part()) << " not exist " << parts.pttid();
                                                }
                                        }
                                }
                        };
                }

                huanzhuang_ptts& huanzhuang_ptts_instance() {
                        static huanzhuang_ptts inst;
                        return inst;
                }

                void huanzhuang_ptts_set_funcs() {
                        huanzhuang_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.end_plot_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " need 3 end plot, got " << ptt.end_plot_size();
                                }
                        };
                        huanzhuang_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_open_plot() && !PTTS_HAS(plot, ptt.open_plot())) {
                                        CONFIG_ELOG << ptt.id() << " open plot not exist " << ptt.open_plot();
                                }
                                for (auto i : ptt.end_plot()) {
                                        if (i > 0 && !PTTS_HAS(plot, i)) {
                                                CONFIG_ELOG << ptt.id() << " end plot not exist " << i;
                                        }
                                }
                        };
                }

                huanzhuang_tag_ptts& huanzhuang_tag_ptts_instance() {
                        static huanzhuang_tag_ptts inst;
                        return inst;
                }

                void huanzhuang_tag_ptts_set_funcs() {
                }

                huanzhuang_pvp_ptts& huanzhuang_pvp_ptts_instance() {
                        static huanzhuang_pvp_ptts inst;
                        return inst;
                }

                void huanzhuang_pvp_ptts_set_funcs() {
                        huanzhuang_pvp_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(huanzhuang, ptt.huanzhuang())) {
                                        CONFIG_ELOG << ptt.id() << " huanzhuang not exist " << ptt.huanzhuang();
                                }

                                if (!PTTS_HAS(rank_reward, ptt.rank_reward())) {
                                        CONFIG_ELOG << ptt.id() << " rank reward not exist " << ptt.rank_reward();
                                }
                        };
                }

                huanzhuang_pvp_round_ptts& huanzhuang_pvp_round_ptts_instance() {
                        static huanzhuang_pvp_round_ptts inst;
                        return inst;
                }

                void huanzhuang_pvp_round_ptts_set_funcs() {
                        huanzhuang_pvp_round_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(huanzhuang_pvp, ptt.pttid())) {
                                        CONFIG_ELOG << ptt.time() << " huanzhuang pvp not exist " << ptt.pttid();
                                }
                        };
                }

                huanzhuang_compose_ptts& huanzhuang_compose_ptts_instance() {
                        static huanzhuang_compose_ptts inst;
                        return inst;
                }

                void huanzhuang_compose_ptts_set_funcs() {
                        huanzhuang_compose_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << "check conditions failed";
                                }
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << "check events failed";
                                }
                        };
                        huanzhuang_compose_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable_events());
                        };
                        huanzhuang_compose_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << "verify conditions failed";
                                }
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << "verify events failed";
                                }
                        };
                }

                huanzhuang_logic_ptts& huanzhuang_logic_ptts_instance() {
                        static huanzhuang_logic_ptts inst;
                        return inst;
                }

                void huanzhuang_logic_ptts_set_funcs() {
                        huanzhuang_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.pvp_conditions())) {
                                        CONFIG_ELOG << "check pvp conditions failed";
                                }
                                if (!check_conditions(ptt.pvp_vote_female_conditions())) {
                                        CONFIG_ELOG << "check pvp vote female conditions failed";
                                }
                                if (!check_conditions(ptt.pvp_vote_male_conditions())) {
                                        CONFIG_ELOG << "check pvp vote male conditions failed";
                                }
                                if (!check_conditions(ptt.pvp_upvote_conditions())) {
                                        CONFIG_ELOG << "check pvp upvote conditions failed";
                                }
                                if (ptt.pvp_personal_level_percent_size() != 2) {
                                        CONFIG_ELOG << "need 2 pvp personal level percent, got " << ptt.pvp_personal_level_percent_size();
                                }
                                if (ptt.pvp_personal_reward_level_factors_size() != 3) {
                                        CONFIG_ELOG << "need 3 pvp personal reward level factors, got " << ptt.pvp_personal_reward_level_factors_size();
                                }
                                if (ptt.pvp_personal_reward_quality_factors_size() != 5) {
                                        CONFIG_ELOG << "need 5 pvp personal reward quality factors, got " << ptt.pvp_personal_reward_quality_factors_size();
                                }
                                if (!check_conditions(ptt.pvp_guess_vote_conditions())) {
                                        CONFIG_ELOG << "check pvp guess vote conditions failed";
                                }
                                if (!check_events(ptt.pvp_guess_vote_reward())) {
                                        CONFIG_ELOG << "check pvp guess vote reward events failed";
                                }
                                if (!check_events(ptt.pvp_guess_vote_miss_reward())) {
                                        CONFIG_ELOG << "check pvp guess vote miss reward events failed";
                                }
                                if (!check_conditions(ptt.inherit_conditions())) {
                                        CONFIG_ELOG << "check inherit conditions failed";
                                }
                                if (!check_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "unlock conditions failed";
                                }
                        };
                        huanzhuang_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.pvp_vote_female_conditions(), *ptt.mutable_pvp_vote_female_events());
                                modify_events_by_conditions(ptt.pvp_vote_male_conditions(), *ptt.mutable_pvp_vote_male_events());
                                modify_events_by_conditions(ptt.pvp_upvote_conditions(), *ptt.mutable_pvp_upvote_events());
                                modify_events_by_conditions(ptt.pvp_guess_vote_conditions(), *ptt.mutable__pvp_guess_vote_events());
                        };
                        huanzhuang_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.pvp_conditions())) {
                                        CONFIG_ELOG << "verify pvp conditions failed";
                                }
                                if (!verify_conditions(ptt.pvp_vote_female_conditions())) {
                                        CONFIG_ELOG << "verify pvp vote female conditions failed";
                                }
                                if (!verify_conditions(ptt.pvp_vote_male_conditions())) {
                                        CONFIG_ELOG << "verify pvp male vote conditions failed";
                                }
                                if (!verify_conditions(ptt.pvp_upvote_conditions())) {
                                        CONFIG_ELOG << "verify pvp upvote conditions failed";
                                }
                                if (!verify_events(ptt.pvp_vote_female_events())) {
                                        CONFIG_ELOG << "verify pvp vote female events failed";
                                }
                                if (!verify_events(ptt.pvp_vote_male_events())) {
                                        CONFIG_ELOG << "verify pvp vote male events failed";
                                }
                                if (!verify_events(ptt.pvp_upvote_events())) {
                                        CONFIG_ELOG << "verify pvp upvote events failed";
                                }
                                if (!PTTS_HAS(system_chat, ptt.pvp_start_vote_system_chat())) {
                                        CONFIG_ELOG << "pvp start vote system chat not exist " << ptt.pvp_start_vote_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.pvp_end_vote_system_chat())) {
                                        CONFIG_ELOG << "pvp end vote system chat not exist " << ptt.pvp_end_vote_system_chat();
                                }
                                if (!PTTS_HAS(mail, ptt.pvp_settle_mail())) {
                                        CONFIG_ELOG << "pvp settle mail not exist " << ptt.pvp_settle_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.pvp_personal_reward_mail())) {
                                        CONFIG_ELOG << "pvp personal rewrd mail not exist " << ptt.pvp_personal_reward_mail();
                                }
                                if (!PTTS_HAS(plot, ptt.pvp_open_plot())) {
                                        CONFIG_ELOG << "pvp open plot not exist " << ptt.pvp_open_plot();
                                }
                                if (!verify_conditions(ptt.pvp_guess_vote_conditions())) {
                                        CONFIG_ELOG << "verify pvp guess vote conditions failed";
                                }
                                if (!verify_events(ptt._pvp_guess_vote_events())) {
                                        CONFIG_ELOG << "verify pvp guess vote events failed";
                                }
                                if (!verify_events(ptt.pvp_guess_vote_reward())) {
                                        CONFIG_ELOG << "verify pvp guess vote events failed";
                                }
                                if (!verify_events(ptt.pvp_guess_vote_miss_reward())) {
                                        CONFIG_ELOG << "verify pvp guess vote miss events failed";
                                }
                                for (const auto& i : ptt.levelup_items()) {
                                        if (!PTTS_HAS(item, i.item())) {
                                                CONFIG_ELOG << "levelup item not exist " << i.item();
                                        }
                                }
                                if (!verify_conditions(ptt.inherit_conditions())) {
                                        CONFIG_ELOG << "verify inherit conditions failed";
                                }
                                if (!PTTS_HAS(item, ptt.inherit_item())) {
                                        CONFIG_ELOG << "inherit item not exist";
                                }
                                if (!verify_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "unlock conditions failed";
                                }
                        };
                }

                huanzhuang_levelup_ptts& huanzhuang_levelup_ptts_instance() {
                        static huanzhuang_levelup_ptts inst;
                        return inst;
                }

                void huanzhuang_levelup_ptts_set_funcs() {
                        huanzhuang_levelup_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_condevents()->mutable_condevents()) {
                                        auto *events = i.mutable__events();
                                        auto *e = events->add_events();
                                        e->set_type(pd::event::HUANZHUANG_LEVELUP);

                                        modify_events_by_conditions(i.conditions(), *events);
                                }
                        };
                }

        }
}
