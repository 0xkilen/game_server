#include "actor_ptts.hpp"
#include "common_ptts.hpp"
#include "skill_ptts.hpp"
#include "buff_ptts.hpp"
#include "player_ptts.hpp"
#include "adventure_ptts.hpp"
#include "battle_ptts.hpp"
#include "item_ptts.hpp"
#include "equipment_ptts.hpp"
#include "lottery_ptts.hpp"
#include "shop_ptts.hpp"
#include "spine_ptts.hpp"
#include "league_ptts.hpp"
#include "huanzhuang_ptts.hpp"
#include "arena_ptts.hpp"
#include "growth_ptts.hpp"
#include "quest_ptts.hpp"
#include "drop_ptts.hpp"
#include "mail_ptts.hpp"
#include "zangzi_ptts.hpp"
#include "area_ip_ptts.hpp"
#include "serverlist_ptts.hpp"
#include "plot_ptts.hpp"
#include "resource_ptts.hpp"
#include "options_ptts.hpp"
#include "lieming_ptts.hpp"
#include "fief_ptts.hpp"
#include "mansion_ptts.hpp"
#include "relation_ptts.hpp"
#include "rank_ptts.hpp"
#include "achievement_ptts.hpp"
#include "activity_ptts.hpp"
#include "title_ptts.hpp"
#include "sundries_ptts.hpp"
#include "xinshou_ptts.hpp"
#include "recharge_ptts.hpp"
#include "chat_ptts.hpp"
#include "marriage_ptts.hpp"
#include "mirror_ptts.hpp"
#include "behavior_tree_ptts.hpp"
#include "gongdou_ptts.hpp"
#include "poetry_pool_ptts.hpp"
#include "help_ptts.hpp"
#include "origin_ptts.hpp"
#include "channel_specify_ptts.hpp"
#include "login_back_ground_ptts.hpp"
#include "feige_ptts.hpp"
#include "tower_ptts.hpp"
#include "guanpin_ptts.hpp"
#include "loading_ptts.hpp"
#include "child_ptts.hpp"
#include "jade_ptts.hpp"
#include "utils.hpp"
#include "utils/log.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

using namespace std;
namespace nc = nora::config;
namespace pc = proto::config;
namespace bpo = boost::program_options;

int main(int argc, char *argv[]) {
        bpo::options_description desc("options");
        desc.add_options()
                ("help", "produce help message")
                ("type", bpo::value<string>(), "config type")
                ("csv", "generate csv headers")
                ("json", "generate json from input csv file")
                ("bin", "generate bin from input csv file")
                ("check", "check all configs")
                ("write_all", "write all configs")
                ("input-file", bpo::value<string>(), "input file")
                ("dir", bpo::value<string>(), "config dir");
        bpo::positional_options_description p;
        p.add("input-file", -1);
        bpo::variables_map vm;
        try {
                bpo::store(bpo::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
                bpo::notify(vm);

        } catch (const exception& e) {
                cerr << e.what() << endl;
                return 1;
        }


        if (vm.count("help")) {
                cout << desc << "\n";
                return 0;
        } else if (vm.count("json") && !vm.count("input-file")) {
                cerr << "need input-file to generate json" << endl;
                return 1;
        } else if (vm.count("bin") && !vm.count("input-file")) {
                cerr << "need input-file to generate bin" << endl;
                return 1;
        } else if (!vm.count("check") && !vm.count("write_all") && !vm.count("type")) {
                cerr << "need type" << endl;
                return 1;
        } else if (!vm.count("dir")) {
                cerr << "need dir" << endl;
                return 1;
        }

        nora::config_path= vm["dir"].as<string>();
        if (vm.count("check") > 0) {
                nora::init_log("check_log", pc::options::TRACE);
                nc::load_config();
                nc::load_dirty_word();
                nc::check_config();
        } else if (vm.count("write_all") > 0) {
                nora::init_log("write_all_log", pc::options::ERROR);
                nc::write_all();
        } else {
                nora::init_log("log", pc::options::ERROR);
                auto type = vm["type"].as<string>();
#define PROCESS_CONFIG_BY_TYPE_IF(str) if (type == #str) {              \
                        if (vm.count("csv")) {                          \
                                nc::str##_ptts_instance().cout_csv_template(); \
                        } else if (vm.count("json")) {                  \
                                pc::str##_file f;                       \
                                nc::str##_ptts_instance().cout_json_file(vm["input-file"].as<string>(), &f); \
                        } else if (vm.count("bin")) {                   \
                                pc::str##_file f;                       \
                                nc::str##_ptts_instance().cout_bin_file(vm["input-file"].as<string>(), &f); \
                        }                                               \
                }
#define PROCESS_CONFIG_BY_TYPE_ELSE_IF(str) else PROCESS_CONFIG_BY_TYPE_IF(str)

                PROCESS_CONFIG_BY_TYPE_IF(actor)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_levelup)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_add_star)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_actor_add_star)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_add_step)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_add_pin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_intimacy)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_voice)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_plot)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(role_levelup)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(role_add_fate)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(role_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(role_present)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(growth)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(not_portional_growth)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(craft_growth)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(skill)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(buff)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(create_role)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(adventure)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(adventure_episode)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(adventure_chapter)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(adventure_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(battle)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(item)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(item_xinwu_exchange)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_levelup)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_add_quality)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_suite)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_rand_attr_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_rand_attr_quality)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_decompose)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_compose)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(equipment_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lottery)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lottery_show)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(shop)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mys_shop)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(spine_limit)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(spine_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(spine_model)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_pvp)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_pvp_round)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_tag)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_deform_face)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_deform_head)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_taozhuang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_tujian)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_compose)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huanzhuang_levelup)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(rank_reward)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_resource)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_record_list_limit)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_levelup)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_campaign)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_drop)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_apply_item)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_creating)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_building)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_urban_management_level)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_daily_quest_level)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_government_level)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_permission)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_permission_function)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(city)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(system_league)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(city_battle_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(city_battle_treasure_box_defender)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(city_battle_treasure_box_offensive)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(city_battle)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(yifu)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(shouchi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(beishi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(toufa)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(jiezhi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(edai)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fazan)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(guanshi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(guanghuan)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(toujin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(erhuan)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(xianglian)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(hongmo)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(yanjing)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(yanshenguang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(saihong)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(meimao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(yanzhuang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(chuncai)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(zui)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(huzi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lian)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(tiehua)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lianxing)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(spine_deform_face)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(spine_deform_head)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(formation_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(arena_npc_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(arena_reward)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(arena)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(public_arena_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(quest)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(everyday_quest)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(everyday_quest_reward)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(drop)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mail)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(zangzi)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(serverlist)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(logind_server)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(specific_server_list)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(plot)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(plot_options)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(plot_options_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(resource)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lieming)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(lieming_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(options)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_thief)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_boss)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_hospital)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_bar)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_cofc)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_guard)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_cloth)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_mine)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_incident)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_furniture_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_furniture_type)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_furniture_collision_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_area)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_furniture)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_hall)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_hall_quest)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_quest)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_incident)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_craft_type_summary)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_pond)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_kitchen)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_bedroom)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_vegetable_plot)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_bedroom_tool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_poison)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_firework)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_dish)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_celebrity)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_celebrity_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_prestige)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_riddle)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_question)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_question_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_thief)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_coin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_coin_stage)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_pintu_game)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_weilie_game)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_link_game)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_rhythm_master_game)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_wedding_ball)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet_wedding_candy)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_banquet)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_farm_plant)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_cook_recipe)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_game)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_exchange)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mansion_game_fish_capper)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(relation_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(relation_gift)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(relation_suggestion)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(achievement)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_login_days)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_online_time)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_seven_days)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_qiandao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_chaozhi_libao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_everyday_libao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_first_recharge)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_vip_qianzhuang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_vip_qianzhuang_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_emperor_qianzhuang)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_emperor_qianzhuang_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_fund)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_limit_libao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_festival)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_seventh_day_rank_award)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_prize_wheel)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_discount_goods)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_shop_recharge)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_continue_recharge)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_limit_play)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_leiji_recharge)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_leiji_consume)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_daiyanren)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_tea_party)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(activity_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(title)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(sundries)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(xinshou)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(xinshou_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(xinshou_guide)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(module_introduce)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(name_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(recharge)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(vip)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_united_chance)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(chat)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(system_chat)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_question)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_guesture)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_guesture_pair)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_child_qian)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(marriage_enai)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(area_ip)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(mirror)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(behavior_tree)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(behavior_tree_root)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_area_incident)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_area_incident_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_area_incident_reward)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(gongdou)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(poetry_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(help)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(help_tips)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(origin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(channel_specify)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(rank_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(robot)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_bubble)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(fief_bubble)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(gm_data)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(gm_sequence)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(ui_quest)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(feige)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(feige_message)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(feige_chuanwen)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(feige_chuanwen_group)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(feige_cg)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(tower)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(tower_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(guanpin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(guanpin_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(guanpin_fenghao)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(loading)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(actor_skin)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(battle_ai_pool)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(login_back_ground)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(major_city_bubble)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(league_war_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_logic)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_value)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_raise)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_study)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_study_item)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_skill)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_learn_skill_actor)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_add_skill_exp_item)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_drop)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_phase)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(child_buff)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(jade)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(system_preview)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(common)
                        PROCESS_CONFIG_BY_TYPE_ELSE_IF(week_calendar)
                        }
        return 0;
}
