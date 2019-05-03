#pragma once

#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"
#include "proto/ss_login.pb.h"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                class login : public proto::processor<login, ps::base>,
                             public server_process_base,
                             public enable_shared_from_this<login> {
                public:
                        login(const string& name);
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const login& m) {
                                return os << m.name_;
                        }

                        static string base_name();
                        void on_register_done() override;
                private:
                        const string name_;
                };

        }
}
