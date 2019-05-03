#pragma once

#include "client_mgr.hpp"
#include "scene.hpp"
#include "gm_mgr_class.hpp"
#include "route/route_mgr.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include "net/websocket/server_conn.hpp"
#include "config/options_ptts.hpp"
#include "config/channel_specify_ptts.hpp"
#include <string>
#include <memory>

using namespace std;

namespace nora {
        namespace login {

                using scene_mgr = client_process_mgr<scene>;

                class logind : public enable_shared_from_this<logind> {
                public:
                        logind(const shared_ptr<service_thread>& st) :st_(st) {
                                if (!st_) {
                                        st_ = make_shared<service_thread>("login-logind");
                                        st_->start();
                                }
                                cmgr_ = make_shared<client_mgr<net::WS_SERVER_CONN>>("login-clientmgr", st_);
                        }
                        void start() {
                                register_signals();
                                gm_mgr_class::instance().login_gonggao_func_ = [cmgr = cmgr_] (auto sid, auto gmd_id, const auto& gonggao) {
                                        cmgr->login_gonggao(sid, gmd_id, gonggao);
                                };
                                gm_mgr_class::instance().add_to_white_list_func_ = [cmgr = cmgr_] (auto sid, auto server_id, auto gmd_id, const auto& ips) {
                                        cmgr->add_to_white_list(sid, server_id, gmd_id, ips);
                                };
                                gm_mgr_class::instance().remove_from_white_list_func_ = [cmgr = cmgr_] (auto sid, auto gmd_id, auto server_id, const auto& ips) {
                                        cmgr->remove_from_white_list(sid, gmd_id, server_id, ips);
                                };
                                gm_mgr_class::instance().fetch_white_list_func_ = [cmgr = cmgr_] (auto sid, auto gmd_id) {
                                        cmgr->fetch_white_list(sid, gmd_id);
                                };
                                gm_mgr_class::instance().fetch_gonggao_list_func_ = [cmgr = cmgr_] (auto aid, auto gmd_id) {
                                        cmgr->fetch_gonggao_list(aid, gmd_id);
                                };
                                cmgr_->send_gonggao_done_cb_ = [] (auto sid, auto gmd_id) {
                                        gm_mgr_class::instance().send_gonggao_done(sid, gmd_id);
                                };
                                cmgr_->add_to_white_list_done_cb_ = [] (auto sid, auto gmd_id, auto result) {
                                        gm_mgr_class::instance().add_to_white_list_done(sid, gmd_id, result);
                                };
                                cmgr_->remove_from_white_list_done_cb_ = [] (auto sid, auto gmd_id, auto result) {
                                        gm_mgr_class::instance().remove_from_white_list_done(sid, gmd_id, result);
                                };
                                cmgr_->fetch_white_list_done_cb_ = [] (auto sid, auto gmd_id, const auto& white_lists, auto result) {
                                        gm_mgr_class::instance().fetch_white_list_done(sid, gmd_id, white_lists, result);
                                };
                                cmgr_->client_fetch_server_ids_func_ = [scene = scene_] (const auto& username) -> set<uint32_t> {
                                        return scene::fetch_server_ids(username);
                                };

                                auto ptt = PTTS_GET_COPY(options, 1u);

                                start_client(ptt.logind_client_ipport().ip(), (unsigned short)(ptt.logind_client_ipport().port()));

                                gm_mgr_class::instance().start();

                                scene_ = make_shared<scene>("login-scene");
                                scene_mgr::instance().init(ptt.login_info().id());
                                scene_mgr::instance().start(ptt.login_info().scened_ipport().ip(), (unsigned short)(ptt.login_info().scened_ipport().port()));
                                scene_mgr::instance().static_init();

                                route::route_mgr::instance().init(ptt.login_info().id());
                                route::route_mgr::instance().start(pd::RCT_LOGIND);
                                add_instance_count_timer();
                        }

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
                        void start_client(const string& client_ip, unsigned short client_port) {
                                cmgr_->start(client_ip, client_port);
                        }
                        void stop() {
                                if (cmgr_) {
                                        cmgr_->stop();
                                        cmgr_.reset();
                                }
                                gm_mgr_class::instance().stop();
                                route::route_mgr::instance().stop();
                                clock::instance().stop();
                                st_->stop();
                        }
                private:
                        void register_signals() {
                                if (!signals_) {
                                        signals_ = make_unique<ba::signal_set>(st_->get_service());
                                        signals_->add(SIGUSR1);
                                        signals_->add(SIGTERM);
                                        signals_->add(SIGINT);
                                }

                                signals_->async_wait(
                                        [this] (auto ec, auto sig) {
                                                if (ec) {
                                                        return;
                                                }

                                                if (sig == SIGTERM || sig == SIGINT) {
                                                        LOGIN_ILOG << "received SIGTERM or SIGINT, stop";
                                                        signals_->cancel();
                                                        this->stop();
                                                } else if (sig == SIGUSR1) {
                                                        LOGIN_ILOG << "received SIGUSR1, reload config";
                                                        PTTS_LOAD(serverlist);
                                                        PTTS_LOAD(options);
                                                        PTTS_LOAD(channel_specify);
                                                        PTTS_LOAD(specific_server_list);
                                                        PTTS_MVP(serverlist);
                                                        PTTS_MVP(channel_specify);
                                                        PTTS_MVP(specific_server_list);
                                                        auto ptt = PTTS_GET_COPY(options, 1);
                                                        change_log_level(ptt.level());

                                                        this->register_signals();
                                                }
                                        });

                        }

                        shared_ptr<service_thread> st_;
                        shared_ptr<client_mgr<net::WS_SERVER_CONN>> cmgr_;
                        shared_ptr<scene> scene_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
