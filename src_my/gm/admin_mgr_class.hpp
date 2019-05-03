#pragma once

#include "utils/service_thread.hpp"
#include "net/server.hpp"
#include "db/connector.hpp"
#include "proto/ss_base.pb.h"
#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/data_admin.pb.h"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace gm {

                class admin;

                class admin_mgr_class : private boost::noncopyable {
                public:
                        admin_mgr_class();
                        static admin_mgr_class& instance() {
                                static admin_mgr_class inst;
                                return inst;
                        }
                        void start(const string& ip, unsigned short port);
                        void stop();

                        void player_online(uint32_t server_id, uint64_t gid);
                        void player_offline(uint32_t server_id, uint64_t gid);
                        void player_register_count_update(uint32_t server_id, uint32_t register_count);

                        void login_done(uint32_t conn_id, const string& username, const string& password);
                        void get_role_info_done(uint32_t conn_id, uint32_t server_id, const pd::role& role_data, pd::result result);
                        void operates_activity_done(uint32_t conn_id, pd::result result);
                        void admin_edit_role_by_gid_done(uint32_t conn_id, pd::result result, pd::admin_edit_role_type edit_type);
                        void admin_edit_role_rollback_by_gid_done(uint32_t conn_id, pd::result result, pd::admin_edit_role_type edit_type);
                        void admin_send_mail_done(uint32_t conn_id, pd::result result);
                        void admin_fetch_gonggao_list_done(uint32_t conn_id, const pd::gonggao_array& gonggao_list);
                        void admin_edit_gonggao_done(uint32_t conn_id, pd::result result);
                        void admin_fetch_white_list_done(uint32_t conn_id, pd::result result, const pd::white_list_array& white_list);
                        void admin_edit_white_list_done(uint32_t conn_id, bool publish, pd::result result);

                        friend ostream& operator<<(ostream& os, const admin_mgr_class& amc) {
                                return os << amc.name_;
                        }

                private:
                        string name_;
                        shared_ptr<service_thread> st_;
                        set<uint32_t> aids_;
                        map<uint32_t, set<uint64_t>> server2onlines_;
                        map<uint32_t, uint32_t> server2register_count_;
                        map<string, string> admin2passwords_;
                        shared_ptr<db::connector> gmdb_;
                        mutex lock_;
                };

        }
}
