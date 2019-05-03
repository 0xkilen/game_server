#include "battle_ptts.hpp"
#include "utils.hpp"
#include "utils/log.hpp"
#include "config/actor_ptts.hpp"
#include "config/behavior_tree_ptts.hpp"
#include "config/plot_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                battle_ptts& battle_ptts_instance() {
                        static battle_ptts inst;
                        return inst;
                }

                void battle_ptts_set_funcs() {
                        battle_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.scene().empty()) {
                                        CONFIG_ELOG << "battle config " << ptt.id() << " scene is empty" ;
                                }
                                if (!ptt.has_type()) {
                                        CONFIG_ELOG << "battle config " << ptt.id() << " type is empty" ;
                                }
                                if (ptt.has_monsters()) {
                                        for (const auto& i : ptt.monsters().actors()) {
                                                for (const auto& j : ptt.monsters().actors()) {
                                                        if (&i != &j && i.row() == j.row() && i.col() == j.col()) {
                                                                CONFIG_ELOG << "battle monster grid can't not same to other monster  actor id" << i.actor();
                                                        }
                                                }
                                        }
                                }
                                if (ptt.has_heros()) {
                                        for (const auto& i : ptt.heros().actors()) {
                                                for (const auto& j : ptt.heros().actors()) {
                                                        if (&i != &j && i.row() == j.row() && i.col() == j.col()) {
                                                                CONFIG_ELOG << "battle monster grid can't not same to other monster  actor id" << i.actor();
                                                        }
                                                }
                                        }
                                }
                                if (ptt.win_conditions_size() == 0) {
                                        CONFIG_ELOG << "battle config " << ptt.id() << " win_condition is error" ;
                                } else {
                                        for (const auto& i : ptt.win_conditions()) {
                                                for (const auto& j : i.conditions()) {
                                                        switch (j.type()) {
                                                                case pc::RIGHT_GRIDS_ALL_DEAD:
                                                                case pc::LEFT_GRIDS_ALL_DEAD:
                                                                case pc::LIGHT_GRID_TIMES_MAX:
                                                                    break;
                                                                case pc::ROUND_OVER:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition ROUND_OVER arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::KILL_RIGHT_GRIDS_ALL_TIMES:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition KILL_RIHGT_GRIDS_ALL_TIMES arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::LEFT_GRIDS_ACTOR_DEAD:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition LEFT_GRIDS_ACTOR_DEAD arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::RIGHT_GRIDS_ACTOR_DEAD:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition RIGHT_GRIDS_ACTOR_DEAD arg is error" ;
                                                                        }
                                                                    break;
                                                                default:
                                                                    break;
                                                        }
                                                }
                                        }
                                }
                                if (ptt.lose_conditions_size() == 0) {
                                        CONFIG_ELOG << "battle config " << ptt.id() << " lose_condition is error" ;
                                } else {
                                        for (const auto& i : ptt.win_conditions()) {
                                                for (const auto& j : i.conditions()) {
                                                        switch (j.type()) {
                                                                case pc::RIGHT_GRIDS_ALL_DEAD:
                                                                case pc::LEFT_GRIDS_ALL_DEAD:
                                                                case pc::LIGHT_GRID_TIMES_MAX:
                                                                    break;
                                                                case pc::ROUND_OVER:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " lose_condition ROUND_OVER arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::KILL_RIGHT_GRIDS_ALL_TIMES:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " lose_condition KILL_RIHGT_GRIDS_ALL_TIMES arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::LEFT_GRIDS_ACTOR_DEAD:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition LEFT_GRIDS_ACTOR_DEAD arg is error" ;
                                                                        }
                                                                    break;
                                                                case pc::RIGHT_GRIDS_ACTOR_DEAD:
                                                                        if (j.arg_size() != 1) {
                                                                                CONFIG_ELOG << "battle config " << ptt.id() << " win_condition RIGHT_GRIDS_ACTOR_DEAD arg is error" ;
                                                                        }
                                                                    break;
                                                                default:
                                                                    break;
                                                        }
                                                }
                                        }
                                }
                                if (ptt.has_begin_battle_plot()) {
                                        if(!PTTS_HAS(plot, ptt.begin_battle_plot())) {
                                                CONFIG_ELOG << "battle config " << ptt.id() << " plot:" << ptt.begin_battle_plot() << " is not exist" ;
                                        }
                                }

                                if (ptt.has_start_battle_plot()) {
                                        if(!PTTS_HAS(plot, ptt.begin_battle_plot())) {
                                                CONFIG_ELOG << "battle config " << ptt.id() << " plot:" << ptt.start_battle_plot() << " is not exist" ;
                                        }
                                }

                                if (ptt.has_end_battle_win_plot()) {
                                        if(!PTTS_HAS(plot, ptt.begin_battle_plot())) {
                                                CONFIG_ELOG << "battle config " << ptt.id() << " plot:" << ptt.end_battle_win_plot() << " is not exist" ;
                                        }
                                }

                                if (ptt.has_end_battle_lose_plot()) {
                                        if(!PTTS_HAS(plot, ptt.end_battle_lose_plot())) {
                                                CONFIG_ELOG << "battle config " << ptt.id() << " plot:" << ptt.end_battle_lose_plot() << " is not exist" ;
                                        }
                                }
                        };
                        battle_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.has_heros()) {
                                        for (auto& i : *ptt.mutable_heros()->mutable_actors()) {
                                                if (!i.has_boss()) {
                                                        i.set_boss(false);
                                                }
                                        }
                                }
                                for (auto& i : *ptt.mutable_monsters()->mutable_actors()) {
                                        if (!i.has_boss()) {
                                                i.set_boss(false);
                                        }
                                }
                        };
                        battle_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_monsters()) {
                                        for (const auto& i : ptt.monsters().actors()) {
                                                if (!PTTS_HAS(actor, i.actor())) {
                                                        CONFIG_ELOG << ptt.id() << " battle monsters actor " << i.actor() << " not exist in actor table";
                                                }
                                        }
                                }

                                if (ptt.has_heros()) {
                                        for (const auto& i : ptt.heros().actors()) {
                                                if (!PTTS_HAS(actor, i.actor())) {
                                                        CONFIG_ELOG << ptt.id() << " battle heros actor " << i.actor() << " not exist in actor table";
                                                }
                                        }
                                }
                        };
                }

                formation_pool_ptts& formation_pool_ptts_instance() {
                        static formation_pool_ptts inst;
                        return inst;
                }

                void formation_pool_ptts_set_funcs() {
                        formation_pool_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.grids_size() > 5) {
                                        CONFIG_ELOG <<" too many grids " << ptt.grids_size();
                                }
                                for (const auto& i : ptt.grids()) {
                                        if (i.grid().row() > 3 || i.grid().row() <= 0) {
                                                CONFIG_ELOG <<" invalid grid row " << i.grid().row();
                                        }
                                        if (i.grid().col() > 3 || i.grid().col() <= 0) {
                                                CONFIG_ELOG <<" invalid grid col " << i.grid().col();
                                        }

                                }
                        };
                        formation_pool_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                auto has_zhu = false;
                                for (const auto& i : ptt.grids()) {
                                        if (!PTTS_HAS(actor, i.actor())) {
                                                CONFIG_ELOG << ptt.id() << " actor not exist" << i.actor();
                                        }
                                        const auto& ptt = PTTS_GET(actor, i.actor());
                                        if (ptt.type() == pd::actor::ZHU) {
                                                if (has_zhu) {
                                                        CONFIG_ELOG << ptt.id() << " has more than 1 zhu";
                                                } else {
                                                        has_zhu = true;
                                                }
                                        }
                                }
                        };
                }

                battle_ai_pool_ptts& battle_ai_pool_ptts_instance() {
                        static battle_ai_pool_ptts inst;
                        return inst;
                }

                void battle_ai_pool_ptts_set_funcs() {
                        battle_ai_pool_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(behavior_tree, ptt.adventure())) {
                                        CONFIG_ELOG << ptt.adventure() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.arena())) {
                                        CONFIG_ELOG << ptt.arena() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.fief())) {
                                        CONFIG_ELOG << ptt.fief() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.lieming())) {
                                        CONFIG_ELOG << ptt.lieming() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.league())) {
                                        CONFIG_ELOG << ptt.league() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.mansion())) {
                                        CONFIG_ELOG << ptt.mansion() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.tower())) {
                                        CONFIG_ELOG << ptt.tower() << " not exist in behavior_tree";
                                }

                                if (!PTTS_HAS(behavior_tree, ptt.guanpin())) {
                                        CONFIG_ELOG << ptt.guanpin() << " not exist in behavior_tree";
                                }
                        };
                }
        }
}
