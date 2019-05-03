#pragma once

#include "net/stream.hpp"
#include "net/server.hpp"
#include "proto/data_base.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/client_process_base.hpp"

using namespace std;
namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace gm {

                class admin : public proto::processor<admin, ps::base>,
                              public client_process_base,
                              public enable_shared_from_this<admin> {
                public:
                        admin(const string& name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);
                        static string base_name();

                        friend ostream& operator<<(ostream& os, const admin& ad) {
                                return os << ad.name_;
                        }

                private:
                        void process_login(const ps::base *msg);
                        void process_get_role_info_by_gid_req(const ps::base *msg);
                        void process_get_role_info_by_name_req(const ps::base *msg);
                        void process_admin_start_activity_req(const ps::base *msg);
                        void process_admin_edit_role_by_gid_req(const ps::base *msg);
                        void process_admin_edit_role_rollback_by_gid_req(const ps::base *msg);
                        void process_admin_send_mail_req(const ps::base *msg);
                        void process_admin_fetch_gonggao_list_req(const ps::base *msg);
                        void process_admin_fetch_white_list_req(const ps::base *msg);
                        void process_admin_edit_gonggao_req(const ps::base *msg);
                        void process_admin_edit_white_list_req(const ps::base *msg);

                        const string name_;
                        bool logined_ = false;
                };


        }
}
