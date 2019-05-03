#pragma once

#include "config/options_ptts.hpp"
#include "utils/string_utils.hpp"
#include "proto/data_admin.pb.h"
#include "gm.hpp"
#include <string>
#include <memory>

using namespace std;
namespace pd = proto::data;
namespace pcs = proto::cs;

namespace nora {
        class service_thread;
        namespace scene {

                class gm_mgr_class {
                public:
                        gm_mgr_class();
                        gm_mgr_class(const shared_ptr<service_thread>& st);
                        static gm_mgr_class& instance() {
                                static gm_mgr_class inst;
                                return inst;
                        }
                        void start();
                        void stop();
                        friend ostream& operator<<(ostream& os, const gm_mgr_class& gms);

                        void publish_announcement(const system_clock::time_point& time_point);
                        void manage_announcement_done(uint32_t sid, uint32_t gm_id, const pd::announcement& ann);
                        void add_images_announcement(const pd::images_announcement_array& anns);
                        void update_images_announcement(const map<string, uint32_t>& images_announcements);
                        map<string, uint32_t> get_images_announcements();
                        void fetch_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t item_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void set_item_change(const pd::log& log, pd::yunying_item_change_array& item_changes);
                        void set_item_use(const pd::log& log, pd::yunying_item_change_array& item_changes);
                        void fetch_punished_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void set_punished(const pd::punish_role& pr, pd::yunying_fetch_punished_info_array& yspia, int server_id);
                        void fetch_mail_log(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void fetch_login_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void fetch_recharge_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void set_recharge_record(pd::yunying_fetch_recharge_record *recharge_record, const pd::log_recharge& log, uint32_t time, const string& server_name);
                        void fetch_currency_record(uint32_t sid, uint32_t gm_id, const string& procedure, uint64_t role, const vector<pd::log::log_type>& types, uint32_t server_id, uint32_t start_time, uint32_t end_time, uint32_t page_idx, uint32_t page_size);
                        void set_currency_record(pd::yunying_fetch_currency_record *currency_record, const pd::log_currency_change& log, uint32_t time);
                        void fetch_ranK_list(uint32_t sid, uint32_t gm_id, const string& type, uint32_t page_size);

                        void gag_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void ban_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void kick_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void remove_punish_role_by_gid_done(uint32_t sid, uint32_t gm_id, pd::result result, pd::admin_edit_role_type type);
                        void fetch_world_chat_done(uint32_t sid, uint32_t gm_id, const pd::world_chat_array& world_chat, pd::result result);
                        void set_role_info(const pd::role& role, pd::yunying_role_info& role_info, uint32_t online);
                        void find_role_by_gid_done(uint32_t sid, uint32_t gm_id, const pd::role& role, pd::result result, uint32_t online);
                        void yunying_fetch_rank_list_done(uint32_t sid, uint32_t gm_id, const pd::rank_list_array& rank_lists);
                        void yunying_recharge_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void send_mail_done(uint32_t sid, uint32_t gm_id, pd::result result, const pd::gid_array& roles);
                        void yunying_fetch_log_done(uint32_t sid, uint32_t gm_id, const pd::yunying_item_change_array& item_changes);
                        void set_role_simple_info(const pd::role& role, pd::yunying_role_simple_info& role_simple_info);
                        void fetch_role_list_done(uint32_t sid, uint32_t gm_id, const pd::role& role, pd::result result);
                        void yunying_add_stuff_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void yunying_dec_stuff_done(uint32_t sid, uint32_t gm_id, pd::result result);
                        void fetch_punished_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_punished_info_array& punished_info);
                        void fetch_sent_mail_done(uint32_t sid, uint32_t gm_id, const vector<uint64_t>& mailids, const map<uint64_t, pd::yunying_fetch_sent_mail_info>& mail_infos);
                        void fetch_login_records_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_login_record_array& login_records);
                        void fetch_recharge_record_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_recharge_record_array& recharge_records);
                        void fetch_currency_record_done(uint32_t sid, uint32_t gm_id, const pd::yunying_fetch_currency_record_array& currency_records);
                        void yunying_operates_activity_done(uint32_t sid, uint32_t gm_id, pd::result result);

                        void admin_role_online(uint32_t server_id, uint64_t gid);
                        void admin_role_offline(uint32_t server_id, uint64_t gid);
                        void admin_update_register_count(uint32_t server_id, uint32_t register_count);

                private:
                        void load_announcement();
                        void in_second();
                        void add_announcement(const pd::announcement& announcement);

                        mutex lock_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<db::connector> game_db_;
                        map<system_clock::time_point, set<uint32_t>> time_point2ids_;
                        map<uint32_t, pd::announcement> id2ann_;
                        shared_ptr<timer_type> announcement_timer_;
                        map<uint32_t, shared_ptr<timer_type>> sid2timeout_timer_;
                        map<string, uint32_t> images_announcements_;
                };

                inline map<string, uint32_t> gm_mgr_class::get_images_announcements() {
                        lock_guard<mutex> lock(lock_);
                        auto now_time = system_clock::to_time_t(system_clock::now());
                        map<string, uint32_t> new_images;
                        for (const auto& i : images_announcements_) {
                                if (i.second >= now_time) {
                                        new_images[i.first] = i.second;
                                }
                        }
                        images_announcements_ = new_images;
                        update_images_announcement(new_images);
                        return images_announcements_;
                }

        }
}
