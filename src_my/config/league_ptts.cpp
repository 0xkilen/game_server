#include "utils.hpp"
#include "league_ptts.hpp"
#include "item_ptts.hpp"
#include "quest_ptts.hpp"
#include "actor_ptts.hpp"
#include "mail_ptts.hpp"
#include "buff_ptts.hpp"
#include "chat_ptts.hpp"
#include "battle_ptts.hpp"
#include "shop_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                league_ptts& league_ptts_instance() {
                        static league_ptts inst;
                        return inst;
                }

                void league_ptts_set_funcs() {
                        league_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.create_conditions())) {
                                        CONFIG_ELOG << "check conditions league create failed ";
                                }
                                if (!check_conditions(ptt.mean().ce().conditions())) {
                                        CONFIG_ELOG << "check league donation mean conditions failed ";
                                }
                                if (!check_events(ptt.mean().reward())) {
                                        CONFIG_ELOG << "check league donation mean reward failed ";
                                }
                                if (!check_league_events(ptt.mean().first_time_reward())) {
                                        CONFIG_ELOG << "check league donation mean first_time_reward failed ";
                                }
                                if (!check_league_events(ptt.mean().league_events())) {
                                        CONFIG_ELOG << "check league donation mean league events failed ";
                                }
                                if (!check_conditions(ptt.normal().ce().conditions())) {
                                        CONFIG_ELOG << "check league donation normal conditions failed ";
                                }
                                if (!check_events(ptt.normal().reward())) {
                                        CONFIG_ELOG << "check league donation normal reward failed ";
                                }
                                if (!check_league_events(ptt.normal().first_time_reward())) {
                                        CONFIG_ELOG << "check league donation normal first_time_reward failed ";
                                }
                                if (!check_league_events(ptt.normal().league_events())) {
                                        CONFIG_ELOG << "check league donation normal league events failed ";
                                }
                                if (!check_conditions(ptt.generous().ce().conditions())) {
                                        CONFIG_ELOG << "check league donation generous conditions failed ";
                                }
                                if (!check_events(ptt.generous().reward())) {
                                        CONFIG_ELOG << "check league donation generous reward failed ";
                                }
                                if (!check_league_events(ptt.generous().first_time_reward())) {
                                        CONFIG_ELOG << "check league donation generous first_time_reward failed ";
                                }
                                if (!check_league_events(ptt.generous().league_events())) {
                                        CONFIG_ELOG << "check league donation generous league events failed ";
                                }
                                if (!check_conditions(ptt.impeach_conditions())) {
                                        CONFIG_ELOG << "check league impeach conditions failed ";
                                }
                                if (!check_conditions(ptt.lucky_bag_issue_ce().conditions())) {
                                        CONFIG_ELOG << "check lucky bag issue conditions failed ";
                                }
                                if (!check_conditions(ptt.lucky_bag_open_ce().conditions())) {
                                        CONFIG_ELOG << "check lucky bag open conditions failed ";
                                }
                                if (!check_conditions(ptt.campaign_ce().conditions())) {
                                        CONFIG_ELOG << "check campaign challenge conditions failed ";
                                }
                                if (!check_league_conditions(ptt.quest_unlock_conditions())) {
                                        CONFIG_ELOG << "quest unlock conditions failed ";
                                }
                                if (!check_league_conditions(ptt.campaign_refresh_conditions())) {
                                        CONFIG_ELOG << "campaign refresh conditions failed ";
                                }

                                if (!check_conditions(ptt.help_quest_ce().conditions())) {
                                        CONFIG_ELOG << "check help quest conditions failed ";
                                }

                                for (const auto& i : ptt.quests()) {
                                        if (!check_events(i.reward())) {
                                                CONFIG_ELOG << " league quest: " << i.pttid() << " check conditions failed";
                                        }
                                }

                                for (auto i = 0; i < ptt.damage_ranking_reward_size(); ++i) {
                                        if (!check_events(ptt.damage_ranking_reward(i).reward_events())) {
                                                CONFIG_ELOG << " damage ranking reward: " << i << " check evnets failed";   
                                        }
                                }


                                for (auto i = 0; i < ptt.trickeries_size(); ++i) {
                                        const auto& trickery = ptt.trickeries(i);
                                        if (!check_conditions(trickery.cost_ce().conditions())) {
                                                CONFIG_ELOG << " trickeries: " << i << " check cost_ce conditions failed";   
                                        }
                                        if (!check_events(trickery.rewards())) {
                                                CONFIG_ELOG << " trickeries: " << i << " check rewards failed";   
                                        }

                                        if (!check_league_events(trickery.target_events())) {
                                                CONFIG_ELOG << " trickeries: " << i << " check target_events failed";   
                                        }
                                }

                                if (!check_conditions(ptt.rename_ce().conditions())) {
                                        CONFIG_ELOG << "rename conditions failed ";
                                }

                                if (!check_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "check unlock conditions failed ";
                                }

                        };
                        league_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.create_conditions(), *(ptt.mutable__create_events()));
                                modify_events_by_conditions(ptt.mean().ce().conditions(), *(ptt.mutable_mean()->mutable_ce()->mutable__events()));
                                modify_events_by_conditions(ptt.normal().ce().conditions(), *(ptt.mutable_normal()->mutable_ce()->mutable__events()));
                                modify_events_by_conditions(ptt.generous().ce().conditions(), *(ptt.mutable_generous()->mutable_ce()->mutable__events()));
                                modify_events_by_conditions(ptt.impeach_conditions(), *(ptt.mutable__impeach_events()));
                                modify_events_by_conditions(ptt.lucky_bag_issue_ce().conditions(), *(ptt.mutable_lucky_bag_issue_ce()->mutable__events()));
                                modify_events_by_conditions(ptt.lucky_bag_open_ce().conditions(), *(ptt.mutable_lucky_bag_open_ce()->mutable__events()));
                                modify_events_by_conditions(ptt.campaign_ce().conditions(), *(ptt.mutable_campaign_ce()->mutable__events()));
                                modify_league_events_by_league_conditions(ptt.campaign_refresh_conditions(), *(ptt.mutable__campaign_refresh_events()));
                                modify_events_by_conditions(ptt.help_quest_ce().conditions(), *(ptt.mutable_help_quest_ce()->mutable__events()));

                                for (auto& i : *(ptt.mutable_trickeries())) {
                                        modify_events_by_conditions(i.cost_ce().conditions(), *(i.mutable_cost_ce()->mutable__events()));
                                }
                                modify_events_by_conditions(ptt.rename_ce().conditions(), *(ptt.mutable_rename_ce()->mutable__events()));
                        };
                        league_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.pttid())) {
                                                CONFIG_ELOG << " league quest " << i.pttid() << " not exist";
                                        }
                                        auto& quest_ptt = PTTS_GET(quest, i.pttid());
                                        quest_ptt.set__league_quest(ptt.id());
                                }

                                for (const auto& i : ptt.lucky_bag()) {
                                        if (!PTTS_HAS(item, i.pttid())) {
                                                CONFIG_ELOG << " league lucky bag item " << i.pttid() << " not exist";
                                        }
                                }
                                if (!PTTS_HAS(mail, ptt.league_allocate_mail())) {
                                        CONFIG_ELOG << " league allocate mail not exit " << ptt.league_allocate_mail();
                                }

                                if (!verify_conditions(ptt.create_conditions())) {
                                        CONFIG_ELOG << "verify conditions league create failed ";
                                }
                                if (!verify_conditions(ptt.mean().ce().conditions())) {
                                        CONFIG_ELOG << "verify league donation mean conditions failed ";
                                }
                                if (!verify_events(ptt.mean().reward())) {
                                        CONFIG_ELOG << "verify league donation mean reward failed ";
                                }
                                if (!verify_league_events(ptt.mean().first_time_reward())) {
                                        CONFIG_ELOG << "verify league donation mean first_time_reward failed ";
                                }
                                if (!verify_league_events(ptt.mean().league_events())) {
                                        CONFIG_ELOG << "verify league donation mean league events failed ";
                                }
                                if (!verify_conditions(ptt.normal().ce().conditions())) {
                                        CONFIG_ELOG << "verify league donation normal conditions failed ";
                                }
                                if (!verify_events(ptt.normal().reward())) {
                                        CONFIG_ELOG << "verify league donation normal reward failed ";
                                }
                                if (!verify_league_events(ptt.normal().first_time_reward())) {
                                        CONFIG_ELOG << "verify league donation normal first_time_reward failed ";
                                }
                                if (!verify_league_events(ptt.normal().league_events())) {
                                        CONFIG_ELOG << "verify league donation normal league events failed ";
                                }
                                if (!verify_conditions(ptt.generous().ce().conditions())) {
                                        CONFIG_ELOG << "verify league donation generous conditions failed ";
                                }
                                if (!verify_events(ptt.generous().reward())) {
                                        CONFIG_ELOG << "verify league donation generous reward failed ";
                                }
                                if (!verify_league_events(ptt.generous().league_events())) {
                                        CONFIG_ELOG << "verify league donation generous league events failed ";
                                }
                                if (!verify_league_events(ptt.generous().first_time_reward())) {
                                        CONFIG_ELOG << "verify league donation generous first_time_reward failed ";
                                }
                                if (!verify_conditions(ptt.impeach_conditions())) {
                                        CONFIG_ELOG << "verify league impeach conditions failed ";
                                }
                                if (!verify_conditions(ptt.lucky_bag_issue_ce().conditions())) {
                                        CONFIG_ELOG << "verify lucky bag issue conditions failed ";
                                }
                                if (!verify_conditions(ptt.lucky_bag_open_ce().conditions())) {
                                        CONFIG_ELOG << "verify lucky bag open conditions failed ";
                                }
                                if (!verify_conditions(ptt.campaign_ce().conditions())) {
                                        CONFIG_ELOG << "verify campaign challenge conditions failed ";
                                }

                                if (!verify_conditions(ptt.help_quest_ce().conditions())) {
                                        CONFIG_ELOG << "verify help quest conditions failed ";
                                }

                                for (const auto& i : ptt.quests()) {
                                        if (!verify_events(i.reward())) {
                                                CONFIG_ELOG << " league quest: " << i.pttid() << " verify conditions failed";
                                        }
                                }

                                for (auto i = 0; i < ptt.damage_ranking_reward_size(); ++i) {
                                        if (!verify_events(ptt.damage_ranking_reward(i).reward_events())) {
                                                CONFIG_ELOG << " damage ranking reward: " << i << " verify evnets failed";   
                                        }
                                }


                                for (auto i = 0; i < ptt.trickeries_size(); ++i) {
                                        const auto& trickery = ptt.trickeries(i);
                                        if (!verify_conditions(trickery.cost_ce().conditions())) {
                                                CONFIG_ELOG << " trickeries: " << i << " verify cost_ce conditions failed";   
                                        }
                                        if (!verify_events(trickery.rewards())) {
                                                CONFIG_ELOG << " trickeries: " << i << " verify rewards failed";   
                                        }

                                        if (!verify_league_events(trickery.target_events())) {
                                                CONFIG_ELOG << " trickeries: " << i << " verify target_events failed";   
                                        }
                                }

                                if (!verify_conditions(ptt.rename_ce().conditions())) {
                                        CONFIG_ELOG << "verify rename conditions failed ";
                                }

                                if (!verify_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << "verify unlock conditions failed ";
                                }
                        };
                }

                league_record_list_limit_ptts& league_record_list_limit_ptts_instance() {
                        static league_record_list_limit_ptts inst;
                        return inst;
                }
                void league_record_list_limit_ptts_set_funcs() {

                }


                league_levelup_ptts& league_levelup_ptts_instance() {
                        static league_levelup_ptts inst;
                        return inst;
                }
                void league_levelup_ptts_set_funcs() {
                        league_levelup_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_league_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.level() << " check conditions failed ";
                                }
                        };

                        league_levelup_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_league_events_by_league_conditions(ptt.conditions(), *(ptt.mutable__events()));
                                auto* e = ptt.mutable__events()->add_events();
                                e->set_type(pd::LET_LEVEL_UP);
                        };
                }

                league_resource_ptts& league_resource_ptts_instance() {
                        static league_resource_ptts inst;
                        return inst;
                }

                void league_resource_ptts_set_funcs() {}

                league_campaign_ptts& league_campaign_ptts_instance() {
                        static league_campaign_ptts inst;
                        return inst;
                }

                void league_campaign_ptts_set_funcs() {
                        league_campaign_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_league_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " unlock conditions failed ";
                                }
                                if (!check_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.id() << " win events failed ";
                                }

                                if (!check_events(ptt.lose_events())) {
                                        CONFIG_ELOG << ptt.id() << " lose events failed ";
                                }

                                if (!check_league_events(ptt.win_le())) {
                                        CONFIG_ELOG << ptt.id() << " win league events failed ";
                                }

                                if (!check_league_events(ptt.lose_le())) {
                                        CONFIG_ELOG << ptt.id() << " lose league events failed ";
                                }

                                if (!check_league_conditions(ptt.unlock_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " unlock conditions failed ";
                                }
                        };

                        league_campaign_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << "actor " << ptt.actor() << " not exist";
                                }

                                if (!verify_events(ptt.win_events())) {
                                        CONFIG_ELOG << ptt.id() << "verify win events failed ";
                                }

                                if (!verify_events(ptt.lose_events())) {
                                        CONFIG_ELOG << ptt.id() << "verify lose events failed ";
                                }

                                if (!verify_league_events(ptt.win_le())) {
                                        CONFIG_ELOG << ptt.id() << "verify win league events failed ";
                                }

                                if (!verify_league_events(ptt.lose_le())) {
                                        CONFIG_ELOG << ptt.id() << "verify lose league events failed ";
                                }
                        };
                        league_campaign_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_league_events_by_league_conditions(ptt.unlock_conditions(), *(ptt.mutable__unlock_events()));
                        };
                }

                league_drop_ptts& league_drop_ptts_instance() {
                        static league_drop_ptts inst;
                        return inst;
                }

                void league_drop_ptts_set_funcs() {
                        league_drop_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!check_league_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check events failed ";
                                        }
                                }
                        };

                        league_drop_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!verify_league_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify league events failed ";
                                        }
                                }
                        };

                        league_drop_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (i.has_odds()) {
                                                if (ptt.has__use_odds()) {
                                                        if (!ptt._use_odds()) {
                                                                CONFIG_ELOG << ptt.id() << " use odds and weight conflict";
                                                        }
                                                } else {
                                                        ptt.set__use_odds(true);
                                                }
                                        } else if (i.has_weight()) {
                                                if (ptt.has__use_odds()) {
                                                        if (ptt._use_odds()) {
                                                                CONFIG_ELOG << ptt.id() << " use odds and weight conflict";
                                                        }
                                                } else {
                                                        ptt.set__use_odds(false);
                                                }
                                        } else {
                                                CONFIG_ELOG << ptt.id() << " no odds and no weight";
                                        }
                                }
                        };
                }

                league_apply_item_ptts& league_apply_item_ptts_instance() {
                        static league_apply_item_ptts inst;
                        return inst;
                }

                void league_apply_item_ptts_set_funcs() {
                        league_apply_item_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.apply_ce().conditions())) {
                                        CONFIG_ELOG << ptt.item() << "check item apply conditions failed ";
                                }
                        };

                        league_apply_item_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG << "league allocated item " << ptt.item() << " not exist";
                                }
                                if (!verify_conditions(ptt.apply_ce().conditions())) {
                                        CONFIG_ELOG << ptt.item() << "verify item apply conditions failed ";
                                }
                        };

                        league_apply_item_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.apply_ce().conditions(), *ptt.mutable_apply_ce()->mutable__events());
                        };
                }

                league_creating_ptts& league_creating_ptts_instance() {
                        static league_creating_ptts inst;
                        return inst;
                }

                void league_creating_ptts_set_funcs() {
                        league_creating_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_league_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed ";
                                }
                        };

                        league_creating_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_league_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed ";
                                }
                        };

                        league_creating_ptts_instance().modify_func_ = [] (auto& ptt) {
                        };
                }

                league_building_ptts& league_building_ptts_instance() {
                        static league_building_ptts inst;
                        return inst;
                }

                void league_building_ptts_set_funcs() {
                        league_building_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.upgrade_accelerations()) {
                                        if (!check_league_conditions(i.upgrade_acceleration_ce().conditions())) {
                                                CONFIG_ELOG << ptt.type() << " check building upgrade acceleration conditions failed ";
                                        }
                                }

                                for (const auto& i : ptt.upgrades()) {
                                        if (!check_league_conditions(i.upgrade_ce().conditions())) {
                                                CONFIG_ELOG << ptt.type() << " check building upgrade conditions failed ";
                                        }
                                }
                        };

                        league_building_ptts_instance().verify_func_ = [] (const auto& ptt) {

                        };

                        league_building_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto& upgrade_accelerations = *ptt.mutable_upgrade_accelerations();
                                for (auto& i : upgrade_accelerations) {
                                        modify_league_events_by_league_conditions(
                                                i.upgrade_acceleration_ce().conditions(), 
                                                *(i.mutable_upgrade_acceleration_ce()->mutable__events())
                                        );
                                }                               

                                auto& upgrades = *ptt.mutable_upgrades();
                                for (auto& i : upgrades) {
                                        modify_league_events_by_league_conditions(i.upgrade_ce().conditions(), *(i.mutable_upgrade_ce()->mutable__events()));
                                }             
                        };
                }

                league_urban_management_level_ptts& league_urban_management_level_ptts_instance() {
                        static league_urban_management_level_ptts inst;
                        return inst;
                }

                void league_urban_management_level_ptts_set_funcs() {
                }

                league_daily_quest_level_ptts& league_daily_quest_level_ptts_instance() {
                        static league_daily_quest_level_ptts inst;
                        return inst;
                }

                void league_daily_quest_level_ptts_set_funcs() {
                        league_daily_quest_level_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.quests_size() < static_cast<int32_t>(ptt.quest_count())) {
                                        CONFIG_ELOG << ptt.level() << " check failed, quests size less than quest_count ";
                                }
                        };

                        league_daily_quest_level_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i.pttid())) {
                                                CONFIG_ELOG << " league quest " << i.pttid() << " not exist";
                                        }
                                        auto& quest_ptt = PTTS_GET(quest, i.pttid());
                                        quest_ptt.set__league_quest(ptt.level());
                                }
                        };
                }

                league_government_level_ptts& league_government_level_ptts_instance() {
                        static league_government_level_ptts inst;
                        return inst;
                }

                void league_government_level_ptts_set_funcs() {
                }

                city_ptts& city_ptts_instance() {
                        static city_ptts inst;
                        return inst;
                }

                void city_ptts_set_funcs() {
                        city_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (auto& i : ptt.trade()) {
                                        if (!check_league_conditions(i.trade_lce().conditions())) { 
                                                CONFIG_ELOG << ptt.pttid() << " check city trade league conditions failed ";
                                        }
                                        if (!check_conditions(i.trade_ce().conditions())) {
                                                CONFIG_ELOG << ptt.pttid() << " check city trade conditions failed ";
                                        }
                                }

                                for (auto& i : ptt.alienation()) {
                                        if (!check_league_conditions(i.alienation_lce().conditions())) { 
                                                CONFIG_ELOG << ptt.pttid() << " check city alienation league conditions failed ";
                                        }
                                        if (!check_conditions(i.alienation_ce().conditions())) {
                                                CONFIG_ELOG << ptt.pttid() << " check city alienation conditions failed ";
                                        }
                                }

                                for (auto& i : ptt.invite()) {
                                        if (!check_league_conditions(i.invite_lce().conditions())) { 
                                                CONFIG_ELOG << ptt.pttid() << " check city invite league conditions failed ";
                                        }
                                        if (!check_conditions(i.invite_ce().conditions())) {

                                                CONFIG_ELOG << ptt.pttid() << " check city invite conditions failed ";
                                        }
                                }
                        };

                        city_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto& i : ptt.trade()) {
                                        if (!verify_conditions(i.trade_ce().conditions())) {
                                                CONFIG_ELOG << ptt.pttid() << " verify city trade conditions failed ";
                                        }
                                }

                                for (auto& i : ptt.alienation()) {
                                        if (!verify_conditions(i.alienation_ce().conditions())) {
                                                CONFIG_ELOG << ptt.pttid() << " verify city alienation conditions failed ";
                                        }
                                }

                                for (auto& i : ptt.invite()) {
                                        if (!verify_conditions(i.invite_ce().conditions())) {
                                                CONFIG_ELOG << ptt.pttid() << " verify city invite conditions failed ";
                                        }
                                }

                                if (!PTTS_HAS(system_league, ptt.origin_league())) {
                                        CONFIG_ELOG << ptt.pttid() << " origin_league " << ptt.origin_league() << " not exist ";
                                }

                                if (!PTTS_HAS(shop, ptt.extra_item_shop())) {
                                        CONFIG_ELOG << ptt.pttid() << " extra_item_shop " << ptt.extra_item_shop() << " not exist ";
                                }

                                const auto& shop_ptt = PTTS_GET(shop, ptt.extra_item_shop());

                                bool has_goods = false;
                                for (const auto& i : shop_ptt.goods()) {
                                        if (i.id() == ptt.extra_item_goods()) {
                                                has_goods = true;
                                                break;
                                        }
                                }
                                if (!has_goods) {
                                        CONFIG_ELOG << ptt.pttid() << " extra_item_goods " << ptt.extra_item_shop() << " not exist "; 
                                }
                        };

                        city_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_trade()) {
                                        modify_league_events_by_league_conditions(i.trade_lce().conditions(), *i.mutable_trade_lce()->mutable__events());
                                        modify_events_by_conditions(i.trade_ce().conditions(), *i.mutable_trade_ce()->mutable__events());
                                }
                                for (auto& i : *ptt.mutable_alienation()) {
                                        modify_league_events_by_league_conditions(i.alienation_lce().conditions(), *i.mutable_alienation_lce()->mutable__events());
                                        modify_events_by_conditions(i.alienation_ce().conditions(), *i.mutable_alienation_ce()->mutable__events());
                                }

                                for (auto& i : *ptt.mutable_invite()) {
                                        modify_league_events_by_league_conditions(i.invite_lce().conditions(), *i.mutable_invite_lce()->mutable__events());
                                        modify_events_by_conditions(i.invite_ce().conditions(), *i.mutable_invite_ce()->mutable__events());
                                }                                
                        };
                }

                system_league_ptts& system_league_ptts_instance() {
                        static system_league_ptts inst;
                        return inst;
                }

                void system_league_ptts_set_funcs() {
                        system_league_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.origin_friendliness()) {
                                        if(!PTTS_HAS(city, i.city())) {
                                                CONFIG_ELOG << ptt.pttid() << " origin friendliness city not exist " << i.city();
                                        }
                                }
                        };
                }

                city_battle_logic_ptts& city_battle_logic_ptts_instance() {
                        static city_battle_logic_ptts inst;
                        return inst;
                }

                void city_battle_logic_ptts_set_funcs() {
                        
                }

                city_battle_treasure_box_defender_ptts& city_battle_treasure_box_defender_ptts_instance() {
                        static city_battle_treasure_box_defender_ptts inst;
                        return inst;
                }

                void city_battle_treasure_box_defender_ptts_set_funcs() {
                        city_battle_treasure_box_defender_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.has_reward() && !check_events(ptt.reward())) {
                                        CONFIG_ELOG << ptt.pttid() << "check defender reward events failed ";
                                }
                        };

                        city_battle_treasure_box_defender_ptts_instance().verify_func_ = [] (const auto& ptt) {                                
                                if (ptt.has_buff() && !PTTS_HAS(buff, ptt.buff())) {
                                        CONFIG_ELOG << ptt.pttid() << " buff " << ptt.buff() << " not exist ";
                                }
                                if (ptt.has_reward() && !verify_events(ptt.reward())) {
                                        CONFIG_ELOG << ptt.pttid() << "verify defender reward events failed ";
                                }
                        };
                }

                city_battle_treasure_box_offensive_ptts& city_battle_treasure_box_offensive_ptts_instance() {
                        static city_battle_treasure_box_offensive_ptts inst;
                        return inst;
                }

                void city_battle_treasure_box_offensive_ptts_set_funcs() {
                        city_battle_treasure_box_offensive_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.has_reward() && !check_events(ptt.reward())) {
                                        CONFIG_ELOG << ptt.pttid() << "check offensive reward events failed ";
                                }
                        };

                        city_battle_treasure_box_offensive_ptts_instance().verify_func_ = [] (const auto& ptt) {                                
                                if (ptt.has_buff() && !PTTS_HAS(buff, ptt.buff())) {
                                        CONFIG_ELOG << ptt.pttid() << " buff " << ptt.buff() << " not exist ";
                                }
                                if (ptt.has_reward() && !verify_events(ptt.reward())) {
                                        CONFIG_ELOG << ptt.pttid() << "verify offensive reward events failed ";
                                }
                        };
                }
                
                city_battle_ptts& city_battle_ptts_instance() {
                        static city_battle_ptts inst;
                        return inst;
                }

                void city_battle_ptts_set_funcs() {
                        city_battle_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.reward().defender_participation_award())) {
                                        CONFIG_ELOG << ptt.pttid() << "check defender participation events failed";
                                }

                                if (!check_events(ptt.reward().defender_win_events())) {
                                        CONFIG_ELOG << ptt.pttid() << "check defender win events failed";
                                }

                                if (!check_league_events(ptt.reward().defender_win_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " check defender win le failed";
                                }

                                if (!check_events(ptt.reward().kill_reward())) {
                                        CONFIG_ELOG << ptt.pttid() << " check kill reward failed";
                                }

                                if (!check_league_events(ptt.reward().kill_reward_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " check kill reward le failed";
                                }

                                if (!check_events(ptt.reward().offensive_participation_award())) {
                                        CONFIG_ELOG << ptt.pttid() << " check offensive participation events failed";
                                }

                                if (!check_events(ptt.reward().offensive_win_events())) {
                                        CONFIG_ELOG << ptt.pttid() << " check offensive win events failed ";
                                }

                                if (!check_league_events(ptt.reward().offensive_win_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " check offensive win le failed";
                                }
                        };

                        city_battle_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.reward().defender_participation_award())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify defender participation event failed";
                                }

                                if (!verify_events(ptt.reward().defender_win_events())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify defender win event failed";
                                }

                                if (!verify_league_events(ptt.reward().defender_win_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify defender win le failed";
                                }

                                if (!verify_events(ptt.reward().kill_reward())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify kill reward failed";
                                }

                                if (!verify_league_events(ptt.reward().kill_reward_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify kill reward le failed";
                                }

                                if (!verify_events(ptt.reward().offensive_participation_award())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify offensive participation event failed";
                                }

                                if (!verify_events(ptt.reward().offensive_win_events())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify offensive win event failed";
                                }

                                if (!verify_league_events(ptt.reward().offensive_win_le())) {
                                        CONFIG_ELOG << ptt.pttid() << " verify offensive win le failed";
                                }
                        };
                }

                league_war_logic_ptts& league_war_logic_ptts_instance() {
                        static league_war_logic_ptts inst;
                        return inst;
                }

                void league_war_logic_ptts_set_funcs() {
                        league_war_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.perfect_win_reward())) {
                                        CONFIG_ELOG << "check perfect win reward failed";
                                }
                                if (!check_events(ptt.chief_reward())) {
                                        CONFIG_ELOG << "check chief reward failed";
                                }
                                if (!check_events(ptt.first_vice_chief_reward())) {
                                        CONFIG_ELOG << "check first vice chief reward failed";
                                }
                                if (!check_events(ptt.second_vice_chief_reward())) {
                                        CONFIG_ELOG << "check second vice chief reward failed";
                                }
                                if (!check_events(ptt.third_vice_chief_reward())) {
                                        CONFIG_ELOG << "check third vice chief reward failed";
                                }
                                if (!check_events(ptt.warriors_reward())) {
                                        CONFIG_ELOG << "check warriors reward failed";
                                }
                                if (!check_events(ptt.cheer_up_events())) {
                                        CONFIG_ELOG << "check cheer up reward failed";
                                }
                        };
                        league_war_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_league_events_by_league_conditions(ptt.sign_up_lce().conditions(), *ptt.mutable_sign_up_lce()->mutable__events());
                        };
                        league_war_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(battle, ptt.battle_pttid())) {
                                        CONFIG_ELOG << "battle pttid not exist " << ptt.battle_pttid();
                                }
                                if (!PTTS_HAS(buff, ptt.cheer_up_buff())) {
                                        CONFIG_ELOG << "cheer up buff not exist " << ptt.cheer_up_buff();
                                }
                                if (!verify_events(ptt.perfect_win_reward())) {
                                        CONFIG_ELOG << "verify perfect win reward failed";
                                }
                                if (!verify_events(ptt.chief_reward())) {
                                        CONFIG_ELOG << "verify chief reward failed";
                                }
                                if (!verify_events(ptt.first_vice_chief_reward())) {
                                        CONFIG_ELOG << "verify first vice chief reward failed";
                                }
                                if (!verify_events(ptt.second_vice_chief_reward())) {
                                        CONFIG_ELOG << "verify second vice chief reward failed";
                                }
                                if (!verify_events(ptt.third_vice_chief_reward())) {
                                        CONFIG_ELOG << "verify third vice chief reward failed";
                                }
                                if (!verify_events(ptt.warriors_reward())) {
                                        CONFIG_ELOG << "verify warriors reward failed";
                                }
                                if (!PTTS_HAS(mail, ptt.perfect_reward_mail_id())) {
                                        CONFIG_ELOG << " perfect reward mail id not exist";
                                }
                                if (!PTTS_HAS(mail, ptt.in_battle_reward_mail_id())) {
                                        CONFIG_ELOG << " in battle reward mail id not exist";
                                }
                                if (!PTTS_HAS(mail, ptt.rank_reward_mail_id())) {
                                        CONFIG_ELOG << " rank reward mail id not exist";
                                }
                                if (!PTTS_HAS(system_chat, ptt.enrolled_system_chat())) {
                                        CONFIG_ELOG << " enrolled system chat not exist";
                                }
                        };
                }

                league_permission_ptts& league_permission_ptts_instance() {
                        static league_permission_ptts inst;
                        return inst;
                }

                void league_permission_ptts_set_funcs() {
                        league_permission_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.functions()) {
                                        if (!PTTS_HAS(league_permission_function, i)) {
                                                CONFIG_ELOG << i <<" verify function failed";
                                        }
                                }
                        };
                }

                league_permission_function_ptts& league_permission_function_ptts_instance() {
                        static league_permission_function_ptts inst;
                        return inst;
                }

                void league_permission_function_ptts_set_funcs() {

                }
        }
}
