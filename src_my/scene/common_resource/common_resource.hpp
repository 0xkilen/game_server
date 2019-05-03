#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                class common_resource : public proto::processor<common_resource, ps::base>,
                             public server_process_base,
                             public enable_shared_from_this<common_resource> {
                public:
                        common_resource(const string& name);
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const common_resource& m) {
                                return os << m.name_;
                        }

                        static string base_name();
                private:
                        void process_lock_fanli_rsp(const ps::base *msg);
                        void process_lock_come_back_rsp(const ps::base *msg);

                        const string name_;
                };

        }
}
