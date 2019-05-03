#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/client_process_base.hpp"
#include "proto/ss_login.pb.h"
#include "utils/client_process_mgr.hpp"

namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace login {

                class scene : public proto::processor<scene, ps::base>,
                              public client_process_base,
                              public enable_shared_from_this<scene> {
                public:
                        scene(const string& name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);

                        friend ostream& operator<<(ostream& os, const scene& s) {
                                return os << s.name_;
                        }

                        static string base_name();
                        static set<uint32_t> fetch_server_ids(const string& username);

                private:
                        const string name_;
                        static map<uint32_t, set<string>> server_id2username_;
                        void process_username_sync(const ps::base *msg);
                };

        }
}
