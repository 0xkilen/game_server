#include "marriage_ptts.hpp"
#include "item_ptts.hpp"
#include "mail_ptts.hpp"
#include "battle_ptts.hpp"
#include "chat_ptts.hpp"
#include "huanzhuang_ptts.hpp"
#include "player_ptts.hpp"
#include "actor_ptts.hpp"
#include "quest_ptts.hpp"
#include "utils.hpp"
#include "utils/time_utils.hpp"

namespace nora {
        namespace config {

                marriage_logic_ptts& marriage_logic_ptts_instance() {
                        static marriage_logic_ptts inst;
                        return inst;
                }

                void marriage_logic_ptts_set_funcs() {
                        marriage_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.propose_conditions())) {
                                        CONFIG_ELOG << "check marriage propose conditions failed";
                                }
                                if (!check_conditions(ptt.star_wish_conditions())) {
                                        CONFIG_ELOG << "check marriage star wish conditions failed";
                                }
                                if (!check_events(ptt.star_wish_success_events())) {
                                        CONFIG_ELOG << "check star wish success events failed";
                                }
                                if (!check_events(ptt.question_win_events())) {
                                        CONFIG_ELOG << "check question win events failed";
                                }
                                if (!check_events(ptt.question_lose_events())) {
                                        CONFIG_ELOG << "check question lose events failed";
                                }
                                if (!check_events(ptt.question_finish_events())) {
                                        CONFIG_ELOG << "check question finish events failed";
                                }
                                for (const auto& i : ptt.divorces()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << "check marriage divorce conditions failed";
                                        }
                                }
                                if (!check_conditions(ptt.pk_conditions())) {
                                        CONFIG_ELOG << "check marriage pk conditions failed";
                                }
                                if (!check_conditions(ptt.pk_challenge_conditions())) {
                                        CONFIG_ELOG << "check marriage pk challenge conditions failed";
                                }
                                if (!check_events(ptt.pk_success_events())) {
                                        CONFIG_ELOG << "check pk success events failed";
                                }
                                if (!check_conditions(ptt.guesture_conditions())) {
                                        CONFIG_ELOG << "check marriage guesture conditions failed";
                                }
                                if (!check_conditions(ptt.guesture_reward_conditions())) {
                                        CONFIG_ELOG << "check marriage guesture reward conditions failed";
                                }
                                if (!check_events(ptt.guesture_round_events())) {
                                        CONFIG_ELOG << "check marriage guesture round events failed";
                                }
                                if (ptt.guesture_init_left_dis() < 0) {
                                        CONFIG_ELOG << "guesture init left dis lesser than 0";
                                }
                                if (ptt.guesture_init_right_dis() < 0) {
                                        CONFIG_ELOG << "guesture init right dis lesser than 0";
                                }
                                if (ptt.guesture_init_left_dis() > ptt.guesture_max_dis()) {
                                        CONFIG_ELOG << "guesture init left dis greater than max dis";
                                }
                                if (ptt.guesture_init_right_dis() > ptt.guesture_max_dis()) {
                                        CONFIG_ELOG << "guesture init right dis greater than max dis";
                                }
                                if (!check_conditions(ptt.qiu_qian_conditions())) {
                                        CONFIG_ELOG << "check marriage qiu qian conditions failed";
                                }
                                if (ptt.gift_minutes() < STAR_WISH_START_SYSTEM_CHAT_MINUTES) {
                                        CONFIG_ELOG << "check marriage gift minutes < start wish system_chat minutes";
                                }
                                if (ptt.pk_challenge_minutes() < PK_CHALLENGE_SYSTEM_CHAT_MINUTES) {
                                        CONFIG_ELOG << "check marriage pk challenge minutes < pk challenge system_chat minutes";
                                }
                                if (!check_conditions(ptt.finish_pregnency_conditions())) {
                                        CONFIG_ELOG << "check marriage finish pregnency conditions failed";
                                }
                        };
                        marriage_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.star_wish_conditions(), *ptt.mutable__star_wish_events());

                                auto *swp = ptt.mutable__star_wish_period();
                                *swp->mutable_begin() = ptt.star_wish_start_period().begin();
                                *swp->mutable_end() = min_after_time_point(ptt.star_wish_start_period().end(), ptt.gift_minutes() + ptt.select_minutes());

                                for (auto& i : *ptt.mutable_divorces()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable__events());
                                }

                                modify_events_by_conditions(ptt.pk_conditions(), *ptt.mutable__pk_events());

                                auto *pp = ptt.mutable__pk_period();
                                *pp->mutable_begin() = ptt.pk_start_period().begin();
                                *pp->mutable_end() = min_after_time_point(ptt.pk_start_period().end(), ptt.pk_challenge_minutes() + ptt.pk_select_minutes());

                                modify_events_by_conditions(ptt.pk_challenge_conditions(), *ptt.mutable_pk_challenge_events());

                                modify_events_by_conditions(ptt.guesture_reward_conditions(), *ptt.mutable__guesture_reward_events());

                                modify_events_by_conditions(ptt.qiu_qian_conditions(), *ptt.mutable__qiu_qian_events());

                                modify_events_by_conditions(ptt.finish_pregnency_conditions(), *ptt.mutable__finish_pregnency_events());
                        };
                        marriage_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.propose_conditions())) {
                                        CONFIG_ELOG << "verify marriage propose conditions failed";
                                }
                                for (const auto& i : ptt.star_wish_gifts()) {
                                        if (!PTTS_HAS(item, i.item())) {
                                                CONFIG_ELOG << ptt.id() << " star wish gift item not exist " << i.item();
                                        }
                                }
                                if (!verify_conditions(ptt.star_wish_conditions())) {
                                        CONFIG_ELOG << "check marriage star wish conditions failed";
                                }
                                if (!verify_events(ptt._star_wish_events())) {
                                        CONFIG_ELOG << "verify marriage star wish events failed";
                                }
                                if (!verify_events(ptt.star_wish_success_events())) {
                                        CONFIG_ELOG << "verify star wish success events failed";
                                }
                                if (!PTTS_HAS(system_chat, ptt.star_wish_success_system_chat())) {
                                        CONFIG_ELOG << "star wish success system chat not exist " << ptt.star_wish_success_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.star_wish_send_gift_system_chat())) {
                                        CONFIG_ELOG << "star wish send gift system chat not exist " << ptt.star_wish_send_gift_system_chat();
                                }
                                if (!PTTS_HAS(mail, ptt.star_wish_not_selected_mail())) {
                                        CONFIG_ELOG << "star wish not selected mail not exist " << ptt.star_wish_not_selected_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.star_wish_no_suitor_mail())) {
                                        CONFIG_ELOG << "star wish no suitor  mail not exist " << ptt.star_wish_no_suitor_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.star_wish_stopped_mail())) {
                                        CONFIG_ELOG << "star wish stopped mail not exist " << ptt.star_wish_stopped_mail();
                                }
                                if (!verify_conditions(ptt.pk_conditions())) {
                                        CONFIG_ELOG << "check marriage pk conditions failed";
                                }
                                if (!verify_events(ptt._pk_events())) {
                                        CONFIG_ELOG << "verify marriage pk events failed";
                                }
                                if (!verify_conditions(ptt.pk_challenge_conditions())) {
                                        CONFIG_ELOG << "check marriage pk challenge conditions failed";
                                }
                                if (!verify_events(ptt.pk_challenge_events())) {
                                        CONFIG_ELOG << "verify marriage pk challenge events failed";
                                }
                                if (!verify_events(ptt.pk_success_events())) {
                                        CONFIG_ELOG << "verify pk success events failed";
                                }
                                if (!PTTS_HAS(battle, ptt.pk_battle_pttid())) {
                                        CONFIG_ELOG << "pk battle pttid not exist " << ptt.pk_battle_pttid();
                                }
                                if (!PTTS_HAS(system_chat, ptt.pk_success_system_chat())) {
                                        CONFIG_ELOG << "pk success system chat not exist " << ptt.pk_success_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.star_wish_start_system_chat())) {
                                        CONFIG_ELOG << "star wish start system chat not exist " << ptt.star_wish_start_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.star_wish_forward_gift_system_chat())) {
                                        CONFIG_ELOG << "star wish forward gift system chat not exist " << ptt.star_wish_forward_gift_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.pk_start_challenge_system_chat())) {
                                        CONFIG_ELOG << "pk start challenge system chat not exist " << ptt.pk_start_challenge_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.pk_forward_challenge_system_chat())) {
                                        CONFIG_ELOG << "pk forward challenge system chat not exist " << ptt.pk_forward_challenge_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.answer_question_system_chat())) {
                                        CONFIG_ELOG << "answer question system chat not exist " << ptt.answer_question_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.guesture_system_chat())) {
                                        CONFIG_ELOG << "guesture system chat not exist " << ptt.guesture_system_chat();
                                }
                                if (!PTTS_HAS(mail, ptt.pk_no_fighter_mail())) {
                                        CONFIG_ELOG << "pk no fighter mail not exist " << ptt.pk_no_fighter_mail();
                                }
                                if (!verify_events(ptt.question_win_events())) {
                                        CONFIG_ELOG << "verify marriage question win events failed";
                                }
                                if (!verify_events(ptt.question_lose_events())) {
                                        CONFIG_ELOG << "verify question lose events failed";
                                }
                                if (!verify_events(ptt.question_finish_events())) {
                                        CONFIG_ELOG << "verify question finish events failed";
                                }
                                if (!verify_conditions(ptt.guesture_conditions())) {
                                        CONFIG_ELOG << "check marriage guesture conditions failed";
                                }
                                if (!verify_conditions(ptt.guesture_reward_conditions())) {
                                        CONFIG_ELOG << "check marriage guesture reward conditions failed";
                                }
                                if (!verify_events(ptt._guesture_reward_events())) {
                                        CONFIG_ELOG << "verify marriage guesture reward events failed";
                                }
                                if (!verify_events(ptt.guesture_round_events())) {
                                        CONFIG_ELOG << "verify marriage guesture round events failed";
                                }
                                for (const auto& i : ptt.divorces()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << "verify marriage divorce conditions failed";
                                        }
                                        if (!verify_events(i._events())) {
                                                CONFIG_ELOG << "verify marriage divorce events failed";
                                        }
                                }
                                if (!HUANZHUANG_PTTS_HAS(pd::HP_GUANGHUAN, ptt.default_guanghuan())) {
                                        CONFIG_ELOG << "default guanghuan not exist " << ptt.default_guanghuan();
                                }
                                const auto& create_role_ptt = PTTS_GET(create_role, 1);
                                auto found = false;
                                for (const auto& i : create_role_ptt.huanzhuang_collection()) {
                                        if (i.part() == pd::HP_GUANGHUAN && i.pttid() == ptt.default_guanghuan()) {
                                                found = true;
                                                break;
                                        }
                                }
                                if (!found) {
                                        CONFIG_ELOG << "create role no default guanghuan " << ptt.default_guanghuan();
                                }
                                if (!PTTS_HAS(mail, ptt.divorce_mail())) {
                                        CONFIG_ELOG << "divorce mail not exist " << ptt.divorce_mail();
                                }
                                if (!verify_conditions(ptt.qiu_qian_conditions())) {
                                        CONFIG_ELOG << "verify marriage qiu qian conditions failed";
                                }
                                if (!verify_events(ptt._qiu_qian_events())) {
                                        CONFIG_ELOG << "verify marriage qiu qian events failed";
                                }
                                if (!verify_conditions(ptt.finish_pregnency_conditions())) {
                                        CONFIG_ELOG << "verify marriage finish pregnency conditions failed";
                                }
                                if (!verify_events(ptt._finish_pregnency_events())) {
                                        CONFIG_ELOG << "verify marriage finish pregnency events failed";
                                }
                                for (auto i : ptt.enai_quest()) {
                                        if (!PTTS_HAS(quest, i)) {
                                              CONFIG_ELOG << "enai quest no such quest" << i;
                                        }
                                }
                        };
                }

                marriage_question_ptts& marriage_question_ptts_instance() {
                        static marriage_question_ptts inst;
                        return inst;
                }

                void marriage_question_ptts_set_funcs() {
                        marriage_question_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.first_options_size() != 4) {
                                        CONFIG_ELOG << ptt.id() << " need 4 first options got " << ptt.first_options_size();
                                }
                                if (ptt.second_options_size() != 4) {
                                        CONFIG_ELOG << ptt.id() << " need 4 second options got " << ptt.second_options_size();
                                }
                        };
                }

                marriage_guesture_ptts& marriage_guesture_ptts_instance() {
                        static marriage_guesture_ptts inst;
                        return inst;
                }

                void marriage_guesture_ptts_set_funcs() {
                }

                marriage_guesture_pair_ptts& marriage_guesture_pair_ptts_instance() {
                        static marriage_guesture_pair_ptts inst;
                        return inst;
                }

                void marriage_guesture_pair_ptts_set_funcs() {
                        marriage_guesture_pair_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.guestures_size() != 2) {
                                        CONFIG_ELOG << ptt.id() << " need 2 guestures, got " << ptt.guestures_size();
                                }
                                for (auto i : ptt.guestures()) {
                                        if (!PTTS_HAS(marriage_guesture, i)) {
                                                CONFIG_ELOG << ptt.id() << " guesture not exist " << i;
                                        }
                                }
                        };
                }

                marriage_child_qian_ptts& marriage_child_qian_ptts_instance() {
                        static marriage_child_qian_ptts inst;
                        return inst;
                }

                void marriage_child_qian_ptts_set_funcs() {
                        marriage_child_qian_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.actors_size() <= 0) {
                                        CONFIG_ELOG << ptt.id() << " no actors";
                                }
                                if (ptt.actors_size() != ptt.desc_size()) {
                                        CONFIG_ELOG << ptt.id() << " actors size and desc size not equal";
                                }
                        };
                        marriage_child_qian_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.actors()) {
                                        if (!PTTS_HAS(actor, i)) {
                                                CONFIG_ELOG << ptt.id() << " actor not exist " << i;
                                        }
                                }
                        };
                }

                marriage_enai_ptts& marriage_enai_ptts_instance() {
                        static marriage_enai_ptts inst;
                        return inst;
                }

                void marriage_enai_ptts_set_funcs() {
                        marriage_enai_ptts_instance().check_func_ = [] (const auto& ptt) {
                        };
                }
        }
}
