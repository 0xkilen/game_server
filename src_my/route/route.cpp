#include "route.hpp"
#include "proto/ss_route.pb.h"
#include "config/options_ptts.hpp"
#include "route/route_mgr.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace route {

                string route::base_name() {
                        return "route";
                }

                route::route(const string& conn_name) : name_("route-" + conn_name) {
                }

                void route::static_init() {
                        route::register_proto_handler(ps::r2s_sync_route_table::descriptor(), &route::process_sync_route_table);
                }

                void route::on_register_done() {
                        auto ptt = PTTS_GET_COPY(options, 1);

                        proto::ss::base msg;
                        auto *sync = msg.MutableExtension(ps::s2r_sync_route_info::ssri);
                        sync->set_type(route_mgr::instance().type());
                        *sync->mutable_route_info() = route_mgr::instance().route_info();

                        server_process_mgr<route>::instance().send_msg(id(), msg);
                }

                void route::process_sync_route_table(const ps::base *msg) {
                        const auto& sync = msg->GetExtension(ps::r2s_sync_route_table::rsrt);
                        route_mgr::instance().update_route_table(sync.route_table());
                }

        }
}
