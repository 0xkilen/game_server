#include "scened.hpp"
#include "db_log.hpp"
#include "log.hpp"
#include "dbcache/dbcache.hpp"
#include "player/player_mgr.hpp"
#include "mirror.hpp"
#include "common_resource/common_resource.hpp"
#include "route/route_mgr.hpp"
#include "scene/login/login.hpp"
#include "utils/gid.hpp"
#include "utils/game_def.hpp"
#include "utils/instance_counter.hpp"
#include "utils/process_utils.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include "proto/data_route.pb.h"
#include <condition_variable>
#include <mutex>
#include <signal.h>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                //所有玩家管理器
                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;
                //镜像
                using mirror_mgr = server_process_mgr<mirror>;
                //运营操作相关
                using gm_mgr = server_process_mgr<gm>;
                using chat_mgr = server_process_mgr<chat>;
                using log_server_mgr = server_process_mgr<log_server>;
                using login_mgr = server_process_mgr<login>;
                //删档返利
                using common_resource_mgr = server_process_mgr<common_resource>;

                namespace {

                        void init_log_server_mgr(const pc::options& ptt) {
                                log_server_mgr::static_init();
                                log_server_mgr::instance().init(ptt.scene_info().server_id(), true);
                                vector<pd::ipport> addrs;
                                addrs.push_back(ptt.scened_logserverd_ipport());
                                log_server_mgr::instance().set_servers(addrs);
                        }

                }

                scened& scened::instance() {
                        static scened inst;
                        return inst;
                }

                void scened::start(const shared_ptr<service_thread>& st) {
                        st_ = st;

                        auto ptt = PTTS_GET_COPY(options, 1u);
                        gid::instance().set_server_id(ptt.scene_info().server_id());
                        //SCENE_ILOG << "sdk start";

                        //bi::instance().init();
                        //SCENE_ILOG << "bi start";
                        db_log::instance().start();

                        // wait submodules
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

                        config::load_config();
                        config::check_config();


                        dbcache::static_init();
                        dbcache::instance().init_done_cb_ = submodule_init_done_cb;
                        player_mgr_class::instance().init_done_cb_ = submodule_init_done_cb;

                        player_mgr_class::instance().stopped_cb_ = [this] {
                                db_log::instance().stop();
                                chat_mgr::instance().stop();
                                mirror_mgr::instance().stop();
                                gm_mgr_class::instance().stop();
                                log_server_mgr::instance().stop();
                                route::route_mgr::instance().stop();
                                common_resource_mgr::instance().stop();
                                login_mgr::instance().stop();

                                ADD_TIMER(
                                        st_,
                                        ([] (auto canceled, const auto& timer) {
                                                if (!canceled) {
                                                        dbcache::instance().stop();
                                                        scened::instance().stop();
                                                }
                                        }),
                                        1s);
                        };
                        dbcache::instance().create_role_done_cb_ = [] (auto result, const auto& username) {
                                player_mgr_class::instance().create_role_done(result, username);
                        };
                        dbcache::instance().find_role_done_cb_ = [] (auto result, const auto& username, const auto& role, auto sid, auto gm_id) {
                                player_mgr_class::instance().find_role_done(result, username, role, sid, gm_id);
                        };
                        dbcache::instance().find_role_by_gid_done_cb_ = [] (auto gid, auto result, const auto& username, const auto& role) {
                                player_mgr_class::instance().find_role_by_gid_done(gid, result, username, role);
                        };
                        /*
                        gongdou_mgr::instance().gongdou_slander_vote_done_cb_ = [] (auto role, auto from, auto target, auto type, auto result) {
                                player_mgr_class::instance().gongdou_slander_vote_done(role, from, target, type, result);
                        };
                        */
                        mirror_mgr::static_init();
                        mirror_mgr::instance().init(ptt.scene_info().server_id());
                        

                        chat_mgr::static_init();
                        chat_mgr::instance().init(ptt.scene_info().server_id());

                        common_resource_mgr::static_init();
                        common_resource_mgr::instance().init(ptt.scene_info().server_id());

                        gm_mgr_class::instance().start();
                        init_log_server_mgr(ptt);

                        login_mgr::static_init();
                        login_mgr::instance().init(ptt.scene_info().server_id());

                        waiting_submodule = 1;
                        route::route_mgr::instance().route_table_updated_cb_ = [this] (const auto& route_table) {
                                vector<pd::ipport> mirror_ipports;
                                vector<pd::ipport> chat_ipports;
                                vector<pd::ipport> gm_ipports;
                                vector<pd::ipport> public_arena_ipports;
                                vector<pd::ipport> common_resource_ipports;
                                vector<pd::ipport> login_ipports;

                                for (const auto& i : route_table.clients()) {
                                        switch (i.type()) {
                                        case pd::RCT_MIRRORD:
                                                mirror_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        case pd::RCT_CHATD:
                                                chat_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        case pd::RCT_GMD:
                                                gm_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        case pd::RCT_PUBLIC_ARENAD:
                                                public_arena_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        case pd::RCT_COMMON_RESOURCED:
                                                common_resource_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        case pd::RCT_LOGIND:
                                                login_ipports.push_back(i.route_info().scened_ipport());
                                                break;
                                        default:
                                                break;
                                        }
                                }

                                mirror_mgr::instance().set_servers(mirror_ipports);
                                chat_mgr::instance().set_servers(chat_ipports);
                                gm_mgr::instance().set_servers(gm_ipports);
                                common_resource_mgr::instance().set_servers(common_resource_ipports);
                                login_mgr::instance().set_servers(login_ipports);
                        };

                        route::route_mgr::instance().init(ptt.scene_info().server_id());
                        route::route_mgr::instance().start(pd::RCT_SCENED);
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_submodule] { return waiting_submodule == 0; });
                        }

                        //dbcache::static_init();

                        waiting_submodule = 2;
                        dbcache::instance().connect(ptt.scened_dbcached_ipport().ip(), ptt.scened_dbcached_ipport().port());
                        player_mgr_class::instance().start(ptt.scened_client_ipport().ip(), static_cast<unsigned short>(ptt.scened_client_ipport().port()));
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_submodule] { return waiting_submodule == 0; });
                        }
/*
                        waiting_submodule = 2;
                        arena::instance().start();
                        fief_mgr::instance().start();
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_submodule] { return waiting_submodule == 0; });
                        }

                        waiting_submodule = 1;
                        activity_mgr::instance().start();
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_submodule] { return waiting_submodule == 0; });
                        }
*/
                        player_mgr_class::instance().start_rest();
                        add_online_count_timer();
                        add_instance_count_timer();
                        add_hold_mirror_role_online_timer();
                        add_sync_data_timer_for_gmd_admin();
                        register_signals();
                }

                void scened::add_hold_mirror_role_online_timer() {
                        ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                SCENE_DLOG << " send hold mirror role online msg ";
                                                this->add_hold_mirror_role_online_timer();

                                                ps::base msg;
                                                auto *notice = msg.MutableExtension(ps::sm_server_roles_event::ssre);
                                                auto *event = notice->mutable_event();
                                                auto roles = player_mgr_class::instance().get_online_roles_except_mirror();
                                                auto *ga = event->mutable_gid_array();
                                                for (auto i : roles) {
                                                        ga->add_gids(i);
                                                }
                                                event->set_type(pd::RMET_HOLD_ONLINE);
                                                server_process_mgr<mirror>::instance().random_send_msg(msg);
                                        }
                                }),
                                1min);
                }

                void scened::add_online_count_timer() {
                        ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                //bi::instance().online_count(player_mgr_class::instance().player_count());
                                                this->add_online_count_timer();
                                        }
                                }),
                                5min);
                }

                void scened::add_sync_data_timer_for_gmd_admin() {
                        ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                const auto& ptt = PTTS_GET(options, 1);
                                                gm_mgr_class::instance().admin_update_register_count(ptt.scene_info().server_id(), player_mgr_class::instance().player_register_count());
                                                this->add_sync_data_timer_for_gmd_admin();
                                        }
                                }),
                                3min);
                }

                void scened::add_instance_count_timer() {
                        ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                instance_counter::instance().print();
                                                this->add_instance_count_timer();

                                                auto timer_counts = service_thread::timer_counts();
                                                stringstream ss;
                                                ss << "\n====== timer counts:\n";
                                                for (const auto& i : timer_counts) {
                                                        ss << setw(STATS_WIDTH) << i.first << setw(STATS_WIDTH) << i.second << endl;
                                                }
                                                ILOG << ss.str();
                                        }
                                }),
                                5s);
                }

                void scened::register_signals() {
                        if (!signals_) {
                                signals_ = make_unique<ba::signal_set>(st_->get_service());
                                signals_->add(SIGUSR1);
                                signals_->add(SIGTERM);
                                signals_->add(SIGINT);

                                auto ptt = PTTS_GET_COPY(options, 1u);
                                if (ptt.minicore()) {
                                        signals_->add(SIGBUS);
                                        signals_->add(SIGSEGV);
                                        signals_->add(SIGABRT);
                                        signals_->add(SIGFPE);
                                }
                        }

                        signals_->async_wait(
                                [this] (auto ec, auto sig) {
                                        if (ec) {
                                                return;
                                        }

                                        if (sig == SIGTERM || sig == SIGINT) {
                                                SCENE_ILOG << "received SIGTERM or SIGINT, stop";
                                                signals_->cancel();
                                                player_mgr_class::instance().stop();
                                        } else if (sig == SIGBUS || sig == SIGSEGV || sig == SIGABRT || sig == SIGFPE) {
                                                write_stacktrace_to_file("scened");
                                                exit(0);
                                        } else if (sig == SIGUSR1) {
                                                SCENE_ILOG << "received SIGUSR1, reload config";
                                                //重新加载配置
                                                //PTTS_LOAD(shop);
                                                //PTTS_MVP(shop);
                                                PTTS_LOAD(options);
                                                PTTS_MVP(options);
                                                auto ptt = PTTS_GET_COPY(options, 1);
                                                change_log_level(ptt.level());

                                                this->register_signals();
                                        }
                                });
                }

                void scened::stop() {
                        SCENE_ILOG << "scened stop";
                        clock::instance().stop();
                        st_->stop();
                }

        }
}

