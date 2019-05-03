#include "dbcache.hpp"
#include "log.hpp"
#include "scene/scened.hpp"
#include "scene/activity/activity_mgr.hpp"
#include "net/stream.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"

namespace nora {
        namespace scene {

                dbcache::dbcache() {
                        st_ = make_shared<service_thread>("dbcache");
                        st_->start();
                }

                dbcache::dbcache(const shared_ptr<service_thread>& st) : st_(st) {
                        ASSERT(st_);
                }

                void dbcache::static_init() {
                        //register some proto handler, handler from dbserver
                        dbcache::register_proto_handler(ps::d2s_create_role_rsp::descriptor(), &dbcache::process_create_role_rsp);
                }

                void dbcache::connect(const string& dbcached_ip, unsigned short dbcached_port) {
                        st_->async_call(
                                [this, dbcached_ip, dbcached_port] {
                                        ncl_ = make_shared<net::client<net::CONN>>("dbcache_netclient");
                                        ncl_->new_conn_cb_ = [this] (const auto& c) -> shared_ptr<service_thread> {
                                                SDBCACHE_DLOG << *this << " connected to dbcached";
                                                conn_ = c;

                                                if (init_done_cb_) {
                                                        init_done_cb_();
                                                        init_done_cb_ = nullptr;
                                                        SCENE_ILOG << *this << " start";
                                                }

                                                return st_;
                                        };
                                        ncl_->msg_cb_ = [this] (const auto& c, const auto& msg) {
                                                SDBCACHE_DLOG << *this << " got msg from dbcached";
                                                this->process_msg(msg);
                                        };
                                        ncl_->error_cb_ = [this, dbcached_ip, dbcached_port] (const auto& c) {
                                                ncl_->disconnect(conn_);
                                                conn_.reset();
                                                this->remove_msg_queue_timer();
                                                if (!stopped_) {
                                                        SDBCACHE_DLOG << *this << " try reconnect dbcached";
                                                        ncl_->connect(dbcached_ip, dbcached_port);
                                                }
                                        };
                                        ncl_->connect_failed_cb_ = [this] (const auto& ip, auto port) {
                                                SDBCACHE_DLOG << *this << " try reconnect " + ip + ":" << port;
                                                ncl_->connect(ip, port);
                                        };

                                        SDBCACHE_DLOG << *this << " connect dbcached";
                                        ncl_->start();
                                        ncl_->connect(dbcached_ip, dbcached_port);
                                        add_msg_queue_timer();
                                });
                }

                void dbcache::add_msg_queue_timer() {
                        msg_queue_timer_ = ADD_TIMER(
                                st_,
                                ([this] (auto canceled, const auto& timer) {
                                        if (msg_queue_timer_ == timer) {
                                                msg_queue_timer_.reset();
                                        }
                                        if (!canceled) {
                                                if (!msg_queue_.empty() && conn_) {
                                                        ncl_->send(conn_, msg_queue_.front());
                                                        msg_queue_.pop_front();
                                                }
                                                this->add_msg_queue_timer();
                                        }
                                }),
                                100ms);
                }

                void dbcache::remove_msg_queue_timer() {
                        if (msg_queue_timer_) {
                                msg_queue_timer_->cancel();
                                msg_queue_timer_.reset();
                        }
                }

                void dbcache::stop() {
                        ASSERT(st_);
                        st_->async_call(
                                [this] {
                                        stopped_ = true;
                                        remove_msg_queue_timer();
                                        if (conn_) {
                                                ncl_->disconnect(conn_);
                                                conn_.reset();
                                        }
                                        if (ncl_) {
                                                ncl_->stop();
                                                ncl_.reset();
                                        }
                                        if (st_) {
                                                st_->stop();
                                                st_.reset();
                                        }
                                });
                }

                void dbcache::process_msg(const shared_ptr<net::recvstream>& msg) {
                        ASSERT(st_->check_in_thread());
                        ps::base ss_msg;
                        ASSERT(ss_msg.ParseFromIstream(msg.get()));
                        SDBCACHE_TLOG << *this << " process msg\n" << ss_msg.DebugString();
                        process(this, &ss_msg);
                }
                
                void dbcache::process_create_role_rsp(const ps::base *msg) {
                        ASSERT(st_->check_in_thread());
                        const auto& rsp = msg->GetExtension(ps::d2s_create_role_rsp::dcrr);
                        ASSERT(create_role_done_cb_);
                        create_role_done_cb_(rsp.username(), rsp.result());
                }
                
/*

                shared_ptr<net::sendstream> dbcache::gen_add_gladiators_req(const pd::gladiator_array& data) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_add_gladiators_req::sagr);
                        *req->mutable_gladiators() = data;
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                shared_ptr<net::sendstream> dbcache::gen_get_gladiator_req(uint64_t gid) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_get_gladiator_req::sggr);
                        req->set_gid(gid);
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                shared_ptr<net::sendstream> dbcache::gen_update_gladiator_req(const pd::gladiator& data) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_update_gladiator_req::sugr);
                        *req->mutable_gladiator() = data;
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }
*/

                shared_ptr<net::sendstream> dbcache::gen_create_role_req(const string& username, const pd::role& role) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_create_role_req::scrr);
                        req->set_username(username);
                        *req->mutable_role() = role;
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                shared_ptr<net::sendstream> dbcache::gen_find_role_req(const string& username, uint32_t sid, uint32_t gm_id) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_find_role_req::sfrr);
                        req->set_username(username);
                        if (sid != 0) {
                                req->set_sid(sid);
                                req->set_gm_id(gm_id);
                        }
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                shared_ptr<net::sendstream> dbcache::gen_find_role_by_gid_req(uint64_t gid) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_find_role_by_gid_req::sfrbgr);
                        req->set_gid(gid);
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                shared_ptr<net::sendstream> dbcache::gen_update_role_req(const pd::role& data) {
                        ps::base msg;
                        auto *req = msg.MutableExtension(ps::s2d_update_role_req::surr);
                        *req->mutable_role() = data;
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        return ss;
                }

                void dbcache::push_msg(const shared_ptr<net::sendstream>& msg) {
                        ASSERT(st_->check_in_thread());
                        msg_queue_.push_back(msg);
                }

                void dbcache::send_to_dbcached(const shared_ptr<net::sendstream>& msg, bool queue) {
                        st_->async_call(
                                [this, msg, queue] {
                                        if (!queue) {
                                                ncl_->send(conn_, msg);
                                        } else {
                                                push_msg(msg);
                                        }
                                });
                }

                ostream& operator<<(ostream& os, const dbcache& dc) {
                        return os << "dbcache";
                }

        }
}
