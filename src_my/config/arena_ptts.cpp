#include "utils.hpp"
#include "arena_ptts.hpp"
#include "actor_ptts.hpp"
#include "spine_ptts.hpp"
#include "huanzhuang_ptts.hpp"
#include "battle_ptts.hpp"
#include "chat_ptts.hpp"
#include "mail_ptts.hpp"

namespace nora {
        namespace config {

                arena_npc_pool_ptts& arena_npc_pool_ptts_instance() {
                        static arena_npc_pool_ptts inst;
                        return inst;
                }

                void arena_npc_pool_ptts_set_funcs() {
                        arena_npc_pool_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.appearance_pools_size() != 2) {
                                        CONFIG_ELOG << ptt.rank() << " need 2 appearance pools, got " << ptt.appearance_pools_size();
                                }
                                for (const auto& i : ptt.appearance_pools()) {
                                        if (!PTTS_HAS(spine_pool, i.spine())) {
                                                CONFIG_ELOG << ptt.rank() << " not exist spine_pool " << i.spine();
                                        }
                                        if (!PTTS_HAS(huanzhuang_pool, i.huanzhuang())) {
                                                CONFIG_ELOG << ptt.rank() << " not exist huanzhuang_pool " << i.huanzhuang();
                                        }
                                }
                        };
                }

                arena_reward_ptts& arena_reward_ptts_instance() {
                        static arena_reward_ptts inst;
                        return inst;
                }

                void arena_reward_ptts_set_funcs() {
                        arena_reward_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.reward_events())) {
                                        CONFIG_ELOG << "check reward events failed";
                                }
                        };
                        arena_reward_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.reward_events())) {
                                        CONFIG_ELOG << "verify reward events failed";
                                }
                        };
                }

                arena_ptts& arena_ptts_instance() {
                        static arena_ptts inst;
                        return inst;
                }

                void arena_ptts_set_funcs() {
                        arena_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.win_events())) {
                                        CONFIG_ELOG << "check win events failed";
                                }
                                if (!check_events(ptt.lose_events())) {
                                        CONFIG_ELOG << "check lose events failed";
                                }
                                if (!check_conditions(ptt.challenge_conditions())) {
                                        CONFIG_ELOG << "check challenge conditions failed";
                                }
                                if (!check_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "check unlock conditions failed";
                                }
                        };
                        arena_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.challenge_conditions(), *(ptt.mutable__challenge_events()));
                        };
                        arena_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(battle, ptt.battle_pttid())) {
                                        CONFIG_ELOG << "battle not exist" << ptt.battle_pttid();
                                }
                                if (!verify_events(ptt.win_events())) {
                                        CONFIG_ELOG << "verify win events failed";
                                }
                                if (!verify_events(ptt.lose_events())) {
                                        CONFIG_ELOG << "verify lose events failed";
                                }
                                if (!verify_conditions(ptt.challenge_conditions())) {
                                        CONFIG_ELOG << "verify challenge conditions failed";
                                }
                                if (!verify_events(ptt._challenge_events())) {
                                        CONFIG_ELOG << "verify challenge events failed";
                                }
                                if (!verify_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "verify unlock conditions failed";
                                }
                                if (!PTTS_HAS(system_chat, ptt.settle_champion_system_chat())) {
                                        CONFIG_ELOG << "settle_champion_system_chat not exist" << ptt.settle_champion_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.challenge_champion_system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " challenge_champion_system_chat not exist " << ptt.challenge_champion_system_chat();
                                }
                                if (!PTTS_HAS(mail, ptt.settle_mail())) {
                                        CONFIG_ELOG << ptt.id() << " settle mail not exist " << ptt.settle_mail();
                                }
                        };
                }

                public_arena_group_ptts& public_arena_group_ptts_instance() {
                        static public_arena_group_ptts inst;
                        return inst;
                }

                void public_arena_group_ptts_set_funcs() {
                }

        }
}
