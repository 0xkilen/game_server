#include "utils.hpp"
#include "adventure_ptts.hpp"
#include "battle_ptts.hpp"
#include "plot_ptts.hpp"
#include "drop_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                adventure_ptts& adventure_ptts_instance() {
                        static adventure_ptts inst;
                        return inst;
                }

                void adventure_ptts_set_funcs() {
                        adventure_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check unlock conditions failed";
                                }
                                if (!check_conditions(ptt.challenge_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check challenge conditions failed";
                                }
                                if (!check_conditions(ptt.sweep_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check sweep conditions failed";
                                }
                                if (!check_events(ptt.first_win_events())) {
                                        CONFIG_ELOG << ptt.id() << " check first_win_events failed";
                                }
                                if (!check_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.id() << " check win_events failed";
                                }
                        };
                        adventure_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *we = ptt.mutable_win_events();
                                auto *event = we->add_events();
                                event->set_type(pd::event::PASS_ADVENTURE);
                                event->add_arg(to_string(ptt.id()));

                                if (ptt.has_challenge_conditions()) {
                                        modify_events_by_conditions(ptt.challenge_conditions(), *we);
                                }
                        };
                        adventure_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!ptt.has_huanzhuang_pttid()) {
                                        if (ptt.has_battle_pttid()) {
                                                if (!PTTS_HAS(battle, ptt.battle_pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " battle_pttid not exist " << ptt.battle_pttid();
                                                }
                                                if (!PTTS_HAS(drop, ptt.battle_drop_pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " battle drop pttid not exist " << ptt.battle_drop_pttid();
                                                }
                                        } else if (ptt.has_plot_pttid()) {
                                                if (!PTTS_HAS(plot, ptt.plot_pttid())) {
                                                        CONFIG_ELOG << ptt.id() << " plot_pttid not exist " << ptt.plot_pttid();
                                                } else {
                                                        auto& plot_ptt = PTTS_GET(plot, ptt.plot_pttid());
                                                        plot_ptt.set__need_start(true);
                                                }
                                        } else {
                                                CONFIG_ELOG << ptt.id() << " no battle_pttid and plot_pttid";
                                        }
                                }
                                if (!verify_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify unlock conditions failed";
                                }
                                if (!verify_conditions(ptt.challenge_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify challenge conditions failed";
                                }
                                if (!verify_conditions(ptt.sweep_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify sweep conditions failed";
                                }
                                if (!verify_events(ptt.first_win_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify first win events failed";
                                }
                                if (!verify_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify win events failed";
                                }
                        };
                }

                adventure_episode_ptts& adventure_episode_ptts_instance() {
                        static adventure_episode_ptts inst;
                        return inst;
                }

                void adventure_episode_ptts_set_funcs() {
                        adventure_episode_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_red()) {
                                        if (!PTTS_HAS(adventure_episode, ptt.red())) {
                                                CONFIG_ELOG << ptt.id() << " red not exist " << ptt.red();
                                        }
                                }
                                if (ptt.has_blue()) {
                                        if (!PTTS_HAS(adventure_episode, ptt.blue())) {
                                                CONFIG_ELOG << ptt.id() << " blue not exist " << ptt.blue();
                                        }
                                }

                                for (auto i : ptt.chapters()) {
                                        if (!PTTS_HAS(adventure_chapter, i)) {
                                                CONFIG_ELOG << ptt.id() << " chapter not exist " << i;
                                                continue;
                                        }
                                        const auto& adventure_chapter_ptt = PTTS_GET(adventure_chapter, i);
                                        for (auto j : adventure_chapter_ptt.adventures()) {
                                                auto& adventure_ptt = PTTS_GET(adventure, j);
                                                adventure_ptt.set__episode_pttid(ptt.id());
                                                adventure_ptt.set__type(ptt.type());
                                        }
                                }
                        };
                }

                adventure_chapter_ptts& adventure_chapter_ptts_instance() {
                        static adventure_chapter_ptts inst;
                        return inst;
                }

                void adventure_chapter_ptts_set_funcs() {
                        adventure_chapter_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " check finish events failed";
                                }
                        };
                        adventure_chapter_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.adventures()) {
                                        if (!PTTS_HAS(adventure, i)) {
                                                CONFIG_ELOG << ptt.id() << " adventure not exist " << i;
                                                continue;
                                        }
                                        auto& adventure_ptt = PTTS_GET(adventure, i);
                                        adventure_ptt.set__chapter_pttid(ptt.id());
                                }
                                if (!verify_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify finish events failed";
                                }
                        };
                }

                adventure_logic_ptts& adventure_logic_ptts_instance() {
                        static adventure_logic_ptts inst;
                        return inst;
                }

                void adventure_logic_ptts_set_funcs() {
                        adventure_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.liezhuan_open_conditions())) {
                                        CONFIG_ELOG << " check liezhuan open conditions failed";
                                }
                        };
                        adventure_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.liezhuan_open_conditions())) {
                                        CONFIG_ELOG << " verify liezhuan open conditions failed";
                                }
                        };
                }
        }
}
