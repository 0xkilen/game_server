#pragma once

#include "net/http/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/ss_gm.pb.h"
#include "proto/config_options.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/data_rank.pb.h"
#include "utils/string_utils.hpp"
#include "config/utils.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>
#include <limits.h>

using namespace std;
namespace py = proto::ys;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        class service_thread;
        namespace gm {

                class yunying : public enable_shared_from_this<yunying>,
                                public proto::processor<yunying, py::base>,
                                private boost::noncopyable {
                public:
                        yunying(const shared_ptr<service_thread>& st, const shared_ptr<net::server<net::HTTP_CONN>>& nsv, const shared_ptr<net::HTTP_CONN>& c, uint32_t sid);
                        void process_msg(const shared_ptr<net::recvstream>& msg);
                        uint32_t sid() const;
                        void stop();
                        friend ostream& operator<<(ostream& os, const yunying& y);

                        function<void(const shared_ptr<net::HTTP_CONN>&, int)> register_cb_;
                        function<void(const shared_ptr<net::HTTP_CONN>&)> stop_cb_;
                        function<void(uint32_t, int, uint64_t, const string&)> kick_role_by_gid_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t, const string&)> gag_role_by_gid_func_;
                        function<void(uint32_t, int, uint64_t)> remove_gag_role_by_gid_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t, const string&)> ban_role_by_gid_func_;
                        function<void(uint32_t, int, uint64_t)> remove_ban_role_by_gid_func_;
                        function<void(uint32_t, int)> fetch_world_chat_func_;
                        function<void(uint32_t, int, uint64_t)> find_role_by_gid_func_;
                        function<void(uint32_t, int, const string&)> find_role_by_rolename_func_;
                        function<void(uint32_t, int, const string&, uint32_t)> fetch_rank_list_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t)> internal_recharge_func_;
                        function<void(uint32_t, int, uint64_t, uint64_t)> reissue_recharge_func_;
                        function<void(uint32_t, uint64_t, const string&, const string&, uint32_t, uint32_t, const string&, const string&)> recharge_func_;
                        function<void(uint32_t, int, uint32_t, const pd::gid_array&, uint64_t, const string&, const string&, const pd::event_array&)> send_mail_func_;
                        function<void(uint32_t, const pd::announcement&)> manage_announcement_func_;
                        function<void(uint32_t, uint32_t, const map<string, uint32_t>&)> server_images_announcement_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t, uint32_t, uint32_t, int, int)> fetch_log_func_;
                        function<void(uint32_t, int, const set<string>& ips)> add_to_white_list_func_;
                        function<void(uint32_t, int, const set<string>& ips)> remove_from_white_list_func_;
                        function<void(uint32_t, int)> fetch_white_list_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t, uint32_t, int, int)> fetch_punished_func_;
                        function<void(uint32_t, int, uint64_t, uint32_t, uint32_t, int, int)> fetch_sent_mail_func_;
                        function<void(uint32_t, int, uint64_t, int, int)> fetch_login_record_func_;
                        function<void(uint32_t, int, uint32_t, uint64_t, uint64_t, uint32_t, uint32_t, int, int)> fetch_recharge_record_func_;
                        function<void(uint32_t, int, const string&,  uint64_t, uint32_t, uint32_t, int, int)> fetch_currency_record_func_;
                        function<void(uint32_t, const pd::gonggao&)> login_gonggao_func_;
                        function<void(uint32_t, const string&)> fetch_role_list_func_;
                        function<void(uint32_t, uint64_t, uint32_t, uint32_t)> add_resource_func_;
                        function<void(uint32_t, uint32_t, uint64_t, const pd::event_array&)> dec_stuff_by_gid_func_;
                        function<void(uint32_t, uint32_t, const string&, const pd::event_array&)> dec_stuff_by_rolename_func_;
                        function<void(uint32_t, uint32_t, uint64_t, const pd::event_array&)> add_stuff_by_gid_func_;
                        function<void(uint32_t, uint32_t, const string&, const pd::event_array&)> add_stuff_by_rolename_func_;
                        function<void(uint32_t, uint32_t, uint32_t, uint32_t, pd::admin_activity_type, const vector<string>&, bool)> operates_activity_func_;

                        void kick_role_by_gid_done(pd::result result);
                        void gag_role_by_gid_done(pd::result result);
                        void ban_role_by_gid_done(pd::result result);
                        void remove_punish_by_gid_done(pd::result result);
                        void fetch_world_chat_done(const pd::world_chat_array& world_chat, pd::result result);
                        void find_role_done(const pd::yunying_role_info& role, pd::result result);
                        void fetch_rank_type_list(uint32_t sid);
                        void fetch_rank_list_done(pd::result result, const pd::rank_list_array& rank_list_array);
                        void fetch_money_type(uint32_t sid);
                        void set_log_level(pc::options::log_level level);
                        void recharge_done(pd::result result);
                        void manage_announcement_done(pd::result result, const vector<int>& keys);
                        void server_images_announcements_done(pd::result result);
                        void send_mail_done(pd::result result, const pd::gid_array& roles);
                        void fetch_log_done(pd::result result, const pd::yunying_item_change_array& item_changes);
                        void remove_from_white_list_done(pd::result result);
                        void add_to_white_list_done(pd::result result);
                        void fetch_white_list_done(pd::result result, const pd::white_list_array& white_lists);
                        void set_money_list(auto& mrla, uint32_t id, const auto& ptts);
                        void fetch_punished_done(pd::result result, const pd::yunying_fetch_punished_info_array& yspia);
                        void fetch_sent_mail_done(pd::result result, const pd::yunying_fetch_sent_mail_info_array& yssmia);
                        void fetch_login_record_done(pd::result result, const pd::yunying_fetch_login_record_array& yflra);
                        void fetch_recharge_record_done(pd::result result, const pd::yunying_fetch_recharge_record_array& yfrra);
                        void fetch_currency_record_done(pd::result result, const pd::yunying_fetch_currency_record_array& yfcra);
                        void fetch_role_list_done(const pd::yunying_role_simple_info_array& role_infos, pd::result result) ;
                        void login_gonggao_done();
                        void add_stuff_done(pd::result result);
                        void dec_stuff_done(pd::result result);
                        void operates_activity_done(pd::result result);
                        void fetch_activity_config_done();

                private:
                        void process_request(const py::base *msg);
                        void send_error_to_yunying(py::error_code ec, const string& msg);
                        void send_to_yunying(const py::base& msg);
                        bool check_params(const map<string, string>& params, const set<string>& params_keys);

                        const string name_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<net::server<net::HTTP_CONN>> nsv_;
                        shared_ptr<net::HTTP_CONN> conn_;
                        uint32_t sid_;
                };

                inline uint32_t yunying::sid() const {
                        return sid_;
                }

        }
}
