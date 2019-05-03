#include "route_mgr.hpp"
#include "route.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "utils/proto_utils.hpp"

namespace nora {
        namespace route {

                void route_mgr::start(pd::route_client_type type) {
                        if (!stopped_) {
                                return;
                        }
                        stopped_ = false;

                        st_ = make_shared<service_thread>("route_mgr");
                        st_->start();
                        st_->async_call(
                                [this, type] {
                                        auto ptt = PTTS_GET_COPY(options, 1);

                                        type_ = type;
                                        switch (type_) {
                                        case pd::RCT_SCENED:
                                                break;
                                        case pd::RCT_CHATD:
                                                *route_info_.mutable_scened_ipport() = ptt.chat_info().scened_ipport();
                                                break;
                                        case pd::RCT_GMD:
                                                *route_info_.mutable_scened_ipport() = ptt.gm_info().scened_ipport();
                                                break;
                                        case pd::RCT_LOGIND:
                                                *route_info_.mutable_gmd_ipport() = ptt.login_info().gmd_ipport();
                                                *route_info_.mutable_scened_ipport() = ptt.login_info().scened_ipport();
                                                break;
                                        case pd::RCT_MIRRORD:
                                                *route_info_.mutable_scened_ipport() = ptt.mirror_info().scened_ipport();
                                                break;
                                        case pd::RCT_PUBLIC_ARENAD:
                                                *route_info_.mutable_scened_ipport() = ptt.scened_public_arenad_ipport();
                                                break;
                                        case pd::RCT_COMMON_RESOURCED:
                                                *route_info_.mutable_scened_ipport() = ptt.common_resource_info().scened_ipport();
                                                break;
                                        default:
                                                ASSERT(false);
                                        }

                                        server_process_mgr<route>::instance().static_init();
                                        server_process_mgr<route>::instance().init(id_, true);
                                        vector<pd::ipport> addrs;
                                        for (const auto& i : ptt.route_info().ipport()) {
                                                addrs.push_back(i);
                                        }
                                        server_process_mgr<route>::instance().set_servers(addrs);
                                });
                }

                void route_mgr::stop() {
                        ASSERT(st_);
                        st_->async_call(
                                [this] {
                                        if (stopped_) {
                                                return;
                                        }
                                        stopped_ = true;

                                        server_process_mgr<route>::instance().stop();
                                        ROUTE_DLOG << *this << " stop";
                                });

                }

                void route_mgr::update_route_table(const pd::route_table& route_table) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, route_table] {
                                        ROUTE_DLOG << "update route table\n" << route_table.DebugString();

                                        route_table_ = route_table;

                                        if (init_done_cb_) {
                                                init_done_cb_();
                                                init_done_cb_ = nullptr;

                                                ROUTE_ILOG << *this << " start";
                                        }

                                        if (route_table_updated_cb_) {
                                                route_table_updated_cb_(route_table_);
                                        }
                                });
                }



        }
}
