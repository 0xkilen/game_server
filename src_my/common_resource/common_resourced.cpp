#include "common_resourced.hpp"
#include "scene.hpp"
#include "log.hpp"
#include "route/route_mgr.hpp"
#include "utils/time_utils.hpp"
#include "utils/client_process_mgr.hpp"
#include "utils/assert.hpp"
#include "proto/data_route.pb.h"

namespace nora {
        namespace common_resource {

                using scene_mgr = client_process_mgr<scene>;

                void common_resourced::start() {
                        st_ = make_shared<service_thread>("common_resource");
                        st_->start();

                        mutex m;
                        condition_variable cv;
                        auto waiting_count = 0;
                        auto done_cb = [&waiting_count, &m, &cv] {
                                {
                                        lock_guard<mutex> lk(m);
                                        waiting_count -= 1;
                                }
                                cv.notify_one();
                        };
                        scene::done_cb_ = done_cb;
                        waiting_count = 2;
                        scene_mgr::static_init(st_);
                        auto ptt = PTTS_GET_COPY(options, 1);
                        scene_mgr::instance().init(ptt.common_resource_info().id());
                        scene_mgr::instance().start(ptt.common_resource_info().scened_ipport().ip(), (unsigned short)(ptt.common_resource_info().scened_ipport().port()));
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_count] { return waiting_count == 0; });
                        }

                        route::route_mgr::instance().init(ptt.common_resource_info().id());
                        route::route_mgr::instance().start(pd::RCT_COMMON_RESOURCED);
                        register_signals();
                }

                void common_resourced::stop() {
                        scene_mgr::instance().stop();
                        route::route_mgr::instance().stop();
                        clock::instance().stop();
                        st_->stop();
                }

                void common_resourced::register_signals() {
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
                                                COMMON_RESOURCE_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                this->stop();
                                        } else {
                                                this->register_signals();
                                        }
                                });
                }

        }
}
