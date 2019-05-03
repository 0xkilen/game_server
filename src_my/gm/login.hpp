#pragma once

#include "net/conn.hpp"
#include "net/client.hpp"
#include "net/stream.hpp"
#include "proto/ss_base.pb.h"
#include "proto/data_yunying.pb.h"
#include <memory>
#include <boost/core/noncopyable.hpp>
#include "proto/processor.hpp"
#include "utils/server_process_base.hpp"

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace gm {
                class login : public enable_shared_from_this<login>,
                                public server_process_base,
                                public proto::processor<login, ps::base> {
                public:
                        login(const string& conn_name);
                        static string base_name();
                        static void static_init();

                        friend ostream& operator<<(ostream& os, const login& lg);
                private:
                        void process_gonggao_rsp(const ps::base *msg);
                        void process_add_to_white_list_rsp(const ps::base *msg);
                        void process_remove_from_white_list_rsp(const ps::base *msg);
                        void process_fetch_white_list_rsp(const ps::base *msg);
                        void process_fetch_gonggao_list_rsp(const ps::base *msg);

                        string name_;
                };
        }
}
