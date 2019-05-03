#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                class mirror : public proto::processor<mirror, ps::base>,
                               public server_process_base,
                               public enable_shared_from_this<mirror> {
                public:
                        mirror(const string& name);
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const mirror& m) {
                                return os << m.name_;
                        }

                        static string base_name();
                private:
                        void process_role_event(const ps::base *msg);
                        void process_server_roles_event(const ps::base *msg);

                        const string name_;
                };

        }
}
