#pragma once

#include "common.hpp"
#include "stream.hpp"
#include "connector.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/assert.hpp"
#include <memory>
#include <chrono>
#include <string>
#include <functional>
#include <set>
#include <map>
#include <boost/core/noncopyable.hpp>

using namespace std;

namespace nora {
        namespace net {

                template <typename T>
                class client : public enable_shared_from_this<client<T>>, private boost::noncopyable {
                public:
                        client(const string& name) :
                                name_(name),
                                st_(make_shared<service_thread>("netclient")),
                                connector_(make_shared<connector<T>>(name + "-connector", name + "-conn-", st_)),
                                stop_(false) {
                                NET_TLOG << *this << " create";
                        }
                        void start() {
                                st_->start();
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                connector_->new_conn_cb_ = [this, self] (const auto& c) {
                                                        NET_TLOG << *this << " got new conn " << c->name();
                                                        conns_[c->name()] = c;
                                                        c->msg_cb_ = [this, self] (const auto& c, const auto& msg) {
                                                                NET_TLOG << *self << " got message from " << c->name();

                                                                if (conn_to_st_.count(c->name()) == 0) {
                                                                        NET_ELOG << *self << " connection not exist, parse msg fail " << c->name();
                                                                        return;
                                                                }
                                                                conn_to_st_.at(c->name())->async_call(
                                                                        [this, self, c, msg] {
                                                                                msg_cb_(c, msg);
                                                                        });
                                                        };

                                                        c->error_cb_ = [this, self] (const auto& c) {
                                                                NET_TLOG << *this << " got error from " << c->name();

                                                                if (conn_to_st_.find(c->name()) == conn_to_st_.end()) {
                                                                        return;
                                                                }
                                                                conn_to_st_.at(c->name())->async_call(
                                                                        [this, self, c] {
                                                                                error_cb_(c);
                                                                        });
                                                                conn_to_st_.erase(c->name());
                                                                conns_.erase(c->name());

                                                                if (stop_.load() && conns_.empty()) {
                                                                        st_->stop();
                                                                        if (stopped_cb_) {
                                                                                stopped_cb_();
                                                                        }
                                                                }
                                                        };
                                                        conn_to_st_[c->name()] = new_conn_cb_(c);
                                                        c->start();
                                                };
                                                connector_->connect_failed_cb_ = [this, self] (const auto& ip, auto port) {
                                                        NET_TLOG << *this << " connect " << ip << ":" << port << " failed";
                                                        ADD_TIMER(
                                                                st_,
                                                                ([this, self = this->shared_from_this(), ip, port] (auto canceled, const auto& timer) {
                                                                        if (!canceled) {
                                                                                if (connect_failed_cb_) {
                                                                                        connect_failed_cb_(ip, port);
                                                                                }
                                                                        }
                                                                }), 1s);
                                                };
                                        });
                                NET_TLOG << *this << " start";
                        }
                        void stop() {
                                ASSERT(!st_->check_in_thread());
                                NET_TLOG << "stop " << name_;
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                if (stop_.load()) {
                                                        return;
                                                }

                                                stop_.store(true);
                                                connector_->stop();
                                                connector_.reset();

                                                if (conns_.empty()) {
                                                        st_->stop();
                                                        if (stopped_cb_) {
                                                                stopped_cb_();
                                                        }
                                                } else {
                                                        for (const auto& i : conns_) {
                                                                i.second->stop();
                                                        }
                                                }
                                        });
                        }
                        void connect(const string& ip, uint16_t port) {
                                st_->async_call(
                                        [this, self = this->shared_from_this(), ip, port] {
                                                connector_->connect(ip, port);
                                        });
                        }
                        void disconnect(const shared_ptr<T>& c) {
                                st_->async_call(
                                        [this, self = this->shared_from_this(), c] {
                                                if (conns_.count(c->name()) == 0) {
                                                        return;
                                                }

                                                c->cancel();
                                                conns_.erase(c->name());
                                                conn_to_st_.erase(c->name());
                                        });
                        }
                        void send(const shared_ptr<T>& c, const shared_ptr<sendstream>& msg) {
                                if (!c) {
                                        return;
                                }
                                st_->async_call(
                                        [this, self = this->shared_from_this(), c, msg] {
                                                if (conns_.find(c->name()) == conns_.end()) {
                                                        return;
                                                }
                                                c->send(msg);
                                        });
                        }
                        const string& name() const {
                                return name_;
                        }
                        friend ostream& operator<<(ostream& os, const client& cl) {
                                return os << cl.name_;
                        }
                        size_t conn_count() const {
                                return conns_.size();
                        }

                        function<shared_ptr<service_thread> (const shared_ptr<T>&)> new_conn_cb_;
                        function<void(const string&, uint16_t)> connect_failed_cb_;
                        function<void(const shared_ptr<T>&, const shared_ptr<recvstream>&)> msg_cb_;
                        function<void(const shared_ptr<T>&)> error_cb_;
                        function<void()> stopped_cb_;
                private:
                        const string name_;
                        shared_ptr<service_thread> st_;
                        shared_ptr<connector<T>> connector_;

                        map<string, shared_ptr<T>> conns_;
                        map<string, shared_ptr<service_thread>> conn_to_st_;
                        atomic<bool> stop_;
                };

        }
}
