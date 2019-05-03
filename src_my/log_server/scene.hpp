#pragma once

#include "net/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "utils/client_process_base.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/ss_log.pb.h"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        namespace log_serverd {

                class scene : public proto::processor<scene, ps::base>,
                              public client_process_base,
                              private boost::noncopyable {
                public:
                        scene(const string& conn_name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);
                        static string base_name();

                        friend ostream& operator<<(ostream& os, const scene& s);
                private:
                        void process_bi_info_sync(const ps::base *msg);

                        const string name_;
                };

                inline string scene::base_name() {
                        return "logserver";
                }

        }
}
