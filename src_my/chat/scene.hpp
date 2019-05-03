#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/client_process_base.hpp"

namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace chat {

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

                private:
                        void process_cross_world_chat(const ps::base *msg);
                        void process_system_chat_all_server(const ps::base *msg);

                        const string name_;
                };

        }
}
