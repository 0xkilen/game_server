#include "log.hpp"
#include "admin_mgr_class.hpp"
#include "admin.hpp"
#include "route/route_mgr.hpp"
#include "utils/client_process_mgr.hpp"
#include "config/options_ptts.hpp"
#include "proto/config_options.pb.h"
#include "proto/ss_gm.pb.h"
#include <condition_variable>
#include <mutex>

namespace nora {
        namespace gm {

                using admin_service = client_process_mgr<admin, net::WS_SERVER_CONN>;

                admin_mgr_class::admin_mgr_class() {
                        name_ = "gmd-adminmgr";
                        st_ = make_shared<service_thread>("adminmgr");
                        st_->start();

                        const auto& ptt = PTTS_GET(options, 1);
                        gmdb_ = make_shared<db::connector>(ptt.gm_info().db().ipport(),
                                                           ptt.gm_info().db().user(),
                                                           ptt.gm_info().db().password(),
                                                           ptt.gm_info().db().database());
                        gmdb_->start();
                }

                void admin_mgr_class::start(const string& ip, unsigned short port) {
                        mutex m;
                        condition_variable cv;
                        auto waiting_count = 1;
                        auto db_msg = make_shared<db::message>("load_gmd_administrators",
                                                               db::message::req_type::rt_select,
                                                               [this, &waiting_count, &cv, &m] (const shared_ptr<db::message>& msg) {
                                                                       lock_guard<mutex> lock(lock_);
                                                                       auto& results = msg->results();
                                                                       for (const auto& i : results) {
                                                                               ASSERT(i.size() == 2);
                                                                               auto username = boost::any_cast<string>(i[0]);
                                                                               auto password = boost::any_cast<string>(i[1]);
                                                                               admin2passwords_[username] = password;
                                                                               ADMIN_TLOG << "add username: " << username << " password: " << password;
                                                                       }

                                                                       {
                                                                               lock_guard<mutex> lk(m);
                                                                               waiting_count -= 1;
                                                                       }
                                                                       cv.notify_one();
                                                               });
                        gmdb_->push_message(db_msg, st_);
                        {
                                unique_lock<mutex> lk(m);
                                cv.wait(lk, [&waiting_count] { return waiting_count == 0; });
                        }

                        const auto& ptt = PTTS_GET(options, 1);
                        admin_service::instance().init(ptt.gm_info().id());
                        admin_service::instance().static_init();
                        admin_service::instance().start(ip, port);
                }

                void admin_mgr_class::player_online(uint32_t server_id, uint64_t gid) {
                        lock_guard<mutex> lock(lock_);
                        server2onlines_[server_id].insert(gid);
                }

                void admin_mgr_class::player_offline(uint32_t server_id, uint64_t gid) {
                        lock_guard<mutex> lock(lock_);
                        if (server2onlines_.count(server_id) > 0) {
                                if (server2onlines_.at(server_id).count(gid) > 0) {
                                        server2onlines_[server_id].erase(gid);
                                }
                        }
                }

                void admin_mgr_class::player_register_count_update(uint32_t server_id, uint32_t register_count) {
                        lock_guard<mutex> lock(lock_);
                        server2register_count_[server_id] = register_count;
                }

                void admin_mgr_class::login_done(uint32_t conn_id, const string& username, const string& password) {
                        st_->async_call(
                                [this, conn_id, username, password] {
                                        ASSERT(st_->check_in_thread());
                                        auto result = pd::OK;
                                        if (admin2passwords_.count(username) < 0 || admin2passwords_.at(username) != password) {
                                                result = pd::NOT_FOUND;
                                        }

                                        if (result != pd::OK) {
                                                if (aids_.count(conn_id) > 0) {
                                                        aids_.erase(conn_id);
                                                }
                                                admin_service::instance().disconnect(conn_id);
                                        } else {
                                                ASSERT(aids_.count(conn_id) == 0);
                                                aids_.insert(conn_id);
                                                proto::ss::base sync_msg;
                                                auto *sync = sync_msg.MutableExtension(ps::g2c_sync_serverlist::gss);
                                                auto route_table = route::route_mgr::instance().route_table();
                                                for (const auto& i : route_table.clients()) {
                                                        auto *server = sync->add_servers();
                                                        server->set_server_type(i.type());
                                                        server->set_server_id(i.id());
                                                        if (i.type() == pd::RCT_SCENED) {
                                                                server->set_online_count(server2onlines_[i.id()].size());
                                                                server->set_register_count(server2register_count_[i.id()]);
                                                        }
                                                }
                                                admin_service::instance().send_msg(conn_id, sync_msg);
                                        }
                                });
                }

                void admin_mgr_class::get_role_info_done(uint32_t conn_id, uint32_t server_id, const pd::role& role_data, pd::result result) {
                        st_->async_call(
                                [this, conn_id, server_id, role_data, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_get_role_info_by_gid_rsp::ggribgr);
                                        sync->set_result(result);
                                        sync->set_server_id(server_id);
                                        if (result == pd::OK) {
                                                *sync->add_role_info() = role_data;
                                        }
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::operates_activity_done(uint32_t conn_id, pd::result result) {
                        st_->async_call(
                                [this, conn_id, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_start_activity_rsp::gasar);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });

                }

                void admin_mgr_class::admin_edit_role_by_gid_done(uint32_t conn_id, pd::result result, pd::admin_edit_role_type edit_type) {
                        st_->async_call(
                                [this, conn_id, result, edit_type] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_edit_role_by_gid_rsp::gaerbgr);
                                        sync->set_edit_type(edit_type);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });

                }

                void admin_mgr_class::admin_edit_role_rollback_by_gid_done(uint32_t conn_id, pd::result result, pd::admin_edit_role_type edit_type) {
                        st_->async_call(
                                [this, conn_id, result, edit_type] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_edit_role_rollback_by_gid_rsp::gaerrbgr);
                                        sync->set_edit_type(edit_type);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });

                }

                void admin_mgr_class::admin_send_mail_done(uint32_t conn_id, pd::result result) {
                        st_->async_call(
                                [this, conn_id, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_send_mail_rsp::gasmr);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::admin_fetch_gonggao_list_done(uint32_t conn_id, const pd::gonggao_array& gonggao_list) {
                        st_->async_call(
                                [this, conn_id, gonggao_list] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_fetch_gonggao_list_rsp::gafglr);
                                        *sync->mutable_gonggao_list() = gonggao_list;
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::admin_edit_gonggao_done(uint32_t conn_id, pd::result result) {
                        st_->async_call(
                                [this, conn_id, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_edit_gonggao_rsp::gaegr);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::admin_fetch_white_list_done(uint32_t conn_id, pd::result result, const pd::white_list_array& white_list) {
                        st_->async_call(
                                [this, conn_id, result, white_list] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_fetch_white_list_rsp::gafwlr);
                                        sync->set_result(result);
                                        *sync->mutable_white_list() = white_list;
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::admin_edit_white_list_done(uint32_t conn_id, bool publish, pd::result result) {
                        st_->async_call(
                                [this, conn_id, result, publish] {
                                        ASSERT(st_->check_in_thread());
                                        if (aids_.count(conn_id) <= 0) {
                                                return;
                                        }

                                        proto::ss::base sync_msg;
                                        auto *sync = sync_msg.MutableExtension(ps::g2c_admin_edit_white_list_rsp::gaewlr);
                                        sync->set_publish(publish);
                                        sync->set_result(result);
                                        admin_service::instance().send_msg(conn_id, sync_msg);
                                });
                }

                void admin_mgr_class::stop() {
                        if (st_) {
                                st_->stop();
                        }
                        ADMIN_DLOG << *this << " stop";
                }

        }
}
