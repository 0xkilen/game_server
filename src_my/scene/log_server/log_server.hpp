#pragma once

#include "proto/processor.hpp"
#include "utils/string_utils.hpp"
#include "utils/server_process_base.hpp"
#include "proto/ss_log.pb.h"
#include <string>
#include <memory>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace scene {

                class log_server : public server_process_base,
                                   public proto::processor<log_server, ps::base> {
                public:
                        log_server(const string& name);
                        static string base_name();
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const log_server& ch) {
                                return os << "logserver";
                        }
                private:
                        const string name_;
                };

        }
}
