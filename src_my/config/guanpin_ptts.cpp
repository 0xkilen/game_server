#include "utils.hpp"
#include "guanpin_ptts.hpp"
#include "battle_ptts.hpp"
#include "chat_ptts.hpp"

namespace nora {
        namespace config {

                guanpin_ptts& guanpin_ptts_instance() {
                        static guanpin_ptts inst;
                        return inst;
                }

                void guanpin_ptts_set_funcs() {
                        guanpin_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.promote_conditions())) {
                                        CONFIG_ELOG << ptt.gpin() << " check promote conditions failed";
                                }
                                if (!check_events(ptt.fenglu_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " check fenglu events failed";
                                }
                                if (!check_events(ptt.challenge().win_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " check challenge events failed";
                                }
                                if (!check_events(ptt.challenge().lose_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " check challenge events failed";
                                }
                                if (ptt.has_dianshi()) {
                                        if (!ptt.has_max_guan()) {
                                                CONFIG_ELOG << ptt.gpin() << " has dianshi but no max guan";
                                        }
                                        const auto& dianshi = ptt.dianshi();
                                        if (ptt.max_guan() < dianshi.max_luqu()) {
                                                CONFIG_ELOG << ptt.gpin() << " max guan less than dianshi max luqu";
                                        }
                                        if (!check_conditions(dianshi.baoming_conditions())) {
                                                CONFIG_ELOG << ptt.gpin() << " check dianshi baoming conditions failed";
                                        }
                                }
                        };
                        guanpin_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.promote_conditions(), *ptt.mutable__promote_events());
                                if (ptt.has_dianshi()) {
                                        auto& dianshi = *ptt.mutable_dianshi();
                                        modify_events_by_conditions(dianshi.baoming_conditions(), *dianshi.mutable__baoming_events());
                                }
                                if (ptt.has_dianshi()) {
                                        auto& dianshi = *ptt.mutable_dianshi();
                                        dianshi.set__max_jiangzhi(dianshi.max_luqu());
                                }
                        };
                        guanpin_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.promote_conditions())) {
                                        CONFIG_ELOG << ptt.gpin() << " verify promote conditions failed";
                                }
                                if (!verify_events(ptt._promote_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " verify promote events failed";
                                }
                                if (!verify_events(ptt.fenglu_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " verify fenglu events failed";
                                }
                                if (!verify_events(ptt.challenge().win_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " verify challenge events failed";
                                }
                                if (!verify_events(ptt.challenge().lose_events())) {
                                        CONFIG_ELOG << ptt.gpin() << " verify challenge events failed";
                                }
                                if (ptt.has_dianshi()) {
                                        const auto& dianshi = ptt.dianshi();
                                        if (!verify_events(dianshi._baoming_events())) {
                                                CONFIG_ELOG << ptt.gpin() << " verify dianshi baoming events failed";
                                        }
                                }
                        };
                }

                guanpin_logic_ptts& guanpin_logic_ptts_instance() {
                        static guanpin_logic_ptts inst;
                        return inst;
                }

                void guanpin_logic_ptts_set_funcs() {
                        guanpin_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.dianshi().interval() == 0) {
                                        CONFIG_ELOG << " invalid dianshi interval " << ptt.dianshi().interval();
                                }
                        };
                        guanpin_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.challenge().conditions(), *ptt.mutable_challenge()->mutable__events());
                        };
                        guanpin_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(battle, ptt.challenge().battle())) {
                                        CONFIG_ELOG << "challenge battle not exist";
                                }
                                if (!PTTS_HAS(battle, ptt.dianshi().battle())) {
                                        CONFIG_ELOG << "dianshi battle not exist";
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().start())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().start();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().before_end())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().before_end();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().end())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().end();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().one_round())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().one_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().two_round())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().two_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().three_round())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().three_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().four_round())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().four_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.dianshi_system_chat().settle())) {
                                        CONFIG_ELOG << "dianshi system chat not exist " << ptt.dianshi_system_chat().settle();
                                }

                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().start())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().start();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().before_end())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().before_end();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().end())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().end();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().one_round())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().one_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().two_round())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().two_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().three_round())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().three_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().four_round())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().four_round();
                                }
                                if (!PTTS_HAS(system_chat, ptt.revolt_system_chat().settle())) {
                                        CONFIG_ELOG << "revolt system chat not exist " << ptt.revolt_system_chat().settle();
                                }
                                if (!PTTS_HAS(system_chat, ptt.login_system_chat())) {
                                        CONFIG_ELOG << "login system chat not exist " << ptt.login_system_chat();
                                }
                        };
                }

                guanpin_fenghao_ptts& guanpin_fenghao_ptts_instance() {
                        static guanpin_fenghao_ptts inst;
                        return inst;
                }

                void guanpin_fenghao_ptts_set_funcs() {
                }

        }
}
