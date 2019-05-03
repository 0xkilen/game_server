#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                class chat : public proto::processor<chat, ps::base>,
                             public server_process_base,
                             public enable_shared_from_this<chat> {
                public:
                        chat(const string& name);
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const chat& m) {
                                return os << m.name_;
                        }

                        static string base_name();
                private:
                        void process_cross_world_chat(const ps::base *msg);
                        void process_system_chat_all_server(const ps::base *msg);

                        const string name_;
                };

        }
}
