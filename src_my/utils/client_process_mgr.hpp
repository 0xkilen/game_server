#pragma once

#include "net/conn.hpp"
#include "net/server.hpp"
#include "client_process.hpp"
#include <boost/core/noncopyable.hpp>
#include <map>
#include <memory>
#include <atomic>

using namespace std;

namespace nora {
        class service_thread;

        template <typename T, typename U = net::CONN>
        class client_process_mgr : private boost::noncopyable {
        public:
                client_process_mgr() : name_(T::base_name()) {
                        st_ = make_shared<service_thread>(name_);
                        st_->start();
                        nsv_ = make_shared<net::server<U>>(name_ + "-netserver");
                }
                static client_process_mgr<T, U>& instance() {
                        static client_process_mgr<T, U> inst;
                        return inst;
                }
                static void static_init(const shared_ptr<service_thread>& st = nullptr) {
                        client_process<T, U>::static_init(st);
                }
                void init(uint32_t server_id) {
                        server_id_ = server_id;
                }
                void start(const string& ip, unsigned short port) {
                        nsv_->new_conn_cb_ = [this] (const auto& c) -> shared_ptr<service_thread> {
                                lock_guard<mutex> lk(lock_);
                                DLOG << *this << " got new conn " << *c;

                                auto cp = make_shared<client_process<T, U>>(st_, nsv_, c, server_id_);
                                conn2client_process_[c] = cp;

                                cp->register_cb_ = [this] (const auto& c) {
                                        lock_guard<mutex> lk(lock_);
                                        ASSERT(conn2client_process_.count(c) > 0);
                                        const auto& cp = conn2client_process_.at(c);
                                        auto iter = id2client_process_.find(cp->id());
                                        if (iter != id2client_process_.end()) {
                                                ELOG << *this << " got client process with duplicated id " << *cp;
                                        }
                                        DLOG << *this << " got client process register " << *cp;
                                        id2client_process_[cp->id()] = cp;
                                };
                                cp->stop_cb_ = [this] (const auto& c, const auto& cp){
                                        lock_guard<mutex> lk(lock_);
                                        auto iter = conn2client_process_.find(c);
                                        if (iter != conn2client_process_.end() && iter->second == cp) {
                                                id2client_process_.erase(iter->second->id());
                                                conn2client_process_.erase(c);
                                        }
                                        if (stopped_.load() && conn2client_process_.empty()) {
                                                ASSERT(st_);
                                                st_->stop();
                                                st_.reset();
                                                ILOG << *this << " stop";
                                        }
                                };
                                return st_;
                        };
                        nsv_->msg_cb_ = [this] (const auto& c, const auto& msg) {
                                lock_guard<mutex> lk(lock_);
                                auto iter = conn2client_process_.find(c);
                                if (iter == conn2client_process_.end()) {
                                        return;
                                }
                                DLOG << *this << " got msg from " << *iter->second;
                                iter->second->process_msg(msg);
                        };
                        nsv_->error_cb_ = [this] (const auto& c) {
                                lock_guard<mutex> lk(lock_);
                                DLOG << *this << " got error from " << c->name();
                                nsv_->disconnect(c);
                                auto iter = conn2client_process_.find(c);
                                if (iter != conn2client_process_.end()) {
                                        iter->second->stop();
                                }
                        };

                        stopped_.store(false);
                        nsv_->start();
                        nsv_->listen(ip, port);
                        ILOG << *this << " start";
                }
                void disconnect(uint32_t id) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, id] {
                                        DLOG << *this << " disconnect " + id;

                                        auto iter = id2client_process_.find(id);
                                        if (iter == id2client_process_.end()) {
                                                return;
                                        }

                                        iter->second->stop();
                                });
                }
                void stop() {
                        if (stopped_.load()) {
                                return;
                        }
                        stopped_.store(true);

                        ASSERT(st_);
                        st_->async_call(
                                [this] {
                                        for (const auto& i : conn2client_process_) {
                                                i.second->stop();
                                        }
                                        if (nsv_) {
                                                nsv_->stop();
                                                nsv_.reset();
                                        }
                                        if (conn2client_process_.empty()) {
                                                ASSERT(st_);
                                                st_->stop();
                                                st_.reset();
                                        }
                                        ILOG << *this << " stop";
                                });
                }
                bool has_client_process(uint32_t id) const {
                        lock_guard<mutex> lk(lock_);
                        return id2client_process_.count(id) > 0;
                }
                uint32_t client_process_count() const {
                        lock_guard<mutex> lk(lock_);
                        return id2client_process_.size();
                }
                void send_msg(uint32_t id, const ps::base& msg) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, id, msg] {
                                        lock_guard<mutex> lk(lock_);
                                        auto iter = id2client_process_.find(id);
                                        if (iter == id2client_process_.end()) {
                                                return;
                                        }
                                        TLOG << *this << " send " << msg.DebugString();
                                        iter->second->send_msg(msg);
                                });
                }
                void broadcast_msg(const ps::base& msg) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, msg] {
                                        lock_guard<mutex> lk(lock_);
                                        TLOG << *this << " broadcast " << msg.DebugString();
                                        for (const auto& i : id2client_process_) {
                                                i.second->send_msg(msg);
                                        }
                                });
                }
                friend ostream& operator<<(ostream& os, const client_process_mgr& cpm) {
                        return os << cpm.name_;
                }
        private:
                mutable mutex lock_;
                const string name_;
                atomic<bool> stopped_{true};
                uint32_t server_id_ = 0;
                shared_ptr<service_thread> st_;
                shared_ptr<net::server<U>> nsv_;
                map<uint32_t, shared_ptr<client_process<T, U>>> id2client_process_;
                map<shared_ptr<U>, shared_ptr<client_process<T, U>>> conn2client_process_;
        };

}
