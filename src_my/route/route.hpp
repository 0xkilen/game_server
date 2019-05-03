#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace route {

                class route : public proto::processor<route, ps::base>,
                             public server_process_base,
                             public enable_shared_from_this<route> {
                public:
                        route(const string& name);
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const route& r) {
                                return os << r.name_;
                        }

                        static string base_name();
                        void on_register_done() override;
                private:
                        void process_sync_route_table(const ps::base *msg);

                        const string name_;
                };

        }
}
