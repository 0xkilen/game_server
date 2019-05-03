#include "routed.hpp"
#include "client.hpp"
#include "log.hpp"
#include "utils/time_utils.hpp"
#include "utils/client_process_mgr.hpp"
#include "proto/ss_route.pb.h"

namespace nora {
        namespace route {

                using client_mgr = client_process_mgr<client>;

                routed::routed(const shared_ptr<service_thread>& st) : st_(st) {
                        if (!st_) {
                                st_ = make_shared<service_thread>("routed");
                                st_->start();
                        }
                        client_mgr::static_init();
                }

                void routed::start() {
                        register_signals();

                        auto ptt = PTTS_GET_COPY(options, 1);
                        ASSERT(ptt.route_info().ipport_size() > 0);
                        client_mgr::instance().init(ptt.route_info().id());
                        client_mgr::instance().start(ptt.route_info().ipport(0).ip(), (unsigned short)(ptt.route_info().ipport(0).port()));

                        add_broadcast_route_table_timer();
                }

                void routed::add_broadcast_route_table_timer() {
                        ASSERT(st_);
                        ADD_TIMER(
                                st_,
                                ([this, self = this->shared_from_this()] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                ps::base notice_msg;
                                                pd::route_table data;
                                                client::serialize_route_table(data);

                                                auto *notice = notice_msg.MutableExtension(ps::r2s_sync_route_table::rsrt);
                                                *notice->mutable_route_table() = data;
                                                client_mgr::instance().broadcast_msg(notice_msg);

                                                this->add_broadcast_route_table_timer();
                                        }
                                }),
                                5s);
                }

                void routed::stop() {
                        client_mgr::instance().stop();
                        clock::instance().stop();
                        st_->stop();
                }

                void routed::register_signals() {
                        if (!signals_) {
                                signals_ = make_unique<ba::signal_set>(st_->get_service());
                                signals_->add(SIGTERM);
                                signals_->add(SIGINT);
                        }

                        signals_->async_wait(
                                [this] (auto ec, auto sig) {
                                        if (ec) {
                                                return;
                                        }

                                        if (sig == SIGTERM || sig == SIGINT) {
                                                ROUTE_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                this->stop();
                                        } else {
                                                this->register_signals();
                                        }
                                });
                }

        }
}
