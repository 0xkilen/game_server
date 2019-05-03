#include "utils.hpp"
#include "fief_ptts.hpp"
#include "battle_ptts.hpp"
#include "mail_ptts.hpp"
#include "plot_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                namespace {

                        void check_fief_building_ptt(const pc::fief_building& ptt) {
                                for (const auto& i : ptt.produce_events()) {
                                        for (const auto& j : i.events()) {
                                                pd::fief_resource_type resource_type = pd::FRT_NONE;
                                                if (j.type() != pd::fief_event::ADD_RESOURCE) {
                                                        CONFIG_ELOG << ptt.level() << " produce events has type other than ADD_RESOURCE";
                                                }

                                                pd::fief_resource_type cur_type;
                                                pd::fief_resource_type_Parse(j.arg(0), &cur_type);
                                                if (resource_type == pd::FRT_NONE) {
                                                        resource_type = cur_type;
                                                } else if (resource_type != cur_type) {
                                                        CONFIG_ELOG << ptt.level() << " produce events add more than one kind of resource";
                                                }
                                        }
                                }
                        }

                        void modify_fief_building_ptt(pc::fief_building& ptt) {
                                modify_fief_events_by_fief_conditions(ptt.levelup_conditions(), *ptt.mutable__levelup_events());

                                if (ptt.craft_ranges_size() != 2) {
                                        CONFIG_ELOG << ptt.level() << " need 2 craft_ranges, got " << ptt.craft_ranges_size();
                                }

                                if (ptt.produce_events_size() != 3) {
                                        CONFIG_ELOG << ptt.level() << " need 3 produce_events, got " << ptt.produce_events_size();
                                }

                                if (ptt.drops_size() != 3) {
                                        CONFIG_ELOG << ptt.level() << " need 3 drops, got " << ptt.drops_size();
                                }

                        }

                }

                fief_ptts& fief_ptts_instance() {
                        static fief_ptts inst;
                        return inst;
                }

                void fief_ptts_set_funcs() {
                        fief_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.thief_count() <= 0 || ptt.thieves_size() <= 0) {
                                        CONFIG_ELOG << ptt.level() << " need thieves";
                                }
                        };
                        fief_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_events_by_fief_conditions(ptt.levelup_conditions(), *ptt.mutable__levelup_events());
                        };
                        fief_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                const auto& logic_ptt = PTTS_GET(fief_logic, 1);
                                if (static_cast<int>(ptt.thief_count()) > logic_ptt.thief_positions_size()) {
                                        CONFIG_ELOG << ptt.level() << " thief more than thief posisions";
                                }
                                for (const auto& i : ptt.thieves()) {
                                        if (!PTTS_HAS(fief_thief, i.thief_pttid())) {
                                                CONFIG_ELOG << ptt.level() << " no such thief " << i.thief_pttid();
                                        }
                                }
                                for (auto i : ptt.dawn_incidents()) {
                                        if (!PTTS_HAS(fief_incident, i)) {
                                                CONFIG_ELOG << ptt.level() << " dawn incident not exist " << i;
                                        }
                                }
                                for (auto i : ptt.day_incidents()) {
                                        if (!PTTS_HAS(fief_incident, i)) {
                                                CONFIG_ELOG << ptt.level() << " day incident not exist " << i;
                                        }
                                }
                                for (auto i : ptt.dust_incidents()) {
                                        if (!PTTS_HAS(fief_incident, i)) {
                                                CONFIG_ELOG << ptt.level() << " dust incident not exist " << i;
                                        }
                                }
                        };
                }

                fief_thief_ptts& fief_thief_ptts_instance() {
                        static fief_thief_ptts inst;
                        return inst;
                }

                void fief_thief_ptts_set_funcs() {
                }

                fief_boss_ptts& fief_boss_ptts_instance() {
                        static fief_boss_ptts inst;
                        return inst;
                }

                void fief_boss_ptts_set_funcs() {
                        fief_boss_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                auto need_size = 1;
                                for (const auto& i : ptt.battle_data()) {
                                        if (!PTTS_HAS(battle, i.battle_id())) {
                                                CONFIG_ELOG << ptt.id() << " battle not exist " << i.battle_id();
                                        }
                                        if (!PTTS_HAS(plot, i.leave_plot())) {
                                                CONFIG_ELOG << ptt.id() << " leave_plot not exist " << i.leave_plot();
                                        }
                                        if (!PTTS_HAS(plot, i.defeated_plot())) {
                                                CONFIG_ELOG << ptt.id() << " defeated_plot not exist " << i.defeated_plot();
                                        }
                                        if (!PTTS_HAS(mail, i.win_mail_id())) {
                                               CONFIG_ELOG << ptt.id() << " win_mail_id not exist " << i.win_mail_id();
                                        }
                                        if (!PTTS_HAS(mail, i.lost_mail_id())) {
                                               CONFIG_ELOG << ptt.id() << " lost_mail_id not exist " << i.lost_mail_id();
                                        }
                                        if (need_size != i.win_events_size()) {
                                                CONFIG_ELOG << ptt.id() << " win_events need_size " << need_size << " got size " << i.win_events_size();
                                        }
                                        ++need_size;
                                }
                                if (!PTTS_HAS(plot, ptt.boss_plot_id())) {
                                        CONFIG_ELOG << ptt.id() << " boss_plot_id " << ptt.boss_plot_id() << " not exist in plot_table";
                                }
                                if (!PTTS_HAS(fief, ptt.need_fief_level())) {
                                        CONFIG_ELOG << ptt.id() << " need fief level not exist " << ptt.need_fief_level();
                                }
                        };
                }

                fief_product_ptts& fief_product_ptts_instance() {
                        static fief_product_ptts inst;
                        return inst;
                }

                void fief_product_ptts_set_funcs() {
                }

                fief_hospital_ptts& fief_hospital_ptts_instance() {
                        static fief_hospital_ptts inst;
                        return inst;
                }

                void fief_hospital_ptts_set_funcs() {
                        fief_hospital_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_hospital_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_bar_ptts& fief_bar_ptts_instance() {
                        static fief_bar_ptts inst;
                        return inst;
                }

                void fief_bar_ptts_set_funcs() {
                        fief_bar_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_bar_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_cofc_ptts& fief_cofc_ptts_instance() {
                        static fief_cofc_ptts inst;
                        return inst;
                }

                void fief_cofc_ptts_set_funcs() {
                        fief_cofc_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_cofc_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_guard_ptts& fief_guard_ptts_instance() {
                        static fief_guard_ptts inst;
                        return inst;
                }

                void fief_guard_ptts_set_funcs() {
                        fief_guard_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_guard_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_cloth_ptts& fief_cloth_ptts_instance() {
                        static fief_cloth_ptts inst;
                        return inst;
                }

                void fief_cloth_ptts_set_funcs() {
                        fief_cloth_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_cloth_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_mine_ptts& fief_mine_ptts_instance() {
                        static fief_mine_ptts inst;
                        return inst;
                }

                void fief_mine_ptts_set_funcs() {
                        fief_mine_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_fief_building_ptt(ptt);
                        };
                        fief_mine_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_fief_building_ptt(ptt);
                        };
                }

                fief_incident_ptts& fief_incident_ptts_instance() {
                        static fief_incident_ptts inst;
                        return inst;
                }

                void fief_incident_ptts_set_funcs() {
                        fief_incident_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.has_minutes() && !ptt.has_expire_plot()) {
                                        CONFIG_ELOG << ptt.id() << " has minutes but no expire plot";
                                }
                        };
                }

                fief_logic_ptts& fief_logic_ptts_instance() {
                        static fief_logic_ptts inst;
                        return inst;
                }

                void fief_logic_ptts_set_funcs() {
                        fief_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                // if (ptt.boss().day_challenge_time() <= 5 || ptt.boss().day_challenge_time() >= 24) {
                                //         CONFIG_ELOG << ptt.id() << " boss challenge time " << ptt.boss().day_challenge_time() << " error";
                                // }
                        };

                        fief_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.join_area_incident_conditions(), *ptt.mutable__join_area_incident_events());
                                modify_events_by_conditions(ptt.challenge_boss_conditions(), *ptt.mutable__challenge_boss_events());
                        };

                        fief_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(mail, ptt.production_mail())) {
                                        CONFIG_ELOG << "production mail not exist " << ptt.production_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.incident_award_mail())) {
                                        CONFIG_ELOG << "incident award mail not exist " << ptt.incident_award_mail();
                                }
                                if (!PTTS_HAS(plot, ptt.open_plot())) {
                                        CONFIG_ELOG << "open plot not exist " << ptt.open_plot();
                                }
                                if (!PTTS_HAS(fief_incident, ptt.first_refresh_incident())) {
                                        CONFIG_ELOG << "first refresh incident not exist " << ptt.first_refresh_incident();
                                }
                        };

                }

                fief_area_incident_ptts& fief_area_incident_ptts_instance() {
                        static fief_area_incident_ptts inst;
                        return inst;
                }

                void fief_area_incident_ptts_set_funcs() {
                        fief_area_incident_ptts_instance().check_func_ = [] (const auto& ptt) {};
                        fief_area_incident_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(mail, ptt.host_award_mail())) {
                                        CONFIG_ELOG << ptt.id() << " host award mail not exit " << ptt.host_award_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.host_extra_award_mail())) {
                                        CONFIG_ELOG << ptt.id() << " host extra award mail not exit " << ptt.host_extra_award_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.guest_award_mail())) {
                                        CONFIG_ELOG << ptt.id() << " host award mail not exit " << ptt.guest_award_mail();
                                }
                        };
                }

                fief_area_incident_pool_ptts& fief_area_incident_pool_ptts_instance() {
                        static fief_area_incident_pool_ptts inst;
                        return inst;
                }

                void fief_area_incident_pool_ptts_set_funcs() {
                        fief_area_incident_pool_ptts_instance().check_func_ = [] (const auto& ptt) {};
                }

                fief_area_incident_reward_ptts& fief_area_incident_reward_ptts_instance() {
                        static fief_area_incident_reward_ptts inst;
                        return inst;
                }

                void fief_area_incident_reward_ptts_set_funcs() {
                        fief_area_incident_reward_ptts_instance().check_func_ = [] (const auto& ptt) {};
                }

                fief_bubble_ptts& fief_bubble_ptts_instance() {
                        static fief_bubble_ptts inst;
                        return inst;
                }

                void fief_bubble_ptts_set_funcs() {
                }

        }
}
