#include "chatd.hpp"
#include "scene.hpp"
#include "route/route_mgr.hpp"
#include "utils/time_utils.hpp"
#include "utils/client_process_mgr.hpp"
#include "log.hpp"

namespace nora {
        namespace chat {

                using scene_mgr = client_process_mgr<scene>;

                void chatd::start(const shared_ptr<service_thread>& st) {
                        ASSERT(st);
                        st_ = st;

                        scene_mgr::static_init();

                        register_signals();

                        auto ptt = PTTS_GET_COPY(options, 1);
                        scene_mgr::instance().init(ptt.chat_info().id());
                        scene_mgr::instance().start(ptt.chat_info().scened_ipport().ip(), (unsigned short)(ptt.chat_info().scened_ipport().port()));

                        route::route_mgr::instance().init(ptt.chat_info().id());
                        route::route_mgr::instance().start(pd::RCT_CHATD);
                }

                void chatd::stop() {
                        scene_mgr::instance().stop();
                        route::route_mgr::instance().stop();
                        clock::instance().stop();
                        st_->stop();
                }

                void chatd::register_signals() {
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
                                                CHAT_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                this->stop();
                                        } else {
                                                this->register_signals();
                                        }
                                });
                }

        }
}
