#pragma once

#include "log.hpp"
#include "scene/ip_logic.hpp"
#include "scene/event.hpp"
#include "scene/yunying/bi.hpp"
#include "utils/gid.hpp"
#include "utils/game_def.hpp"
#include "utils/event_center.hpp"
#include "utils/proto_utils.hpp"
#include "utils/time_utils.hpp"
#include "utils/game_def.hpp"
#include "utils/random_utils.hpp"
#include "utils/service_thread.hpp"
#include "utils/instance_counter.hpp"
#include "proto/data_resource.pb.h"
#include "proto/data_shop.pb.h"
#include "proto/data_relation.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_recharge.pb.h"
#include "proto/data_room.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_xinshou.pb.h"
#include "proto/data_chat.pb.h"
#include "proto/config_shop.pb.h"
#include "config/quest_ptts.hpp"
#include "config/recharge_ptts.hpp"
#include "config/xinshou_ptts.hpp"
#include "config/relation_ptts.hpp"
#include <map>
#include <set>
#include <string>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class item;

                class role : public enable_shared_from_this<role>,
                             private instance_countee {
                public:
                        role() : instance_countee(ICT_ROLE) {
                        }
                        role(uint64_t gid, const pd::yunying_client_info& yci, const string& ip) : instance_countee(ICT_ROLE), gid_(gid), yci_(yci), ip_(ip) {

                        }
                        role(const shared_ptr<service_thread>& st, const pd::yunying_client_info& yci, const string& ip) :
                                instance_countee(ICT_ROLE), st_(st), yci_(yci), ip_(ip) {

                        }
                        void serialize(pd::role *data) const;
                        void serialize_for_client(pd::role *data) const;
                        void serialize_role_info(pd::role_info *data, pd::battle_origin_type bo = pd::BOT_NONE) const;
                        void serialize_role_info_for_client(pd::role_info *info, bool simple) const;
                        void serialize(pd::role_simple_info *data) const;
                        void serialize_for_room(pd::role_info *info, bool init = true) const;
                        bool serialize_changed_data(pd::role *data) const;
                        void serialize_role_data(pd::role_data *data) const;
                        void serialize_changing_data(pd::role_changing_data *data) const;
                        void serialize_other_data(pd::role_other_data *data) const;
                        void serialize_resources(pd::resources *data) const;
                        void serialize_items(pd::item_array *data) const;
                        void serialize_shop(pd::shop *data) const;
                        void serialize_relation(pd::relation *data) const;
                        void serialize_accepted_quest(uint32_t pttid, pd::quest *data) const;
                        void serialize_quests(pd::quests *data) const;
                        void serialize_mys_shop(uint32_t pttid, pd::mys_shop *data) const;
                        void serialize_xinshou(pd::xinshou *data) const;
                        void serialize_yunying(pd::yunying *data) const;
                        void serialize_activity(pd::activity *data) const;
                        void serialize_titles(pd::title_array *data) const;
                        void serialize_recharge(pd::recharge *data) const;
                        void serialize_comments(pd::comment_array *data) const;
                        void serialize_social(pd::social *data) const;
                        void serialize_mails(pd::mail_array *data) const;
                        void serialize_chat_data(pd::chat_data_array *data) const;

                        void parse_from(const pd::role& data);
                        void parse_role_data(const pd::role_data *data);
                        void parse_role_data_by_mirror(const pd::role_data *data);
                        void parse_changing_data(const pd::role_changing_data *data);
                        void parse_other_data(const pd::role_other_data *data);
                        void parse_resources(const pd::resources *data);
                        void parse_items(const pd::item_array *data);
                        void parse_shop(const pd::shop *data);
                        void parse_relation(const pd::relation *relation);
                        void parse_quests(const pd::quests *data);
                        void parse_xinshou(const pd::xinshou *data);
                        void parse_yunying(const pd::yunying *data);
                        void parse_activity(const pd::activity *data);
                        void parse_titles(const pd::title_array *data);
                        void parse_recharge(const pd::recharge *data);
                        void parse_comments(const pd::comment_array *data);
                        void parse_social(const pd::social *data);
                        void parse_mails(const pd::mail_array *data);
                        void parse_chat_data(const pd::chat_data_array *data);
                        void parse_from_mirror(const pd::role& data);

                        void mirror_update(const pd::role& data);

                        pd::lieming_room_role lieming_room_role() const;

                        uint64_t gid() const;
                        bool mirror_role() const;
                        void set_mirror_role();
                        const string& name() const;
                        void change_name(const string& new_name);
                        bool check_former_names_exist(const string& name) const;
                        string username() const;
                        const pd::yunying_client_info& yci() const;
                        const string& ip() const;
                        uint32_t level() const;
                        pd::gender gender() const;
                        uint32_t offline_time() const;
                        void set_icon(int icon);
                        void set_greeting(const pd::voice& greeting);
                        void set_welcome(const string& welcome);
                        void save_kaichang(int32_t kaichang);
                        uint32_t vip_level() const;
                        uint32_t spouse_vip_level() const;
                        void try_levelup();
                        void levelup();
                        uint32_t last_levelup_time() const;
                        uint32_t fate() const;
                        void add_fate();
                        uint32_t share_day() const;
                        void share();
                        void grow_resource(pd::resource_type type);
                        int64_t get_resource(pd::resource_type type) const;
                        int64_t change_resource(pd::resource_type type, int64_t value, bool sync_devotion = false, bool revert = false);
                        void set_resource(pd::resource_type type, int64_t value);
                        uint32_t resource_grow_time(pd::resource_type type) const;
                        void set_resource_grow_time(pd::resource_type type, uint32_t time);
                        void reset_resources();
                        uint32_t most_intimate_actor() const;
                        const map<uint32_t, shared_ptr<actor>>& actors() const;


                        void recharge_update_paid_money(uint32_t money);
                        uint32_t paid_money() const;
                        bool has_item(uint32_t pttid) const;
                        bool has_suit(uint32_t actor_pttid, uint32_t suit_pttid) const;
                        shared_ptr<item> add_item(uint32_t pttid, int count, bool confirmed = false);
                        shared_ptr<item> remove_item(uint32_t pttid, int count);
                        shared_ptr<item> get_item(uint32_t pttid) const;
                        int item_count(uint32_t pttid) const;
                        void update_item_everyday_use_count(uint32_t pttid, int count);
                        int get_item_everyday_use_count(uint32_t pttid) const;
                        void confirm_items();
                        void organize_items();

                        uint32_t shop_buy_times(uint32_t pttid, uint32_t good_id) const;
                        uint32_t shop_buy_all_times(uint32_t pttid, uint32_t good_id) const;
                        struct shop_record {
                                uint32_t shop_day_ = 0;
                                uint32_t times_ = 0;
                                uint32_t all_times_ = 0;
                        };
                        const shop_record& shop_add_buy_times(uint32_t pttid, uint32_t good_id);
                        void reset_shop_records_times(uint32_t pttid, uint32_t good_id);
                        uint32_t shop_record_times(uint32_t pttid, uint32_t good_id) const ;

                        bool relation_has_friend(uint64_t gid) const;
                        int relation_friend_intimacy(uint64_t gid) const;
                        int relation_friend_change_intimacy(uint64_t gid, int value);
                        int relation_friend_change_intimacy_passive(uint64_t gid, int value);
                        void relation_set_friend_intimacy(uint64_t gid, int value);
                        void relation_add_friend(uint64_t gid);
                        bool relation_friend_full() const;
                        void relation_remove_friend(uint64_t gid);
                        size_t relation_friend_count() const;
                        const map<uint64_t, int>& relation_friends() const;
                        const map<uint64_t, uint32_t>& relation_follows() const;
                        bool relation_application_full() const;
                        bool relation_has_application(uint64_t gid) const;
                        void relation_add_application(uint64_t gid, uint64_t& remove_application);
                        void relation_remove_application(uint64_t gid);
                        size_t relation_application_count() const;
                        const list<pd::relation_application>& relation_applications() const;
                        bool relation_in_blacklist(uint64_t gid) const;
                        void relation_add_in_blacklist(uint64_t gid);
                        const map<uint64_t, uint32_t>& relation_blacklist() const;
                        bool relation_blacklist_full();
                        void relation_remove_from_blacklist(uint64_t gid);
                        size_t relation_blacklist_count() const;
                        bool relation_in_followlist(uint64_t gid) const;
                        void relation_add_in_followlist(uint64_t gid);
                        bool relation_followlist_full() const;
                        void relation_remove_from_followlist(uint64_t gid);

                        struct quest_target_counter {
                                pd::quest_target_type type_;
                                uint32_t cur_ = 0;
                                pd::quest_param param_;
                                vector<pd::quest_param> params_;
                                int32_t cur_param_index_ = 0;
                        };
                        struct accepted_quest {
                                vector<quest_target_counter> counters_;
                                map<ec_event, list<list<event_handler>::iterator>> event_handlers_;
                                uint32_t accept_time_ = 0;
                                bool registered_ = false;
                        };
                        bool quest_has_accepted(uint32_t pttid) const;
                        const map<uint32_t, accepted_quest>& quest_accepted_quests() const;
                        accepted_quest& get_accepted_quest(uint32_t pttid);
                        void quest_accept(uint32_t pttid, bool register_event = true, uint32_t accept_time = 0);
                        void quest_register_event(uint32_t pttid, bool not_parse = false);
                        void quest_unregister_event(uint32_t pttid);
                        pd::result quest_check_commit(uint32_t pttid) const;
                        void quest_commit(uint32_t pttid);
                        void quest_remove(uint32_t pttid);
                        void quest_helped_by_other(uint32_t pttid, uint32_t cur_need);
                        void quest_move_to_next_param(uint32_t pttid);
                        void quest_commit_forever_quest(uint32_t quest);
                        bool quest_check_reset_cur(const pc::quest_param& a, const pc::quest_param& b);
                        pd::event_array get_quest_cur_pass_events(uint32_t pttid);
                        void refresh_everyday_quests(pd::event_res& event_res);
                        bool got_everyday_quest_reward(uint32_t pttid) const;
                        void add_got_everyday_quest_reward(uint32_t pttid);
                        bool quest_committed(uint32_t pttid) const;
                        void quest_remove_commited_quest(uint32_t pttid);
                        void update_mys_shop(const pd::mys_shop& ms);
                        bool has_mys_shop_good(uint32_t pttid, uint32_t good_id) const;
                        void bought_mys_shop_good(uint32_t pttid, uint32_t good_id);
                        uint32_t mys_shop_free_refreshed_time(uint32_t pttid) const;

                        void login();
                        void logout();
                        void on_new_day();
                        uint32_t today_online_seconds() const;
                        uint32_t last_login_time() const;
                        uint32_t first_login_time() const;

                        void xinshou_add_passed_group(uint32_t group_pttid);
                        bool xinshou_group_passed(uint32_t group_pttid) const;
                        const set<uint32_t>& xinshou_passed_groups() const;
                        uint32_t xinshou_guide() const;

                        uint32_t gag_until_time() const;
                        string gag_reason() const;
                        void set_gag_until_time(uint32_t gag_until_time);
                        void set_gag_reason(const string& reason);
                        uint32_t ban_until_time() const;
                        string ban_reason() const;
                        void set_ban_until_time(uint32_t ban_until_time);
                        void set_ban_reason(const string& reason);

                        void relation_application_set_read(const pd::gid_array& roles);
                        const set<uint64_t>& relation_saved_suggestion() const;
                        void relation_save_suggestion(const set<uint64_t>& roles);
                        void relation_add_helper(uint64_t helper);
                        const set<uint64_t>& relation_helpers() const;
                        void relation_add_teammate(uint64_t teammate);
                        const set<uint64_t>& relation_teammates() const;
                        pd::record_entity relation_receive_gift(uint64_t role, uint32_t gift, uint32_t count);
                        uint32_t login_days() const;
                        bool activity_login_days_got_reward(uint32_t day) const;
                        void activity_login_days_add_got_reward(uint32_t day);

                        bool activity_online_time_got_reward(int idx) const;
                        void activity_online_time_add_got_reward(uint32_t idx);

                        bool activity_seven_days_fuli_quests_accepted(uint32_t begin_day) const;
                        void activity_seven_days_set_fuli_quests_accepted(uint32_t begin_day);
                        bool activity_seven_days_quests_accepted(uint32_t begin_day) const;
                        void activity_seven_days_set_quests_accepted(uint32_t begin_day);
                        void activity_seven_days_update_fuli_quests(pd::event_res& event_res);
                        void activity_seven_days_update_quests(pd::event_res& event_res);
                        bool activity_seven_days_got_reward(uint32_t begin_day) const;
                        void activity_seven_days_set_got_reward(uint32_t begin_day);
                        bool activity_seven_days_passed(uint32_t begin_day);
                        void activity_seven_days_set_passed(uint32_t begin_day);
                        void activity_continue_recharge_update_quests(pd::event_res& event_res);
                        void activity_continue_recharge_clear_quests(pd::event_res& event_res, bool sync_client = false);
                        void activity_limit_play_update_quests(pd::event_res& event_res);
                        void activity_leiji_recharge_update_quests(pd::event_res& event_res);
                        void activity_leiji_consume_update_quests(pd::event_res& event_res);
                        pd::event_res activity_stop_limit_play();
                        void activity_stop_continue_recharge(pd::event_res& event_res);
                        pd::event_res activity_stop_leiji_recharge();
                        pd::event_res activity_stop_leiji_consume();

                        bool has_activity_qiandao(uint32_t month, uint32_t day) const;
                        void activity_add_qiandao(uint32_t qiandao_month, uint32_t qiandao_mday);
                        int get_activity_qiandao_times(uint32_t month) const;
                        void activity_add_leiqian(uint32_t month, uint32_t day);

                        bool has_activity_qiandao_leiqian_reward(uint32_t month, uint32_t day) const;
                        bool activity_bought_everyday_libao_today(uint32_t pttid) const;
                        void activity_set_bought_everyday_libao_today(uint32_t pttid);
                        void activity_add_chaozhi_libao(pd::activity_chaozhi_libao_type libao_type, int libao_idx);
                        int activity_get_chaozhi_libao_bug_count(pd::activity_chaozhi_libao_type libao_type) const;
                        bool activity_first_recharge() const;
                        void update_activity_first_recharge_get_reward();
                        void activity_update_leiji_consume(uint32_t count);
                        void activity_recharge_rank_update();
                        void activity_clear_leiji_consume();
                        uint32_t activity_leiji_consume() const;
                        void activity_update_leiji_recharge(uint32_t money);
                        void activity_clear_leiji_recharge();
                        uint32_t activity_leiji_recharge() const;
                        void activity_set_vip_buy();
                        bool activity_vip_buy() const;
                        void activity_set_emperor_buy();
                        bool activity_emperor_buy() const;
                        void activity_update_emperor_got(uint32_t pttid);
                        bool activity_emperor_got(uint32_t pttid) const;
                        void activity_update_vip_got(uint32_t pttid);
                        bool activity_vip_got(uint32_t pttid) const;
                        void activity_update_fund_got(uint32_t pttid);
                        bool activity_fund_got(uint32_t pttid) const;
                        void activity_update_limit_libao(uint32_t pttid);
                        void activity_add_limit_libao_bought(uint32_t pttid);
                        uint32_t activity_limit_libao_cur() const;
                        uint32_t activity_limit_libao_time() const;
                        const map<uint32_t, uint32_t>& activity_limit_libao_passed() const;
                        bool activity_bought_limit_libao(uint32_t pttid) const;
                        uint32_t activity_festival_exchange_times(uint32_t pttid, uint32_t exchange) const;
                        void update_activity_festival_exchange_times(uint32_t pttid, uint32_t exchange);
                        void reset_activity_festival_exchange_times();
                        uint32_t activity_prize_wheel_start_day() const;
                        uint32_t activity_prize_wheel_duration() const;
                        uint32_t activity_prize_wheel_pttid() const;
                        bool activity_prize_wheel_ongoing() const;
                        void activity_start_prize_wheel(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        void activity_prize_wheel_check_start(pd::event_res& event_res);
                        void activity_prize_wheel_check_finish(pd::event_res& event_res, bool force = false);
                        uint32_t activity_tea_party_start_day() const;
                        uint32_t activity_tea_party_duration() const;
                        uint32_t activity_tea_party_pttid() const;
                        bool activity_tea_party_ongoing() const;
                        void activity_start_tea_party(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        void activity_tea_party_check_start(pd::event_res& event_res);
                        void activity_tea_party_check_finish(pd::event_res& event_res, bool force = false);
                        bool activity_tea_party_got_favor_reward(uint32_t idx) const;
                        void activity_tea_party_add_got_favor_reward(uint32_t idx);
                        void activity_reset_discount_goods(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        pd::result check_activity_discount_goods(uint32_t pttid, const pc::good& good) const;
                        uint32_t activity_discount_goods_start_day() const ;
                        uint32_t activity_discount_goods_duration() const;
                        uint32_t activity_continue_recharge_start_day() const;
                        uint32_t activity_continue_recharge_duration() const;
                        uint32_t activity_continue_recharge_pttid() const;
                        uint32_t activity_continue_recharge_cur_quest() const;
                        void activity_continue_recharge_set_cur_quest(uint32_t quest, bool add_idx = true);
                        uint32_t activity_continue_recharge_quest_idx() const ;
                        void activity_reset_continue_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        void activity_continue_recharge_update_dayrecharge(uint32_t price);
                        void activity_continue_recharge_reset_dayrecharge();
                        bool activity_daiyanren_got_reward(uint32_t idx) const;
                        void activity_daiyanren_set_got_reward(uint32_t idx);
                        bool activity_daiyanren_finished() const;
                        void activity_daiyanren_set_finished();
                        uint32_t activity_limit_play_start_day() const;
                        uint32_t activity_limit_play_duration() const;
                        uint32_t activity_limit_play_pttid() const;
                        void activity_reset_limit_play(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        uint32_t activity_leiji_recharge_start_day() const;
                        uint32_t activity_leiji_recharge_duration() const;
                        uint32_t activity_leiji_recharge_pttid() const;
                        void activity_reset_leiji_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        uint32_t activity_leiji_consume_start_day() const;
                        uint32_t activity_leiji_consume_duration() const;
                        uint32_t activity_leiji_consume_pttid() const;
                        void activity_reset_leiji_consume(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        uint32_t activity_recharge_rank_start_day() const;
                        uint32_t activity_recharge_rank_duration() const;
                        void activity_reset_recharge_rank(uint32_t start_day, uint32_t duration);
                        uint32_t activity_festival_start_day() const;
                        uint32_t activity_festival_duration() const;
                        uint32_t activity_festival_pttid() const;
                        void activity_reset_festival(uint32_t start_day, uint32_t duration, uint32_t pttid);
                        pd::event_res activity_stop_festival();
                        void activity_festival_update_quests(pd::event_res& event_res);

                        void accept_ui_quest();
                        void accept_achievement_quest();
                        void set_quest_counter(quest_target_counter& counter, uint32_t quest, uint32_t value);
                        void change_quest_counter(quest_target_counter& counter, uint32_t quest, uint32_t value);
                        void clear_quest_counter(uint32_t quest, bool sync_client = false);
                        void exec_chat_gm_pass_quest(uint32_t quest, uint32_t cur);
                        bool has_title(uint32_t pttid) const;
                        void add_title(uint32_t pttid);
                        void remove_title(uint32_t pttid);
                        void select_title(uint32_t pttid);
                        void title_set_read(uint32_t pttid);
                        const pd::title& get_title(uint32_t pttid) const;
                        void add_title_timers();
                        bool has_recharge_order(uint64_t gid) const;
                        bool recharge_order_recharged(uint64_t gid) const;
                        uint32_t recharge_order_pttid(uint64_t gid) const;
                        uint64_t add_recharge_order(uint32_t pttid);
                        void recharge_order_done(uint64_t gid);
                        bool has_recharged(uint32_t pttid) const;
                        bool has_recharged() const;
                        bool has_recharge_day(uint32_t pttid) const;
                        bool recharge_day_vaild() const;
                        uint32_t recharge_day(uint32_t pttid) const;
                        void update_recharge_day(uint32_t pttid, uint32_t ptt_day);
                        const map<uint32_t, uint32_t>& recharge_days() const;
                        uint32_t recharge_last_process_day(uint32_t pttid) const;
                        void recharge_update_last_process_day(uint32_t pttid, uint32_t day);
                        bool recharge_got_vip_box(uint32_t box) const;
                        void recharge_set_got_vip_box(uint32_t box);
                        bool recharge_got_vip_buy_box(uint32_t box) const;
                        void recharge_set_got_vip_buy_box(uint32_t box);

                        uint32_t last_comment_time(uint64_t role) const;
                        bool has_comment(uint64_t gid) const;
                        const pd::comment& get_comment(uint64_t gid) const;
                        void add_comment(uint64_t from, uint64_t reply_to, const string& content, uint64_t& gid);
                        void add_comment(uint64_t from, uint32_t gift, uint32_t count, uint64_t& gid);
                        void delete_comment(uint64_t gid);
                        bool has_present() const;
                        uint32_t get_present() const;
                        int present_count(uint32_t present) const;
                        void add_present(uint32_t present, int count);
                        void dec_present(uint32_t present, int count);

                        void add_mail(const pd::mail& mail);
                        bool mail_already_read(uint64_t gid) const;
                        void mail_set_already_read(uint64_t gid);
                        bool has_mail(uint64_t gid) const;
                        const pd::mail& get_mail(uint64_t gid) const;
                        const map<uint64_t, pd::mail>& mails() const;
                        void del_mail(uint64_t gid);
                        bool mail_can_fetch(uint64_t gid) const;
                        pd::event_array fetch_mail(uint64_t gid);

                        uint32_t room_last_invite_time(uint64_t role) const;
                        uint32_t room_last_inviter_time(uint64_t role) const;
                        void room_update_last_invite_time(uint64_t role, uint32_t time);
                        void room_clear_last_invite_time();
                        map<uint64_t, uint32_t> room_inviters() const;
                        void room_set_inviter(uint64_t role, uint32_t time);
                        void room_delete_inviter(uint64_t role);
                        bool room_has_inviter(uint64_t role) const;
                        void room_delete_invited(uint64_t role);

                        void broadcast_system_chat(uint32_t pttid);

                        void update_chat_data(pd::chat_type type);
                        uint32_t get_last_chat_time(pd::chat_type type);


                        void on_event(nora::ec_event event, const vector<boost::any>& args);
                        void register_event(nora::ec_event event, const function<void(const vector<boost::any>&)>& cb);
                        void unregister_event_by_type(nora::ec_event event);

                        void yunying_set_item(uint32_t pttid, uint32_t count);

#define DEFINE_DATA_CHANGE_FUNCS(type) bool type##_changed_ = false;    \
                        bool type##_changed() const {                   \
                                return type##_changed_;                 \
                        }                                               \
                        void reset_##type##_changed() {                 \
                                type##_changed_ = false;                \
                        }
                        DEFINE_DATA_CHANGE_FUNCS(role_data);
                        DEFINE_DATA_CHANGE_FUNCS(other_data);
                        DEFINE_DATA_CHANGE_FUNCS(resources);
                        DEFINE_DATA_CHANGE_FUNCS(items);
                        DEFINE_DATA_CHANGE_FUNCS(shop);
                        DEFINE_DATA_CHANGE_FUNCS(relation);
                        DEFINE_DATA_CHANGE_FUNCS(quests);
                        DEFINE_DATA_CHANGE_FUNCS(xinshou);
                        DEFINE_DATA_CHANGE_FUNCS(yunying);
                        DEFINE_DATA_CHANGE_FUNCS(activity);
                        DEFINE_DATA_CHANGE_FUNCS(recharge);
                        DEFINE_DATA_CHANGE_FUNCS(comments);
                        DEFINE_DATA_CHANGE_FUNCS(mails);
                        DEFINE_DATA_CHANGE_FUNCS(chat_data);

                        void reset_data_changed();

                        function<string(uint64_t)> gid2rolename_func_;
                        function<uint32_t()> server_open_day_func_;
                        function<string()> username_func_;
                        function<void(uint64_t, uint64_t, int)> friend_intimacy_changed_cb_;
                        function<void(uint64_t, const pd::quest&)> quest_update_func_;
                        function<void(uint64_t, const pd::quest&, const pd::event_res& event_res)> forever_quest_update_func_;
                        function<void(uint64_t, uint32_t)> quest_auto_commit_func_;
                        function<void(uint64_t, uint32_t, uint32_t, const pd::event_res&, bool)> levelup_cb_;
                        function<void()> update_db_func_;
                        function<void(const pd::event_res&, const map<uint32_t,uint32_t>&)> on_new_day_cb_;
                        function<void(uint64_t, uint64_t, uint64_t)> vip_exp_sync_to_spouse_func_;
                        function<void(uint64_t, pd::record_type, int, const pd::record_entity&)> role_add_new_record_notice_;
                        function<void(uint64_t, uint32_t)> broadcast_system_chat_func_;
                        function<void(uint32_t, const pd::event_array&)> send_mail_func_;

                        friend ostream& operator<<(ostream& os, const role& role) {
                                return os << "<role:" << role.gid2rolename_func_(role.gid_) << ">";
                        }
                private:
                        void reset_activity_qiandao();
                        void clean_up_comment();
                        void other_system_update_role(uint32_t pttid);

                        uint64_t gid_ = 0;
                        bool mirror_role_ = false;
                        struct {
                                string name_;
                                string username_;
                                set<string> former_names_;
                                pd::gender gender_;
                                uint32_t level_ = 1;
                                uint32_t last_levelup_time_ = 0;
                                int icon_;
                                vector<string> custom_icons_;
                                int32_t custom_icon_ = -1;
                                uint32_t offline_time_ = 0;
                                uint32_t fate_ = 0;
                                uint32_t last_login_time_ = 0;
                                uint32_t first_login_time_ = 0;
                                uint32_t last_online_day_ = 0;
                                uint32_t login_days_ = 0;
                                uint32_t today_online_seconds_ = 0;
                                uint32_t today_intimate_actor_ = 0;
                                pd::voice greeting_;
                                string welcome_;
                                int32_t kaichang_ = 0;
                                uint32_t channel_id_ = 0;
                        } data_;
                        pd::role_other_data other_data_;
                        struct {
                                map<pd::resource_type, int64_t> values_;
                                uint32_t reset_day_ = 0;
                                int reset_monday_ = 0;
                                map<pd::resource_type, uint32_t> grow_time_;
                        } resources_;
                        struct {
                                set<uint32_t> passed_groups_;
                        } xinshou_;
                        map<uint32_t, shared_ptr<item>> pttid2item_;
                        map<uint32_t, map<uint32_t, int>> day2used_item_counts_;
                        vector<uint32_t> unorganized_items_;
                        map<uint32_t, lottery> lotteries_;
                        struct {
                                map<uint32_t, pd::mys_shop> mys_shops_;
                                map<uint32_t, map<int, shop_record>> records_;
                        } shop_;
                        struct relation{
                                map<uint64_t, int> friends_;
                                map<uint64_t, list<pd::relation_application>::iterator> applications_iters_;
                                list<pd::relation_application> applications_;
                                map<uint64_t, uint32_t> blacklist_;
                                map<uint64_t, uint32_t> follows_;
                                set<uint64_t> saved_suggestion_;
                                set<uint64_t> helpers_;
                                set<uint64_t> teammates_;
                        } relation_;
                        struct {
                                map<uint32_t, accepted_quest> accepted_;
                                set<uint32_t> passed_;
                                uint32_t everyday_quest_day_ = 0;
                                int everyday_quest_monday_ = 0;
                                set<uint32_t> finished_everyday_quests_;
                                set<uint32_t> got_everyday_quest_rewards_;
                                set<uint32_t> received_achievement_;
                                map<pd::quest_target_type, quest_target_counter> achievement_stats_;
                        } quest_;
                        pd::yunying yunying_;
                        struct {
                                struct {
                                        set<uint32_t> got_reward_days_;
                                } login_days_;
                                struct {
                                        uint32_t got_reward_day_ = 0;
                                        set<int> got_reward_idx_;
                                } online_time_;
                                struct one_seven_days {
                                        bool got_reward_ = false;
                                        bool quests_accepted_ = false;
                                        bool fuli_quests_accepted_ = false;
                                        bool passed_ = false;
                                };
                                map<uint32_t, one_seven_days> seven_days_;
                                bool reset_point_ = false;

                                struct {
                                        uint32_t month_ = 0;
                                        set<uint32_t> mdays_;
                                        set<uint32_t> got_leiqian_days_;
                                } qiandao_;

                                map<uint32_t, uint32_t> everyday_libao_;
                                map<pd::activity_chaozhi_libao_type, int> chaozhi_libao_idxs_;
                                bool recharge_get_first_reward_ = false;
                                uint32_t leiji_consume_count_ = 0;
                                uint32_t leiji_recharge_count_ = 0;

                                struct {
                                        bool vip_buy_ = false;
                                        bool emperor_buy_ = false;
                                        set<uint32_t> vip_got_;
                                        set<uint32_t> emperor_got_;
                                        set<uint32_t> fund_got_;
                                } qianzhuang_;

                                struct {
                                        set<uint32_t> bought_;
                                        map<uint32_t, uint32_t> passed_;
                                        uint32_t cur_ = 0;
                                        uint32_t time_ = 0;
                                } limit_libao_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                        map<uint32_t, map<uint32_t, uint32_t>> festival_exchange_times_;
                                        // map<uint32_t, map<uint32_t, uint32_t>> festival_good_times_;
                                } festival_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                        bool ongoing_ = false;
                                } prize_wheel_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                        set<uint32_t> got_favor_rewards_;
                                        bool ongoing_ = false;
                                } tea_party_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                } discount_goods_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                        uint32_t cur_quest_ = 0;
                                        uint32_t quest_idx_ = 0;
                                        uint32_t update_quests_activate_day_ = 0;
                                        uint32_t day_recharge_ = 0;
                                } continue_recharge_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                } limit_play_;
                                struct {
                                        set<uint32_t> got_reward_;
                                        bool finished_ = false;
                                } daiyanren_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                } leiji_recharge_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                        uint32_t pttid_ = 0;
                                } leiji_consume_;
                                struct {
                                        uint32_t start_day_ = 0;
                                        uint32_t duration_ = 0;
                                } recharge_rank_;
                        } activity_;
                        struct {
                                map<uint32_t, pd::title> titles_;
                                uint32_t selected_ = 0;
                                map<uint32_t, shared_ptr<timer_type>> timers_;
                        } title_;
                        struct {
                                map<uint64_t, uint32_t> gid2pttid_;
                                map<uint32_t, set<uint64_t>> pttid2gids_;
                                set<uint32_t> recharged_pttids_;
                                map<uint32_t, uint32_t> pttid2day_;
                                map<uint32_t, uint32_t> pttid2last_process_day_;
                                set<uint32_t> got_vip_boxes_;
                                set<uint32_t> got_vip_buy_boxes_;
                                uint32_t paid_money_ = 0;
                                set<uint64_t> recharged_gids_;
                        } recharge_;
                        struct {
                                map<uint64_t, pd::comment> gid2comment_;
                                list<pd::comment> comment_list_;
                                map<uint64_t, uint32_t> role2last_comment_time_;
                                uint64_t gid_seed_ = 1;
                        } comments_;
                        map<uint64_t, pd::mail> mails_;
                        struct {
                                map<uint64_t, uint32_t> invited_;
                                map<uint64_t, uint32_t> inviters_;
                        } room_data_;

                        struct {
                                map<pd::chat_type, uint32_t> type2time_;
                        } chat_data_;
                        struct {
                                uint64_t gid_seed_ = 1ull;
                                struct record_one {
                                        bool read_ = false;
                                        list<pd::record_entity> records_;
                                };
                                map<pd::record_type, map<int, record_one>> records_;
                        } records_;

                        event_center ec_;
                        shared_ptr<service_thread> st_;
                        pd::yunying_client_info yci_;
                        string ip_;
                };

                inline uint64_t role::gid() const {
                        return gid_;
                }

                inline bool role::mirror_role() const {
                        return mirror_role_;
                }

                inline void role::set_mirror_role() {
                        mirror_role_ = true;
                }

                inline string role::username() const {
                        return username_func_();
                }

                inline const pd::yunying_client_info& role::yci() const {
                        return yci_;
                }

                inline const string& role::ip() const {
                        return ip_;
                }

                inline void role::update_chat_data(pd::chat_type type) {
                        chat_data_.type2time_[type] = system_clock::to_time_t(system_clock::now());
                        chat_data_changed_ = true;
                        SPLAYER_TLOG << gid_ << " update chat data " << pd::chat_type_Name(type);
                }

                inline uint32_t role::get_last_chat_time(pd::chat_type type) {
                        if (chat_data_.type2time_.count(type) > 0) {
                                return chat_data_.type2time_.at(type);
                        }
                        return 0u;
                }

                inline void role::xinshou_add_passed_group(uint32_t group_pttid) {
                        if (xinshou_.passed_groups_.count(group_pttid) > 0) {
                                return;
                        }
                        xinshou_.passed_groups_.insert(group_pttid);
                        xinshou_changed_ = true;
                }

                inline bool role::xinshou_group_passed(uint32_t group_pttid) const {
                        return xinshou_.passed_groups_.count(group_pttid) > 0;
                }

                inline const set<uint32_t>& role::xinshou_passed_groups() const {
                        return xinshou_.passed_groups_;
                }

                inline uint32_t role::xinshou_guide() const {
                        return gender() == pd::MALE ? 2 : 1;
                }

                inline const string& role::name() const {
                        return data_.name_;
                }

                inline bool role::check_former_names_exist(const string& name) const {
                        return data_.former_names_.count(name) > 0 ? true : false;
                }

                inline uint32_t role::level() const {
                        return data_.level_;
                }

                inline uint32_t role::offline_time() const {
                        return data_.offline_time_;
                }

                inline pd::gender role::gender() const {
                        return data_.gender_;
                }

                inline void role::set_icon(int icon) {
                        if (data_.icon_ != icon) {
                                data_.icon_ = icon;
                                role_data_changed_ = true;
                        }
                }

                inline void role::change_custom_icon(const pd::custom_icon& custom_icon) {
                        if (custom_icon.has_icon()) {
                                data_.custom_icons_[custom_icon.idx()] = custom_icon.icon();
                        } else {
                                if (custom_icon.idx() == data_.custom_icon_) {
                                        data_.custom_icon_ = -1;
                                }
                                data_.custom_icons_[custom_icon.idx()] = "";
                        }
                        role_data_changed_ = true;
                }

                inline void role::set_custom_icon_select(int32_t custom_icon_select) {
                        if (custom_icon_select >= 0) {
                                data_.custom_icon_ = custom_icon_select;
                                role_data_changed_ = true;
                        }
                }

                inline void role::set_greeting(const pd::voice& greeting) {
                        if (data_.greeting_ != greeting) {
                                data_.greeting_ = greeting;
                                role_data_changed_ = true;
                        }
                }

                inline void role::set_welcome(const string& welcome) {
                        if (data_.welcome_ != welcome) {
                                data_.welcome_ = welcome;
                                role_data_changed_ = true;
                        }
                }

                inline void role::save_kaichang(int32_t kaichang) {
                        if (data_.kaichang_ != kaichang) {
                                data_.kaichang_ = kaichang;
                                role_data_changed_ = true;
                        }
                }

                inline void role::levelup() {
                        data_.level_ += 1;
                        role_data_changed_ = true;
                        on_event(ECE_LEVELUP, {});
                        other_system_update_role(role_actor_->pttid());
                }

                inline bool role::has_unlock_grid(const pd::grid& grid) {
                        if (battle_.grids_.count(grid.row()) <= 0) {
                                return false;
                        }
                        if (battle_.grids_.at(grid.row()).count(grid.col()) <= 0) {
                                return false;
                        }
                        return true;
                }

                inline void role::add_unlocked_grid(const pd::grid& grid) {
                        battle_.grids_[grid.row()].insert(grid.col());
                        battle_changed_ = true;
                }

                inline bool role::relation_has_friend(uint64_t gid) const {
                        return relation_.friends_.count(gid) > 0;
                }

                inline int role::relation_friend_intimacy(uint64_t gid) const {
                        ASSERT(relation_.friends_.count(gid) > 0);
                        return relation_.friends_.at(gid);
                }

                inline int role::relation_friend_change_intimacy(uint64_t gid, int value) {
                        ASSERT(relation_.friends_.count(gid) > 0);
                        int ret = relation_.friends_.at(gid) + value;
                        relation_.friends_.at(gid) = ret;
                        relation_changed_ = true;
                        friend_intimacy_changed_cb_(gid_, gid, value);
                        on_event(ECE_RELATION_FRIEND_INTIMACY, { ret });
                        return ret;
                }

                inline int role::relation_friend_change_intimacy_passive(uint64_t gid, int value) {
                        ASSERT(relation_.friends_.count(gid) > 0);
                        int ret = relation_.friends_.at(gid) + value;
                        relation_.friends_.at(gid) = ret;
                        relation_changed_ = true;
                        on_event(ECE_RELATION_FRIEND_INTIMACY, { ret });
                        return ret;
                }

                inline void role::relation_add_friend(uint64_t gid) {
                        ASSERT(relation_.friends_.count(gid) == 0);
                        relation_.friends_[gid] = 0;
                        if (relation_.applications_iters_.count(gid) > 0) {
                                relation_remove_application(gid);
                        }
                        on_event(ECE_ADD_FRIEND, {});
                        relation_changed_ = true;
                }

                inline bool role::relation_friend_full() const {
                        const auto& ptt = PTTS_GET(relation_logic, 1);
                        return relation_.friends_.size() >= ptt.relation_max().friends();
                }

                inline void role::relation_remove_friend(uint64_t gid) {
                        relation_.friends_.erase(gid);
                        on_event(ECE_REMOVE_FRIEND, {});
                        relation_changed_ = true;
                }

                inline size_t role::relation_friend_count() const {
                        return relation_.friends_.size();
                }

                inline const map<uint64_t, int>& role::relation_friends() const {
                        return relation_.friends_;
                }

                inline const map<uint64_t, uint32_t>& role::relation_follows() const {
                        return relation_.follows_;
                }

                inline bool role::relation_application_full() const {
                        const auto& ptt = PTTS_GET(relation_logic, 1);
                        return relation_.applications_iters_.size() > ptt.relation_max().apply();
                }

                inline bool role::relation_has_application(uint64_t gid) const {
                        return relation_.applications_iters_.count(gid) > 0;
                }

                inline void role::relation_add_application(uint64_t gid, uint64_t& remove_application) {
                        pd::relation_application application;
                        application.set_role(gid);
                        application.set_time(system_clock::to_time_t(system_clock::now()));
                        relation_.applications_.push_back(application);
                        relation_.applications_iters_[gid] = --relation_.applications_.end();

                        while (relation_application_full()) {
                                ASSERT(!relation_.applications_.empty());
                                remove_application = relation_.applications_.front().role();
                                relation_remove_application(remove_application);
                        }
                        relation_changed_ = true;
                }

                inline void role::relation_remove_application(uint64_t gid) {
                        ASSERT(relation_.applications_iters_.count(gid) > 0);
                        relation_.applications_.erase(relation_.applications_iters_.at(gid));
                        relation_.applications_iters_.erase(gid);
                        relation_changed_ = true;
                }

                inline size_t role::relation_application_count() const {
                        return relation_.applications_iters_.size();
                }

                inline const list<pd::relation_application>& role::relation_applications() const {
                        return relation_.applications_;
                }

                inline bool role::relation_in_blacklist(uint64_t gid) const {
                        return relation_.blacklist_.count(gid) > 0;
                }

                inline void role::relation_add_in_blacklist(uint64_t gid) {
                        ASSERT(relation_.blacklist_.count(gid) <= 0);
                        if (relation_.friends_.count(gid) > 0) {
                                relation_.friends_.erase(gid);
                                on_event(ECE_REMOVE_FRIEND, {});
                        } else if (relation_.applications_iters_.count(gid) > 0) {
                                relation_remove_application(gid);
                        }
                        if (relation_.follows_.count(gid) > 0) {
                              relation_.follows_.erase(gid);
                        }
                        relation_.blacklist_[gid] = system_clock::to_time_t(system_clock::now());
                        relation_changed_ = true;
                }

                inline const map<uint64_t, uint32_t>& role::relation_blacklist() const {
                        return relation_.blacklist_;
                }

                inline bool role::relation_blacklist_full() {
                        const auto& ptt = PTTS_GET(relation_logic, 1);
                        return relation_.blacklist_.size() >= ptt.relation_max().blacklist();
                }

                inline void role::relation_remove_from_blacklist(uint64_t gid) {
                        ASSERT(relation_.blacklist_.count(gid) > 0);
                        relation_.blacklist_.erase(gid);
                        relation_changed_ = true;
                }

                inline size_t role::relation_blacklist_count() const {
                        return relation_.blacklist_.size();
                }

                inline bool role::relation_in_followlist(uint64_t gid) const {
                        return relation_.follows_.count(gid) > 0;
                }

                inline void role::relation_add_in_followlist(uint64_t gid) {
                        ASSERT(relation_.follows_.count(gid) <= 0);
                        relation_.follows_[gid] = system_clock::to_time_t(system_clock::now());
                        relation_changed_ = true;
                }

                inline bool role::relation_followlist_full() const {
                        const auto& ptt = PTTS_GET(relation_logic, 1);
                        return relation_.follows_.size() >= ptt.relation_max().follow();
                }

                inline void role::relation_remove_from_followlist(uint64_t gid) {
                        ASSERT(relation_.follows_.count(gid) > 0);
                        relation_.follows_.erase(gid);
                        relation_changed_ = true;
                }

                inline bool role::mail_can_fetch(uint64_t gid) const {
                        ASSERT(mails_.count(gid) > 0);
                        const auto& m = mails_.at(gid);
                        return m.events().events_size() > 0 && !m.fetched();
                }

                inline pd::event_array role::fetch_mail(uint64_t gid) {
                        ASSERT(mails_.count(gid) > 0);
                        auto& m = mails_.at(gid);
                        m.set_fetched(true);
                        mails_changed_ = true;
                        return m.events();
                }

                inline bool role::quest_committed(uint32_t pttid) const {
                        return quest_.passed_.count(pttid) > 0;
                }

                inline const map<uint32_t, role::accepted_quest>& role::quest_accepted_quests() const {
                        return quest_.accepted_;
                }

                inline void role::quest_remove_commited_quest(uint32_t pttid) {
                        ASSERT(quest_.passed_.count(pttid) > 0);
                        quest_.passed_.erase(pttid);
                        quests_changed_ = true;
                }

                inline bool role::got_everyday_quest_reward(uint32_t pttid) const {
                        ASSERT(quest_.everyday_quest_day_ == refresh_day());
                        ASSERT(quest_.everyday_quest_monday_ == refresh_monday());
                        return quest_.got_everyday_quest_rewards_.count(pttid) > 0;
                }

                inline void role::add_got_everyday_quest_reward(uint32_t pttid) {
                        ASSERT(quest_.everyday_quest_day_ == refresh_day());
                        ASSERT(quest_.everyday_quest_monday_ == refresh_monday());
                        quest_.got_everyday_quest_rewards_.insert(pttid);
                        quests_changed_ = true;
                }

                inline role::accepted_quest& role::get_accepted_quest(uint32_t pttid) {
                        return quest_.accepted_.at(pttid);
                }

                inline void role::update_mys_shop(const pd::mys_shop& ms) {
                        shop_.mys_shops_[ms.pttid()] = ms;
                        shop_changed_ = true;
                }

                inline bool role::has_mys_shop_good(uint32_t pttid, uint32_t good_id) const {
                        if (shop_.mys_shops_.count(pttid) < 0) {
                                return false;
                        }
                        const auto& shop = shop_.mys_shops_.at(pttid);
                        for (const auto& i : shop.goods()) {
                                if (i.id() == good_id && !i.bought()) {
                                        return true;
                                }
                        }
                        return false;
                }

                inline void role::bought_mys_shop_good(uint32_t pttid, uint32_t good_id) {
                        ASSERT(shop_.mys_shops_.count(pttid) > 0);
                        auto& shop = shop_.mys_shops_.at(pttid);
                        for (auto& i : *shop.mutable_goods()) {
                                if (i.id() == good_id) {
                                        i.set_bought(true);
                                        shop_changed_ = true;
                                        return;
                                }
                        }
                        ASSERT(false);
                }

                inline void role::reset_shop_records_times(uint32_t pttid, uint32_t good_id) {
                        if (shop_.records_.count(pttid) > 0) {
                                shop_.records_.erase(pttid);
                                shop_changed_ = true;
                        }
                }

                inline uint32_t role::shop_record_times(uint32_t pttid, uint32_t good_id) const {
                        if (shop_.records_.count(pttid) > 0 && shop_.records_.at(pttid).count(good_id) > 0) {
                                return shop_.records_.at(pttid).at(good_id).times_;
                        }
                        return 0;
                }

                inline uint32_t role::mys_shop_free_refreshed_time(uint32_t pttid) const {
                        if (shop_.mys_shops_.count(pttid) <= 0) {
                                return 0;
                        } else {
                                return shop_.mys_shops_.at(pttid).free_refreshed_time();
                        }
                }

                inline void role::recharge_update_paid_money(uint32_t money) {
                        recharge_.paid_money_ += money;
                        role_data_changed_ = true;
                        on_event(ECE_RECHARGE, { money });
                }

                inline uint32_t role::paid_money() const {
                        return recharge_.paid_money_;
                }

                inline bool role::has_item(uint32_t pttid) const {
                        return pttid2item_.count(pttid) > 0;
                }

                inline const shared_ptr<equipment>& role::get_equipment(uint64_t gid) const {
                        ASSERT(equipments_.gid2equipment_.count(gid) > 0);
                        return equipments_.gid2equipment_.at(gid);
                }

                inline list<uint64_t> role::equipments() const {
                        list<uint64_t> ret;
                        for (const auto& i : equipments_.gid2equipment_) {
                                ret.push_back(i.first);
                        }
                        return ret;
                }

                inline void role::locked_equipment(uint64_t gid) {
                        ASSERT(equipments_.gid2equipment_.count(gid) > 0);
                        equipments_.gid2equipment_.at(gid)->set_is_locked();
                        equipments_changed_ = true;
                }

                inline void role::unlocked_equipment(uint64_t gid) {
                        ASSERT(equipments_.gid2equipment_.count(gid) > 0);
                        equipments_.gid2equipment_.at(gid)->set_is_unlocked();
                        equipments_changed_ = true;
                }

                inline uint32_t role::gag_until_time() const {
                        return yunying_.gag().until_time();
                }

                inline string role::gag_reason() const {
                        return yunying_.gag().reason();
                }

                inline void role::set_gag_until_time(uint32_t gag_until_time) {
                        auto *gag = yunying_.mutable_gag();
                        gag->set_until_time(gag_until_time);
                        yunying_changed_ = true;
                }

                inline void role::set_gag_reason(const string& reason) {
                        auto *gag = yunying_.mutable_gag();
                        gag->set_reason(reason);
                        yunying_changed_ = true;
                }

                inline uint32_t role::ban_until_time() const {
                        return yunying_.ban().until_time();
                }

                inline string role::ban_reason() const {
                        return yunying_.ban().reason();
                }

                inline void role::set_ban_until_time(uint32_t ban_until_time) {
                        auto *ban = yunying_.mutable_ban();
                        ban->set_until_time(ban_until_time);
                        yunying_changed_ = true;
                }

                inline void role::set_ban_reason(const string& reason) {
                        auto *ban = yunying_.mutable_ban();
                        ban->set_reason(reason);
                        yunying_changed_ = true;
                }

                inline void role::relation_application_set_read(const pd::gid_array& roles) {
                        pd::relation_application application;
                        for (auto i : roles.gids()) {
                                if (relation_.applications_iters_.count(i) > 0) {
                                        relation_.applications_iters_.at(i)->set_read(true);
                                }
                        }
                        relation_changed_ = true;
                }
                inline const set<uint64_t>& role::relation_saved_suggestion() const {
                        return relation_.saved_suggestion_;
                }

                inline void role::relation_save_suggestion(const set<uint64_t>& suggestion) {
                        relation_.saved_suggestion_ = suggestion;
                }

                inline void role::relation_add_helper(uint64_t helper) {
                        ASSERT(gid_ != helper);
                        relation_.helpers_.insert(helper);
                }

                inline const set<uint64_t>& role::relation_helpers() const {
                        return relation_.helpers_;
                }

                inline void role::relation_add_teammate(uint64_t teammate) {
                        ASSERT(gid_ != teammate);
                        relation_.teammates_.insert(teammate);
                }

                inline const set<uint64_t>& role::relation_teammates() const {
                        return relation_.teammates_;
                }

                inline uint32_t role::today_online_seconds() const {
                        auto last_login_day = refresh_day_from_time_t(data_.last_login_time_);
                        auto now = system_clock::to_time_t(system_clock::now());
                        if (last_login_day == refresh_day()) {
                                return data_.today_online_seconds_ + now - data_.last_login_time_;
                        } else {
                                return now - refresh_day_to_time_t(refresh_day());
                        }
                }

                inline uint32_t role::last_login_time() const {
                        return data_.last_login_time_;
                }

                inline uint32_t role::first_login_time() const {
                        return data_.first_login_time_;
                }

                inline uint32_t role::login_days() const {
                        return data_.login_days_;
                }

                inline bool role::activity_login_days_got_reward(uint32_t day) const {
                        return activity_.login_days_.got_reward_days_.count(day) > 0;
                }

                inline void role::activity_login_days_add_got_reward(uint32_t day) {
                        activity_.login_days_.got_reward_days_.insert(day);
                        activity_changed_ = true;
                }

                inline bool role::activity_online_time_got_reward(int idx) const {
                        if (activity_.online_time_.got_reward_day_ != refresh_day()) {
                                return false;
                        }
                        return activity_.online_time_.got_reward_idx_.count(idx) > 0;
                }

                inline void role::activity_online_time_add_got_reward(uint32_t idx) {
                        if (activity_.online_time_.got_reward_day_ != refresh_day()) {
                                activity_.online_time_.got_reward_day_ = refresh_day();
                                activity_.online_time_.got_reward_idx_.clear();
                        }
                        activity_.online_time_.got_reward_idx_.insert(idx);
                        activity_changed_ = true;
                }

                inline bool role::activity_seven_days_fuli_quests_accepted(uint32_t begin_day) const {
                        if (activity_.seven_days_.count(begin_day) == 0) {
                                return false;
                        }
                        return activity_.seven_days_.at(begin_day).fuli_quests_accepted_;
                }

                inline void role::activity_seven_days_set_fuli_quests_accepted(uint32_t begin_day) {
                        activity_.seven_days_[begin_day].fuli_quests_accepted_ = true;
                        activity_changed_ = true;
                }

                inline bool role::activity_seven_days_quests_accepted(uint32_t begin_day) const {
                        if (activity_.seven_days_.count(begin_day) == 0) {
                                return false;
                        }
                        return activity_.seven_days_.at(begin_day).quests_accepted_;
                }

                inline void role::activity_seven_days_set_quests_accepted(uint32_t begin_day) {
                        activity_.seven_days_[begin_day].quests_accepted_ = true;
                        activity_changed_ = true;
                }

                inline bool role::activity_seven_days_got_reward(uint32_t begin_day) const {
                        if (activity_.seven_days_.count(begin_day) == 0) {
                                return false;
                        }
                        return activity_.seven_days_.at(begin_day).got_reward_;
                }

                inline void role::activity_seven_days_set_got_reward(uint32_t begin_day) {
                        activity_.seven_days_[begin_day].got_reward_ = true;
                        activity_changed_ = true;
                }

                inline bool role::activity_seven_days_passed(uint32_t begin_day) {
                        if (activity_.seven_days_.count(begin_day) == 0) {
                              return true;
                        }
                        auto day = refresh_day() - refresh_day_from_time_t(data_.first_login_time_) + 1;
                        if (day >= begin_day && day < begin_day + 7) {
                                    return true;
                        }
                        return activity_.seven_days_.at(begin_day).passed_;
                }

                inline void role::activity_seven_days_set_passed(uint32_t begin_day) {
                        ASSERT(activity_.seven_days_.count(begin_day) > 0);
                        activity_.seven_days_.at(begin_day).passed_ = true;
                        activity_changed_ = true;
                }

                inline void role::activity_add_qiandao(uint32_t qiandao_month, uint32_t qiandao_mday) {
                        if (activity_.qiandao_.month_ != qiandao_month) {
                                activity_.qiandao_.month_ = qiandao_month;
                                activity_.qiandao_.mdays_.clear();
                                activity_.qiandao_.got_leiqian_days_.clear();
                        }
                        activity_.qiandao_.mdays_.insert(qiandao_mday);
                        activity_changed_ = true;
                }

                inline bool role::has_activity_qiandao(uint32_t month, uint32_t day) const {
                        if (activity_.qiandao_.month_ == month) {
                                if (activity_.qiandao_.mdays_.count(day) > 0) {
                                        return true;
                                }
                        }
                        return false;
                }

                inline int role::get_activity_qiandao_times(uint32_t month) const {
                        if (activity_.qiandao_.month_ != month) {
                                return 0;
                        }
                        return activity_.qiandao_.mdays_.size();
                }

                inline bool role::has_activity_qiandao_leiqian_reward(uint32_t month, uint32_t day) const {
                        if (activity_.qiandao_.month_ != month) {
                                return false;
                        }
                        if (activity_.qiandao_.got_leiqian_days_.count(day) > 0) {
                                return false;
                        }
                        return true;
                }

                inline void role::activity_add_leiqian(uint32_t month, uint32_t day) {
                        activity_.qiandao_.got_leiqian_days_.insert(day);
                        activity_changed_ = true;
                }

                inline bool role::activity_bought_everyday_libao_today(uint32_t pttid) const {
                        auto it = activity_.everyday_libao_.find(pttid);
                        if (it == activity_.everyday_libao_.end()) {
                                return false;
                        } else {
                                return it->second == refresh_day();
                        }
                }

                inline void role::activity_set_bought_everyday_libao_today(uint32_t pttid) {
                        activity_.everyday_libao_[pttid] = refresh_day();
                        activity_changed_ = true;
                }

                inline int role::activity_get_chaozhi_libao_bug_count(pd::activity_chaozhi_libao_type libao_type) const {
                        if (activity_.chaozhi_libao_idxs_.count(libao_type) > 0) {
                                return activity_.chaozhi_libao_idxs_.at(libao_type) + 1;
                        }
                        return 0;
                }

                inline void role::activity_add_chaozhi_libao(pd::activity_chaozhi_libao_type libao_type, int libao_idx) {
                        activity_.chaozhi_libao_idxs_[libao_type] = libao_idx;
                        activity_changed_ = true;
                }

                inline bool role::activity_first_recharge() const {
                        return activity_.recharge_get_first_reward_;
                }

                inline void role::update_activity_first_recharge_get_reward() {
                        activity_.recharge_get_first_reward_ = true;
                        activity_changed_ = true;
                }

                inline bool role::has_title(uint32_t pttid) const {
                        return title_.titles_.count(pttid) > 0;
                }

                inline const pd::title& role::get_title(uint32_t pttid) const {
                        ASSERT(has_title(pttid));
                        return title_.titles_.at(pttid);
                }

                inline bool role::has_recharge_order(uint64_t gid) const {
                        return recharge_.gid2pttid_.count(gid) > 0;
                }

                inline bool role::recharge_order_recharged(uint64_t gid) const {
                        return recharge_.recharged_gids_.count(gid) > 0;
                }

                inline uint32_t role::recharge_order_pttid(uint64_t gid) const {
                        ASSERT(has_recharge_order(gid));
                        return recharge_.gid2pttid_.at(gid);
                }

                inline uint64_t role::add_recharge_order(uint32_t pttid) {
                        auto gid = gid::instance().new_gid(gid_type::RECHARGE_ORDER);
                        recharge_.gid2pttid_[gid] = pttid;
                        recharge_.pttid2gids_[pttid].insert(gid);
                        recharge_changed_ = true;
                        return gid;
                }

                inline void role::recharge_order_done(uint64_t gid) {
                        ASSERT(has_recharge_order(gid));
                        auto pttid = recharge_.gid2pttid_.at(gid);
                        recharge_.gid2pttid_.erase(gid);
                        recharge_.pttid2gids_.at(pttid).erase(gid);
                        recharge_.recharged_pttids_.insert(pttid);
                        recharge_.recharged_gids_.insert(gid);
                        recharge_changed_ = true;
                }

                inline bool role::has_recharged(uint32_t pttid) const {
                        return recharge_.recharged_pttids_.count(pttid) > 0;
                }

                inline bool role::has_recharged() const {
                        return !recharge_.recharged_pttids_.empty();
                }

                inline bool role::has_recharge_day(uint32_t pttid) const {
                        return recharge_.pttid2day_.count(pttid) > 0;
                }

                inline bool role::recharge_day_vaild() const {
                        bool vaild = false;
                        for (const auto& i : recharge_.pttid2day_) {
                                if (i.second >= refresh_day()) {
                                        vaild = true;
                                        break;
                                }
                        }
                        return vaild;
                }

                inline uint32_t role::recharge_day(uint32_t pttid) const {
                        ASSERT(has_recharge_day(pttid));
                        return recharge_.pttid2day_.at(pttid);
                }

                inline void role::update_recharge_day(uint32_t pttid, uint32_t ptt_day) {
                        auto today = refresh_day();
                        auto day = 0u;
                        auto it = recharge_.pttid2day_.find(pttid);
                        if (it != recharge_.pttid2day_.end()) {
                                day = recharge_.pttid2day_.at(pttid);
                        }
                        if (day > today) {
                                recharge_.pttid2day_[pttid] = day + ptt_day;
                        } else {
                                recharge_.pttid2day_[pttid] = today + ptt_day;
                                recharge_.pttid2last_process_day_[pttid] = today - 1;
                        }
                        recharge_changed_ = true;
                }

                inline const map<uint32_t, uint32_t>& role::recharge_days() const {
                        return recharge_.pttid2day_;
                }

                inline uint32_t role::recharge_last_process_day(uint32_t pttid) const {
                        auto it = recharge_.pttid2last_process_day_.find(pttid);
                        ASSERT(it != recharge_.pttid2last_process_day_.end());
                        return it->second;
                }

                inline void role::recharge_update_last_process_day(uint32_t pttid, uint32_t day) {
                        recharge_.pttid2last_process_day_[pttid] = day;
                        recharge_changed_ = true;
                }

                inline bool role::recharge_got_vip_box(uint32_t vip_level) const {
                        return recharge_.got_vip_boxes_.count(vip_level) > 0;
                }

                inline void role::recharge_set_got_vip_box(uint32_t vip_level) {
                        recharge_.got_vip_boxes_.insert(vip_level);
                        recharge_changed_ = true;
                }

                inline bool role::recharge_got_vip_buy_box(uint32_t vip_level) const {
                        return recharge_.got_vip_buy_boxes_.count(vip_level) > 0;
                }

                inline void role::activity_update_leiji_consume(uint32_t count) {
                        activity_.leiji_consume_count_ += count;
                        activity_changed_ = true;
                }

                inline void role::activity_clear_leiji_consume() {
                        activity_.leiji_consume_count_ = 0;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_leiji_consume() const {
                        return activity_.leiji_consume_count_;
                }

                inline void role::activity_update_leiji_recharge(uint32_t money) {
                        activity_.leiji_recharge_count_ += money;
                        activity_changed_ = true;
                }

                inline void role::activity_clear_leiji_recharge() {
                        activity_.leiji_recharge_count_ = 0;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_leiji_recharge() const {
                        return activity_.leiji_recharge_count_;
                }

                inline void role::activity_set_vip_buy() {
                        activity_.qianzhuang_.vip_buy_ = true;
                        activity_changed_ = true;
                }

                inline bool role::activity_vip_buy() const {
                        return activity_.qianzhuang_.vip_buy_;
                }

                inline void role::activity_set_emperor_buy() {
                        activity_.qianzhuang_.emperor_buy_ = true;
                        activity_changed_ = true;
                }

                inline bool role::activity_emperor_buy() const {
                        return activity_.qianzhuang_.emperor_buy_;
                }

                inline void role::activity_update_vip_got(uint32_t pttid) {
                        ASSERT(!activity_vip_got(pttid));
                        activity_.qianzhuang_.vip_got_.insert(pttid);
                        activity_changed_ = true;
                }

                inline bool role::activity_vip_got(uint32_t pttid) const {
                        return activity_.qianzhuang_.vip_got_.count(pttid) > 0 ? true : false;
                }

                inline void role::activity_update_emperor_got(uint32_t pttid) {
                        ASSERT(!activity_emperor_got(pttid));
                        activity_.qianzhuang_.emperor_got_.insert(pttid);
                        activity_changed_ = true;
                }

                inline bool role::activity_emperor_got(uint32_t pttid) const {
                        return activity_.qianzhuang_.emperor_got_.count(pttid) > 0 ? true : false;
                }

                inline void role::activity_update_fund_got(uint32_t pttid) {
                        ASSERT(!activity_fund_got(pttid));
                        activity_.qianzhuang_.fund_got_.insert(pttid);
                        activity_changed_ = true;
                }

                inline bool role::activity_fund_got(uint32_t pttid) const {
                        return activity_.qianzhuang_.fund_got_.count(pttid) > 0 ? true : false;
                }

                inline void role::activity_update_limit_libao(uint32_t pttid) {
                        const auto& ptt = PTTS_GET(activity_logic, 1);
                        activity_.limit_libao_.cur_ = pttid;
                        activity_.limit_libao_.time_ = system_clock::to_time_t(system_clock::now()) + ptt.limit_libao_update_time() * HOUR;
                        activity_.limit_libao_.passed_[pttid] = activity_.limit_libao_.time_;
                        activity_changed_ = true;
                }

                inline void role::activity_add_limit_libao_bought(uint32_t pttid) {
                        ASSERT(activity_.limit_libao_.bought_.count(pttid) <= 0);
                        activity_.limit_libao_.bought_.insert(pttid);
                        activity_.limit_libao_.cur_ = 0;
                        activity_.limit_libao_.time_ = 0;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_limit_libao_cur() const {
                        return activity_.limit_libao_.cur_;
                }

                inline uint32_t role::activity_limit_libao_time() const {
                        return activity_.limit_libao_.time_;
                }

                inline const map<uint32_t, uint32_t>& role::activity_limit_libao_passed() const {
                        return activity_.limit_libao_.passed_;
                }

                inline bool role::activity_bought_limit_libao(uint32_t pttid) const {
                        return activity_.limit_libao_.bought_.count(pttid) > 0;
                }

                inline uint32_t role::activity_festival_exchange_times(uint32_t pttid, uint32_t exchange) const {
                        if (activity_.festival_.festival_exchange_times_.count(pttid) > 0 && activity_.festival_.festival_exchange_times_.at(pttid).count(exchange) > 0) {
                                return activity_.festival_.festival_exchange_times_.at(pttid).at(exchange);
                        }
                        return 0u;
                }

                inline void role::update_activity_festival_exchange_times(uint32_t pttid, uint32_t exchange) {
                        if (activity_.festival_.festival_exchange_times_.count(pttid) <= 0) {
                                activity_.festival_.festival_exchange_times_[pttid][exchange] = 1;
                        } else {
                                auto& exchange_times = activity_.festival_.festival_exchange_times_.at(pttid);
                                if (exchange_times.count(exchange) > 0) {
                                        exchange_times.at(exchange) += 1;
                                } else {
                                        exchange_times[exchange] = 1;
                                }
                        }
                        activity_changed_ = true;
                }

                inline void role::reset_activity_festival_exchange_times() {
                        activity_.festival_.festival_exchange_times_.clear();
                        activity_.festival_.pttid_ = 0;
                        activity_.festival_.duration_ = 0;
                        activity_.festival_.start_day_ = 0;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_prize_wheel_start_day() const {
                        return activity_.prize_wheel_.start_day_;
                }

                inline uint32_t role::activity_prize_wheel_duration() const {
                        return activity_.prize_wheel_.duration_;
                }

                inline uint32_t role::activity_prize_wheel_pttid() const {
                        return activity_.prize_wheel_.pttid_;
                }

                inline bool role::activity_prize_wheel_ongoing() const {
                        return activity_.prize_wheel_.ongoing_;
                }

                inline uint32_t role::activity_tea_party_start_day() const {
                        return activity_.tea_party_.start_day_;
                }

                inline uint32_t role::activity_tea_party_duration() const {
                        return activity_.tea_party_.duration_;
                }

                inline uint32_t role::activity_tea_party_pttid() const {
                        return activity_.tea_party_.pttid_;
                }

                inline bool role::activity_tea_party_ongoing() const {
                        return activity_.tea_party_.ongoing_;
                }

                inline bool role::activity_tea_party_got_favor_reward(uint32_t idx) const {
                        ASSERT(activity_tea_party_ongoing());
                        return activity_.tea_party_.got_favor_rewards_.count(idx) > 0;
                }

                inline void role::activity_tea_party_add_got_favor_reward(uint32_t idx) {
                        ASSERT(activity_tea_party_ongoing());
                        activity_.tea_party_.got_favor_rewards_.insert(idx);
                        activity_changed_ = true;
                }

                inline void role::activity_reset_discount_goods(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.discount_goods_.pttid_ > 0) {
                                const auto& ptt = PTTS_GET(activity_discount_goods, activity_.discount_goods_.pttid_);
                                if (shop_.records_.count(ptt.shop_pttid()) > 0) {
                                        shop_.records_.erase(ptt.shop_pttid());
                                        shop_changed_ = true;
                                }
                        }
                        activity_.discount_goods_.start_day_ = start_day;
                        activity_.discount_goods_.duration_ = duration;
                        activity_.discount_goods_.pttid_ = pttid;
                        activity_changed_ = true;
                        /*if (start_day != 0) {
                                bi::instance().activity_discount_goods_start(username_func_(), yci_, ip_, pttid, pd::AAT_DISCOUNT_GOODS, gid_, data_.level_);
                        } else {
                                bi::instance().activity_discount_goods_stop(username_func_(), yci_, ip_, pttid, pd::AAT_DISCOUNT_GOODS, duration, gid_, data_.level_);
                        }*/
                }

                inline pd::result role::check_activity_discount_goods(uint32_t pttid, const pc::good& good) const {
                        auto today = refresh_day();
                        if (activity_.discount_goods_.pttid_ != pttid) {
                                return pd::ACTIVITY_DISCOUNT_GOODS_NOT_START;
                        }
                        if (today < activity_.discount_goods_.start_day_ ||
                            today > activity_.discount_goods_.start_day_ + activity_.discount_goods_.duration_) {
                                return pd::ACTIVITY_DISCOUNT_GOODS_HAS_END;
                        }
                        return pd::OK;
                }

                inline uint32_t role::activity_discount_goods_start_day() const {
                        return activity_.discount_goods_.start_day_;
                }

                inline uint32_t role::activity_discount_goods_duration() const {
                        return activity_.discount_goods_.duration_;
                }

                inline uint32_t role::activity_continue_recharge_start_day() const {
                        return activity_.continue_recharge_.start_day_;
                }

                inline uint32_t role::activity_continue_recharge_duration() const {
                        return activity_.continue_recharge_.duration_;
                }

                inline uint32_t role::activity_continue_recharge_pttid() const {
                        return activity_.continue_recharge_.pttid_;
                }

                inline void role::activity_continue_recharge_set_cur_quest(uint32_t quest, bool add_idx) {
                        activity_.continue_recharge_.cur_quest_ = quest;
                        if (add_idx) {
                              activity_.continue_recharge_.quest_idx_ += 1;
                              activity_.continue_recharge_.update_quests_activate_day_ = refresh_day();
                        } else {
                              activity_.continue_recharge_.quest_idx_ -= 1;
                        }
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_continue_recharge_quest_idx() const {
                        return activity_.continue_recharge_.quest_idx_;
                }

                inline uint32_t role::activity_continue_recharge_cur_quest() const {
                        return activity_.continue_recharge_.cur_quest_;
                }

                inline void role::activity_reset_continue_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.continue_recharge_.start_day_ == 0) {
                                activity_.continue_recharge_.cur_quest_ = 0;
                                activity_.continue_recharge_.quest_idx_ = 0;
                        }
                        if (start_day == 0) {
                                activity_.continue_recharge_.update_quests_activate_day_ = 0;
                        }
                        activity_.continue_recharge_.start_day_ = start_day;
                        activity_.continue_recharge_.duration_ = duration;
                        activity_.continue_recharge_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                inline void role::activity_continue_recharge_update_dayrecharge(uint32_t price) {
                        activity_.continue_recharge_.day_recharge_ += price;
                        activity_changed_ = true;
                }

                inline void role::activity_continue_recharge_reset_dayrecharge() {
                        if (activity_.continue_recharge_.update_quests_activate_day_ != refresh_day()) {
                              activity_.continue_recharge_.day_recharge_ = 0;
                              activity_changed_ = true;
                        }
                }

                inline uint32_t role::activity_limit_play_start_day() const {
                        return activity_.limit_play_.start_day_;
                }

                inline uint32_t role::activity_limit_play_duration() const {
                        return activity_.limit_play_.duration_;
                }

                inline uint32_t role::activity_limit_play_pttid() const {
                        return activity_.limit_play_.pttid_;
                }

                inline void role::activity_reset_limit_play(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (refresh_day() >= activity_.limit_play_.start_day_ + activity_.limit_play_.duration_ - 1) {
                                activity_stop_limit_play();
                        }
                        activity_.limit_play_.start_day_ = start_day;
                        activity_.limit_play_.duration_ = duration;
                        activity_.limit_play_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_recharge_rank_start_day() const {
                        return activity_.recharge_rank_.start_day_;
                }

                inline uint32_t role::activity_recharge_rank_duration() const {
                        return activity_.recharge_rank_.duration_;
                }

                inline void role::activity_reset_recharge_rank(uint32_t start_day, uint32_t duration) {
                        if (refresh_day() >= activity_.recharge_rank_.start_day_ + activity_.recharge_rank_.duration_ - 1) {
                                activity_.recharge_rank_.start_day_ = 0;
                                activity_.recharge_rank_.duration_ = 0;
                        } else {
                                activity_.recharge_rank_.start_day_ = start_day;
                                activity_.recharge_rank_.duration_ = duration;
                        }
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_leiji_recharge_start_day() const {
                        return activity_.leiji_recharge_.start_day_;
                }

                inline uint32_t role::activity_leiji_recharge_duration() const {
                        return activity_.leiji_recharge_.duration_;
                }

                inline uint32_t role::activity_leiji_recharge_pttid() const {
                        return activity_.leiji_recharge_.pttid_;
                }

                inline void role::activity_reset_leiji_recharge(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.leiji_recharge_.start_day_ != 0 && refresh_day() >= activity_.leiji_recharge_.start_day_ + activity_.leiji_recharge_.duration_ - 1) {
                                activity_stop_leiji_recharge();
                        }
                        activity_.leiji_recharge_.start_day_ = start_day;
                        activity_.leiji_recharge_.duration_ = duration;
                        activity_.leiji_recharge_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_leiji_consume_start_day() const {
                        return activity_.leiji_consume_.start_day_;
                }

                inline uint32_t role::activity_leiji_consume_duration() const {
                        return activity_.leiji_consume_.duration_;
                }

                inline uint32_t role::activity_leiji_consume_pttid() const {
                        return activity_.leiji_consume_.pttid_;
                }

                inline void role::activity_reset_leiji_consume(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.leiji_consume_.start_day_ != 0 && refresh_day() >= activity_.leiji_consume_.start_day_ + activity_.leiji_consume_.duration_ - 1) {
                                activity_stop_leiji_consume();
                        }
                        activity_.leiji_consume_.start_day_ = start_day;
                        activity_.leiji_consume_.duration_ = duration;
                        activity_.leiji_consume_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                inline uint32_t role::activity_festival_start_day() const {
                        return activity_.festival_.start_day_;
                }

                inline uint32_t role::activity_festival_duration() const {
                        return activity_.festival_.duration_;
                }

                inline uint32_t role::activity_festival_pttid() const {
                        return activity_.festival_.pttid_;
                }

                inline void role::activity_reset_festival(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.festival_.start_day_ != 0 && refresh_day() >= activity_.festival_.start_day_ + activity_.festival_.duration_ - 1) {
                                activity_stop_festival();
                        }
                        activity_.festival_.start_day_ = start_day;
                        activity_.festival_.duration_ = duration;
                        activity_.festival_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                inline bool role::activity_daiyanren_got_reward(uint32_t idx) const {
                        return activity_.daiyanren_.got_reward_.count(idx) > 0;
                }
                inline void role::activity_daiyanren_set_got_reward(uint32_t idx) {
                        ASSERT(activity_.daiyanren_.got_reward_.count(idx) == 0);
                        activity_.daiyanren_.got_reward_.insert(idx);
                        activity_changed_ = true;
                }
                inline bool role::activity_daiyanren_finished() const {
                        return activity_.daiyanren_.finished_;
                }
                inline void role::activity_daiyanren_set_finished() {
                        activity_.daiyanren_.finished_ = true;
                        activity_changed_ = true;
                }

                inline void role::recharge_set_got_vip_buy_box(uint32_t vip_level) {
                        recharge_.got_vip_buy_boxes_.insert(vip_level);
                        recharge_changed_ = true;
                }

                inline uint32_t role::last_comment_time(uint64_t role) const {
                        if (comments_.role2last_comment_time_.count(role) == 0) {
                                return 0;
                        }
                        return comments_.role2last_comment_time_.at(role);
                }

                inline bool role::has_comment(uint64_t gid) const {
                        return comments_.gid2comment_.count(gid) > 0;
                }

                inline const pd::comment& role::get_comment(uint64_t gid) const {
                        ASSERT(has_comment(gid));
                        return comments_.gid2comment_.at(gid);
                }

                inline bool role::has_present() const {
                        if (social_.presents_.empty()) {
                                return false;
                        } else {
                                ASSERT(social_.presents_.size() == 1);
                                return true;
                        }
                }

                inline uint32_t role::get_present() const {
                        ASSERT(social_.presents_.size() == 1);
                        return social_.presents_.begin()->first;
                }

                inline int role::present_count(uint32_t present) const {
                        if (social_.presents_.count(present) == 0) {
                                return 0;
                        } else {
                                return social_.presents_.at(present);
                        }
                }

                inline void role::add_present(uint32_t present, int count) {
                        social_.presents_[present] += count;
                        ASSERT(social_.presents_.size() == 1);
                        social_changed_ = true;
                }

                inline void role::dec_present(uint32_t present, int count) {
                        ASSERT(present_count(present) >= count);
                        social_.presents_[present] -= count;
                        if (social_.presents_[present] <= 0) {
                                social_.presents_.erase(present);
                        }
                        social_changed_ = true;
                }

                inline void role::record_read_records(pd::record_type type, int sub_type) {
                        if (records_.records_.count(type) <= 0) {
                                return;
                        }
                        if (records_.records_.at(type).count(sub_type) <= 0) {
                                return;
                        }
                        auto& record_info = records_.records_[type][sub_type];
                        if (!record_info.read_) {
                                record_info.read_ = true;
                                records_changed_ = true;
                        }
                }

                inline void role::broadcast_system_chat(uint32_t pttid) {
                        ASSERT(broadcast_system_chat_func_);
                        broadcast_system_chat_func_(gid_, pttid);
                }

                inline uint32_t role::room_last_invite_time(uint64_t role) const {
                        auto iter = room_data_.invited_.find(role);
                        if (iter != room_data_.invited_.end()) {
                                return iter->second;
                        }
                        return 0;
                }

                inline uint32_t role::room_last_inviter_time(uint64_t role) const {
                        auto iter = room_data_.inviters_.find(role);
                        if (iter != room_data_.inviters_.end()) {
                                return iter->second;
                        }
                        return 0;
                }

                inline void role::room_delete_invited(uint64_t role) {
                        room_data_.invited_.erase(role);
                }

                inline void role::room_update_last_invite_time(uint64_t role, uint32_t time) {
                        room_data_.invited_[role] = time;
                }

                inline void role::room_clear_last_invite_time() {
                        room_data_.invited_.clear();
                }

                inline void role::room_set_inviter(uint64_t role, uint32_t time) {
                        room_data_.inviters_[role] = time;
                }

                inline map<uint64_t, uint32_t> role::room_inviters() const{
                        return room_data_.inviters_;
                }

                inline void role::room_delete_inviter(uint64_t role) {
                        room_data_.inviters_.erase(role);
                }

                inline bool role::room_has_inviter(uint64_t role) const{
                        return room_data_.inviters_.count(role) > 0;
                }

                inline bool role::quest_check_reset_cur(const pc::quest_param& a, const pc::quest_param& b) {
                        ASSERT(a.arg_size() == b.arg_size());
                        for (auto i = 0; i < a.arg_size(); i++) {
                                if (a.arg(i) != b.arg(i)) {
                                        return true;
                                }
                        }
                        return false;
                }

                inline void role::quest_move_to_next_param(uint32_t pttid) {
                        auto& aq = get_accepted_quest(pttid);

                        const auto& ptt = PTTS_GET(quest, pttid);
                        ASSERT(static_cast<int>(aq.counters_.size()) == ptt.targets_size());
                        for (auto i = 0; i < ptt.targets_size(); ++i) {
                                auto& counter = aq.counters_[i];
                                auto& target = ptt.targets(i);
                                counter.cur_param_index_ += 1;
                                if (counter.cur_param_index_ < target.params_size()) {
                                        if (quest_check_reset_cur(target.params(counter.cur_param_index_ - 1),
                                                                  target.params(counter.cur_param_index_))) {
                                                counter.cur_ = 0;
                                                counter.param_.clear_arg();
                                                for (auto j : target.params(counter.cur_param_index_).arg()) {
                                                        counter.param_.add_arg(j);
                                                }
                                        }
                                        counter.param_.set_need(target.params(counter.cur_param_index_).need());
                                }
                        }
                }

                inline pd::event_array role::get_quest_cur_pass_events(uint32_t pttid) {
                        pd::event_array event_array;
                        const auto& ptt = PTTS_GET(quest, pttid);
                        auto& aq = get_accepted_quest(pttid);
                        ASSERT(static_cast<int>(aq.counters_.size()) == ptt.targets_size());
                        for (auto i = 0; i < ptt.targets_size(); ++i) {
                                auto& counter = aq.counters_[i];
                                auto& target = ptt.targets(i);
                                if (counter.cur_param_index_ < target.params_size()) {
                                        event_merge(event_array, target.params(counter.cur_param_index_).pass_events());
                                }
                        }
                        return event_array;
                }

                inline void role::add_mail(const pd::mail& mail) {
                        mails_[mail.gid()] = mail;
                        mails_changed_ = true;
                }

                inline bool role::mail_already_read(uint64_t gid) const {
                        ASSERT(mails_.count(gid) > 0);
                        return mails_.at(gid).already_read();
                }

                inline void role::mail_set_already_read(uint64_t gid) {
                        ASSERT(mails_.count(gid) > 0);
                        mails_.at(gid).set_already_read(true);
                        mails_changed_ = true;
                }

                inline bool role::has_mail(uint64_t gid) const {
                        if (mails_.count(gid) <= 0) {
                                return false;
                        }
                        if (system_clock::now() - system_clock::from_time_t(mails_.at(gid).time()) >= MAIL_EXPIRE_DURATION) {
                                return false;
                        }
                        return true;
                }

                inline const pd::mail& role::get_mail(uint64_t gid) const {
                        ASSERT(mails_.count(gid) > 0);
                        return mails_.at(gid);
                }

                inline const map<uint64_t, pd::mail>& role::mails() const {
                        return mails_;
                }

                inline void role::del_mail(uint64_t gid) {
                        ASSERT(mails_.count(gid) > 0);
                        mails_.erase(gid);
                        mails_changed_ = true;
                }

                inline void role::record_add_record(pd::record_type type, int sub_type, const pd::record_entity& record) {
                        auto& rs = records_.records_[type][sub_type];
                        rs.records_.push_back(record);
                        rs.read_ = false;
                        while (rs.records_.size() > 100u) {
                                rs.records_.pop_front();
                        }
                        records_changed_ = true;

                        role_add_new_record_notice_(gid_, type, sub_type, record);
                }

                inline void role::record_remove_record(pd::record_type type, int sub_type, uint64_t gid) {
                        auto& rs = records_.records_[type][sub_type].records_;
                        for (auto iter = rs.begin(); iter != rs.end(); ++iter) {
                                if (iter->gid() == gid) {
                                        rs.erase(iter);
                                        records_changed_ = true;
                                        break;
                                }
                        }
                }

                inline void role::record_remove_all_records(pd::record_type type, int sub_type) {
                        records_.records_[type][sub_type].records_.clear();
                        records_changed_ = true;
                }

                inline void role::record_init_record(pd::record_entity& record) {
                        record.set_gid(records_.gid_seed_);
                        records_.gid_seed_ = max(1ul, records_.gid_seed_ + 1);
                        record.set_time(system_clock::to_time_t(system_clock::now()));
                }

                inline void role::reset_data_changed() {
                        resources_changed_ = false;
                        role_data_changed_ = false;
                        other_data_changed_ = false;
                        items_changed_ = false;
                        shop_changed_ = false;
                        relation_changed_ = false;
                        quests_changed_ = false;
                        yunying_changed_ = false;
                        activity_changed_ = false;
                        recharge_changed_ = false;
                        comments_changed_ = false;
                        mails_changed_ = false;
                        chat_data_changed_ = false;
                }

        }
}
