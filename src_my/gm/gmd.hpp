#pragma once

#include "scene_mgr_class.hpp"
#include "route/route_mgr.hpp"
#include "yunying_mgr.hpp"
#include "admin_mgr_class.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include "utils/client_process_mgr.hpp"
#include "login_mgr_class.hpp"
#include "config/options_ptts.hpp"
#include "config/serverlist_ptts.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include <string>
#include <memory>
#include <condition_variable>
#include <mutex>

using namespace std;

namespace nora {
        namespace gm {

                using login_mgr = server_process_mgr<login>;

                class gmd {
                public:
                        gmd(const shared_ptr<service_thread>& st) : st_(st) {
                                if (!st_) {
                                        st_ = make_shared<service_thread>("gmd");
                                        st_->start();
                                }
                        }

                        void start() {
                                register_signals();
                                config::load_config();
                                auto ptt = PTTS_GET_COPY(options, 1);

                                mutex m;
                                condition_variable cv;
                                auto waiting_submodule = 0;
                                auto submodule_init_done_cb = [&waiting_submodule, &m, &cv] {
                                        {
                                                lock_guard<mutex> lk(m);
                                                waiting_submodule -= 1;
                                        }
                                        cv.notify_one();
                                };

                                login_mgr_class::instance().start();
                                route::route_mgr::instance().init_done_cb_ = submodule_init_done_cb;
                                waiting_submodule = 1;
                                route::route_mgr::instance().route_table_updated_cb_ = [this] (const auto& route_table) {
                                        vector<pd::ipport> login_ipports;

                                        for (const auto& i : route_table.clients()) {
                                                switch (i.type()) {
                                                case pd::RCT_LOGIND:
                                                        login_ipports.push_back(i.route_info().gmd_ipport());
                                                        break;
                                                default:
                                                        break;
                                                }
                                        }

                                        login_mgr::instance().set_servers(login_ipports);
                                };

                                route::route_mgr::instance().init(ptt.gm_info().id());
                                route::route_mgr::instance().start(pd::RCT_GMD);
                                {
                                        unique_lock<mutex> lk(m);
                                        cv.wait(lk, [&waiting_submodule] { return waiting_submodule == 0; });
                                }

                                scene_mgr_class::instance().start();
                                yunying_mgr::instance().start(ptt.gmd_yunying_ipport().ip(), (unsigned short)(ptt.gmd_yunying_ipport().port()));
                                admin_mgr_class::instance().start(ptt.gmd_admin_ipport().ip(), (unsigned short)(ptt.gmd_admin_ipport().port()));

                                add_instance_count_timer();
                        }

                        void stop() {
                                scene_mgr_class::instance().stop();
                                route::route_mgr::instance().stop();
                                yunying_mgr::instance().stop();
                                login_mgr_class::instance().stop();
                                admin_mgr_class::instance().stop();
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
                                                        GM_ILOG << "received SIGTERM or SIGINT, stop";
                                                        signals_->cancel();
                                                        this->stop();
                                                } else if (sig == SIGUSR1) {
                                                        GM_ILOG << "received SIGUSR1, reload config";
                                                        PTTS_LOAD(serverlist);
                                                        PTTS_LOAD(options);
                                                        PTTS_MVP(serverlist);
                                                        auto ptt = PTTS_GET_COPY(options, 1);
                                                        change_log_level(ptt.level());

                                                        this->register_signals();
                                                }
                                        });
                        }
                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
