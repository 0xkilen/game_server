#include "utils.hpp"
#include "activity_ptts.hpp"
#include "actor_ptts.hpp"
#include "rank_ptts.hpp"
#include "lottery_ptts.hpp"
#include "item_ptts.hpp"
#include "quest_ptts.hpp"
#include "recharge_ptts.hpp"
#include "shop_ptts.hpp"
#include "utils/time_utils.hpp"
#include "mail_ptts.hpp"
#include "origin_ptts.hpp"

namespace nora {
        namespace config {

                activity_login_days_ptts& activity_login_days_ptts_instance() {
                        static activity_login_days_ptts inst;
                        return inst;
                }

                void activity_login_days_ptts_set_funcs() {
                        activity_login_days_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.day() << " check events failed";
                                }
                        };
                        activity_login_days_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.day() << " verify events failed";
                                }
                        };
                }

                activity_online_time_ptts& activity_online_time_ptts_instance() {
                        static activity_online_time_ptts inst;
                        return inst;
                }

                void activity_online_time_ptts_set_funcs() {
                        activity_online_time_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.rewards()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.day() << " check reward events failed";
                                        }
                                }
                        };
                        activity_online_time_ptts_instance().modify_func_ = [] (auto& ptt) {
                                uint32_t minutes = 0;
                                for (auto& i : *ptt.mutable_rewards()) {
                                        minutes += i.minutes();
                                        i.set_minutes(minutes);
                                }
                        };
                        activity_online_time_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.rewards()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.day() << " verify reward events failed";
                                        }
                                }
                        };
                }

                activity_seven_days_ptts& activity_seven_days_ptts_instance() {
                        static activity_seven_days_ptts inst;
                        return inst;
                }

                void activity_seven_days_ptts_set_funcs() {
                        activity_seven_days_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.fuli_size() != 7) {
                                        CONFIG_ELOG << ptt.begin_day() << " need 7 fuli, got " << ptt.fuli_size();
                                }
                                if (ptt.quests_size() != 7) {
                                        CONFIG_ELOG << ptt.begin_day() << " need 7 quests, got " << ptt.quests_size();
                                }
                                if (ptt.goods_size() != 7) {
                                        CONFIG_ELOG << ptt.begin_day() << " need 7 goods, got " << ptt.goods_size();
                                }
                        };
                        activity_seven_days_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.reward().conditions(), *ptt.mutable_reward()->mutable_male_events());
                                modify_events_by_conditions(ptt.reward().conditions(), *ptt.mutable_reward()->mutable_female_events());
                        };
                        activity_seven_days_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i = 0; i < ptt.goods_size(); ++i) {
                                        const auto& good = ptt.goods(i);
                                        if (!PTTS_HAS(shop, good.shop())) {
                                                CONFIG_ELOG << ptt.begin_day() << " shop not exist " << good.shop();
                                        }
                                        auto& shop_ptt = PTTS_GET(shop, good.shop());
                                        auto found = false;
                                        for (auto j = 0; j < good.shop_goods_size(); ++j) {
                                                auto shop_good = good.shop_goods(j);
                                                for (auto& k : *shop_ptt.mutable_goods()) {
                                                        if (k.id() == shop_good) {
                                                                k.set__activity_seven_days_begin_day(ptt.begin_day());
                                                                k.set__activity_seven_days_day(i);
                                                                found = true;
                                                                break;
                                                        }
                                                }
                                                if (!found) {
                                                        CONFIG_ELOG << ptt.begin_day() << " shop good not exist " << good.shop() << " " << shop_good;
                                                }
                                        }
                                }
                                for (const auto& i : ptt.fuli()) {
                                        for (auto j : i.quests()) {
                                                if (!PTTS_HAS(quest, j)) {
                                                        CONFIG_ELOG << ptt.begin_day() << " fuli quest not exist " << j;
                                                        continue;
                                                }
                                                auto& quest_ptt = PTTS_GET(quest, j);
                                                quest_ptt.set__activity_seven_days_fuli_quest(true);
                                        }
                                }
                                for (const auto& i : ptt.quests()) {
                                        for (auto j : i.quests()) {
                                                if (!PTTS_HAS(quest, j)) {
                                                        CONFIG_ELOG << ptt.begin_day() << " quest not exist " << j;
                                                        continue;
                                                }
                                                auto& quest_ptt = PTTS_GET(quest, j);
                                                quest_ptt.set__activity_seven_days_quest(true);
                                        }
                                }

                        };
                }

                activity_everyday_libao_ptts& activity_everyday_libao_ptts_instance() {
                        static activity_everyday_libao_ptts inst;
                        return inst;
                }

                void activity_everyday_libao_ptts_set_funcs() {
                        activity_everyday_libao_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(recharge, ptt.recharge())) {
                                        CONFIG_ELOG << ptt.id() << " verify everyday libao invalid recharge id " << ptt.recharge();
                                        return;
                                }
                                auto& recharge_ptt = PTTS_GET(recharge, ptt.recharge());
                                recharge_ptt.set__activity_everyday_libao(ptt.id());
                        };
                }

                activity_qiandao_ptts& activity_qiandao_ptts_instance() {
                        static activity_qiandao_ptts inst;
                        return inst;
                }
                void activity_qiandao_ptts_set_funcs() {
                        activity_qiandao_ptts_instance().check_func_ = [] (const auto& ptt) {
                                set<int> big_month { 1, 3, 5, 7, 8, 10, 12 };
                                set<int> little_month { 4, 6, 9, 11 };
                                if (big_month.count(ptt.month()) > 0) {
                                        if (ptt.rewards_size() != 31) {
                                                CONFIG_ELOG << ptt.month() << " rewards size error, need 31, got " << ptt.rewards_size();
                                        }
                                } else if (little_month.count(ptt.month()) > 0) {
                                        if (ptt.rewards_size() != 30) {
                                                CONFIG_ELOG << ptt.month() << " rewards size error, need 30, got " << ptt.rewards_size();
                                        }
                                } else {
                                        if (ptt.month() == 2) {
                                                if (ptt.rewards_size() != 29) {
                                                        CONFIG_ELOG << ptt.month() << " rewards size error, need 29, got " << ptt.rewards_size();
                                                }
                                        } else {
                                                CONFIG_ELOG << ptt.month() << " error month";
                                        }
                                }

                                for (const auto& i : ptt.rewards()) {
                                        if (!check_condevents(i.qiandao_events())) {
                                                CONFIG_ELOG << ptt.month() << " check qiandao events failed";
                                        }

                                        if (!check_conditions(i.buqian_conditions())) {
                                                CONFIG_ELOG << ptt.month() << " check buqian conditions failed";
                                        }

                                        if (!check_events(i.leiqian_events())) {
                                                CONFIG_ELOG << ptt.month() << " check leiji events failed";
                                        }
                                }
                        };

                        activity_qiandao_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_rewards()) {
                                        auto *events = i.mutable__buqian_events();
                                        modify_events_by_conditions(i.buqian_conditions(), *events);
                                }
                        };

                        activity_qiandao_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.rewards()) {
                                        if (!verify_condevents(i.qiandao_events())) {
                                                CONFIG_ELOG << ptt.month() << " verify qiandao events failed";
                                        }

                                        if (!verify_conditions(i.buqian_conditions())) {
                                                CONFIG_ELOG << ptt.month() << " verify buqian conditions failed";
                                        }

                                        if (!verify_events(i.leiqian_events())) {
                                                CONFIG_ELOG << ptt.month() << " verify leiji events failed";
                                        }
                                }
                        };
                }

                activity_chaozhi_libao_ptts& activity_chaozhi_libao_ptts_instance() {
                        static activity_chaozhi_libao_ptts inst;
                        return inst;
                }
                void activity_chaozhi_libao_ptts_set_funcs() {
                        activity_chaozhi_libao_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.resource_recharges()) {
                                        if (!PTTS_HAS(recharge, i)) {
                                                CONFIG_ELOG << ptt.id() << " resource_recharges not exist " << i;
                                        }
                                        auto& recharge_ptt = PTTS_GET(recharge, i);
                                        recharge_ptt.set__activity_chaozhi_resource_libao(ptt.id());
                                }

                                for (auto i : ptt.huanzhuang_recharges()) {
                                        if (!PTTS_HAS(recharge, i)) {
                                                CONFIG_ELOG << ptt.id() << " fashion_recharges not exist " << i;
                                        }
                                        auto& recharge_ptt = PTTS_GET(recharge, i);
                                        recharge_ptt.set__activity_chaozhi_huanzhuang_libao(ptt.id());
                                }
                        };
                }

                activity_first_recharge_ptts& activity_first_recharge_ptts_instance() {
                        static activity_first_recharge_ptts inst;
                        return inst;
                }
                void activity_first_recharge_ptts_set_funcs() {
                }

                activity_emperor_qianzhuang_ptts& activity_emperor_qianzhuang_ptts_instance() {
                        static activity_emperor_qianzhuang_ptts inst;
                        return inst;
                }
                void activity_emperor_qianzhuang_ptts_set_funcs() {
                }

                activity_emperor_qianzhuang_logic_ptts& activity_emperor_qianzhuang_logic_ptts_instance() {
                        static activity_emperor_qianzhuang_logic_ptts inst;
                        return inst;
                }
                void activity_emperor_qianzhuang_logic_ptts_set_funcs() {
                }

                activity_vip_qianzhuang_ptts& activity_vip_qianzhuang_ptts_instance() {
                        static activity_vip_qianzhuang_ptts inst;
                        return inst;
                }
                void activity_vip_qianzhuang_ptts_set_funcs() {
                }

                activity_vip_qianzhuang_logic_ptts& activity_vip_qianzhuang_logic_ptts_instance() {
                        static activity_vip_qianzhuang_logic_ptts inst;
                        return inst;
                }
                void activity_vip_qianzhuang_logic_ptts_set_funcs() {
                }

                activity_fund_ptts& activity_fund_ptts_instance() {
                        static activity_fund_ptts inst;
                        return inst;
                }
                void activity_fund_ptts_set_funcs() {
                }

                activity_limit_libao_ptts& activity_limit_libao_ptts_instance() {
                        static activity_limit_libao_ptts inst;
                        return inst;
                }
                void activity_limit_libao_ptts_set_funcs() {
                        activity_limit_libao_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check limit libao conditions failed";
                                }
                        };
                        activity_limit_libao_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(recharge, ptt.recharge())) {
                                        CONFIG_ELOG << ptt.id() << " verify limit libao invalid recharge id " << ptt.recharge();
                                        return;
                                }
                                auto& recharge_ptt = PTTS_GET(recharge, ptt.recharge());
                                recharge_ptt.set__activity_limit_libao(ptt.id());
                        };
                }

                activity_festival_ptts& activity_festival_ptts_instance() {
                        static activity_festival_ptts inst;
                        return inst;
                }
                void activity_festival_ptts_set_funcs() {
                        activity_festival_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.exchange_size() > 0) {
                                        for (const auto& i : ptt.exchange()) {
                                                if (!check_conditions(i.conditions())) {
                                                        CONFIG_ELOG << ptt.id() << " check festival commons conditions failed";
                                                }
                                        }
                                }
                        };
                        activity_festival_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.exchange_size() > 0) {
                                        for (auto& i : *ptt.mutable_exchange()) {
                                                modify_events_by_conditions(i.conditions(), *i.mutable_events());
                                        }
                                }
                        };
                        activity_festival_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(shop, ptt.shop())) {
                                        CONFIG_ELOG << ptt.id() << " no such shop " << ptt.shop();
                                } else {
                                        auto& shop_ptt = PTTS_GET(shop, ptt.shop());
                                        shop_ptt.set__activity_festival(ptt.id());
                                }
                                for (auto i : ptt.recharge()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << ptt.id() << " no such quest " << i;
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i);
                                                quest_ptt.set__activity_festival_recharge_quest(true);
                                        }
                                }
                                for (auto i : ptt.fuli()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << ptt.id() << " no such quest " << i;
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i);
                                                quest_ptt.set__activity_festival_fuli_quest(true);
                                        }
                                }
                        };
                }

                activity_seventh_day_rank_award_ptts& activity_seventh_day_rank_award_ptts_instance() {
                        static activity_seventh_day_rank_award_ptts inst;
                        return inst;
                }
                void activity_seventh_day_rank_award_ptts_set_funcs() {
                        activity_seventh_day_rank_award_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.rank_awards()) {
                                        if (!check_events(i.award_events())) {
                                                CONFIG_ELOG << pd::rank_type_Name(ptt.rank_type()) << " check award_events failed";
                                        }
                                }
                                if (!check_events(ptt.comfort_award())) {
                                        CONFIG_ELOG << pd::rank_type_Name(ptt.rank_type()) << " check comfort_award failed";
                                }
                        };

                        activity_seventh_day_rank_award_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(mail, ptt.mail_id())) {
                                        CONFIG_ELOG << pd::rank_type_Name(ptt.rank_type()) << " mail table not exist mail_id " << ptt.mail_id();
                                }
                                if (!PTTS_HAS(origin, ptt.origin_pttid())) {
                                        CONFIG_ELOG << pd::rank_type_Name(ptt.rank_type()) << " origin table not exist origin_pttid " << ptt.origin_pttid();
                                }
                        };
                }

                activity_prize_wheel_ptts& activity_prize_wheel_ptts_instance() {
                        static activity_prize_wheel_ptts inst;
                        return inst;
                }
                void activity_prize_wheel_ptts_set_funcs() {
                        activity_prize_wheel_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.start_events())) {
                                        CONFIG_ELOG << ptt.id() << " check start events failed";
                                }
                                if (!check_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " check finish events failed";
                                }
                                for (const auto& i : ptt.prize_wheels()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check conditions failed";
                                        }
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check events failed";
                                        }
                                }
                        };
                        activity_prize_wheel_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_prize_wheels()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable_events());
                                }
                        };
                        activity_prize_wheel_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.start_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify start events failed";
                                }
                                if (!verify_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify finish events failed";
                                }
                                for (const auto& i : ptt.prize_wheels()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                        }
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify events failed";
                                        }
                                }
                        };
                }

                activity_discount_goods_ptts& activity_discount_goods_ptts_instance() {
                        static activity_discount_goods_ptts inst;
                        return inst;
                }

                void activity_discount_goods_ptts_set_funcs() {
                        activity_discount_goods_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(shop, ptt.shop_pttid())) {
                                        CONFIG_ELOG << ptt.id() << " shop table not exist " << ptt.shop_pttid();
                                } else {
                                        auto& shop_ptt = PTTS_GET(shop, ptt.shop_pttid());
                                        shop_ptt.set__discount_activity_id(ptt.id());
                                }
                        };
                }

                activity_shop_recharge_ptts& activity_shop_recharge_ptts_instance() {
                        static activity_shop_recharge_ptts inst;
                        return inst;
                }

                void activity_shop_recharge_ptts_set_funcs() {
                }

                activity_continue_recharge_ptts& activity_continue_recharge_ptts_instance() {
                        static activity_continue_recharge_ptts inst;
                        return inst;
                }

                void activity_continue_recharge_ptts_set_funcs() {
                        activity_continue_recharge_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.auto_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.auto_quest();
                                        }
                                        if (!PTTS_HAS(quest, i.event_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.event_quest();
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i.event_quest());
                                                quest_ptt.set__activity_continue_recharge_quest(true);
                                        }
                                }
                        };
                }

                activity_daiyanren_ptts& activity_daiyanren_ptts_instance() {
                        static activity_daiyanren_ptts inst;
                        return inst;
                }

                void activity_daiyanren_ptts_set_funcs() {
                        activity_daiyanren_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.reward()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check reward events failed";
                                        }
                                }
                        };
                        activity_daiyanren_ptts_instance().modify_func_ = [] (auto& ptt) {
                                set<uint32_t> check_finish_quests;
                                for (auto i : ptt.fuli_quests()) {
                                        if (PTTS_HAS(quest, i)) {
                                                auto quests = quest_to_all_quests(i);
                                                check_finish_quests.insert(quests.begin(), quests.end());
                                        }
                                }
                                for (auto i : check_finish_quests) {
                                        ptt.add__check_finish_quests(i);
                                }
                        };
                        activity_daiyanren_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(mail, ptt.reward_mail())) {
                                        CONFIG_ELOG << ptt.id() << " reward table not exist " << ptt.reward_mail();
                                }
                                for (const auto& i : ptt.reward()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify reward events failed";
                                        }
                                }
                                for (auto i : ptt.fuli_quests()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << ptt.id() << " fuli quests not exist " << i;
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i);
                                                quest_ptt.set__activity_daiyanren_fuli_quest(true);
                                        }
                                }
                                for (auto i : ptt.duihuan_quests()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << ptt.id() << " duihuan quests not exist " << i;
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i);
                                                quest_ptt.set__activity_daiyanren_duihuan_quest(true);
                                        }
                                }
                        };
                }

                activity_limit_play_ptts& activity_limit_play_ptts_instance() {
                        static activity_limit_play_ptts inst;
                        return inst;
                }

                void activity_limit_play_ptts_set_funcs() {
                        activity_limit_play_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.auto_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.auto_quest();
                                        }
                                        if (!PTTS_HAS(quest, i.event_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.event_quest();
                                        } else {
                                                auto& quest_ptt = PTTS_GET(quest, i.event_quest());
                                                quest_ptt.set__activity_limit_play_quest(true);
                                        }
                                }
                        };
                }

                activity_leiji_recharge_ptts& activity_leiji_recharge_ptts_instance() {
                        static activity_leiji_recharge_ptts inst;
                        return inst;
                }

                void activity_leiji_recharge_ptts_set_funcs() {
                        activity_leiji_recharge_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.auto_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.auto_quest();
                                                continue;
                                        }
                                        if (!PTTS_HAS(quest, i.event_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.event_quest();
                                                continue;
                                        }
                                        auto& quest_ptt = PTTS_GET(quest, i.event_quest());
                                        quest_ptt.set__activity_leiji_recharge_quest(true);
                                }
                        };
                }

                activity_leiji_consume_ptts& activity_leiji_consume_ptts_instance() {
                        static activity_leiji_consume_ptts inst;
                        return inst;
                }

                void activity_leiji_consume_ptts_set_funcs() {
                        activity_leiji_consume_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.auto_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.auto_quest();
                                                continue;
                                        }
                                        if (!PTTS_HAS(quest, i.event_quest())) {
                                                CONFIG_ELOG << ptt.id() << " has not quest " << i.event_quest();
                                                continue;
                                        }
                                        auto& quest_ptt = PTTS_GET(quest, i.event_quest());
                                        quest_ptt.set__activity_leiji_consume_quest(true);
                                }
                        };
                }

                activity_tea_party_ptts& activity_tea_party_ptts_instance() {
                        static activity_tea_party_ptts inst;
                        return inst;
                }

                void activity_tea_party_ptts_set_funcs() {
                        activity_tea_party_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.start_events())) {
                                        CONFIG_ELOG << ptt.id() << " check start events failed";
                                }
                                if (!check_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " check finish events failed";
                                }
                                for (const auto& i : ptt.favor_rewards()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check favor reward conditions failed";
                                        }
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check favor reward events failed";
                                        }
                                }
                        };
                        activity_tea_party_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << ptt.id() << " actor not exist " << ptt.actor();
                                }
                                if (!PTTS_HAS(lottery, ptt.lottery())) {
                                        CONFIG_ELOG << ptt.id() << " lottery not exist " << ptt.lottery();
                                } else {
                                        auto& lottery_ptt = PTTS_GET(lottery, ptt.lottery());
                                        lottery_ptt.set__activity_tea_party(ptt.id());
                                }
                                for (const auto& i : ptt.favor_rewards()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify favor reward conditions failed";
                                        }
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify favor reward events failed";
                                        }
                                }
                                if (!verify_events(ptt.start_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify start events failed";
                                }
                                if (!verify_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify finish events failed";
                                }
                                if (!PTTS_HAS(rank_reward, ptt.rank_reward())) {
                                        CONFIG_ELOG << ptt.id() << " rank reward not exist " << ptt.rank_reward();
                                }
                        };
                }

                activity_logic_ptts& activity_logic_ptts_instance() {
                        static activity_logic_ptts inst;
                        return inst;
                }

                void activity_logic_ptts_set_funcs() {
                        activity_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(mail, ptt.continue_recharge_mail())) {
                                        CONFIG_ELOG << ptt.id() << " has not mail " << ptt.continue_recharge_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.limit_play_mail())) {
                                        CONFIG_ELOG << ptt.id() << " has not mail " << ptt.limit_play_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.leiji_recharge_mail())) {
                                        CONFIG_ELOG << ptt.id() << " has not mail " << ptt.leiji_recharge_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.festival_mail())) {
                                        CONFIG_ELOG << ptt.id() << " has not mail " << ptt.festival_mail();
                                }
                                if (!PTTS_HAS(mail, ptt.leiji_consume_mail())) {
                                        CONFIG_ELOG << ptt.id() << " has not mail " << ptt.leiji_consume_mail();
                                }
                                if (!PTTS_HAS(activity_leiji_recharge, ptt.activity_start().leiji_recharge())) {
                                        CONFIG_ELOG << ptt.id() << " has not activity leiji recharge " << ptt.activity_start().leiji_recharge();
                                }
                                if (!PTTS_HAS(activity_leiji_consume, ptt.activity_start().leiji_consume())) {
                                        CONFIG_ELOG << ptt.id() << " has not activity leiji consume " << ptt.activity_start().leiji_consume();
                                }
                                if (!PTTS_HAS(activity_tea_party, ptt.activity_start().tea_party())) {
                                        CONFIG_ELOG << ptt.id() << " has not activity tea party " << ptt.activity_start().tea_party();
                                }
                        };
                }
        }
}
