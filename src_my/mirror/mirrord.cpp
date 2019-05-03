#include "mirrord.hpp"
#include "scene.hpp"
#include "route/route_mgr.hpp"
#include "log.hpp"
#include "config/mirror_ptts.hpp"
#include "utils/client_process_mgr.hpp"
#include "utils/time_utils.hpp"

namespace nora {
        namespace mirror {

                using scene_mgr = client_process_mgr<scene>;

                mirrord::mirrord(const shared_ptr<service_thread>& st) : st_(st) {
                        if (!st_) {
                                st_ = make_shared<service_thread>("mirrord");
                                st_->start();
                        }
                        scene_mgr::static_init();
                }

                void mirrord::start() {
                        register_signals();

                        auto ptt = PTTS_GET_COPY(options, 1);
                        scene_mgr::instance().init(ptt.mirror_info().id());
                        scene_mgr::instance().start(ptt.mirror_info().scened_ipport().ip(), (unsigned short)(ptt.mirror_info().scened_ipport().port()));

                        route::route_mgr::instance().init(ptt.mirror_info().id());
                        route::route_mgr::instance().start(pd::RCT_MIRRORD);
                }

                void mirrord::stop() {
                        scene_mgr::instance().stop();
                        clock::instance().stop();
                        st_->stop();
                }

                void mirrord::register_signals() {
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
                                                MIRROR_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                this->stop();
                                        } else {
                                                this->register_signals();
                                        }
                                });
                }

        }
}
