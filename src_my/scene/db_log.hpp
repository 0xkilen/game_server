#pragma once

#include "proto/data_log.pb.h"
#include "proto/data_league.pb.h"
#include "db/message.hpp"
#include "db/connector.hpp"
#include "log.hpp"
#include "proto/cs_mansion.pb.h"
#include "proto/cs_arena.pb.h"
#include "proto/data_gongdou.pb.h"
#include "proto/config_options.pb.h"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "proto/data_child.pb.h"
#include "utils/ptts.hpp"

using namespace std;
namespace pd = proto::data;
namespace pcs = proto::cs;

namespace nora {
        class message;
        class connector;

        namespace scene {

	        class role;
                class db_log {
                public:
                        static db_log& instance() {
                                static db_log inst;
                                return inst;
                        }
                        //player log
                        void start();
                        void stop();
                        void log_login(const role& role, const string& ip);
                        void log_logout(const role& role, const string& ip);
                        void log_create_role(const pd::role& data, const role& role);
                        void log_exec_gm(const pd::event_array& ea,
                                         const pd::event_res_array& res_array,
                                         const role& role,
                                         uint32_t pttid = 0,
                                         const pd::league_event_res_array& league_res_array = pd::league_event_res_array(),
                                         const pd::fief_event_res_array& fief_res_array = pd::fief_event_res_array());
                        void log_sweep_adventure(uint32_t pttid, int times, const pd::event_res& event_res, const role& role);
                        void log_collect_adventure_chapter_reward(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_challenge_adventure(pd::result result, uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_challenge_lieming(uint32_t pttid, pd::result result, const pd::event_res& event_res, const role& role);
                        void log_challenge_multi_lieming(uint32_t pttid, pd::result result, const pd::event_res& event_res, const role& role);
                        void log_lottery_draw(const vector<pd::event_res>& er, const role& role);
                        void log_shop_buy(const pd::event_res& event_res, const role& role);
                        void log_relation_remove_friend(uint64_t gid, const role& role);
                        void log_relation_add_friend(uint64_t gid, const role& role);
                        void log_mys_shop_refresh(uint32_t pttid, const pd::event_res& event_res, const pd::mys_shop ms, const role& role);
                        void log_mys_shop_buy(uint32_t pttid, uint32_t good_id, const pd::event_res* er, const role& role);
                        void log_actor_levelup(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_actor_add_star(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_actor_add_step(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_actor_add_pin(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_actor_play_star_plot(uint32_t actor_pttid, uint32_t star, const pd::event_res& event_res, const role& role);
                        void log_quest_commit(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_equipment_merge(uint64_t gid, const pd::event_res& event_res, const role& role);
                        void log_equipment_add_quality(uint64_t gid, const pd::event_res& event_res, const role& role);
                        void log_equipment_change_rand_attr(uint64_t gid, int idx, const pd::event_res& event_res, const role& role);
                        void log_equipment_decompose(const pd::gid_array& gids, const pd::event_res& event_res, const role& role);
                        void log_equipment_compose(uint32_t quality, uint32_t times, const pd::event_res& event_res, const role& role);
                        void log_equipment_xilian(uint64_t gid, const pd::idx_array& lock_idx, const pd::event_res& event_res, const role& role);
                        void log_equipment_xilian_confirm(uint64_t gid, const pd::event_res& event_res, const role& role);
                        void log_item_use(uint32_t pttid, const string& item_name, int count, const pd::event_res& event_res, const role& role);
                        void log_item_change(uint32_t pttid, const string& item_name, int count, pd::ce_origin origin, const role& role, pd::item_change_type type);
                        void log_send_gift(uint64_t gid, uint32_t gift, uint32_t count, const pd::event_res& event_res, const role& role);
                        void log_receive_gift(uint64_t gid, uint32_t gift, uint32_t count, const pd::event_res& event_res, const role& role);
                        void log_send_mail(uint64_t role, uint64_t mail_id, uint32_t server_id, const pd::mail& mail);
                        void log_read_mail(uint64_t role, uint64_t mail_id, uint32_t server_id, const pd::mail& mail);
                        void log_mail_delete(uint64_t role, uint64_t mail_id, uint32_t server_id, const pd::mail& mail);
                        void log_mail_fetch(uint64_t role, const map<uint64_t, uint64_t>& mail_ids, uint32_t server_id, const pd::event_res& event_res);
                        void log_create_league(const pd::league& league, const pd::event_res& event_res, const role& role);
                        void log_dismiss_league(const pd::role_league_data& league_data, const role& role);
                        void log_promote_league_vp(uint64_t member, const role& role);
                        void log_depose_league_vp(uint64_t vp, const role& role);
                        void log_promote_league_president(uint64_t vp, const role& role);
                        void log_kick_league_member(uint64_t member, const role& role);
                        void log_league_impeach(const pd::league_joiners& joiners,  const pd::event_res& event_res, const role& role);
                        void log_league_donate(const pd::event_res& event_res, bool boomed, pd::league::donate_type type, const role& role);
                        void log_league_collect_donation_reward(int idx, const pd::event_res& event_res, const role& role);
                        void log_league_help_quest(const pd::event_res& event_res, const role& role);
                        void log_league_issue_lucky_bag(uint32_t diamond, uint32_t item, const pd::event_res& event_res, const role& role);
                        void log_league_open_lucky_bag(uint32_t diamond, uint32_t item, const pd::event_res& event_res, const role& role);
                        void log_league_lucky_bag_buy_issue(const pd::event_res& event_res, const role& role);
                        void log_league_lucky_bag_buy_open(const pd::event_res& event_res, const role& role);
                        void log_league_item_apply(uint32_t item, const role& role, const pd::event_res& event_res);
                        void log_league_allocate_item(uint32_t item, const role& role);
                        void log_league_active_campaign_stage(uint32_t stage, const role& role);
                        void log_league_mark_campaign_stage(uint32_t stage, const role& role);
                        void log_league_challenge_campaign(uint32_t stage, const pd::battle_team& enemy, uint32_t battle_seed, const pd::event_res& event_res, const role& role);
                        void log_league_cancel_item_apply(uint32_t item, const role& role, const pd::event_res& event_res);
                        void log_league_get_league_war_battle_reward(const role& role, const pd::event_res& event_res);
                        void log_league_get_league_war_rank_reward(const role& role, const pd::event_res& event_res);
                        void log_league_cheer_up_league_war(const role& role, const pd::event_res& event_res);
                        void log_create_fief(const pd::fief& data, const role& role);
                        void log_trigger_fief_incident(uint32_t incident, const pd::event_res& event_res, const pd::fief_event_res& fief_event_res, const role& role);
                        void log_fief_building_levelup(pd::fief_building_type type, uint32_t time, const pd::fief_event_res& event_res, const role& role);
                        void log_fief_building_cancel_levelup(pd::fief_building_type type, const pd::fief_event_res& event_res, const role& role);
                        void log_fief_building_levelup_speed(pd::fief_building_type type, const pd::event_res& event_res, const role& role);
                        void log_fief_challenge_thief(uint64_t gid, const pd::event_res& event_res, const role& role);
                        void log_fief_boss_challenge(uint64_t fief, const pd::battle_team& boss_team, uint32_t battle_seed, int damage, const pd::battle_damage_info& damage_info, int damage_percent, const role& role);
                        void log_fief_pick_up_grow(pd::fief_building_type building, const pd::fief_event_res& event_res, const role& role);
                        void log_fief_open_area_incident(const pd::fief_incident& incident, const role& role);
                        void log_fief_join_area_incident(uint64_t to_role, const pd::fief_incident& incident, const pd::event_res& event_res, const role& role);
                        void log_create_mansion(const pd::mansion& mansion, const role& role);
                        void log_commit_mansion_hall_quest(uint32_t quest, const pd::event_res& event_res, const role& role);
                        void log_add_mansion_furniture(const pcs::mansion_add_furniture_array& furniture, const pd::event_res& event_res, uint32_t fancy, bool by_spouse, const role& role);
                        void log_accept_mansion_quest(uint32_t quest, uint32_t actor, const pd::mansion_quest& accepted_quest, const role& role);
                        void log_commit_mansion_quest (uint64_t quest, uint32_t pttid, pd::actor::craft_type craft, uint32_t actor, const pd::event_res_array& res_array, const role& role);
                        void log_giveup_mansion_quest(uint32_t quest, const role& role);
                        void log_giveup_mansion_other_quest(uint64_t mansion, uint64_t quest, const role& role);
                        void log_help_mansion_quest(uint64_t mansion, const pd::mansion_quest& quest, const pd::event_res& event_res, const role& role);
                        void log_accept_mansion_other_quest(uint64_t mansion, pd::mansion_building_type building, uint32_t actor, const pd::mansion_quest& accepted_quest, const role& role);
                        void log_commit_mansion_other_quest(uint64_t mansion, uint64_t quest, const pd::event_res_array& res_array, const role& role);
                        void log_mansion_banquet_poison(uint64_t mansion, pd::mansion_building_type building, uint32_t poison, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_use_antidote(uint64_t mansion, const pd::event_res& event_res, const role& role);
                        void log_mansion_fishing_get_fish(int level, const pd::event_res& cost_event_res, const pd::event_res& event_res, const role& role);
                        void log_mansion_quick_fishing(uint32_t times, uint32_t drop, const pd::event_res& event_res, const role& role);
                        void log_mansion_visit(uint64_t mansion, const pd::event_res& event_res, const role& role);
                        void log_mansion_start_game(uint64_t other, pd::mansion_game_type game, const pd::event_res& event_res, const role& role);
                        void log_mansion_switch_banquet_dish(uint32_t new_dish, const pd::event_res& event_res, const role& role);
                        void log_mansion_host_banquet(const pd::mansion_banquet& data, const pd::event_res& event_res, const role& role);
                        void log_mansion_reserve_banquet(uint64_t mansion, const pd::event_res& event_res, const role& role);
                        void log_mansion_enter_banquet(uint64_t mansion, pd::mansion_building_type building, const pcs::mansion_banquet_building& banquet_building, bool huanzhuang_level_changed, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_eat_dish(uint64_t mansion, int activity, int atmosphere, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_get_coin(uint64_t mansion, const map<uint32_t, uint32_t>& coins, int activity, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_open_riddle_box(uint64_t mansion, pd::mansion_building_type building, int activity, int atmosphere, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_challenge_thief(uint64_t mansion, int activity, int atmosphere, const pd::battle_damage_info& damage_info, uint32_t battle_seed, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_challenge_celebrity(uint64_t mansion, uint32_t celebrity, int level, uint32_t activity, int atmosphere, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_exchange_item(uint64_t mansion, uint32_t item_idx, int used_activity, const pd::event_res& event_res, const role& role);
                        void log_mansion_banquet_use_firework(uint64_t mansion, uint32_t firework, const pd::event_res& event_res, const role& role);
                        void log_mansion_farm_sow(uint32_t land, uint32_t plant, const pd::event_res& event_res, const role& role);
                        void log_mansion_farm_harvest(uint64_t mansion, uint32_t land, uint32_t plant, uint32_t count, const pd::event_res& event_res, const role& role);
                        void log_mansion_farm_fertilize(uint64_t mansion, uint32_t land, const pd::event_res& event_res, const role& role);
                        void log_mansion_exchange(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_compose(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_pvp_vote(uint64_t vote_to, int votes, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_pvp_upvote(uint64_t vote_to, int upvotes, const pd::event_res& event_res, const role& role);
                        void log_arena_challenge(uint32_t rank, uint32_t battle_seed, const pd::battle_team& other_formation, const pd::battle_damage_info& damage_info, const pcs::arena_challenge_record& record, const pd::event_res& event_res, const role& role);
                        void log_quest_everyday_reward (uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_role_eat(const pd::event_res& event_res, const role& role);
                        void log_role_add_fate(const pd::event_res& event_res, const role& role);
                        void log_role_buy_present(uint32_t present, const pd::event_res& event_res, const role& role);
                        void log_upvote(uint64_t upvotee, uint32_t present_count, const pd::event_res& event_res, const role& role);
                        void log_activity_login_days_reward(uint32_t day, const pd::event_res& event_res, const role& role);
                        void log_activity_online_time_reward(uint32_t idx, const pd::event_res& event_res, const role& role);
                        void log_activity_seven_days_get_fuli(uint32_t begin_days, uint32_t day, uint32_t idx, const pd::event_res& event_res, const role& role);
                        void log_activity_seven_days_commit_quest(uint32_t begin_days, uint32_t day, uint32_t idx, const pd::event_res& event_res, const role& role);
                        void log_activity_seven_days_get_reward(uint32_t begin_days, const pd::event_res& event_res, const role& role);
                        void log_activity_recharge_first_get(const pd::event_res& event_res, const role& role);
                        void log_recharge_day_get(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_recharge_day_bufa(const pd::log_recharge_day_bufa_array& recharge_day_bufa_array, const role& role);
                        void log_recharge_get_vip_box(uint32_t vip_level, const pd::event_res& event_res, const role& role);
                        void log_recharge_get_vip_buy_box(uint32_t vip_level, const pd::event_res& event_res, const role& role);
                        void log_yunying_cdkey(const pd::event_res& event_res, const role& role);
                        void log_marriage_other_accept_propose(uint64_t accepter, const role& role);
                        void log_marriage_answer_question(uint32_t question_idx, uint32_t option_idx, const pd::event_res& event_res, const role& role);
                        void log_marriage_start_star_wish(const string& declaration, const pd::event_res& event_res, const role& role);
                        void log_marriage_star_wish_send_gift(const pd::event_res& event_res, const role& role);
                        void log_marriage_star_wish_select(const pd::event_res& event_res, const role& role);
                        void log_marriage_star_wish_selected(const pd::event_res& event_res, const role& role);
                        void log_marriage_divorce(const pd::event_res& event_res, const role& role);
                        void log_marriage_start_pk(const pd::event_res& event_res, const role& role);
                        void log_marriage_pk_challenge(const pd::event_res& event_res, const role& role);
                        void log_marriage_guesture_round_reward(const pd::event_res& event_res, const role& role);
                        void log_marriage_pk_select(uint64_t target, const pd::event_res& event_res, const role& role);
                        void log_marriage_pk_selected(const pd::event_res& event_res, const role& role);
                        void log_gongdou(pd::gongdou_type type, uint64_t gid, pd::result result, uint64_t target, const pd::event_res& event_res, const role& role);
                        void log_feige_get_reward(uint32_t feige, const pd::event_res& event_res, const role& role);
                        void log_feige_read_chuanwen(uint32_t chuanwen, const pd::event_res& event_res, const role& role);
                        void log_plot_continue(const pd::event_res& event_res, const role& role);
                        void log_xinshou_continue(uint32_t group_pttid, uint32_t pttid, const role& role);
                        void log_guanpin_challenge(uint32_t target_gpin, uint32_t target_idx, const pd::event_res& event_res, const role& role);
                        void log_guanpin_collect_fenglu(uint32_t gpin, uint32_t idx, const pd::event_res& event_res, const role& role);
                        void log_guanpin_baoming(uint32_t gpin, const pd::event_res& event_res, const role& role);
                        void log_guanpin_check_promote(uint32_t gpin, const pd::event_res& event_res, const role& role);
                        void log_guanpin_promote(uint32_t gpin, uint32_t idx, const pd::event_res& event_res, const role& role);
                        void log_guanpin_dianshi_demote(uint32_t gpin, uint32_t idx, const role& role);
                        void log_tower_challenge(uint32_t level, const pd::event_res& event_res, const role& role);
                        void log_tower_refresh(uint32_t level, const pd::event_res& event_res, const role& role);
                        void log_tower_sweep(uint32_t level, const pd::event_res_array& event_array, const role& role);
                        void log_activity_vip_qianzhuang_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_activity_emperor_qianzhuang_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_activity_fund_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_major_city_bubble_award(uint32_t bubble_pttid, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_get_taozhuang_reward(uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_pvp_guess_vote(uint64_t target, int target_votes, int other_votes, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_levelup(uint32_t pttid, pd::huanzhuang_part part, const pd::event_res& event_res, const role& role);
                        void log_huanzhuang_inherit(const pd::huanzhuang_item_id& from, const pd::huanzhuang_item_id& to, const pd::event_res& event_res, const role& role);
                        void log_child_change_phase(uint64_t child, const pd::child_event_res& event_res, const role& role);
                        void log_child_raise(uint64_t child, uint32_t pttid, const pd::event_res& event_res, const role& role);
                        void log_child_study(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_learn_skill(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_add_skill_exp(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_use_grow_item(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_put_on_jade(uint64_t child, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_take_off_jade(uint64_t child, const pd::child_event_res& child_event_res, const role& role);
                        void log_child_change_name(uint64_t child, const pd::event_res& event_res, const role& role);
                        void log_child_change_gender(uint64_t child, const pd::event_res& event_res, const role& role);
                        void log_child_change_type(uint64_t child, const pd::event_res& event_res, const role& role);
                        void log_chat(const string& content, const pd::event_res& event_res, const role& role);

                        //gm log
                        void log_punish_role_by_gid(const role& role, const string& reason, pd::log::log_type log_type, uint32_t punish_time = 0);
                        void log_unpunish_role_by_gid(const role& role, pd::log::log_type log_type);
                        void log_yunying_internal_recharge(const pd::event_res& event_res, const role& role, uint32_t price);
                        void log_yunying_reissue_recharge(uint64_t order, const pd::event_res& events, const role& role);
                        void log_yunying_recharge(uint64_t order, const string& yunying_order, const string& currency, uint32_t price, uint32_t paytime, const string& product_name, const pd::event_res& event_res, const role& role);
                        void log_yunying_send_mail(uint64_t role, uint32_t mail_id, const string& title, const string& content, const pd::event_array& events);
                        void log_currency_record(const role& role, pd::resource_type type, pd::ce_origin origin, int32_t cur_count, pd::currency_change_type change_type);
                        void log_yunying_add_stuff(const pd::event_res& event_res, const role& role);
                        void log_yunying_dec_stuff(const pd::event_res& event_res, const role& role);

                        void fetch_log(const string& procedure, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size, const shared_ptr<service_thread>& st, const function<void(const vector<vector<boost::any>>&)>& cb, const vector<pd::log::log_type>& type);
                private:
                        shared_ptr<db::connector> logdb_;
                        bool write_ = false;
                };

	        inline void db_log::start() {
                        auto ptt = PTTS_GET_COPY(options, 1);
                        logdb_ = make_shared<db::connector>(ptt.log_db().ipport(),
                                                                          ptt.log_db().user(),
                                                                          ptt.log_db().password(),
                                                                          ptt.log_db().database());
                        logdb_->start();
                        write_ = ptt.scene_info().write_dblog();
                }
	        inline void db_log::stop() {
                        if (logdb_) {
                                logdb_->stop();
                                logdb_.reset();
                        }
                }

        }
}
