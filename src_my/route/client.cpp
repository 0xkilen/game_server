#include "client.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_route.pb.h"
#include "utils/client_process_mgr.hpp"

namespace nora {
        namespace route {

                using client_mgr = client_process_mgr<client>;

                map<pd::route_client_type, map<uint32_t, pd::route_info>> client::route_table;
                mutex client::route_table_lock_;

                void client::serialize_route_table(pd::route_table& data) {
                        lock_guard<mutex> lock(route_table_lock_);
                        for (const auto& i : route_table) {
                                for (const auto& j : i.second) {
                                        auto *c = data.add_clients();
                                        c->set_type(i.first);
                                        c->set_id(j.first);
                                        *c->mutable_route_info() = j.second;
                                }
                        }
                }

                string client::base_name() {
                        return "client";
                }

                client::client(const string& conn_name) : name_("route-client-" + conn_name) {
                }

                void client::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::s2r_sync_route_info::descriptor(), &client::process_sync_route_info);
                }

                void client::process_sync_route_info(const ps::base *msg) {
                        const auto& sync = msg->GetExtension(ps::s2r_sync_route_info::ssri);
                        type_ = sync.type();

                        lock_guard<mutex> lock(route_table_lock_);
                        route_table[type_][id()] = sync.route_info();
                }

                void client::on_stop() {
                        if (type_ == pd::RCT_NONE) {
                                return;
                        }

                        lock_guard<mutex> lock(route_table_lock_);
                        route_table[type_].erase(id());
                }

        }
}
