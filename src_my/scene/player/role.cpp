#include "role.hpp"
#include "log.hpp"
#include "scene/item.hpp"
#include "scene/shop.hpp"
#include "scene/condition.hpp"
#include "scene/event.hpp"
#include "scene/drop.hpp"
#include "scene/recharge.hpp"
#include "scene/activity/activity_mgr.hpp"
#include "scene/quest.hpp"
#include "proto/data_player.pb.h"
#include "config/title_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "config/player_ptts.hpp"
#include "config/item_ptts.hpp"
#include "config/resource_ptts.hpp"
#include "config/activity_ptts.hpp"
#include "config/shop_ptts.hpp"
#include "utils/exception.hpp"
#include "utils/assert.hpp"
#include "utils/game_def.hpp"
#include "utils/pool.hpp"
#include <algorithm>
#include <limits>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                void role::serialize(pd::role *data) const {
                        data->set_gid(gid_);
                        serialize_role_data(data->mutable_data());
                        serialize_changing_data(data->mutable_changing_data());
                        serialize_other_data(data->mutable_other_data());
                        serialize_resources(data->mutable_resources());
                        serialize_items(data->mutable_items());
                        serialize_shop(data->mutable_shop());
                        serialize_relation(data->mutable_relation());
                        serialize_quests(data->mutable_quests());
                        serialize_xinshou(data->mutable_xinshou());
                        serialize_yunying(data->mutable_yunying());
                        serialize_activity(data->mutable_activity());
                        serialize_titles(data->mutable_titles());
                        serialize_recharge(data->mutable_recharge());
                        serialize_comments(data->mutable_comments());
                        serialize_social(data->mutable_social());
                        serialize_mails(data->mutable_mails());
                        serialize_chat_data(data->mutable_chat_data());
                        serialize_records(data->mutable_records());
                }

                void role::serialize_for_client(pd::role *data) const {
                        serialize(data);
                }

                void role::serialize_role_info(pd::role_info *data, pd::battle_origin_type bo) const {
                        serialize(data->mutable_simple_info());
                        if (bo != pd::BOT_NONE) {
                                *data->mutable_full_formation() = battle_team(bo);
                        }
                        serialize_resources(data->mutable_resources());
                        if (arena_data_.has_rank()) {
                                data->set_arena_rank(arena_data_.rank());
                        }
                        *data->mutable_greeting() = data_.greeting_;
                        data->set_welcome(data_.welcome_);
                        data->set_receive_gift(get_resource(pd::RECEIVE_GIFT));
                        if (has_present()) {
                                data->set_present(get_present());
                                data->set_present_count(present_count(get_present()));
                        }
                }

                void role::serialize_role_info_for_client(pd::role_info *info, bool simple) const {
                        if (simple) {
                                serialize(info->mutable_simple_info());
                        } else {
                                serialize_role_info(info, pd::BOT_NONE);
                                for (const auto& i : battle_.formations_) {
                                        *info->add_formations() = i;
                                }
                        }
                }

                void role::serialize(pd::role_simple_info *data) const {
                        data->set_gid(gid_);
                        data->set_name(gid2rolename_func_(gid_));
                        data->set_gender(data_.gender_);
                        data->set_level(data_.level_);
                        data->set_vip_exp(get_resource(pd::VIP_EXP));
                        data->set_icon(data_.icon_);
                        for (const auto& i : data_.custom_icons_) {
                                data->add_custom_icons(i);
                        }
                        data->set_custom_icon(data_.custom_icon_);
                        data->set_offline_time(data_.offline_time_);
                        data->set_login_ip(ip_);

                        if (title_.selected_ > 0) {
                                data->set_title(title_.selected_);
                        }
                }

                void role::serialize_for_room(pd::role_info *info, bool init) const {
                        serialize(info->mutable_simple_info());
                        if (init) {
                                serialize_spine(info->mutable_spine());
                                serialize_huanzhuang(info->mutable_huanzhuang());
                        }
                        serialize_resources(info->mutable_resources());
                }

                bool role::serialize_changed_data(pd::role *data) const {
                        auto ret = false;
                        data->set_gid(gid_);
                        if (role_data_changed_) {
                                serialize_role_data(data->mutable_data());
                                ret = true;
                        }
                        if (changing_data_changed_) {
                                serialize_changing_data(data->mutable_changing_data());
                                ret = true;
                        }
                        if (other_data_changed_) {
                                serialize_other_data(data->mutable_other_data());
                                ret = true;
                        }
                        if (resources_changed_) {
                                serialize_resources(data->mutable_resources());
                                ret = true;
                        }
                        if (items_changed_) {
                                serialize_items(data->mutable_items());
                                ret = true;
                        }
                        if (shop_changed_) {
                                serialize_shop(data->mutable_shop());
                                ret = true;
                        }
                        if (relation_changed_) {
                                serialize_relation(data->mutable_relation());
                                ret = true;
                        }
                        if (quests_changed_) {
                                serialize_quests(data->mutable_quests());
                                ret = true;
                        }
                        if (xinshou_changed_) {
                                serialize_xinshou(data->mutable_xinshou());
                                ret = true;
                        }
                        if (yunying_changed_) {
                                serialize_yunying(data->mutable_yunying());
                                ret = true;
                        }
                        if (activity_changed_) {
                                serialize_activity(data->mutable_activity());
                                ret = true;
                        }
                        if (titles_changed_) {
                                serialize_titles(data->mutable_titles());
                                ret = true;
                        }
                        if (recharge_changed_) {
                                serialize_recharge(data->mutable_recharge());
                                ret = true;
                        }
                        if (comments_changed_) {
                                serialize_comments(data->mutable_comments());
                                ret = true;
                        }
                        if (social_changed_) {
                                serialize_social(data->mutable_social());
                                ret = true;
                        }
                        if (mails_changed_) {
                                serialize_mails(data->mutable_mails());
                                ret = true;
                        }
                        if (chat_data_changed_) {
                                serialize_chat_data(data->mutable_chat_data());
                                ret = true;
                        }
                        return ret;
                }

                void role::serialize_role_data(pd::role_data *data) const {
                        data->set_name(data_.name_);
                        data->set_username(data_.username_);
                        data->set_gender(data_.gender_);
                        data->set_level(data_.level_);
                        data->set_last_levelup_time(data_.last_levelup_time_);
                        data->set_icon(data_.icon_);
                        *data->mutable_greeting() = data_.greeting_;
                        data->set_welcome(data_.welcome_);
                        for (const auto& i : data_.custom_icons_) {
                                data->add_custom_icons(i);
                        }
                        data->set_custom_icon(data_.custom_icon_);
                        data->set_fate(data_.fate_);
                        data->set_offline_time(data_.offline_time_);
                        data->set_last_login_time(data_.last_login_time_);
                        data->set_first_login_time(data_.first_login_time_);
                        data->set_last_online_day(data_.last_online_day_);
                        data->set_login_days(data_.login_days_);
                        data->set_today_online_seconds(data_.today_online_seconds_);
                        data->set_today_intimate_actor(data_.today_intimate_actor_);
                        data->set_kaichang(data_.kaichang_);
                        data->set_channel_id(data_.channel_id_);
                        for (const auto& i : data_.former_names_) {
                                data->add_former_names(i);
                        }
                }

                void role::serialize_changing_data(pd::role_changing_data *data) const {
                        data->set_eat_time(changing_data_.eat_time_);
                        data->set_eat_times(changing_data_.eat_times_);
                        data->set_share_day(changing_data_.share_day_);
                }

                void role::serialize_other_data(pd::role_other_data *data) const {
                        *data = other_data_;
                }

                void role::serialize_resources(pd::resources *data) const {
                        for (auto& i : resources_.values_) {
                                auto *resource = data->add_values();
                                resource->set_type(i.first);
                                resource->set_value(i.second);
                        }
                        for (auto& i : resources_.grow_time_) {
                                auto *gt = data->add_grow_time();
                                gt->set_type(i.first);
                                gt->set_time(i.second);
                        }
                        data->set_reset_day(resources_.reset_day_);
                        data->set_reset_monday(resources_.reset_monday_);
                }

                void role::serialize_xinshou(pd::xinshou *data) const {
                        for (auto i : xinshou_.passed_groups_) {
                                data->add_passed_groups(i);
                        }
                }

                void role::serialize_items(pd::item_array *data) const {
                        for (const auto& i : pttid2item_) {
                                auto *it = data->add_items();
                                i.second->serialize(it);
                        }
                        for (auto i : unorganized_items_) {
                                data->add_unorganized_items(i);
                        }
                        auto today = refresh_day();
                        if (day2used_item_counts_.count(today) > 0) {
                                data->set_item_used_day(today);
                                for (const auto& i : day2used_item_counts_.at(today)) {
                                        auto *euc = data->add_everyday_used_counts();
                                        euc->set_pttid(i.first);
                                        euc->set_count(i.second);
                                }
                        }
                }


                void role::serialize_shop(pd::shop *data) const {
                        for (const auto& i : shop_.mys_shops_) {
                                *data->add_mys_shops() = i.second;
                        }
                        for (const auto& i : shop_.records_) {
                                for (const auto& j : i.second) {
                                        auto *record = data->add_records();
                                        record->set_shop_pttid(i.first);
                                        record->set_good_id(j.first);
                                        record->set_shop_day(j.second.shop_day_);
                                        record->set_times(j.second.times_);
                                        record->set_all_times(j.second.all_times_);
                                }
                        }
                }


                void role::serialize_relation(pd::relation *data) const {
                        for (auto i : relation_.friends_) {
                                auto *f = data->add_friends();
                                f->set_role(i.first);
                                f->set_intimacy(i.second);
                        }
                        for (const auto& i : relation_.applications_) {
                                *data->add_applications() = i;
                        }
                        for (const auto& i : relation_.blacklist_) {
                                auto *b = data->add_blacklist();
                                b->set_role(i.first);
                                b->set_time(i.second);
                        }
                        for (auto i : relation_.follows_) {
                                auto *f = data->add_follows();
                                f->set_role(i.first);
                                f->set_time(i.second);
                        }
                        for (auto i : relation_.helpers_) {
                                data->add_helpers(i);
                        }
                        for (auto i : relation_.teammates_) {
                                data->add_teammates(i);
                        }
                }

                void role::serialize_accepted_quest(uint32_t pttid, pd::quest *data) const {
                        auto iter = quest_.accepted_.find(pttid);
                        ASSERT(iter != quest_.accepted_.end());
                        data->set_pttid(pttid);
                        const auto& aq = iter->second;
                        data->set_accept_time(aq.accept_time_);
                        if (!aq.registered_) {
                                data->set_not_registered(true);
                        }
                        for (const auto& i : aq.counters_) {
                                pd::quest_target qt;
                                qt.set_type(i.type_);
                                qt.set_cur(i.cur_);
                                *qt.mutable_param() = i.param_;
                                qt.set_cur_param_index(i.cur_param_index_);
                                *data->add_targets() = qt;
                        }
                }

                void role::serialize_quests(pd::quests *data) const {
                        for (const auto& i : quest_.accepted_) {
                                serialize_accepted_quest(i.first, data->add_accepted());
                        }
                        for (auto i : quest_.passed_) {
                                data->add_passed(i);
                        }
                        data->set_everyday_quest_day(quest_.everyday_quest_day_);
                        data->set_everyday_quest_monday(quest_.everyday_quest_monday_);
                        for (auto i : quest_.got_everyday_quest_rewards_) {
                                data->add_got_everyday_quest_rewards(i);
                        }
                        for (auto i : quest_.finished_everyday_quests_) {
                                data->add_finished_everyday_quests(i);
                        }
                        for (auto i : quest_.received_achievement_) {
                                data->add_received_achievements(i);
                        }
                }

                void role::serialize_yunying(pd::yunying *data) const {
                        *data = yunying_;
                }

                void role::serialize_activity(pd::activity *data) const {
                        for (auto i : activity_.login_days_.got_reward_days_) {
                                data->mutable_login_days()->add_got_reward_days(i);
                        }
                        data->mutable_online_time()->set_got_reward_day(activity_.online_time_.got_reward_day_);
                        for (auto i : activity_.online_time_.got_reward_idx_) {
                                data->mutable_online_time()->add_got_reward_idx(i);
                        }
                        for (const auto& i : activity_.seven_days_) {
                                auto *osd = data->mutable_seven_days()->add_one_seven_days();
                                osd->set_begin_day(i.first);
                                osd->set_got_reward(i.second.got_reward_);
                                osd->set_quests_accepted(i.second.quests_accepted_);
                                osd->set_fuli_quests_accepted(i.second.fuli_quests_accepted_);
                                osd->set_passed(i.second.passed_);
                        }
                        data->mutable_seven_days()->set_reset_point(activity_.reset_point_);

                        if (activity_.qiandao_.month_ != 0u) {
                                auto *qiandao = data->mutable_qiandao_data();
                                qiandao->set_month(activity_.qiandao_.month_);
                                for (auto i : activity_.qiandao_.mdays_) {
                                        qiandao->add_days(i);
                                }
                                for (auto i : activity_.qiandao_.got_leiqian_days_) {
                                        qiandao->add_received_leiqian_days(i);
                                }
                        }

                        for (const auto& i : activity_.everyday_libao_) {
                                auto *el = data->add_everyday_libao();
                                el->set_pttid(i.first);
                                el->set_day(i.second);
                        }

                        for (const auto& i : activity_.chaozhi_libao_idxs_) {
                                auto *cl = data->add_chaozhi_libao();
                                cl->set_type(i.first);
                                cl->set_buy_idx(i.second);
                        }
                        data->set_recharge_get_first_reward(activity_.recharge_get_first_reward_);
                        data->set_leiji_consume_count(activity_.leiji_consume_count_);
                        data->set_leiji_recharge_count(activity_.leiji_recharge_count_);
                        auto *qianzhuang = data->mutable_activity_qianzhuang();
                        qianzhuang->set_vip_buy(activity_.qianzhuang_.vip_buy_);
                        qianzhuang->set_emperor_buy(activity_.qianzhuang_.emperor_buy_);
                        for (auto i : activity_.qianzhuang_.vip_got_) {
                                qianzhuang->add_vip_got(i);
                        }
                        for (auto i : activity_.qianzhuang_.emperor_got_) {
                                qianzhuang->add_emperor_got(i);
                        }
                        for (auto i : activity_.qianzhuang_.fund_got_) {
                                qianzhuang->add_fund_got(i);
                        }
                        auto *limit_libao = data->mutable_limit_libao();
                        for(auto i : activity_.limit_libao_.bought_) {
                                limit_libao->add_bought(i);
                        }
                        for(const auto& i : activity_.limit_libao_.passed_) {
                                auto *passed = limit_libao->add_passed();
                                passed->set_pttid(i.first);
                                passed->set_until_time(i.second);
                        }
                        limit_libao->set_cur(activity_.limit_libao_.cur_);
                        limit_libao->set_time(activity_.limit_libao_.time_);

                        data->mutable_festival()->set_start_day(activity_.festival_.start_day_);
                        data->mutable_festival()->set_duration(activity_.festival_.duration_);
                        data->mutable_festival()->set_pttid(activity_.festival_.pttid_);
                        auto* festival = data->mutable_festival();
                        for (const auto& i : activity_.festival_.festival_exchange_times_) {
                                auto *exchange_times = festival->add_festival_exchange();
                                exchange_times->set_id(i.first);
                                for (const auto& j : i.second) {
                                        auto* times = exchange_times->add_exchange_times();
                                        times->set_exchange(j.first);
                                        times->set_times(j.second);
                                }
                        }
                        data->mutable_prize_wheel()->set_start_day(activity_.prize_wheel_.start_day_);
                        data->mutable_prize_wheel()->set_duration(activity_.prize_wheel_.duration_);
                        data->mutable_prize_wheel()->set_pttid(activity_.prize_wheel_.pttid_);
                        data->mutable_prize_wheel()->set_ongoing(activity_.prize_wheel_.ongoing_);

                        data->mutable_tea_party()->set_start_day(activity_.tea_party_.start_day_);
                        data->mutable_tea_party()->set_duration(activity_.tea_party_.duration_);
                        data->mutable_tea_party()->set_pttid(activity_.tea_party_.pttid_);
                        data->mutable_tea_party()->set_ongoing(activity_.tea_party_.ongoing_);
                        for (auto i : activity_.tea_party_.got_favor_rewards_) {
                                data->mutable_tea_party()->add_got_favor_rewards(i);
                        }

                        data->mutable_continue_recharge()->set_start_day(activity_.continue_recharge_.start_day_);
                        data->mutable_continue_recharge()->set_duration(activity_.continue_recharge_.duration_);
                        data->mutable_continue_recharge()->set_pttid(activity_.continue_recharge_.pttid_);
                        data->mutable_continue_recharge()->set_cur_quest(activity_.continue_recharge_.cur_quest_);
                        data->mutable_continue_recharge()->set_quest_idx(activity_.continue_recharge_.quest_idx_);
                        data->mutable_continue_recharge()->set_update_quests_activate_day(activity_.continue_recharge_.update_quests_activate_day_);
                        data->mutable_continue_recharge()->set_day_recharge(activity_.continue_recharge_.day_recharge_);

                        auto *discount = data->mutable_discount_goods();
                        discount->set_start_day(activity_.discount_goods_.start_day_);
                        discount->set_duration(activity_.discount_goods_.duration_);
                        discount->set_pttid(activity_.discount_goods_.pttid_);

                        for (auto i : activity_.daiyanren_.got_reward_) {
                                data->mutable_daiyanren()->add_got_reward(i);
                        }
                        data->mutable_daiyanren()->set_finished(activity_.daiyanren_.finished_);

                        data->mutable_limit_play()->set_start_day(activity_.limit_play_.start_day_);
                        data->mutable_limit_play()->set_duration(activity_.limit_play_.duration_);
                        data->mutable_limit_play()->set_pttid(activity_.limit_play_.pttid_);

                        data->mutable_leiji_recharge()->set_start_day(activity_.leiji_recharge_.start_day_);
                        data->mutable_leiji_recharge()->set_duration(activity_.leiji_recharge_.duration_);
                        data->mutable_leiji_recharge()->set_pttid(activity_.leiji_recharge_.pttid_);

                        data->mutable_leiji_consume()->set_start_day(activity_.leiji_consume_.start_day_);
                        data->mutable_leiji_consume()->set_duration(activity_.leiji_consume_.duration_);
                        data->mutable_leiji_consume()->set_pttid(activity_.leiji_consume_.pttid_);

                        data->mutable_recharge_rank()->set_start_day(activity_.recharge_rank_.start_day_);
                        data->mutable_recharge_rank()->set_duration(activity_.recharge_rank_.duration_);
                }

                void role::serialize_titles(pd::title_array *data) const {
                        if (title_.selected_ > 0) {
                                data->set_selected(title_.selected_);
                        }
                        for (const auto& i : title_.titles_) {
                                *data->add_titles() = i.second;
                        }
                }

                void role::serialize_recharge(pd::recharge *data) const {
                        data->set_paid_money(recharge_.paid_money_);
                        for (const auto& i : recharge_.gid2pttid_) {
                                auto *o = data->add_orders();
                                o->set_pttid(i.second);
                                o->set_gid(i.first);
                        }
                        for (auto i : recharge_.recharged_pttids_) {
                                data->add_recharged_pttids(i);
                        }
                        for (const auto& i : recharge_.pttid2day_) {
                                auto *d = data->add_days();
                                d->set_pttid(i.first);
                                d->set_day(i.second);
                        }
                        for (const auto& i : recharge_.pttid2last_process_day_) {
                                auto *d = data->add_last_process_days();
                                d->set_pttid(i.first);
                                d->set_day(i.second);
                        }
                        for (auto i : recharge_.got_vip_boxes_) {
                                data->add_got_vip_boxes(i);
                        }
                        for (auto i : recharge_.got_vip_buy_boxes_) {
                                data->add_got_vip_buy_boxes(i);
                        }
                        for (auto i : recharge_.recharged_gids_) {
                                data->add_recharged_gids(i);
                        }
                }

                void role::serialize_comments(pd::comment_array *data) const {
                        data->set_gid_seed(comments_.gid_seed_);
                        for (const auto& i : comments_.comment_list_) {
                                *data->add_comments() = i;
                        }
                }

                void role::serialize_social(pd::social *data) const {
                        for (const auto& i : social_.presents_) {
                                auto *p = data->add_presents();
                                p->set_pttid(i.first);
                                p->set_count(i.second);
                        }
                        data->set_upvote_count(social_.upvote_count_);
                        data->set_upvote_roles_day(social_.upvote_roles_day_);
                        for (auto i : social_.upvote_roles_) {
                                data->add_upvote_roles(i);
                        }
                        for (auto i : social_.upvoted_roles_) {
                                data->add_upvoted_roles(i);
                        }
                }

                void role::serialize_mails(pd::mail_array *data) const {
                        for (const auto& i : mails_) {
                                if (system_clock::now() - system_clock::from_time_t(i.second.time()) >= MAIL_EXPIRE_DURATION) {
                                        continue;
                                }
                                *data->add_mails() = i.second;
                        }
                }

                void role::serialize_chat_data(pd::chat_data_array *data) const {
                        for (const auto& i : chat_data_.type2time_) {
                                auto *ca = data->add_chats();
                                ca->set_type(i.first);
                                ca->set_last_send_time(i.second);
                        }
                }

                void role::serialize_records(pd::record_array *data) const {
                        data->set_gid_seed(records_.gid_seed_);
                        for (const auto& i : records_.records_) {
                                auto& rs = *data->add_records();
                                rs.set_type(i.first);
                                for (const auto& j : i.second) {
                                        auto& os = *rs.add_records();
                                        os.set_sub_type(j.first);
                                        os.set_read(j.second.read_);
                                        for (const auto& k : j.second.records_) {
                                                *os.add_entities() = k;
                                        }
                                }
                        }
                }

                void role::parse_from(const pd::role& data) {
                        gid_ = data.gid();
                        parse_role_data(&data.data());
                        parse_changing_data(&data.changing_data());
                        parse_other_data(&data.other_data());
                        parse_resources(&data.resources());
                        parse_items(&data.items());
                        parse_shop(&data.shop());
                        parse_relation(&data.relation());
                        parse_quests(&data.quests());
                        parse_xinshou(&data.xinshou());
                        parse_yunying(&data.yunying());
                        parse_activity(&data.activity());
                        parse_titles(&data.titles());
                        parse_recharge(&data.recharge());
                        parse_comments(&data.comments());
                        parse_social(&data.social());
                        parse_mails(&data.mails());
                        parse_chat_data(&data.chat_data());
                }

                void role::mirror_update(const pd::role& data) {
                        parse_role_data_by_mirror(&data.data());
                        role_data_changed_ = true;
                        parse_changing_data(&data.changing_data());
                        changing_data_changed_ = true;
                        parse_resources(&data.resources());
                        resources_changed_ = true;
                        parse_shop(&data.shop());
                        shop_changed_ = true;

                        parse_activity(&data.activity());
                        activity_changed_ = true;
                        parse_titles(&data.titles());
                        titles_changed_ = true;
                }

                void role::parse_role_data(const pd::role_data *data) {
                        data_.name_ = data->name();
                        data_.username_ = data->username();
                        data_.gender_ = data->gender();
                        data_.level_ = data->level();
                        data_.last_levelup_time_ = data->last_levelup_time();
                        if (data->has_icon()) {
                                data_.icon_ = data->icon();
                        }
                        if (data->has_greeting()) {
                                data_.greeting_ = data->greeting();
                        }
                        if (data->has_welcome()) {
                                data_.welcome_ = data->welcome();
                        }
                        for (const auto& i : data->custom_icons()) {
                                data_.custom_icons_.push_back(i);
                        }
                        if (data->has_custom_icon()) {
                                data_.custom_icon_ = data->custom_icon();
                        }
                        data_.fate_ = data->fate();
                        data_.offline_time_ = data->offline_time();
                        data_.last_login_time_ = data->last_login_time();
                        data_.first_login_time_ = data->first_login_time();
                        data_.last_online_day_ = data->last_online_day();
                        data_.login_days_ = data->login_days();
                        data_.today_online_seconds_ = data->today_online_seconds();
                        data_.today_intimate_actor_ = data->today_intimate_actor();
                        data_.kaichang_ = data->kaichang();
                        if (data->has_channel_id()) {
                                data_.channel_id_ = data->channel_id();
                        }
                        for (const auto& i : data->former_names()) {
                                data_.former_names_.insert(i);
                        }
                }

                void role::parse_role_data_by_mirror(const pd::role_data *data) {
                        data_.level_ = data->level();
                }

                void role::parse_changing_data(const pd::role_changing_data *data) {
                        changing_data_.eat_time_ = data->eat_time();
                        auto day = refresh_day_from_time_t(changing_data_.eat_time_);
                        if (day == refresh_day()) {
                                changing_data_.eat_times_ = data->eat_times();
                        } else {
                                changing_data_.eat_times_ = 0;
                        }
                        changing_data_.share_day_ = data->share_day();
                }

                void role::parse_other_data(const pd::role_other_data *data) {
                        other_data_ = *data;
                }

                void role::parse_resources(const pd::resources *data) {
                        for (const auto& i : data->values()) {
                                resources_.values_[i.type()] = i.value();
                        }
                        for (const auto& i : data->grow_time()) {
                                resources_.grow_time_[i.type()] = i.time();
                        }
                        resources_.reset_day_ = data->reset_day();
                        resources_.reset_monday_ = data->reset_monday();
                }

                void role::parse_xinshou(const pd::xinshou *data) {
                        for (auto i : data->passed_groups()) {
                                xinshou_.passed_groups_.insert(i);
                        }
                }

                void role::parse_items(const pd::item_array *data) {
                        for (const auto& i : data->items()) {
                                auto it = make_shared<item>();
                                it->parse(&i);
                                pttid2item_[it->pttid()] = it;
                        }
                        for (auto i : data->unorganized_items()) {
                                unorganized_items_.push_back(i);
                        }
                        auto today = refresh_day();
                        if (data->item_used_day() == today) {
                                auto& dic = day2used_item_counts_[today];
                                for (const auto& i : data->everyday_used_counts()) {
                                        dic[i.pttid()] = i.count();
                                }
                        }
                }
                void role::parse_shop(const pd::shop *data) {
                        for (const auto& i : data->mys_shops()) {
                                shop_.mys_shops_[i.pttid()] = i;
                        }
                        for (const auto& i : data->records()) {
                                auto& record = shop_.records_[i.shop_pttid()][i.good_id()];
                                record.shop_day_ = i.shop_day();
                                record.times_ = i.times();
                                record.all_times_ = i.all_times();
                        }
                }

                void role::parse_relation(const pd::relation *relation) {
                        for (const auto& i : relation->friends()) {
                                relation_.friends_[i.role()] = i.intimacy();
                        }
                        for (const auto& i : relation->applications()) {
                                relation_.applications_.push_back(i);
                                relation_.applications_iters_[i.role()] = --relation_.applications_.end();
                        }
                        for (const auto& i : relation->blacklist()) {
                                relation_.blacklist_[i.role()] = i.time();
                        }
                        for (auto i : relation->follows()) {
                                relation_.follows_[i.role()] = i.time();
                        }
                        for (auto i : relation->helpers()) {
                                relation_.helpers_.insert(i);
                        }
                        for (auto i : relation->teammates()) {
                                relation_.teammates_.insert(i);
                        }
                }

                void role::parse_quests(const pd::quests *data) {
                        for (const auto& i : data->accepted()) {
                                auto& aq = quest_.accepted_[i.pttid()];
                                aq.accept_time_ = i.accept_time();
                                for (auto j = 0; j < i.targets_size(); ++j) {
                                        const auto& target = i.targets(j);
                                        aq.counters_.emplace_back();
                                        auto& counter = aq.counters_.back();
                                        counter.type_ = target.type();
                                        counter.cur_ = target.cur();
                                        counter.cur_param_index_ = target.cur_param_index();
                                        if (target.has_param()) {
                                                counter.param_ = target.param();
                                        } else {
                                                const auto& ptt = PTTS_GET(quest, i.pttid());
                                                const auto& ptt_target = ptt.targets(j);
                                                const auto& param = ptt_target.params(counter.cur_param_index_);
                                                for (auto i : param.arg()) {
                                                        counter.param_.add_arg(i);
                                                }
                                                counter.param_.set_need(param.need());
                                        }
                                }
                                if (!i.not_registered()) {
                                        quest_register_event(i.pttid());
                                }
                        }
                        for (auto i : data->passed()) {
                                quest_.passed_.insert(i);
                        }

                        quest_.everyday_quest_day_ = data->everyday_quest_day();
                        quest_.everyday_quest_monday_ = data->everyday_quest_monday();
                        for (auto i : data->got_everyday_quest_rewards()) {
                                quest_.got_everyday_quest_rewards_.insert(i);
                        }
                        for (auto i : data->finished_everyday_quests()) {
                                quest_.finished_everyday_quests_.insert(i);
                        }
                        for (auto i : data->received_achievements()) {
                                quest_.received_achievement_.insert(i);
                        }
                }

                void role::parse_yunying(const pd::yunying *data) {
                        yunying_ = *data;
                }

                void role::parse_activity(const pd::activity *data) {
                        for (auto i : data->login_days().got_reward_days()) {
                                activity_.login_days_.got_reward_days_.insert(i);
                        }
                        activity_.online_time_.got_reward_day_ = data->online_time().got_reward_day();
                        for (auto i : data->online_time().got_reward_idx()) {
                                activity_.online_time_.got_reward_idx_.insert(i);
                        }
                        for (const auto& i : data->seven_days().one_seven_days()) {
                                auto& osd = activity_.seven_days_[i.begin_day()];
                                osd.got_reward_ = i.got_reward();
                                osd.quests_accepted_ = i.quests_accepted();
                                osd.fuli_quests_accepted_ = i.fuli_quests_accepted();
                                osd.passed_ = i.passed();
                        }
                        activity_.reset_point_ = data->seven_days().reset_point();

                        if (data->has_qiandao_data()) {
                                activity_.qiandao_.month_ = data->qiandao_data().month();
                                for (auto i : data->qiandao_data().days()) {
                                        activity_.qiandao_.mdays_.insert(i);
                                }
                                for (auto i : data->qiandao_data().received_leiqian_days()) {
                                        activity_.qiandao_.got_leiqian_days_.insert(i);
                                }
                        }

                        for (const auto& i : data->everyday_libao()) {
                                activity_.everyday_libao_[i.pttid()] = i.day();
                        }

                        for (const auto& i : data->chaozhi_libao()) {
                                activity_.chaozhi_libao_idxs_[i.type()] = i.buy_idx();
                        }
                        activity_.recharge_get_first_reward_ = data->recharge_get_first_reward();
                        activity_.leiji_consume_count_ = data->leiji_consume_count();
                        activity_.leiji_recharge_count_ = data->leiji_recharge_count();
                        activity_.qianzhuang_.vip_buy_ = data->activity_qianzhuang().vip_buy();
                        activity_.qianzhuang_.emperor_buy_ = data->activity_qianzhuang().emperor_buy();
                        for (auto i : data->activity_qianzhuang().vip_got()) {
                                activity_.qianzhuang_.vip_got_.insert(i);
                        }
                        for (auto i : data->activity_qianzhuang().emperor_got()) {
                                activity_.qianzhuang_.emperor_got_.insert(i);
                        }
                        for (auto i : data->activity_qianzhuang().fund_got()) {
                                activity_.qianzhuang_.fund_got_.insert(i);
                        }
                        for (auto i : data->limit_libao().bought()) {
                                activity_.limit_libao_.bought_.insert(i);
                        }
                        for (auto i : data->limit_libao().passed()) {
                                activity_.limit_libao_.passed_[i.pttid()] = i.until_time();
                        }
                        activity_.limit_libao_.cur_ = data->limit_libao().cur();
                        activity_.limit_libao_.time_ = data->limit_libao().time();

                        activity_.festival_.start_day_ = data->festival().start_day();
                        activity_.festival_.duration_ = data->festival().duration();
                        activity_.festival_.pttid_ = data->festival().pttid();
                        for (const auto& i : data->festival().festival_exchange()) {
                                for (const auto& j : i.exchange_times()) {
                                        activity_.festival_.festival_exchange_times_[i.id()][j.exchange()] = j.times();
                                }
                        }
                        activity_.prize_wheel_.start_day_ = data->prize_wheel().start_day();
                        activity_.prize_wheel_.duration_ = data->prize_wheel().duration();
                        activity_.prize_wheel_.pttid_ = data->prize_wheel().pttid();
                        activity_.prize_wheel_.ongoing_ = data->prize_wheel().ongoing();

                        activity_.tea_party_.start_day_ = data->tea_party().start_day();
                        activity_.tea_party_.duration_ = data->tea_party().duration();
                        activity_.tea_party_.pttid_ = data->tea_party().pttid();
                        activity_.tea_party_.ongoing_ = data->tea_party().ongoing();
                        for (auto i : data->tea_party().got_favor_rewards()) {
                                activity_.tea_party_.got_favor_rewards_.insert(i);
                        }

                        activity_.continue_recharge_.start_day_ = data->continue_recharge().start_day();
                        activity_.continue_recharge_.duration_ = data->continue_recharge().duration();
                        activity_.continue_recharge_.pttid_ = data->continue_recharge().pttid();
                        activity_.continue_recharge_.cur_quest_ = data->continue_recharge().cur_quest();
                        activity_.continue_recharge_.quest_idx_ = data->continue_recharge().quest_idx();
                        activity_.continue_recharge_.update_quests_activate_day_ = data->continue_recharge().update_quests_activate_day();
                        activity_.continue_recharge_.day_recharge_ = data->continue_recharge().day_recharge();

                        activity_.discount_goods_.start_day_ = data->discount_goods().start_day();
                        activity_.discount_goods_.duration_ = data->discount_goods().duration();
                        activity_.discount_goods_.pttid_ = data->discount_goods().pttid();
                        if (activity_.discount_goods_.pttid_ > 0) {
                                auto today = refresh_day();
                                if (today < activity_.discount_goods_.start_day_ ||
                                    today > activity_.discount_goods_.start_day_ + activity_.discount_goods_.duration_) {
                                        const auto& ptt = PTTS_GET(activity_discount_goods, activity_.discount_goods_.pttid_);
                                        if (shop_.records_.count(ptt.shop_pttid()) > 0) {
                                                shop_.records_.erase(ptt.shop_pttid());
                                        }
                                }
                        }

                        for (auto i : data->daiyanren().got_reward()) {
                                activity_.daiyanren_.got_reward_.insert(i);
                        }
                        activity_.daiyanren_.finished_ = data->daiyanren().finished();

                        activity_.limit_play_.start_day_ = data->limit_play().start_day();
                        activity_.limit_play_.duration_ = data->limit_play().duration();
                        activity_.limit_play_.pttid_ = data->limit_play().pttid();

                        activity_.leiji_recharge_.start_day_ = data->leiji_recharge().start_day();
                        activity_.leiji_recharge_.duration_ = data->leiji_recharge().duration();
                        activity_.leiji_recharge_.pttid_ = data->leiji_recharge().pttid();

                        activity_.leiji_consume_.start_day_ = data->leiji_consume().start_day();
                        activity_.leiji_consume_.duration_ = data->leiji_consume().duration();
                        activity_.leiji_consume_.pttid_ = data->leiji_consume().pttid();

                        activity_.recharge_rank_.start_day_ = data->recharge_rank().start_day();
                        activity_.recharge_rank_.duration_ = data->recharge_rank().duration();
                }

                void role::parse_titles(const pd::title_array *data) {
                        title_.selected_ = data->selected();
                        for (const auto& i : data->titles()) {
                                title_.titles_[i.pttid()] = i;
                                const auto& ptt = PTTS_GET(title, i.pttid());
                                if (ptt.days() > 0) {
                                        auto seconds_passed = system_clock::now() - system_clock::from_time_t(i.got_time());
                                        auto title_seconds = seconds(ptt.days() * DAY);
                                        if (seconds_passed >= title_seconds) {
                                                title_.titles_.erase(i.pttid());
                                                if (title_.selected_ == i.pttid()) {
                                                        title_.selected_ = 0;
                                                }
                                        }
                                }
                        }
                        set<uint32_t> titles;
                        for (const auto& i : title_.titles_) {
                                titles.insert(i.first);
                        }
                        for (auto& i : actors_.pttid2actor_) {
                                i.second->set_titles(titles);
                        }
                }

                void role::parse_recharge(const pd::recharge *data) {
                        recharge_.paid_money_ = data->paid_money();
                        for (const auto& i : data->orders()) {
                                recharge_.gid2pttid_[i.gid()] = i.pttid();
                                recharge_.pttid2gids_[i.pttid()].insert(i.gid());
                        }
                        for (auto i : data->recharged_pttids()) {
                                recharge_.recharged_pttids_.insert(i);
                        }
                        for (const auto& i : data->days()) {
                                recharge_.pttid2day_[i.pttid()] = i.day();
                        }
                        for (const auto& i : data->last_process_days()) {
                                recharge_.pttid2last_process_day_[i.pttid()] = i.day();
                        }
                        for (auto i : data->got_vip_boxes()) {
                                recharge_.got_vip_boxes_.insert(i);
                        }
                        for (auto i : data->got_vip_buy_boxes()) {
                                recharge_.got_vip_buy_boxes_.insert(i);
                        }
                        for (auto i : data->recharged_gids()) {
                                recharge_.recharged_gids_.insert(i);
                        }
                }

                void role::parse_comments(const pd::comment_array *data) {
                        comments_.gid_seed_ = data->gid_seed();
                        for (const auto& i : data->comments()) {
                                comments_.comment_list_.push_back(i);
                                comments_.gid2comment_[i.gid()] = i;
                                comments_.role2last_comment_time_[i.role()] = max(comments_.role2last_comment_time_[i.role()], i.time());
                        }
                }

                void role::parse_social(const pd::social *data) {
                        for (const auto& i : data->presents()) {
                                social_.presents_[i.pttid()] = i.count();
                        }
                        social_.upvote_count_ = data->upvote_count();
                        social_.upvote_roles_day_ = data->upvote_roles_day();
                        for (auto i : data->upvote_roles()) {
                                social_.upvote_roles_.insert(i);
                        }
                        for (auto i : data->upvoted_roles()) {
                                social_.upvoted_roles_.push_back(i);
                        }
                }

                void role::parse_mails(const pd::mail_array *data) {
                        for (const auto& i : data->mails()) {
                                if (system_clock::now() - system_clock::from_time_t(i.time()) >= MAIL_EXPIRE_DURATION) {
                                        continue;
                                }
                                mails_[i.gid()] = i;
                        }
                }

                void role::parse_chat_data(const pd::chat_data_array *data) {
                        for (const auto& i : data->chats()) {
                                chat_data_.type2time_[i.type()] = i.last_send_time();
                        }
                }

                void role::parse_records(const pd::record_array *data) {
                        records_.gid_seed_ = data->gid_seed();
                        for (const auto& i : data->records()) {
                                auto& rs = records_.records_[i.type()];
                                for (const auto& j : i.records()) {
                                        auto& os = rs[j.sub_type()];
                                        os.read_ = j.read();
                                        for (const auto& k : j.entities()) {
                                                os.records_.push_back(k);
                                        }
                                }
                        }
                }

                bool role::plot_option_locked(uint32_t pttid, int option_idx) const {
                        const auto& ptt = PTTS_GET(plot_options, pttid);
                        if (ptt._group() > 0) {
                                auto iter = plot_.group_locked_options_.find(ptt._group());
                                return iter != plot_.group_locked_options_.end() && iter->second.count(option_idx) > 0;
                        } else {
                                auto iter = plot_.locked_options_.find(pttid);
                                return iter != plot_.locked_options_.end() && iter->second.count(option_idx) > 0;
                        }
                }
                void role::serialize_mys_shop(uint32_t pttid, pd::mys_shop *data) const {
                        ASSERT(shop_.mys_shops_.count(pttid) > 0);
                        *data = shop_.mys_shops_.at(pttid);
                }

                uint32_t role::vip_level() const {
                        return vip_exp_to_level(get_resource(pd::VIP_EXP));
                }

                uint32_t role::spouse_vip_level() const {
                        return vip_exp_to_level(marriage_.spouse_vip_exp_);
                }

                void role::try_levelup() {
                        pd::event_res res;
                        auto old_level = data_.level_;
                        auto last_levelup_time = data_.last_levelup_time_;
                        bool send_bi = false;
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_LEVELUP);

                        do {
                                const auto& levelup_ptt = PTTS_GET(role_levelup, data_.level_);
                                auto ret = condition_check(levelup_ptt.conditions(), *this);
                                if (ret != pd::OK) {
                                        break;
                                }

                                auto events = levelup_ptt.events();
                                auto *e = events.mutable_events(1);
                                ASSERT(e->type() == pd::event::ACTOR_LEVELUP);
                                e->add_arg(to_string(role_actor_->pttid()));

                                res = event_process(events, *this, ce, &res, false);
                                send_bi = true;
                                refresh_everyday_quests(res);
                        } while (true);
                        if (send_bi) {
                                send_event_res_to_bi(*this, res, ce.origin());
                        }

                        if (data_.level_ > old_level) {
                                if (!mirror_role()) {
                                        //bi::instance().levelup(username_func_(), yci_, ip_, gid_, data_.level_, old_level, last_levelup_time, first_login_time());

                                }
                                levelup_cb_(gid(), old_level, data_.level_, res, mansion_exist());
                        }
                }

                shared_ptr<item> role::add_item(uint32_t pttid, int count, bool confirmed) {
                        if (pttid2item_.count(pttid) <= 0) {
                                pttid2item_[pttid] = make_shared<item>(pttid, count);
                                unorganized_items_.push_back(pttid);
                        } else {
                                pttid2item_.at(pttid)->change_count(count);
                        }

                        pttid2item_.at(pttid)->set_confirmed(confirmed);
                        items_changed_ = true;

                        on_event(ECE_ADD_ITEM, { pttid });
                        return pttid2item_.at(pttid);
                }

                shared_ptr<item> role::remove_item(uint32_t pttid, int count) {
                        if (pttid2item_.count(pttid) == 0) {
                                return make_shared<item>(pttid, 0);
                        }
                        auto i = pttid2item_.at(pttid);
                        i->change_count(-count);
                        items_changed_ = true;

                        if (i->count() == 0) {
                                pttid2item_.erase(pttid);
                        }

                        on_event(ECE_REMOVE_ITEM, { pttid });
                        return i;
                }

                shared_ptr<item> role::get_item(uint32_t pttid) const {
                        return pttid2item_.count(pttid) > 0 ? pttid2item_.at(pttid) : nullptr;
                }

                int role::item_count(uint32_t pttid) const {
                        return pttid2item_.count(pttid) > 0 ? pttid2item_.at(pttid)->count() : 0;
                }

                void role::update_item_everyday_use_count(uint32_t pttid, int count) {
                        auto today = refresh_day();
                        if (day2used_item_counts_.count(today) == 0) {
                                day2used_item_counts_.clear();
                        }
                        day2used_item_counts_[today][pttid] += count;
                        items_changed_ = true;
                }

                int role::get_item_everyday_use_count(uint32_t pttid) const {
                        auto today = refresh_day();
                        if (day2used_item_counts_.count(today) > 0 && day2used_item_counts_.at(today).count(pttid) > 0) {
                                return day2used_item_counts_.at(today).at(pttid);
                        }
                        return 0;
                }

                void role::confirm_items() {
                        for (auto& i : pttid2item_) {
                                if (!i.second->confirmed()) {
                                        i.second->set_confirmed(true);
                                        items_changed_ = true;
                                }
                        }
                }

                void role::organize_items() {
                        if (unorganized_items_.empty()) {
                                return;
                        }
                        unorganized_items_.clear();
                        items_changed_ = true;
                }


                uint32_t role::shop_buy_times(uint32_t pttid, uint32_t good_id) const {
                        if (shop_.records_.count(pttid) > 0) {
                                const auto& one_shop_records = shop_.records_.at(pttid);
                                if (one_shop_records.count(good_id) > 0) {
                                        const auto& record = one_shop_records.at(good_id);
                                        const auto& ptt = PTTS_GET(shop, pttid);
                                        if (ptt.has__discount_activity_id()) {
                                                if (refresh_day() >= activity_.discount_goods_.start_day_ &&
                                                    refresh_day() <= activity_.discount_goods_.duration_) {
                                                        return record.times_;
                                                }
                                        } else {
                                                if (record.shop_day_ == refresh_day()) {
                                                        return record.times_;
                                                }
                                        }
                                }
                        }
                        return 0u;
                }

                uint32_t role::shop_buy_all_times(uint32_t pttid, uint32_t good_id) const {
                        if (shop_.records_.count(pttid) > 0) {
                                const auto& one_shop_records = shop_.records_.at(pttid);
                                if (one_shop_records.count(good_id) > 0) {
                                        const auto& record = one_shop_records.at(good_id);
                                        return record.all_times_;
                                }
                        }
                        return 0u;
                }

                const role::shop_record& role::shop_add_buy_times(uint32_t pttid, uint32_t good_id) {
                        auto today = refresh_day();

                        const auto& ptt = PTTS_GET(shop, pttid);
                        if (ptt.has__discount_activity_id()) {
                                if (today < activity_.discount_goods_.start_day_ || today > activity_.discount_goods_.start_day_ + activity_.discount_goods_.duration_) {
                                        shop_.records_[pttid].clear();
                                }
                        }

                        if (shop_.records_.count(pttid) > 0) {
                                auto& one_shop_records = shop_.records_.at(pttid);
                                if (one_shop_records.count(good_id) > 0) {
                                        auto& record = one_shop_records.at(good_id);
                                        record.all_times_ += 1;
                                        if (ptt.has__discount_activity_id()) {
                                                record.shop_day_ = today;
                                                record.times_ += 1;
                                        } else {
                                                if (record.shop_day_ == today) {
                                                        record.times_ += 1;
                                                } else {
                                                        record.shop_day_ = today;
                                                        record.times_ = 1;
                                                }
                                        }
                                } else {
                                        auto& record = one_shop_records[good_id];
                                        record.shop_day_ = today;
                                        record.times_ = 1;
                                        record.all_times_ = 1;
                                }

                        } else {
                                auto& record = shop_.records_[pttid][good_id];
                                record.shop_day_ = today;
                                record.times_ = 1;
                                record.all_times_ = 1;
                        }
                        shop_changed_ = true;
                        return shop_.records_.at(pttid).at(good_id);
                }

                void role::accept_ui_quest() {
                        const auto& ptts = PTTS_GET_ALL(ui_quest);
                        for (auto& i : ptts) {
                                ASSERT(i.second.quests_size() > 0);
                                if (quest_has_accepted(i.second.quests(0))) {
                                        continue;
                                }
                                quest_accept(i.second.quests(0));
                                for (auto j : i.second.accept_record_quests()) {
                                        if (quest_has_accepted(j)) {
                                                continue;
                                        }
                                        quest_accept(j);
                                }
                        }
                }

                void role::accept_achievement_quest() {
                        const auto& ptts = PTTS_GET_ALL(achievement);
                        for (auto& i : ptts) {
                                if (quest_has_accepted(i.second.quest())) {
                                        continue;
                                }
                                if (quest_.received_achievement_.count(i.second.quest()) > 0) {
                                        continue;
                                }
                                quest_accept(i.second.quest());
                        }
                }

                void role::quest_accept(uint32_t pttid, bool register_event, uint32_t accept_time) {
                        SPLAYER_DLOG << " quest accept " << pttid;
                        ASSERT_MSG(quest_.accepted_.count(pttid) <= 0, to_string(pttid));
                        const auto& ptt = PTTS_GET(quest, pttid);
                        auto& aq = quest_.accepted_[pttid];
                        if (accept_time == 0) {
                                aq.accept_time_ = system_clock::to_time_t(system_clock::now());
                        } else {
                                aq.accept_time_ = accept_time;
                        }
                        for (const auto& i : ptt.targets()) {
                                aq.counters_.emplace_back();
                                auto& counter = aq.counters_.back();
                                counter.type_ = i.type();
                                counter.cur_param_index_ = 0;
                                ASSERT(i.params_size() > 0);

                                auto first_target = i.params(counter.cur_param_index_);
                                for (const auto i : first_target.arg()) {
                                        counter.param_.add_arg(i);
                                }
                                counter.param_.set_need(first_target.need());
                                switch (i.type()) {
                                case pd::TURN_IN_ITEM: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        auto pttid = stoul(counter.param_.arg(0));
                                        auto iter = pttid2item_.find(pttid);
                                        if (iter != pttid2item_.end()) {
                                                counter.cur_ = iter->second->count();
                                        }
                                        break;
                                }
                                case pd::TURN_IN_RESOURCE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        pd::resource_type type;
                                        ASSERT(pd::resource_type_Parse(counter.param_.arg(0), &type));
                                        auto iter = resources_.values_.find(type);
                                        if (iter != resources_.values_.end()) {
                                                counter.cur_ = iter->second;
                                        }
                                        break;
                                }
                                case pd::LEVELUP_REACH: {
                                        counter.cur_ = min(counter.param_.need(), level());
                                        break;
                                }
                                case pd::FRIEND_NUMBER_GE: {
                                        counter.cur_ = static_cast<uint32_t>(relation_.friends_.size());
                                        break;
                                }
                                default:
                                        counter.cur_ = 0;
                                        break;
                                }

                        }

                        if (register_event) {
                                quest_register_event(pttid, true);
                        }

                        if (!mirror_role()) {
                                //bi::instance().accept_quest(username_func_(), yci_, ip_, gid_, pttid, league_gid(), ptt.desc());
                        }
                        quests_changed_ = true;
                }

                void role::quest_unregister_event(uint32_t pttid) {
                        ASSERT(quest_.accepted_.count(pttid) > 0);
                        auto& aq = quest_.accepted_.at(pttid);
                        if (!aq.registered_) {
                                return;
                        }

                        for (const auto& j : aq.event_handlers_) {
                                for (auto k : j.second) {
                                        ec_.unregister_event(j.first, k);
                                }
                        }
                        aq.event_handlers_.clear();
                        aq.registered_ = false;
                        quests_changed_ = false;
                }

                void role::quest_register_event(uint32_t pttid, bool not_parse) {
                        ASSERT(quest_.accepted_.count(pttid) > 0);
                        auto& aq = quest_.accepted_.at(pttid);
                        if (aq.registered_) {
                                return;
                        }

                        if (not_parse) {
                                quests_changed_ = true;
                        }
                        aq.registered_ = true;

                        for (auto& counter : aq.counters_) {
                                switch (counter.type_) {
                                case pd::TURN_IN_ITEM: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_ADD_ITEM].push_back(
                                                ec_.register_event(ECE_ADD_ITEM,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           auto add_pttid = boost::any_cast<uint32_t>(args[0]);
                                                                           auto pttid = stoul(counter.param_.arg(0));
                                                                           if (add_pttid != pttid) {
                                                                                   return;
                                                                           }
                                                                           ASSERT(pttid2item_.count(pttid) > 0);
                                                                           set_quest_counter(counter, quest, pttid2item_.at(pttid)->count());
                                                                   }));
                                        aq.event_handlers_[ECE_REMOVE_ITEM].push_back(
                                                ec_.register_event(ECE_REMOVE_ITEM,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           auto add_pttid = boost::any_cast<uint32_t>(args[0]);
                                                                           auto pttid = stoul(counter.param_.arg(0));
                                                                           if (add_pttid != pttid) {
                                                                                   return;
                                                                           }
                                                                           if (pttid2item_.count(pttid) == 0) {
                                                                                   set_quest_counter(counter, quest, 0);
                                                                           } else {
                                                                                   set_quest_counter(counter, quest, pttid2item_.at(pttid)->count());
                                                                           }
                                                                   }));
                                        break;
                                }
                                case pd::TURN_IN_RESOURCE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_CHANGE_RESOURCE].push_back(
                                                ec_.register_event(ECE_CHANGE_RESOURCE,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           pd::resource_type type;
                                                                           ASSERT(pd::resource_type_Parse(counter.param_.arg(0), &type));
                                                                           if (type != boost::any_cast<pd::resource_type>(args[0])) {
                                                                                   return;
                                                                           }
                                                                           auto iter = resources_.values_.find(type);
                                                                           if (iter != resources_.values_.end()) {
                                                                                   set_quest_counter(counter, quest, iter->second);
                                                                           } else {
                                                                                   set_quest_counter(counter, quest, 0);
                                                                           }
                                                                   }));
                                        break;
                                }
                                case pd::ACTIVITY_FESTIVAL_RECHARGE_MONEY_GE: {
                                        aq.event_handlers_[ECE_RECHARGE].push_back(
                                                ec_.register_event(ECE_RECHARGE,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           const auto& ptts = PTTS_GET_ALL(activity_festival);
                                                                           auto id = 0u;
                                                                           for (const auto& i : ptts) {
                                                                                   for (auto j : i.second.recharge()) {
                                                                                           if (j == quest) {
                                                                                                   id = i.second.id();
                                                                                           }
                                                                                   }
                                                                           }
                                                                           ASSERT(id != 0u);
                                                                           auto start_day = activity_festival_start_day();
                                                                           auto duration = activity_festival_duration();
                                                                           if (start_day == 0) {
                                                                                   return;
                                                                           }
                                                                           if (start_day + duration > refresh_day()) {
                                                                                   return;
                                                                           }
                                                                           ASSERT(args.size() == 1);
                                                                           auto money = boost::any_cast<uint32_t>(args[0]);
                                                                           change_quest_counter(counter, quest, money);
                                                                   }));
                                        break;
                                }
                                case pd::RECHARGE_GE:
                                case pd::CUR_DAY_RECHARGE_GE: {
                                        aq.event_handlers_[ECE_RECHARGE].push_back(
                                                ec_.register_event(ECE_RECHARGE,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           ASSERT(quest_.accepted_.count(quest) > 0);
                                                                           auto money = boost::any_cast<uint32_t>(args[0]);
                                                                           change_quest_counter(counter, quest, money);
                                                                   }));
                                        break;
                                }
                                case pd::FORMATION_GRIDS_ACTOR_COUNT: {
                                        aq.event_handlers_[ECE_SET_FORMATION].push_back(
                                                ec_.register_event(ECE_SET_FORMATION,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           auto idx = boost::any_cast<int>(args[0]);
                                                                           ASSERT(idx >= 0 && idx < static_cast<int>(battle_.formations_.size()));
                                                                           const auto& formation = battle_.formations_[idx];
                                                                           if (static_cast<int>(counter.cur_) < formation.grids().grids_size()) {
                                                                                   auto value = min(static_cast<uint32_t>(formation.grids().grids_size()), counter.param_.need());
                                                                                   set_quest_counter(counter, quest, value);
                                                                           } else {
                                                                                   change_quest_counter(counter, quest, 0);
                                                                           }
                                                                   }));
                                        break;
                                }
                                case pd::WORLD_CHAT_TIMES: {
                                        aq.event_handlers_[ECE_WORLD_CHAT].push_back(
                                                ec_.register_event(ECE_WORLD_CHAT,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           change_quest_counter(counter, quest, 1);
                                                                   }));
                                        break;
                                }
                                case pd::PRIVATE_CHAT_TIMES: {
                                        aq.event_handlers_[ECE_PRIVATE_CHAT].push_back(
                                                ec_.register_event(ECE_PRIVATE_CHAT,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 0);
                                                                           change_quest_counter(counter, quest, 1);
                                                                   }));
                                        break;
                                }
                                case pd::LEVELUP_REACH: {
                                        aq.event_handlers_[ECE_LEVELUP].push_back(
                                                ec_.register_event(ECE_LEVELUP,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           set_quest_counter(counter, quest, level());
                                                                   }));
                                        break;
                                }
                                case pd::COST_USE_ITEM_COUNT: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_COST_ITEM].push_back(
                                                ec_.register_event(ECE_COST_ITEM,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 2);
                                                                           auto pttid = boost::any_cast<uint32_t>(args[0]);
                                                                           auto need_pttid = stoul(counter.param_.arg(0));
                                                                           if (pttid != need_pttid) {
                                                                                   return;
                                                                           }
                                                                           auto count = boost::any_cast<int>(args[1]);
                                                                           change_quest_counter(counter, quest, count);
                                                                   }));
                                        break;
                                }
                                case pd::COST_RESOURCES_GE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_CHANGE_RESOURCE].push_back(
                                                ec_.register_event(ECE_CHANGE_RESOURCE,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 2);
                                                                           auto need_resource_type = counter.param_.arg(0);
                                                                           pd::resource_type need_type;
                                                                           pd::resource_type_Parse(need_resource_type, &need_type);
                                                                           auto res_type = boost::any_cast<pd::resource_type>(args[0]);
                                                                           auto count = boost::any_cast<int64_t>(args[1]);
                                                                           if (res_type != need_type || count >= 0) {
                                                                                   return;
                                                                           }
                                                                           change_quest_counter(counter, quest, -count);
                                                                   }));
                                        break;
                                }
                                case pd::OWN_RESOURCES_GE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_CHANGE_RESOURCE].push_back(
                                                ec_.register_event(ECE_CHANGE_RESOURCE,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 2);
                                                                           auto need_resource_type = counter.param_.arg(0);
                                                                           pd::resource_type need_type;
                                                                           pd::resource_type_Parse(need_resource_type, &need_type);
                                                                           auto res_type = boost::any_cast<pd::resource_type>(args[0]);
                                                                           auto count = boost::any_cast<int64_t>(args[1]);
                                                                           if (res_type != need_type || count == 0) {
                                                                                   return;
                                                                           }
                                                                           set_quest_counter(counter, quest, static_cast<uint32_t>(get_resource(res_type)));
                                                                   }));
                                        break;
                                }
                                case pd::FRIEND_NUMBER_GE: {
                                        aq.event_handlers_[ECE_ADD_FRIEND].push_back(
                                                ec_.register_event(ECE_ADD_FRIEND,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           change_quest_counter(counter, quest, 1);
                                                                   }));
                                        aq.event_handlers_[ECE_REMOVE_FRIEND].push_back(
                                                ec_.register_event(ECE_REMOVE_FRIEND,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           change_quest_counter(counter, quest, -1);
                                                                   }));
                                        break;
                                }
                                case pd::SEND_GIFT_TO_FRIEND_GE_COUNT: {
                                        ASSERT(counter.param_.arg_size() == 0);
                                        aq.event_handlers_[ECE_ROLE_SEND_GIFT].push_back(
                                                ec_.register_event(ECE_ROLE_SEND_GIFT,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 3);
                                                                           auto count = boost::any_cast<uint32_t>(args[0]);
                                                                           change_quest_counter(counter, quest, count);
                                                                   }));
                                        break;
                                }
                                case pd::RELATION_FRIEND_INTIMACY_GE: {
                                        aq.event_handlers_[ECE_RELATION_FRIEND_INTIMACY].push_back(
                                                ec_.register_event(ECE_RELATION_FRIEND_INTIMACY,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           auto intimacy = static_cast<uint32_t>(boost::any_cast<int>(args[0]));
                                                                           if (intimacy <= counter.cur_) {
                                                                                return;
                                                                           }
                                                                           set_quest_counter(counter, quest, intimacy);
                                                                   }));
                                        break;
                                }
                                case pd::SHOP_BUY_GE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_SHOP_BUY].push_back(
                                                ec_.register_event(ECE_SHOP_BUY,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 3);
                                                                           if (boost::any_cast<uint32_t>(args[0]) == stoul(counter.param_.arg(0))) {
                                                                                   change_quest_counter(counter, quest, boost::any_cast<uint32_t>(args[1]));
                                                                           }
                                                                   }));
                                        break;
                                }
                                case pd::SPINE_ADD_COLLECTION_GE: {
                                        aq.event_handlers_[ECE_SPINE_ADD_COLLECTION].push_back(
                                                ec_.register_event(ECE_SPINE_ADD_COLLECTION,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 2);
                                                                           set_quest_counter(counter, quest, boost::any_cast<uint32_t>(args[1]));
                                                                   }));
                                        break;
                                }
                                case pd::MYS_SHOP_REFRESH_TIMES: {
                                        aq.event_handlers_[ECE_MYS_SHOP_REFRESH].push_back(
                                                ec_.register_event(ECE_MYS_SHOP_REFRESH,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 1);
                                                                           change_quest_counter(counter, quest, 1);
                                                                   }));
                                        break;
                                }
                                case pd::EACH_SHOP_BUG_TIMES_GE: {
                                        ASSERT(counter.param_.arg_size() == 1);
                                        aq.event_handlers_[ECE_SHOP_BUY].push_back(
                                                ec_.register_event(ECE_SHOP_BUY,
                                                                   [this, &counter, quest = pttid] (const vector<boost::any>& args) {
                                                                           ASSERT(args.size() == 3);
                                                                           if (boost::any_cast<uint32_t>(args[2]) == stoul(counter.param_.arg(0))) {
                                                                                   change_quest_counter(counter, quest, boost::any_cast<uint32_t>(args[1]));
                                                                           }
                                                                   }));
                                        break;
                                }
                                default:
                                        break;
                                }
                        }
                }

                void role::quest_commit(uint32_t pttid) {
                        SPLAYER_DLOG << "quest commit " << pttid;
                        ASSERT(quest_.accepted_.count(pttid) > 0);
                        auto& aq = quest_.accepted_[pttid];
                        auto accept_time = aq.accept_time_;

                        const auto& ptt = PTTS_GET(quest, pttid);

                        bool not_finish = false;
                        ASSERT(static_cast<int>(aq.counters_.size()) == ptt.targets_size());
                        for (auto i = 0u; i < aq.counters_.size(); ++i) {
                                const auto& counter = aq.counters_[i];
                                const auto& target = ptt.targets(i);
                                if (counter.cur_param_index_ < target.params_size()) {
                                        not_finish = true;
                                        break;
                                }
                        }

                        if (not_finish) {
                                quests_changed_ = true;
                                return;
                        }
                        if (ptt.has__everyday_quest()) {
                                quest_.finished_everyday_quests_.insert(pttid);
                        } else {
                                if (ptt.has__achievement_quest()) {
                                        quest_.received_achievement_.insert(pttid);
                                }
                        }
                        quest_.passed_.insert(pttid);
                        quest_remove(pttid);
                        quests_changed_ = true;

                        if (!mirror_role()) {
                                //bi::instance().commit_quest(username_func_(), yci_, ip_, gid_, league_gid(), ptt.desc(), pttid, system_clock::to_time_t(system_clock::now()) - accept_time);
                        }
                }

                void role::quest_remove(uint32_t pttid) {
                        SPLAYER_DLOG << " quest remove " << pttid;
                        ASSERT(quest_.accepted_.count(pttid) > 0);
                        auto& aq = quest_.accepted_[pttid];
                        for (const auto& j : aq.event_handlers_) {
                                for (auto k : j.second) {
                                        ec_.unregister_event(j.first, k);
                                }
                        }
                        aq.event_handlers_.clear();
                        quest_.accepted_.erase(pttid);
                        quests_changed_ = true;
                }

                pd::result role::quest_check_commit(uint32_t pttid) const {
                        ASSERT(quest_.accepted_.count(pttid) > 0);

                        if (quest_.finished_everyday_quests_.count(pttid) > 0) {
                                return pd::QUEST_ALREADY_COMMITTED;
                        }

                        const auto& aq = quest_.accepted_.at(pttid);
                        for (const auto& i : aq.counters_) {
                                if (i.cur_ < i.param_.need()) {
                                        return pd::QUEST_NOT_FINISHED;
                                }
                        }
                        return pd::OK;
                }

                void role::quest_helped_by_other(uint32_t pttid, uint32_t cur_need) {
                        ASSERT(quest_.accepted_.count(pttid) > 0);
                        auto& aq = quest_.accepted_.at(pttid);
                        for (auto& i : aq.counters_) {
                                if (i.type_ == pd::TURN_IN_ITEM) {
                                        i.param_.set_need(cur_need);
                                        break;
                                }
                        }
                        quests_changed_ = true;
                }

                void role::refresh_everyday_quests(pd::event_res& event_res) {
                        auto today = refresh_day();
                        auto monday = refresh_monday();
                        if (quest_.everyday_quest_day_ != today) {
                                const auto& ptts = PTTS_GET_ALL(everyday_quest);
                                for (const auto& i : ptts) {
                                        auto quest = i.second.quest();
                                        if (quest_has_accepted(quest)) {
                                                quest_remove(quest);
                                                event_res.add_remove_quests(quest);
                                        }
                                }
                                quest_.finished_everyday_quests_.clear();
                                vector<uint32_t> to_remove;
                                for (auto i : quest_.got_everyday_quest_rewards_) {
                                        const auto& reward_ptt = PTTS_GET(everyday_quest_reward, i);
                                        if (!reward_ptt.weekly() || quest_.everyday_quest_monday_ != monday) {
                                                to_remove.push_back(i);
                                        }
                                }
                                for (auto i : to_remove) {
                                        quest_.got_everyday_quest_rewards_.erase(i);
                                }
                                quest_.everyday_quest_day_ = today;
                                quest_.everyday_quest_monday_ = monday;
                                quests_changed_ = true;
                        }

                        const auto& ptts = PTTS_GET_ALL(everyday_quest);
                        for (const auto& i : ptts) {
                                if (quest_has_accepted(i.second.quest())) {
                                        continue;
                                }
                                if (quest_.finished_everyday_quests_.count(i.second.quest()) > 0) {
                                        continue;
                                }
                                if (level() >= i.second.min_level() && level() <= i.second.max_level()) {
                                        quest_accept(i.second.quest());
                                        serialize_accepted_quest(i.second.quest(), event_res.add_accept_quests());
                                }
                                quests_changed_ = true;
                        }
                }

                void role::grow_resource(pd::resource_type type) {
                        auto now = system_clock::to_time_t(system_clock::now());
                        const auto& ptt = PTTS_GET(resource, type);
                        ASSERT(ptt.grow_minutes() > 0);
                        auto grow_time = resource_grow_time(type);
                        if (now - grow_time >= ptt.grow_minutes() * MINUTE) {
                                auto grow_max = ptt.max_value();
                                if (type == pd::POWER) {
                                        grow_max = get_resource(pd::POWER_GROW_MAX);
                                } else if (type == pd::VIGOR) {
                                        grow_max = get_resource(pd::VIGOR_GROW_MAX);
                                }
                                auto cur_value = get_resource(type);
                                if (cur_value < grow_max) {
                                        auto change_value = min(grow_max - cur_value,
                                                                static_cast<int>((now - grow_time) / (ptt.grow_minutes() * MINUTE)) * ptt.grow_value());
                                        pd::event_array events;
                                        auto *e = events.add_events();
                                        e->set_type(pd::event::ADD_RESOURCE);
                                        e->add_arg(pd::resource_type_Name(type));
                                        e->add_arg(to_string(change_value));
                                        pd::ce_env ce;
                                        ce.set_origin(pd::CO_ROLE_GROW_RESOURCE);
                                        event_process(events, *this, ce);
                                }
                                set_resource_grow_time(type, now - (now - resource_grow_time(type)) % (ptt.grow_minutes() * MINUTE));
                                resources_changed_ = true;
                        }
                }

                void role::add_title(uint32_t pttid) {
                        if (title_.titles_.count(pttid) == 0) {
                                title_.titles_[pttid].set_pttid(pttid);
                                for (auto& i : actors_.pttid2actor_) {
                                        i.second->add_title(pttid);
                                }
                                actors_changed_ = true;

                                // all actors' attr are changed, here use role actor takes advantage of the fact that
                                // role actor is in every formation
                                other_system_update_role(role_actor_->pttid());
                        }

                        const auto& ptt = PTTS_GET(title, pttid);
                        if (ptt.days() > 0) {
                                title_.titles_[pttid].set_got_time(system_clock::to_time_t(system_clock::now()));
                                if (st_) {
                                        auto it = title_.timers_.find(pttid);
                                        if (it != title_.timers_.end()) {
                                                it->second->cancel();
                                                title_.timers_.erase(it);
                                        }

                                        add_title_timers();
                                }
                        }
                        titles_changed_ = true;
                }

                void role::remove_title(uint32_t pttid) {
                        ASSERT(has_title(pttid));
                        if (title_.selected_ == pttid) {
                                ASSERT(role_actor_);
                                title_.selected_ = 0;
                        }
                        title_.titles_.erase(pttid);
                        for (auto& i : actors_.pttid2actor_) {
                                i.second->delete_title(pttid);
                        }
                        titles_changed_ = true;
                        other_system_update_role(role_actor_->pttid());
                }

                void role::title_set_read(uint32_t pttid) {
                        if (!has_title(pttid)) {
                                return;
                        }

                        title_.titles_.at(pttid).set_read(true);
                        titles_changed_ = true;
                }

                void role::select_title(uint32_t pttid) {
                        if (title_.selected_ == pttid) {
                                return;
                        }
                        if (pttid > 0) {
                                ASSERT(has_title(pttid));
                        }
                        title_.selected_ = pttid;
                        titles_changed_ = true;
                }

                void role::add_title_timers() {
                        for (const auto& i : title_.titles_) {
                                const auto& ptt = PTTS_GET(title, i.first);
                                if (ptt.days() > 0 && title_.timers_.count(i.first) == 0) {
                                        auto seconds_passed = system_clock::now() - system_clock::from_time_t(i.second.got_time());
                                        auto title_seconds = seconds(ptt.days() * DAY);
                                        ASSERT(seconds_passed < title_seconds);
                                        title_.timers_[i.first] = ADD_TIMER(
                                                st_,
                                                ([this, self = shared_from_this(), pttid = i.first] (auto canceled, const auto& timer) {
                                                        if (title_.timers_[pttid] == timer) {
                                                                title_.timers_.erase(pttid);
                                                        }
                                                        if (!canceled) {
                                                                this->remove_title(pttid);
                                                                update_db_func_();
                                                        }
                                                }),
                                                title_seconds);
                                }

                        }
                }

                void role::yunying_set_item(uint32_t pttid, uint32_t count) {
                        if (pttid2item_.count(pttid) > 0) {
                                pttid2item_.erase(pttid);
                        }
                        add_item(pttid, count);
                }

                void role::on_event(nora::ec_event event, const vector<boost::any>& args) {
                        ec_.on_event(event, args);
                }

                void role::register_event(nora::ec_event event, const function<void(const vector<boost::any>&)>& cb) {
                        ec_.register_event(event, cb);
                }

                void role::unregister_event_by_type(nora::ec_event event) {
                        ec_.unregister_event_by_type(event);
                }

                void role::quest_commit_forever_quest(uint32_t quest) {
                        auto& eq = get_accepted_quest(quest);

                        auto pass_events = get_quest_cur_pass_events(quest);
                        pd::event_array events;
                        while (quest_check_commit(quest) == pd::OK) {
                                for (auto& i : eq.counters_) {
                                        i.cur_ -= i.param_.need();
                                }
                                event_merge(events, pass_events);
                        }
                        quests_changed_ = true;

                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ROLE_GROW_RESOURCE);
                        auto event_res = event_process(events, *this, ce);
                        pd::quest data;
                        serialize_accepted_quest(quest, &data);
                        forever_quest_update_func_(gid(), data, event_res);
                }

                void role::set_quest_counter(quest_target_counter& counter, uint32_t quest, uint32_t value) {
                        if (counter.cur_ == value) {
                                return;
                        }
                        counter.cur_ = value;
                        quests_changed_ = true;
                        const auto& ptt = PTTS_GET(quest, quest);
                        if (ptt.auto_quest()) {
                                quest_auto_commit_func_(gid_, quest);
                        } else if (ptt.forever_quest() && quest_check_commit(quest) == pd::OK) {
                                quest_commit_forever_quest(quest);
                        } else {
                                pd::quest data;
                                serialize_accepted_quest(quest, &data);
                                quest_update_func_(gid(), data);
                        }
                }

                void role::change_quest_counter(quest_target_counter& counter, uint32_t quest, uint32_t value) {
                        counter.cur_ = counter.cur_ + value;
                        if (value != 0) {
                                quests_changed_ = true;
                        }
                        const auto& ptt = PTTS_GET(quest, quest);
                        if (ptt.auto_quest()) {
                                quest_auto_commit_func_(gid_, quest);
                        } else if (ptt.forever_quest() && quest_check_commit(quest) == pd::OK) {
                                quest_commit_forever_quest(quest);
                        } else {
                                pd::quest data;
                                serialize_accepted_quest(quest, &data);
                                quest_update_func_(gid(), data);
                        }
                }

                void role::clear_quest_counter(uint32_t quest, bool sync_client) {
                        ASSERT(quest_.accepted_.count(quest) > 0);
                        ASSERT(PTTS_HAS(quest, quest));
                        const auto& ptt = PTTS_GET(quest, quest);
                        auto& aq = quest_.accepted_.at(quest);
                        for (auto& counter : aq.counters_) {
                                if (ptt.clear_quest()) {
                                        if (counter.cur_ == 0) {
                                                break;
                                        }
                                        counter.cur_ = 0;
                                        quests_changed_ = true;
                                        if (sync_client) {
                                                pd::quest data;
                                                serialize_accepted_quest(quest, &data);
                                                quest_update_func_(gid(), data);
                                        }
                                }
                        }
                }

                void role::exec_chat_gm_pass_quest(uint32_t quest, uint32_t cur) {
                        ASSERT(quest_.accepted_.count(quest) > 0);
                        for (auto& i : quest_.accepted_.at(quest).counters_) {
                                set_quest_counter(i, quest, cur);
                        }
                }

                void role::add_comment(uint64_t from, uint64_t reply_to, const string& content, uint64_t& gid) {
                        auto& comment = comments_.gid2comment_[comments_.gid_seed_];
                        comment.set_gid(comments_.gid_seed_);
                        if (reply_to > 0) {
                                ASSERT(has_comment(reply_to));
                                comment.set_reply_to(reply_to);
                        }
                        comment.set_role(from);
                        comment.set_content(content);
                        auto now = system_clock::to_time_t(system_clock::now());
                        comment.set_time(now);

                        gid = comment.gid();
                        comments_.gid_seed_ = max(1ul, comments_.gid_seed_ + 1);

                        comments_.comment_list_.push_back(comment);
                        comments_.role2last_comment_time_[from] = now;
                        comments_changed_ = true;

                        clean_up_comment();
                }

                void role::add_comment(uint64_t from, uint32_t gift, uint32_t count, uint64_t& gid) {
                        auto& comment = comments_.gid2comment_[comments_.gid_seed_];
                        comment.set_gid(comments_.gid_seed_);
                        comment.set_role(from);
                        comment.set_gift(gift);
                        comment.set_gift_count(count);
                        auto now = system_clock::to_time_t(system_clock::now());
                        comment.set_time(now);

                        gid = comment.gid();
                        comments_.gid_seed_ = max(1ul, comments_.gid_seed_ + 1);

                        comments_.comment_list_.push_back(comment);
                        comments_.role2last_comment_time_[from] = now;
                        comments_changed_ = true;

                        clean_up_comment();
                }

                void role::clean_up_comment() {
                        while (comments_.comment_list_.size() > 256) {
                                auto remove_gid = comments_.comment_list_.front().gid();
                                delete_comment(remove_gid);
                        }
                }

                void role::delete_comment(uint64_t gid) {
                        ASSERT(has_comment(gid));
                        comments_.gid2comment_.erase(gid);
                        for (auto i = comments_.comment_list_.begin(); i != comments_.comment_list_.end(); ++i) {
                                if (i->gid() == gid) {
                                        comments_.comment_list_.erase(i);
                                        break;
                                } else if (i->reply_to() == gid) {
                                        comments_.gid2comment_.erase(i->gid());
                                        comments_.comment_list_.erase(i);
                                        break;
                                }
                        }
                        comments_changed_ = true;
                }

                void role::login() {
                        data_.offline_time_ = 0;
                        if (data_.first_login_time_ == 0) {
                                data_.first_login_time_ = system_clock::to_time_t(system_clock::now());
                        }
                        data_.last_login_time_ = system_clock::to_time_t(system_clock::now());
                        if (refresh_day() != data_.last_online_day_) {
                                data_.today_online_seconds_ = 0;
                                data_.login_days_ += 1;
                                data_.last_online_day_ = refresh_day();
                                data_.today_intimate_actor_ = most_intimate_actor();
                        }
                        role_data_changed_ = true;

                        reset_resources();


                        pd::event_res event_res;
                        refresh_everyday_quests(event_res);

                        activity_seven_days_update_quests(event_res);

                        activity_seven_days_update_fuli_quests(event_res);

                        add_title_timers();

                        activity_festival_update_quests(event_res);

                        activity_continue_recharge_reset_dayrecharge();
                        activity_continue_recharge_clear_quests(event_res);
                        activity_continue_recharge_update_quests(event_res);

                        activity_limit_play_update_quests(event_res);

                        activity_leiji_recharge_update_quests(event_res);
                        activity_leiji_consume_update_quests(event_res);
                        activity_recharge_rank_update();

                        activity_prize_wheel_check_finish(event_res);
                        activity_prize_wheel_check_start(event_res);

                        activity_tea_party_check_finish(event_res);
                        activity_tea_party_check_start(event_res);
                }

                void role::logout() {
                        data_.offline_time_ = system_clock::to_time_t(system_clock::now());
                        data_.today_online_seconds_ = today_online_seconds();
                        role_data_changed_ = true;

                        for (const auto& i : title_.timers_) {
                                i.second->cancel();
                        }
                        title_.timers_.clear();
                }

                void role::on_new_day() {
                        data_.today_online_seconds_ = 0;
                        data_.login_days_ += 1;
                        data_.last_online_day_ = refresh_day();
                        data_.today_intimate_actor_ = most_intimate_actor();
                        role_data_changed_ = true;

                        changing_data_.eat_times_ = 0;
                        changing_data_changed_ = true;

                        pd::event_res event_res;
                        refresh_everyday_quests(event_res);
                        activity_seven_days_update_quests(event_res);
                        activity_seven_days_update_fuli_quests(event_res);
                        //activity_leiji_update_quest(event_res);
                        activity_festival_update_quests(event_res);
                        activity_continue_recharge_reset_dayrecharge();
                        activity_continue_recharge_clear_quests(event_res, true);
                        activity_continue_recharge_update_quests(event_res);
                        activity_limit_play_update_quests(event_res);
                        activity_leiji_recharge_update_quests(event_res);
                        activity_leiji_consume_update_quests(event_res);
                        activity_recharge_rank_update();

                        activity_prize_wheel_check_finish(event_res);
                        activity_prize_wheel_check_start(event_res);

                        reset_activity_qiandao();

                        on_new_day_cb_(event_res, lieming_.battles_);

                        reset_resources();
                }

                void role::activity_continue_recharge_clear_quests(pd::event_res& event_res, bool sync_client) {
                        if (activity_.continue_recharge_.update_quests_activate_day_ != refresh_day()) {
                                auto pttid = activity_continue_recharge_pttid();
                                auto start_day = activity_continue_recharge_start_day();
                                auto duration = activity_continue_recharge_duration();
                                auto cur_quest = activity_continue_recharge_cur_quest();
                                if (start_day != 0) {
                                        ASSERT(pttid != 0);
                                        ASSERT(PTTS_HAS(activity_continue_recharge, pttid));
                                        const auto& ptt = PTTS_GET(activity_continue_recharge, pttid);
                                        if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                                auto quest_idx = activity_continue_recharge_quest_idx();
                                                if (quest_.accepted_.count(cur_quest) > 0 && !quest_committed(cur_quest)) {
                                                        clear_quest_counter(ptt.quests(quest_idx - 1).auto_quest(), sync_client);
                                                        clear_quest_counter(ptt.quests(quest_idx - 1).event_quest(), sync_client);
                                                        activity_continue_recharge_set_cur_quest(ptt.quests(quest_idx).auto_quest(), false);
                                                        if (!sync_client) {
                                                                event_res.add_remove_quests(ptt.quests(quest_idx).auto_quest());
                                                                event_res.add_remove_quests(ptt.quests(quest_idx).event_quest());
                                                        }
                                                }
                                        }
                                }
                        }
                }

                void role::activity_stop_continue_recharge(pd::event_res& event_res) {
                        auto pttid = activity_continue_recharge_pttid();
                        activity_reset_continue_recharge(0, 0 , pttid);
                        activity_continue_recharge_reset_dayrecharge();
                        ASSERT(pttid != 0);
                        const auto& ptt = PTTS_GET(activity_continue_recharge, pttid);
                        pd::event_array events;
                        for (auto i : ptt.quests()) {
                                if (quest_committed(i.auto_quest())) {
                                        quest_.passed_.erase(i.auto_quest());
                                        if (!quest_committed(i.event_quest())) {
                                                event_res.add_remove_quests(i.event_quest());
                                                event_merge(events, quest_commit_to_events(i.event_quest(), *this));
                                        }
                                        quest_.passed_.erase(i.event_quest());
                                        activity_changed_ = true;
                                }
                        }
                        if (events.events_size() != 0) {
                                const auto& ptt = PTTS_GET(activity_logic, 1);
                                ASSERT(send_mail_func_);
                                send_mail_func_(ptt.continue_recharge_mail(), events);
                        }
                        for (const auto& i : ptt.quests()) {
                                if (quest_has_accepted(i.auto_quest())) {
                                        quest_remove(i.auto_quest());
                                        event_res.add_remove_quests(i.auto_quest());
                                }
                        }
                }

                void role::activity_continue_recharge_update_quests(pd::event_res& event_res) {
                        if (activity_.continue_recharge_.update_quests_activate_day_ != refresh_day()) {
                                auto pttid = activity_continue_recharge_pttid();
                                auto start_day = activity_continue_recharge_start_day();
                                auto duration = activity_continue_recharge_duration();
                                auto cur_quest = activity_continue_recharge_cur_quest();
                                if (start_day != 0) {
                                        ASSERT(pttid != 0);
                                        ASSERT(PTTS_HAS(activity_continue_recharge, pttid));
                                        const auto& ptt = PTTS_GET(activity_continue_recharge, pttid);
                                        if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                                auto quest_idx = activity_continue_recharge_quest_idx();
                                                if (cur_quest == 0) {
                                                        //bi::instance().activity_continue_recharge_start(username_func_(), yci_, ip_, pttid, pd::AAT_CONTINUE_RECHARGE,gid_, data_.level_);
                                                }
                                                if (quest_idx < static_cast<uint32_t>(ptt.quests_size())) {
                                                        pd::event_array events;
                                                        set_quest_to_events(ptt.quests(quest_idx).auto_quest(), events);
                                                        set_quest_to_events(ptt.quests(quest_idx).event_quest(), events);
                                                        activity_continue_recharge_set_cur_quest(ptt.quests(quest_idx).auto_quest());
                                                        event_res = event_process(events, *this, pd::ce_env());
                                                }
                                        } else if (refresh_day() >= start_day + duration - 1) {
                                                //bi::instance().activity_continue_recharge_stop(username_func_(), yci_, ip_, pttid, pd::AAT_CONTINUE_RECHARGE, duration, gid_, data_.level_);
                                                activity_stop_continue_recharge(event_res);
                                        }
                                }
                        }
                }

                pd::event_res role::activity_stop_limit_play() {
                        pd::event_res ret;
                        auto start_day = activity_limit_play_start_day();
                        auto duration = activity_limit_play_duration();
                        auto pttid = activity_limit_play_pttid();
                        ASSERT(pttid != 0);
                        if (duration == 0 || refresh_day() >= start_day + duration - 1) {
                                const auto& ptt = PTTS_GET(activity_limit_play, pttid);
                                pd::event_array events;
                                for (auto i : ptt.quests()) {
                                        if (quest_committed(i.auto_quest())) {
                                                if (!quest_committed(i.event_quest())) {
                                                        event_merge(events, quest_commit_to_events(i.event_quest(), *this));
                                                }
                                                quest_remove_commited_quest(i.event_quest());
                                                quest_remove_commited_quest(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                                if (events.events_size() != 0) {
                                        const auto& ptt = PTTS_GET(activity_logic, 1);
                                        ASSERT(send_mail_func_);
                                        send_mail_func_(ptt.limit_play_mail(), events);
                                }
                                for (const auto& i : ptt.quests()) {
                                        if (quest_has_accepted(i.auto_quest())) {
                                                quest_remove(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                        }
                                        if (quest_has_accepted(i.event_quest())) {
                                                quest_remove(i.event_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                        }
                        return ret;
                }

                void role::activity_limit_play_update_quests(pd::event_res& event_res) {
                        auto start_day = activity_limit_play_start_day();
                        auto duration = activity_limit_play_duration();
                        auto pttid = activity_limit_play_pttid();
                        if (start_day != 0) {
                                ASSERT(pttid != 0);
                                const auto& quest_ptt = PTTS_GET(activity_limit_play, pttid);
                                if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                        pd::event_array events;
                                        bool first_start_activity = false;
                                        for (const auto& i : quest_ptt.quests()) {
                                                if (!quest_has_accepted(i.auto_quest()) && !quest_committed(i.auto_quest())) {
                                                        set_quest_to_events(i.auto_quest(), events);
                                                        set_quest_to_events(i.event_quest(), events);
                                                        first_start_activity = true;
                                                }
                                        }
                                        if (first_start_activity) {
                                                //bi::instance().activity_limit_play_start(username_func_(), yci_, ip_, pttid, pd::AAT_LIMIT_PLAY, gid_, data_.level_);
                                        }
                                        if (events.events_size() > 0) {
                                                event_res = event_process(events, *this, pd::ce_env());
                                        }
                                } else if (refresh_day() >= start_day + duration - 1) {
                                        //bi::instance().activity_limit_play_stop(username_func_(), yci_, ip_, pttid, pd::AAT_LIMIT_PLAY, duration, gid_, data_.level_);
                                        event_res = activity_stop_limit_play();
                                        activity_reset_limit_play(0, 0 , pttid);
                                }
                        }
                }

                pd::event_res role::activity_stop_leiji_recharge() {
                        pd::event_res ret;
                        auto start_day = activity_leiji_recharge_start_day();
                        auto duration = activity_leiji_recharge_duration();
                        auto pttid = activity_leiji_recharge_pttid();
                        ASSERT(pttid != 0);
                        if (duration == 0 || refresh_day() >= start_day + duration - 1) {
                                const auto& ptt = PTTS_GET(activity_leiji_recharge, pttid);
                                pd::event_array events;
                                for (auto i : ptt.quests()) {
                                        if (quest_committed(i.auto_quest())) {
                                                if (!quest_committed(i.event_quest())) {
                                                        event_merge(events, quest_commit_to_events(i.event_quest(), *this));
                                                }
                                                quest_remove_commited_quest(i.event_quest());
                                                quest_remove_commited_quest(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                                if (events.events_size() != 0) {
                                        //bi::instance().activity_leiji_recharge_stop(username_func_(), yci_, ip_, pttid, pd::AAT_LEIJI_RECHARGE, duration, gid_, data_.level_);
                                        const auto& ptt = PTTS_GET(activity_logic, 1);
                                        ASSERT(send_mail_func_);
                                        send_mail_func_(ptt.leiji_recharge_mail(), events);
                                }
                                for (const auto& i : ptt.quests()) {
                                        if (quest_has_accepted(i.auto_quest())) {
                                                quest_remove(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                        }
                                        if (quest_has_accepted(i.event_quest())) {
                                                quest_remove(i.event_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                                activity_clear_leiji_recharge();
                        }
                        return ret;
                }

                void role::activity_leiji_recharge_update_quests(pd::event_res& event_res) {
                        auto start_day = activity_leiji_recharge_start_day();
                        auto duration = activity_leiji_recharge_duration();
                        auto pttid = activity_leiji_recharge_pttid();
                        if (start_day != 0) {
                                ASSERT(pttid != 0);
                                const auto& quest_ptt = PTTS_GET(activity_leiji_recharge, pttid);
                                if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                        bool first_start_activity = false;
                                        pd::event_array events;
                                        for (const auto& i : quest_ptt.quests()) {
                                                if (!quest_has_accepted(i.auto_quest()) && !quest_committed(i.auto_quest())) {
                                                        set_quest_to_events(i.auto_quest(), events);
                                                        set_quest_to_events(i.event_quest(), events);
                                                        first_start_activity = true;
                                                }
                                        }
                                        if (first_start_activity) {
                                                //bi::instance().activity_leiji_recharge_start(username_func_(), yci_, ip_, pttid, pd::AAT_LEIJI_RECHARGE, gid_, data_.level_);
                                        }
                                        if (events.events_size() > 0) {
                                                event_res = event_process(events, *this, pd::ce_env());
                                        }
                                } else if (refresh_day() >= start_day + duration - 1) {
                                        event_res = activity_stop_leiji_recharge();
                                        activity_reset_leiji_recharge(0, 0 , pttid);
                                }
                        }
                }

                pd::event_res role::activity_stop_leiji_consume() {
                        pd::event_res ret;
                        auto start_day = activity_leiji_consume_start_day();
                        auto duration = activity_leiji_consume_duration();
                        auto pttid = activity_leiji_consume_pttid();
                        ASSERT(pttid != 0);
                        if (duration == 0 || refresh_day() >= start_day + duration - 1) {
                                const auto& ptt = PTTS_GET(activity_leiji_consume, pttid);
                                pd::event_array events;
                                for (auto i : ptt.quests()) {
                                        if (quest_committed(i.auto_quest())) {
                                                if (!quest_committed(i.event_quest())) {
                                                        event_merge(events, quest_commit_to_events(i.event_quest(), *this));
                                                }
                                                quest_remove_commited_quest(i.event_quest());
                                                quest_remove_commited_quest(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                                if (events.events_size() != 0) {
                                        //bi::instance().activity_leiji_consume_stop(username_func_(), yci_, ip_, pttid, pd::AAT_LEIJI_CONSUME, duration, gid_, data_.level_);
                                        const auto& ptt = PTTS_GET(activity_logic, 1);
                                        ASSERT(send_mail_func_);
                                        send_mail_func_(ptt.leiji_consume_mail(), events);
                                }
                                for (const auto& i : ptt.quests()) {
                                        if (quest_has_accepted(i.auto_quest())) {
                                                quest_remove(i.auto_quest());
                                                ret.add_remove_quests(i.auto_quest());
                                        }
                                        if (quest_has_accepted(i.event_quest())) {
                                                quest_remove(i.event_quest());
                                                ret.add_remove_quests(i.event_quest());
                                        }
                                }
                                activity_clear_leiji_consume();
                        }
                        return ret;
                }

                void role::activity_leiji_consume_update_quests(pd::event_res& event_res) {
                        auto start_day = activity_leiji_consume_start_day();
                        auto duration = activity_leiji_consume_duration();
                        auto pttid = activity_leiji_consume_pttid();
                        if (start_day != 0) {
                                ASSERT(pttid != 0);
                                const auto& quest_ptt = PTTS_GET(activity_leiji_consume, pttid);
                                if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                        bool first_start_activity = false;
                                        pd::event_array events;
                                        for (const auto& i : quest_ptt.quests()) {
                                                if (!quest_has_accepted(i.auto_quest()) && !quest_committed(i.auto_quest())) {
                                                        set_quest_to_events(i.auto_quest(), events);
                                                        set_quest_to_events(i.event_quest(), events);
                                                        first_start_activity = true;
                                                }
                                        }
                                        if (first_start_activity) {
                                                //bi::instance().activity_leiji_consume_start(username_func_(), yci_, ip_, pttid, pd::AAT_LEIJI_CONSUME, gid_, data_.level_);
                                        }
                                        if (events.events_size() > 0) {
                                                event_res = event_process(events, *this, pd::ce_env());
                                        }
                                } else if (refresh_day() >= start_day + duration - 1) {
                                        event_res = activity_stop_leiji_consume();
                                        activity_reset_leiji_consume(0, 0 , pttid);
                                }
                        }
                }

                void role::activity_recharge_rank_update() {
                        auto start_day = activity_recharge_rank_start_day();
                        auto duration = activity_recharge_rank_duration();
                        if (refresh_day() > start_day + duration - 1) {
                                activity_reset_recharge_rank(0, 0);
                                activity_mgr::instance().reset_recharge_rank(0, 0);
                        }
                }

                void role::reset_resources() {
                        auto today = refresh_day();
                        const auto& ptts = PTTS_GET_ALL(resource);
                        if (today != resources_.reset_day_) {
                                for (const auto& i : ptts) {
                                        if (i.second.has_reset_everyday()) {
                                                if (get_resource(i.second.type()) != i.second.reset_everyday()) {
                                                        set_resource(i.second.type(), i.second.reset_everyday());
                                                }
                                        }
                                }
                                resources_.reset_day_ = refresh_day();
                                resources_changed_ = true;
                        }

                        auto monday = refresh_monday();
                        if (resources_.reset_monday_ != monday) {
                                for (const auto& i : ptts) {
                                        if (i.second.has_reset_every_week()) {
                                                if (get_resource(i.second.type()) != i.second.reset_every_week()) {
                                                        set_resource(i.second.type(), i.second.reset_every_week());
                                                }
                                        }
                                }
                                resources_.reset_monday_ = monday;
                                resources_changed_ = true;
                        }
                }

                void role::reset_activity_qiandao() {
                        if (activity_.qiandao_.month_ != static_cast<uint32_t>(refresh_month())) {
                                activity_.qiandao_.mdays_.clear();
                                activity_.qiandao_.got_leiqian_days_.clear();
                                activity_changed_ = true;
                        }
                }

                void role::activity_seven_days_update_quests(pd::event_res& event_res) {
                        auto day = refresh_day() - refresh_day_from_time_t(data_.first_login_time_) + 1;
                        const auto& ptts = PTTS_GET_ALL(activity_seven_days);
                        const pc::activity_seven_days *ptt = nullptr;
                        for (const auto& i : ptts) {
                                if (day >= i.first && day < i.first + 7) {
                                        ptt = &i.second;
                                        break;
                                }
                        }
                        if (!ptt) {
                                return;
                        }

                        if (!activity_seven_days_quests_accepted(ptt->begin_day())) {
                                SPLAYER_ILOG << *this << " accept seven days quests " << ptt->begin_day();
                                for (const auto& i : ptt->quests()) {
                                        for (auto j : i.quests()) {
                                                quest_accept(j);
                                                serialize_accepted_quest(j, event_res.add_accept_quests());
                                        }
                                }
                                activity_seven_days_set_quests_accepted(ptt->begin_day());
                        }
                }

                pd::event_res role::activity_stop_festival() {
                        auto pttid = activity_festival_pttid();
                        const auto& af_ptt = PTTS_GET(activity_festival, pttid);
                        pd::event_res ret;
                        pd::event_array events;
                        const auto& s_ptt = PTTS_GET(shop, af_ptt.shop());
                        for (const auto& i : s_ptt.goods()) {
                                auto* event = events.add_events();
                                event->set_type(pd::event::REMOVE_BUY_GOOD_TIMES);
                                event->add_arg(to_string(af_ptt.shop()));
                                event->add_arg(to_string(i.id()));
                        }
                        ret = event_process(events, *this, pd::ce_env());
                        set<uint32_t> quests;
                        for (auto i : af_ptt.recharge()) {
                                quests.insert(i);
                        }
                        for (auto i : af_ptt.fuli()) {
                                quests.insert(i);
                        }
                        pd::event_array mail_events;
                        for (auto i : quests) {
                                ret.add_remove_quests(i);
                                while(quest_has_accepted(i) && quest_check_commit(i) == pd::OK) {
                                        event_merge(mail_events, quest_commit_to_events(i, *this));
                                }
                                if (quest_has_accepted(i)) {
                                        quest_remove(i);
                                }
                                if (quest_committed(i)) {
                                        quest_.passed_.erase(i);
                                }
                        }
                        if (mail_events.events_size() != 0) {
                                //bi::instance().activity_festival_stop(username_func_(), yci_, ip_, pttid, pd::AAT_FESTIVAL, activity_festival_duration(), gid_, data_.level_);
                                const auto& ptt = PTTS_GET(activity_logic, 1);
                                ASSERT(send_mail_func_);
                                send_mail_func_(ptt.festival_mail(), mail_events);
                        }
                        reset_activity_festival_exchange_times();
                        return ret;
                }

                void role::activity_festival_update_quests(pd::event_res& event_res) {
                        auto pttid = activity_festival_pttid();
                        auto start_day = activity_festival_start_day();
                        auto duration = activity_festival_duration();
                        if (PTTS_HAS(activity_festival, pttid)) {
                                const auto& ptt = PTTS_GET(activity_festival, pttid);
                                if (start_day <= refresh_day() && start_day + duration >= refresh_day()) {
                                        set<uint32_t> quests;
                                        for (auto i : ptt.recharge()) {
                                                quests.insert(i);
                                        }
                                        for (auto i : ptt.fuli()) {
                                                quests.insert(i);
                                        }
                                        bool first_start_activity = false;
                                        for (auto i : quests) {
                                                if (!quest_has_accepted(i) && !quest_committed(i)) {
                                                        quest_accept(i);
                                                        serialize_accepted_quest(i, event_res.add_accept_quests());
                                                        first_start_activity = true;
                                                }
                                        }
                                        if (first_start_activity) {
                                                //bi::instance().activity_festival_start(username_func_(), yci_, ip_, pttid, pd::AAT_FESTIVAL, gid_, data_.level_);
                                        }
                                } else {
                                        event_res = activity_stop_festival();
                                }
                        }
                }

                void role::activity_start_prize_wheel(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.prize_wheel_.ongoing_) {
                                return;
                        }

                        activity_.prize_wheel_.start_day_ = start_day;
                        activity_.prize_wheel_.duration_ = duration;
                        activity_.prize_wheel_.pttid_ = pttid;
                        activity_changed_ = true;
                }

                void role::activity_prize_wheel_check_start(pd::event_res& event_res) {
                        if (activity_.prize_wheel_.ongoing_) {
                                return;
                        }

                        auto end_day = activity_.prize_wheel_.start_day_ + activity_.prize_wheel_.duration_;
                        auto today = refresh_day();
                        if (today < activity_.prize_wheel_.start_day_ || today >= end_day) {
                                return;
                        }

                        ASSERT(PTTS_HAS(activity_prize_wheel, activity_.prize_wheel_.pttid_));
                        const auto& ptt = PTTS_GET(activity_prize_wheel, activity_.prize_wheel_.pttid_);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ACTIVITY_START_PRIZE_WHEEL);
                        event_res = event_process(ptt.start_events(), *this, ce, &event_res);

                        activity_.prize_wheel_.ongoing_ = true;
                        activity_changed_ = true;
                }

                void role::activity_prize_wheel_check_finish(pd::event_res& event_res, bool force) {
                        if (!activity_.prize_wheel_.ongoing_) {
                                return;
                        }

                        auto end_day = activity_.prize_wheel_.start_day_ + activity_.prize_wheel_.duration_;
                        auto today = refresh_day();
                        if (today >= activity_.prize_wheel_.start_day_ && today < end_day && !force) {
                                return;
                        }

                        ASSERT(PTTS_HAS(activity_prize_wheel, activity_.prize_wheel_.pttid_));
                        const auto& ptt = PTTS_GET(activity_prize_wheel, activity_.prize_wheel_.pttid_);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ACTIVITY_FINISH_PRIZE_WHEEL);
                        event_res = event_process(ptt.finish_events(), *this, ce, &event_res);

                        activity_.prize_wheel_.ongoing_ = false;
                        activity_changed_ = true;
                }

                void role::activity_seven_days_update_fuli_quests(pd::event_res& event_res) {
                        auto day = refresh_day() - refresh_day_from_time_t(data_.first_login_time_) + 1;
                        const auto& ptts = PTTS_GET_ALL(activity_seven_days);
                        const pc::activity_seven_days *ptt = nullptr;
                        uint32_t begin_day = 0;

                        auto today = refresh_day();
                        for (const auto& i : ptts) {
                                if (day >= i.first && day < i.first + 7) {
                                        ptt = &i.second;
                                        day -= i.first;
                                        begin_day = today - day;
                                        break;
                                }
                        }

                        for (const auto& i : ptts) {
                                for (const auto& j : i.second.fuli()) {
                                        for (auto k : j.quests()) {
                                                if (quest_has_accepted(k)) {
                                                        quest_unregister_event(k);
                                                }
                                        }
                                }
                        }

                        if (!ptt) {
                                return;
                        }

                        if (!activity_seven_days_fuli_quests_accepted(ptt->begin_day())) {
                                for (const auto& i : ptt->fuli()) {
                                        for (auto j : i.quests()) {
                                                quest_accept(j, false, refresh_day_to_time_t(begin_day));
                                                serialize_accepted_quest(j, event_res.add_accept_quests());
                                        }
                                        begin_day += 1;
                                }
                                activity_seven_days_set_fuli_quests_accepted(ptt->begin_day());
                        }

                        for (auto i = 0; i < ptt->fuli_size(); i++) {
                                const auto& quests = ptt->fuli(i).quests();
                                for (auto j : quests) {
                                        if (quest_has_accepted(j) && i == static_cast<int>(day)) {
                                                quest_register_event(j, true);
                                        }
                                }
                        }
                }

                void role::activity_start_tea_party(uint32_t start_day, uint32_t duration, uint32_t pttid) {
                        if (activity_.tea_party_.ongoing_) {
                                return;
                        }

                        activity_.tea_party_.start_day_ = start_day;
                        activity_.tea_party_.duration_ = duration;
                        activity_.tea_party_.pttid_ = pttid;
                        activity_.tea_party_.got_favor_rewards_.clear();
                        activity_changed_ = true;
                }

                void role::activity_tea_party_check_start(pd::event_res& event_res) {
                        if (activity_.tea_party_.ongoing_) {
                                return;
                        }

                        auto end_day = activity_.tea_party_.start_day_ + activity_.tea_party_.duration_;
                        auto today = refresh_day();
                        if (today < activity_.tea_party_.start_day_ || today >= end_day) {
                                return;
                        }

                        ASSERT(PTTS_HAS(activity_tea_party, activity_.tea_party_.pttid_));
                        const auto& ptt = PTTS_GET(activity_tea_party, activity_.tea_party_.pttid_);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ACTIVITY_START_TEA_PARTY);
                        event_res = event_process(ptt.start_events(), *this, ce, &event_res);

                        activity_.tea_party_.ongoing_ = true;
                        activity_changed_ = true;
                }

                void role::activity_tea_party_check_finish(pd::event_res& event_res, bool force) {
                        if (!activity_.tea_party_.ongoing_) {
                                return;
                        }

                        auto end_day = activity_.tea_party_.start_day_ + activity_.tea_party_.duration_;
                        auto today = refresh_day();
                        auto before_finish = today < end_day - 1 || (today == end_day - 1 && clock::instance().hour() < 22);

                        if (today >= activity_.tea_party_.start_day_ && before_finish && !force) {
                                return;
                        }

                        ASSERT(PTTS_HAS(activity_tea_party, activity_.tea_party_.pttid_));
                        const auto& ptt = PTTS_GET(activity_tea_party, activity_.tea_party_.pttid_);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ACTIVITY_FINISH_TEA_PARTY);

                        pd::event_array events;
                        for (auto i = 0; i < ptt.favor_rewards_size(); i++) {
                                if (!activity_tea_party_got_favor_reward(i)) {
                                        auto ret = condition_check(ptt.favor_rewards(i).conditions(), *this, ce);
                                        if (ret != pd::OK) {
                                                continue;
                                        }
                                        event_merge(events, ptt.favor_rewards(i).events());
                                }
                        }
                        if (events.events_size() != 0) {
                                ASSERT(send_mail_func_);
                                send_mail_func_(ptt.favor_rewards_mail(), events);
                        }

                        event_res = event_process(ptt.finish_events(), *this, ce, &event_res);

                        activity_.tea_party_.ongoing_ = false;
                        activity_changed_ = true;
                }


                pd::record_entity role::relation_receive_gift(uint64_t from, uint32_t gift, uint32_t count) {
                        pd::record_entity record;
                        record_init_record(record);
                        auto *rg_record = record.mutable_relation_gift();
                        rg_record->set_from(from);
                        rg_record->set_gift(gift);
                        rg_record->set_count(count);

                        record_add_record(pd::RT_SOCIAL_GIFT, 0, record);
                        records_changed_ = true;
                        return record;
                }

                void role::change_name(const string& new_name) {
                        auto old_name = data_.name_;
                        data_.former_names_.insert(old_name);
                        data_.name_ = new_name;
                        role_data_changed_ = true;
                }
        }
}
