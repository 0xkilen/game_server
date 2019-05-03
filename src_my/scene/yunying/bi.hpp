#pragma once

/*#include "proto/data_base.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/data_resource.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_admin.pb.h"
#include "proto/config_shop.pb.h"
#include "proto/ys_base.pb.h"
#include <string>
#include <memory>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;
namespace bpt = boost::property_tree;
namespace py = proto::ys;

namespace nora {

        class service_thread;
        namespace scene {

                class bi {
                public:
                        static bi& instance() {
                                static bi inst;
                                return inst;
                        }
                        bi();
                        void init();
                        void init(const shared_ptr<service_thread>& st);
                        void stop();

                        void set_common_bi_msg(pd::yunying_bi_info* bi, const pd::yunying_client_info& yci, const string& username, const string& ip, uint64_t role, uint64_t league_id);

                        void create_role(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, const string& role_name);
                        void login(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, const string& role_name, uint32_t level, uint32_t zhanli, uint32_t exp, uint32_t diamond, uint32_t gold, const string& gender, uint32_t vip_exp, uint32_t vip_level);
                        void online_count(size_t count);
                        void logout(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, time_t login_time, uint64_t league_id, pd::ce_origin origin, uint32_t level, uint32_t exp, uint32_t diamond, uint32_t gold, uint32_t zhanli, const string& role_name, const string& gender, uint32_t vip_exp, uint32_t vip_level);
                        void levelup(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t cur_level, uint32_t old_level, uint32_t last_levelup_time, uint32_t fl_time);
                        void change_resource(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                         pd::resource_type type, int64_t old_count, int64_t cur_count, uint64_t league_id, pd::ce_origin origin);
                        void add_stuff(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint64_t league_id, pd::ce_origin origin, const string& item, int count,
                                   pd::resource_type cost_resource_type, int cost_resource_count);
                        void dec_stuff(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint64_t league_id, pd::ce_origin origin, const string& item, int count,
                                   uint32_t next_adventure, uint32_t cur_quest);
                        void accept_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint64_t league_id, const string& quest_name);
                        void commit_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, const string& quest_name, uint32_t pttid, uint32_t duration);
                        void challenge_adventure(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                                uint64_t role, uint64_t league_id, uint32_t pttid, uint32_t duration, pd::result result, pd::ce_origin origin, const pd::event_res& event_res);
                        void activity_login_days(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t day);
                        void activity_online_time(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t day, int idx);
                        void activity_seven_days_fuli(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, uint32_t day, uint32_t idx, const pd::event_res& event_res);
                        void activity_seven_days_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, uint32_t day, uint32_t idx, const pd::event_res& event_res);
                        void activity_seven_days_reward(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, const pd::event_res& event_res);
                        void cdkey(const string& username, const string& rolename, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const string& code, const function<void(pd::result, const pd::event_array&, const string&)>& cb);
                        void challenge_arena(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                             uint64_t role, uint64_t league_id, uint32_t duration, pd::result result);
                        void challenge_league_campaign(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                                        uint64_t role, uint64_t league_id, uint32_t pttid, pd::result result);
                        void league_issue_lucky_bag(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t lucky_bag_id);
                        void league_open_lucky_bag(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t lucky_bag_id);
                        void xinshou(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint64_t league_id, pd::ce_origin origin, const string& role_name, uint32_t level, uint32_t zhanli, uint32_t exp, uint32_t diamond);
                        void challenge_lieming(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, pd::ce_origin origin, uint32_t level, uint32_t zhanli, uint32_t pttid, uint32_t duration, pd::yunying_play_id type, const pd::event_res& event_res);

                        void change_virtual_currency(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, pd::resource_type type, int old_count, int cur_count, uint64_t league_id, pd::ce_origin origin);
                        void recharge(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                      uint32_t pttid, uint32_t price, const string& yy_order, uint64_t order, const pd::event_res& event_res);
                        void stuff_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, pd::ce_origin origin,
                                      const string& pttid, uint32_t old_count, uint32_t cur_count);
                        void yunying_add_bi_info(pd::yunying_bi_info* bi, pc::good::good_type type, uint32_t good_id, uint32_t old_count, uint32_t cur_count);
                        void yunying_bi_parse_event_res(pd::yunying_bi_info* bi, pc::good::good_type type, uint32_t shop_id, uint32_t good_id, const pd::event_res& event_res);
                        void shop_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                     uint64_t league_id, pd::yunying_bi_info bi, pc::good::good_type type, uint32_t shop_id,
                                     uint32_t good_id, const pd::event_res& event_res);
                        void gift_exchange(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, const string& pttid, const string& item_name, const string& code);
                        void change_exp(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, pd::resource_type type, int old_count, int cur_count, pd::ce_origin origin);
                        void game_play_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, const string& role_name, uint32_t level, uint32_t zhanli, const string& play_id, const pd::event_res& event_res, uint64_t league_id, pd::ce_origin origin);
                        void game_play_end(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, const string& role_name, uint32_t level, uint32_t zhanli, const string& play_id, const pd::event_res& event_res, uint64_t league_id, pd::ce_origin origin);
                        void order_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint32_t price, uint64_t order);
                        void add_actor(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint32_t actor, pd::ce_origin origin);
                        void actor_add_star(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const pd::event_res& event_res);
                        void actor_levelup(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, pd::ce_origin origin, const pd::event_res_actor_levelup& actor_levelup);
                        void actor_add_pin(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, pd::ce_origin origin,const pd::event_res& event_res);
                        void actor_add_step(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const pd::event_res& event_res);
                        void equipment_compose(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const pd::event_res& event_res);
                        void equipment_merge(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint32_t pttid, const pd::event_res& event_res);
                        void equipment_xilian(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint32_t befor_score, uint32_t after_score, uint64_t equipment, const pd::event_res& event_res);
                        void activity_continue_recharge_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_continue_recharge_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_limit_play_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_limit_play_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_leiji_recharge_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_leiji_recharge_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_leiji_consume_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_leiji_consume_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_prize_wheel_spine(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint64_t league_id, uint32_t activity, pd::admin_activity_type type, const pd::event_res_array& event_array, pd::ce_origin origin);
                        void activity_prize_wheel_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_discount_goods_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_discount_goods_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_festival_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level);
                        void activity_festival_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level);
                        void activity_qianzhuang_buy(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin);
                        void activity_play(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin, uint32_t play_id);
                        void activity_tea_party(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint64_t league_id, uint32_t activity, pd::admin_activity_type type, const pd::event_res& event_res, pd::ce_origin origin);
                        void guanpin_promote(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin);
                        void battle_set_formation(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level,  uint32_t formation_idx);
                        function<void(const string&, const pd::yunying_bi_info&)> send_to_log_server_func_;
                        friend ostream& operator<<(ostream& os, const bi& ch);
                private:
                        void post(const string& addr, const map<string, string>& params, const function<void(pd::result, const bpt::ptree&)>& cb);

                        const string name_ = "bi";
                        shared_ptr<service_thread> st_;
                        FILE *null_;

                        bool send_bi_ = false;
                };
        }
}
*/