#pragma once

#include "proto/ss_base.pb.h"
#include "proto/ss_mirror.pb.h"
#include "proto/processor.hpp"
#include "utils/client_process_base.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace mirror {

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
                        void send_to_mirror(const ps::base *msg);

                        const string name_;
                };

        }
}
