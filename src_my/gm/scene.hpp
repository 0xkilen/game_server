#pragma once

#include "net/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/ss_gm.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_yunying.pb.h"
#include "utils/client_process_base.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        namespace gm {

                class scene : public enable_shared_from_this<scene>,
                        public client_process_base,
                        public proto::processor<scene, ps::base>{
                public:
                        scene(const string& conn_name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);
                        static string base_name();
                        friend ostream& operator<<(ostream& os, const scene& s);

                private:
                        void process_gag_role_by_gid_rsp(const ps::base *msg);
                        void process_ban_role_by_gid_rsp(const ps::base *msg);
                        void process_remove_punish_by_gid_rsp(const ps::base *msg);
                        void process_kick_role_by_gid_rsp(const ps::base *msg);
                        void process_fetch_world_chat_rsp(const ps::base *msg);
                        void process_find_role_rsp(const ps::base *msg);
                        void process_recharge_rsp(const ps::base *msg);
                        void process_fetch_rank_list_rsp(const ps::base *msg);
                        void process_send_mail_rsp(const ps::base *msg);
                        void process_manage_announcement_rsp(const ps::base *msg);
                        void process_fetch_log_rsp(const ps::base *msg);
	                void process_fetch_white_list_rsp(const ps::base *msg);
                        void process_fetch_punished_rsp(const ps::base *msg);
                        void process_fetch_sent_mail_rsp(const ps::base *msg);
                        void process_fetch_login_record_rsp(const ps::base *msg);
                        void process_fetch_recharge_record_rsp(const ps::base *msg);
                        void process_fetch_currency_record_rsp(const ps::base *msg);
                        void process_fetch_role_list_rsp(const ps::base *msg);
                        void process_add_stuff_rsp(const ps::base *msg);
                        void process_dec_stuff_rsp(const ps::base *msg);
                        void process_operates_activity_rsp(const ps::base *msg);

                        // for admin_mgr
                        void process_admin_role_online_notice(const ps::base *msg);
                        void process_admin_role_offline_notice(const ps::base *msg);
                        void process_admin_role_register_count_update_notice(const ps::base *msg);

                        const string name_;
                };

        }
}
