#pragma once

#include "net/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "db/message.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/data_yunying.pb.h"
#include "utils/client_process_base.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace login {

                class gm : public proto::processor<gm, ps::base>,
                           public client_process_base,
                           private boost::noncopyable {
                public:
                        gm(const string& conn_name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);
                        static string base_name();
                        friend ostream& operator<<(ostream& os, const gm& g);
                private:
                        void process_gonggao_req(const ps::base *msg);
                        void process_add_to_white_list_req(const ps::base *msg);
                        void process_remove_from_white_list_req(const ps::base *msg);
                        void process_fetch_white_list_req(const ps::base *msg);
                        void process_fetch_gonggao_list_req(const ps::base *msg);

                private:
                        string name_;
                };
        }
}
