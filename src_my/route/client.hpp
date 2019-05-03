#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "proto/data_route.pb.h"
#include "utils/client_process_base.hpp"
#include <map>

using namespace std;
namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace route {

                class client : public proto::processor<client, ps::base>,
                               public client_process_base,
                               public enable_shared_from_this<client> {
                public:
                        client(const string& name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);

                        friend ostream& operator<<(ostream& os, const client& c) {
                                return os << c.name_;
                        }

                        static string base_name();

                        static map<pd::route_client_type, map<uint32_t, pd::route_info>> route_table;
                        static mutex route_table_lock_;
                        static void serialize_route_table(pd::route_table& data);
                        void on_stop() override;
                private:
                        void process_sync_route_info(const ps::base *msg);

                        const string name_;
                        pd::route_client_type type_ = pd::RCT_NONE;
                };

        }
}
