#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/data_chat.pb.h"
#include "proto/data_arena.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/data_log.pb.h"
#include "net/http/conn.hpp"
#include "net/server.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace gm {

                class yunying;

                class yunying_mgr : private boost::noncopyable {
                public:
                        yunying_mgr();
                        static yunying_mgr& instance() {
                                static yunying_mgr inst;
                                return inst;
                        }
                        void start(const string& ip, unsigned short port);
                        void stop();
                        friend ostream& operator<<(ostream& os, const yunying_mgr& ym);

                        bool has_yunying(uint32_t sid);
                        shared_ptr<yunying> get_yunying(uint32_t sid);
                        void kick_role_by_gid_done(uint32_t sid, pd::result result);
                        void gag_role_by_gid_done(uint32_t sid, pd::result result);
                        void ban_role_by_gid_done(uint32_t sid, pd::result result);
                        void remove_punish_by_gid_done(uint32_t sid, pd::result result);
                        void fetch_world_chat_done(uint32_t sid, const pd::world_chat_array& world_chat, pd::result result);
                        void find_role_done(uint32_t sid, const pd::yunying_role_info& role, pd::result result);
                        void fetch_rank_list_done(uint32_t sid, pd::result result, const pd::rank_list_array& rank_list_array);
                        void recharge_done(uint32_t sid, pd::result result);
                        void manage_announcement_done(uint32_t sid, pd::result result, const vector<int>& keys);
                        void server_images_announcements_done(uint32_t sid, pd::result result);
                        void send_mail_done(uint32_t sid, pd::result result, const pd::gid_array& roles);
                        void fetch_log_done(uint32_t sid, pd::result result, const pd::yunying_item_change_array& item_changes);
                        void add_to_white_list_done(uint32_t sid, pd::result result);
                        void remove_from_white_list_done(uint32_t sid, pd::result result);
                        void fetch_white_list_done(uint32_t sid, pd::result result, const pd::white_list_array& white_lists);
                        void fetch_punished_done(uint32_t sid, pd::result result, const pd::yunying_fetch_punished_info_array& yspia);
                        void fetch_sent_mail_done(uint32_t sid, pd::result result, const pd::yunying_fetch_sent_mail_info_array& yssmia);
                        void fetch_login_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_login_record_array& yflra);
                        void fetch_recharge_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_recharge_record_array& yfrra);
                        void fetch_currency_record_done(uint32_t sid, pd::result result, const pd::yunying_fetch_currency_record_array& yfrra);
                        void fetch_role_list_done(uint32_t sid, const pd::yunying_role_simple_info_array& role_infos, pd::result result);
                        void login_gonggao_done(uint32_t sid);
                        void add_stuff_done(uint32_t sid, pd::result result);
                        void dec_stuff_done(uint32_t sid, pd::result result);
                        void operates_activity_done(uint32_t sid, pd::result result);

                private:
                        string name_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<net::server<net::HTTP_CONN>> nsv_;
                        uint32_t sid_seed_ = 1;
                        map<uint64_t, shared_ptr<yunying>> sid2yunying_;
                        map<shared_ptr<net::HTTP_CONN>, shared_ptr<yunying>> conn2yunying_;
                        mutex lock_;
                };

                inline bool yunying_mgr::has_yunying(uint32_t sid) {
                        return sid2yunying_.count(sid) > 0;
                }

                inline shared_ptr<yunying> yunying_mgr::get_yunying(uint32_t sid) {
                        ASSERT(sid2yunying_.count(sid) > 0);
                        return sid2yunying_.at(sid);
                }

        }
}
