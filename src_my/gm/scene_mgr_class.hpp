#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/data_chat.pb.h"
#include "proto/data_arena.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/data_log.pb.h"
#include "proto/data_admin.pb.h"
#include "net/conn.hpp"
#include "net/server.hpp"
#include "db/message.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace gm {

                class scene;

                class scene_mgr_class : private boost::noncopyable {
                public:
                        scene_mgr_class();
                        static scene_mgr_class& instance() {
                                static scene_mgr_class inst;
                                return inst;
                        }
                        void start();
                        void stop();
                        friend ostream& operator<<(ostream& os, const scene_mgr_class& sm);

                        void manage_announcement(uint32_t sid, const pd::announcement& announcement);
                        void server_images_announcement(uint32_t sid, uint32_t server_id, const map<string, uint32_t>& announcements);
                        void gag_role_by_gid(uint32_t sid, int server_id, uint64_t gid, uint32_t gag_until_time, const string& reason);
                        void remove_gag_role_by_gid(uint32_t sid, int server_id, uint64_t gid);
                        void ban_role_by_gid(uint32_t sid, int server_id, uint64_t gid, uint32_t ban_until_time, const string& reason);
                        void remove_ban_role_by_gid(uint32_t sid, int server_id, uint64_t gid);
                        void kick_role_by_gid(uint32_t sid, int server_id, uint64_t gid, const string& reason);
                        void fetch_world_chat(uint32_t sid, int server_id);
                        void find_role_by_gid(uint32_t sid, int server_id, uint64_t gid);
                        void find_role_by_rolename(uint32_t sid, int server_id, const string& rolename);
                        void fetch_rank_list(uint32_t sid, int server_id, const string& rank_type, uint32_t page_size);
                        void internal_recharge(uint32_t sid, int server_id, uint64_t role, uint32_t recharge_id);
                        void reissue_recharge(uint32_t sid, int server_id, uint64_t role, uint64_t order);
                        void recharge(uint32_t sid, uint64_t  order, const string& yy_orderno, const string& currency, uint32_t price, uint32_t paytime, const string& product_name, const string& ext_info);
                        void send_mail(uint32_t sid, int server_id, uint32_t channel_id, const pd::gid_array& roles, uint64_t mail_id, const string& title, const string& content, const pd::event_array& events);
                        void fetch_log(uint32_t sid, int server_id, uint64_t role, uint32_t item_id, uint32_t start_time, uint32_t end_time, int page_idx, int page_size);
                        void fetch_punished(uint32_t sid, int server_id, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size);
                        void fetch_sent_mail(uint32_t sid, int server_id, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size);
                        void fetch_login_record(uint32_t sid, int server_id, uint64_t role, int page_idx, int page_size);
                        void fetch_recharge_record(uint32_t sid, int server_id, uint32_t channel_id, uint64_t order, uint64_t role, int start_time, int end_time, int page_idx, int page_size);
                        void fetch_currency_record(uint32_t sid, int server_id, const string& type, uint64_t role, int start_time, int end_time, int page_idx, int page_size);
                        void fetch_role_list(uint32_t sid, const string& username);
                        void set_role_list(uint32_t sid, const pd::yunying_role_simple_info& rsi);
                        bool has_sid(uint32_t sid) const;
                        bool has_server_id(uint32_t sid, int server_id) const;
                        bool has_role_simple_infos(uint32_t sid) const;
                        void add_resource(uint32_t sid, uint64_t gid, uint32_t resource_type, uint32_t resource_count);
                        void add_stuff_by_rolename(uint32_t sid, uint32_t server_id, const string& rolename, const pd::event_array& events);
                        void add_stuff_by_gid(uint32_t sid, uint32_t server_id, uint64_t role, const pd::event_array& events);
                        void dec_stuff_by_rolename(uint32_t sid, uint32_t server_id, const string& rolename, const pd::event_array& events);
                        void dec_stuff_by_gid(uint32_t sid, uint32_t server_id, uint64_t role, const pd::event_array& events);
                        void operates_activity(uint32_t sid, uint32_t activity, uint32_t start_time, uint32_t duration, pd::admin_activity_type type, const vector<string>& server_ids, bool open_activity);

                private:
                        string name_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<timer_type> timer_;
                        pd::announcement announcement_;
                        set<uint32_t> sids_;
                        map<uint32_t, pd::yunying_role_simple_info_array> sid2role_infos_;
                        uint32_t client_process_count_ = 0;
                };

                inline bool scene_mgr_class::has_sid(uint32_t sid) const {
                        return sids_.count(sid) > 0;
                }

                inline bool scene_mgr_class::has_role_simple_infos(uint32_t sid) const {
                        return sid2role_infos_.count(sid) > 0;
                }

        }
}
