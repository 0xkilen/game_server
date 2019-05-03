#pragma once

#include "net/conn.hpp"
#include "net/client.hpp"
#include "server_process.hpp"
#include "proto/data_base.pb.h"
#include <boost/core/noncopyable.hpp>
#include <map>
#include <mutex>
#include <memory>
#include <atomic>

using namespace std;
namespace pd = proto::data;

namespace nora {
        class service_thread;

        template <typename T, typename U = net::CONN>
        class server_process_mgr : private boost::noncopyable {
        public:
                server_process_mgr() : name_(T::base_name()) {
                        st_ = make_shared<service_thread>(name_);
                        st_->start();
                }
                static server_process_mgr<T, U>& instance() {
                        static server_process_mgr<T, U> inst;
                        return inst;
                }
                static void static_init() {
                        server_process<T, U>::static_init();
                }
                void init(uint32_t server_id, bool reconnect = false) {
                        server_id_ = server_id;
                        reconnect_ = reconnect;
                }
                void set_servers(const vector<pd::ipport>& ipports) {
                        if (stopped_.load()) {
                                stopped_.store(false);
                        }
                        st_->async_call(
                                [this, ipports] {
                                        set<string> servers;
                                        for (const auto& i : connecting_) {
                                                servers.insert(i);
                                        }
                                        for (const auto& i : server_process_) {
                                                servers.insert(i.first);
                                        }

                                        vector<pd::ipport> to_connect;
                                        for (const auto& i : ipports) {
                                                auto key = ipport_to_key(i);
                                                if (servers.count(key) == 0) {
                                                        to_connect.push_back(i);
                                                }
                                        }

                                        for (const auto& i : to_connect) {
                                                connect(i);
                                        }
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
                                        for (const auto& i : conn2server_process_) {
                                                i.second->stop();
                                        }
                                        for (const auto& i : ncls_) {
                                                i.second->stop();
                                        }
                                        ncls_.clear();
                                        if (conn2server_process_.empty()) {
                                                ASSERT(st_);
                                                st_->stop();
                                                st_.reset();
                                                ILOG << *this << " stop";
                                        }
                                });
                }
                bool has_server_process(uint32_t id) const {
                        lock_guard<mutex> lk(lock_);
                        return id2server_process_.count(id) > 0;
                }
                void send_msg(uint32_t id, const ps::base& msg) {
                        lock_guard<mutex> lk(lock_);
                        TLOG << *this << " send to " << id << " " << msg.DebugString();
                        auto iter = id2server_process_.find(id);
                        if (iter == id2server_process_.end()) {
                                return;
                        }

                        iter->second->send_msg(msg);
                }
                void random_send_msg(const ps::base& msg) {
                        lock_guard<mutex> lk(lock_);
                        TLOG << *this << " send " << msg.DebugString();

                        if (conn2server_process_.empty()) {
                                return;
                        }

                        auto iter = conn2server_process_.begin();
                        advance(iter, rand() % conn2server_process_.size());

                        iter->second->send_msg(msg);
                }
                void broadcast_msg(const ps::base& msg) {
                        lock_guard<mutex> lk(lock_);
                        TLOG << *this << " broadcast " << msg.DebugString();

                        if (conn2server_process_.empty()) {
                                return;
                        }
                        for (const auto& i : conn2server_process_) {
                                i.second->send_msg(msg);
                        }
                }

                friend ostream& operator<<(ostream& os, const server_process_mgr& spm) {
                        return os << spm.name_;
                }
        private:
                static string ipport_to_key(const pd::ipport& ipport) {
                        string ret(ipport.ip() + ":" + to_string(ipport.port()));
                        return ret;
                }
                void connect(const pd::ipport& ipport) {
                        ASSERT(st_->check_in_thread());

                        auto key = ipport_to_key(ipport);
                        if (connecting_.count(key) > 0 || server_process_.count(key) > 0) {
                                return;
                        }

                        DLOG << *this << " connect " + key;

                        auto iter = ncls_.find(key);
                        shared_ptr<net::client<U>> ncl;
                        if (iter != ncls_.end()) {
                                ncl = iter->second;
                        } else {
                                ncl = make_shared<net::client<U>>(name_ + "_netclient_" + key);
                                ncls_[key] = ncl;
                                ncl->new_conn_cb_ = [this, key] (const auto& c) -> shared_ptr<service_thread> {
                                        lock_guard<mutex> lk(lock_);

                                        DLOG << *this << " connected to " + key;
                                        auto sp = make_shared<server_process<T, U>>(st_, ncls_[key], c);
                                        conn2server_process_[c] = sp;
                                        server_process_[key] = sp;
                                        connecting_.erase(key);

                                        sp->register_done_cb_ = [this, c] {
                                                lock_guard<mutex> lk(lock_);
                                                ASSERT(conn2server_process_.count(c) > 0);
                                                const auto& sp = conn2server_process_.at(c);
                                                if (id2server_process_.count(sp->id()) > 0) {
                                                        ELOG << *this << " got server process with duplicated id " << *sp;
                                                }
                                                DLOG << *this << " got client process register " << *sp;
                                                id2server_process_[sp->id()] = sp;
                                        };
                                        sp->stop_cb_ = [this, key] (const auto& c, const auto& sp){
                                                lock_guard<mutex> lk(lock_);
                                                DLOG << *this << " got server process stop " << *sp;
                                                auto iter = conn2server_process_.find(c);
                                                if (iter == conn2server_process_.end()) {
                                                        return;
                                                }
                                                if (iter->second == sp) {
                                                        conn2server_process_.erase(iter);
                                                        id2server_process_.erase(iter->second->id());
                                                        server_process_.erase(key);
                                                }
                                                if (stopped_.load() && conn2server_process_.empty()) {
                                                        ASSERT(st_);
                                                        st_->stop();
                                                        st_.reset();
                                                        ILOG << *this << " stop";
                                                }
                                        };
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::s2s_register_req::srr);
                                        req->set_id(server_id_);
                                        sp->send_msg(msg);
                                        return st_;
                                };
                                ncl->connect_failed_cb_ = [this, key] (const auto& ip, auto port) {
                                        lock_guard<mutex> lk(lock_);
                                        connecting_.erase(key);

                                        if (reconnect_) {
                                                DLOG << *this << " try reconnect " + key;
                                                ncls_[key]->connect(ip, port);
                                        }
                                };
                                ncl->msg_cb_ = [this] (const auto& c, const auto& msg) {
                                        lock_guard<mutex> lk(lock_);
                                        auto iter = conn2server_process_.find(c);
                                        if (iter == conn2server_process_.end()) {
                                                return;
                                        }
                                        DLOG << *this << " got msg from " << *iter->second;
                                        iter->second->process_msg(msg);
                                };
                                ncl->error_cb_ = [this, ipport, key] (const auto& c) {
                                        lock_guard<mutex> lk(lock_);
                                        if (stopped_.load()) {
                                                return;
                                        }

                                        DLOG << *this << key << " got error";

                                        auto iter = conn2server_process_.find(c);
                                        if (iter != conn2server_process_.end()) {
                                                iter->second->stop();
                                        }

                                        if (reconnect_) {
                                                DLOG << *this << " try reconnect " + key;
                                                ncls_[key]->connect(ipport.ip(), ipport.port());
                                        }
                                };
                                ncl->start();
                        }
                        connecting_.insert(key);
                        ncl->connect(ipport.ip(), ipport.port());
                }

                mutable mutex lock_;
                const string name_;
                uint32_t server_id_ = 0;
                bool reconnect_;
                atomic<bool> stopped_{true};
                shared_ptr<service_thread> st_;
                map<string, shared_ptr<net::client<U>>> ncls_;
                map<string, shared_ptr<server_process<T, U>>> server_process_;
                map<shared_ptr<U>, shared_ptr<server_process<T, U>>> conn2server_process_;
                map<uint32_t, shared_ptr<server_process<T, U>>> id2server_process_;
                set<string> connecting_;
        };

}
