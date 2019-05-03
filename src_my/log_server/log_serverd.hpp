#pragma once

#include "scene.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include "config/options_ptts.hpp"
#include "config/serverlist_ptts.hpp"
#include "config/utils.hpp"
#include "utils/client_process_mgr.hpp"
#include <string>
#include <memory>
#include "log.hpp"

using namespace std;

namespace nora {
        namespace log_serverd {

                using scene_mgr = client_process_mgr<scene>;

                class log_serverd {
                public:
                        log_serverd(const shared_ptr<service_thread>& st) : st_(st) {
                                if (!st_) {
                                        st_ = make_shared<service_thread>("log_serverd");
                                        st_->start();
                                }
                        }

                        void start() {
                                register_signals();
                                config::load_config();
                                auto ptt = PTTS_GET_COPY(options, 1);
                                scene_mgr::instance().init(ptt.logserver_info().id());
                                scene_mgr::instance().start(ptt.scened_logserverd_ipport().ip(), (unsigned short)(ptt.scened_logserverd_ipport().port()));
                                scene_mgr::instance().static_init();
                                add_instance_count_timer();
                        }

                        void stop() {
                                scene_mgr::instance().stop();
                                clock::instance().stop();
                                st_->stop();
                        }
                private:
                        void add_instance_count_timer() {
                                ADD_TIMER(
                                        st_,
                                        ([this] (auto canceled, const auto& timer) {
                                                if (!canceled) {
                                                        instance_counter::instance().print();
                                                        this->add_instance_count_timer();
                                                }
                                        }),
                                        30s);
                        }
                        void register_signals() {
                                if (!signals_) {
                                        signals_ = make_unique<ba::signal_set>(st_->get_service());
                                        signals_->add(SIGTERM);
                                        signals_->add(SIGINT);
                                        signals_->add(SIGUSR1);
                                }

                                signals_->async_wait(
                                        [this] (auto ec, auto sig) {
                                                if (ec) {
                                                        return;
                                                }

                                                if (sig == SIGTERM || sig == SIGINT) {
                                                        LOG_ILOG << "received SIGTERM or SIGINT, stop";
                                                        signals_->cancel();
                                                        this->stop();
                                                } else if (sig == SIGUSR1) {
                                                        LOG_ILOG << "received SIGUSR1, reload config";
                                                        PTTS_LOAD(serverlist);
                                                        PTTS_MVP(serverlist);
                                                }

                                                this->register_signals();
                                        });
                        }
                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
