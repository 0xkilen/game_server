#pragma once

#include "server.hpp"
#include "listener_ssl.hpp"

using namespace std;

namespace nora {
namespace net {

        template <>
        class server<WSS_SERVER_CONN> : public enable_shared_from_this<server<WSS_SERVER_CONN>>,
                                        private boost::noncopyable {
        public:
                server(const string& name) :
                        name_(name),
                        st_(make_shared<service_thread>("netserverssl")),
                        stop_(false) {
                        NET_TLOG << *this << " create";
                }
                void start() {
                        st_->start();
                        NET_TLOG << name_ << " start";
                }
                void stop() {
                        st_->async_call(
                                [this, self = this->shared_from_this()] {
                                        if (stop_.load()) {
                                                return;
                                        }

                                        NET_TLOG << *this << " stop";

                                        stop_.store(true);
                                        for (const auto& i : listeners_) {
                                                i->stop();
                                        }
                                        listeners_.clear();
                                });
                }
                void listen(const string& ip, unsigned short port) {
                        st_->async_call(
                                [this, self = this->shared_from_this(), ip, port] () {
                                        listeners_.push_back(make_unique<listener<WSS_SERVER_CONN>>(name_ + "-conn-", ip, port, st_));
                                        listeners_.back()->new_conn_cb_ = [this, self] (const auto& c) {
                                                NET_TLOG << *this << " got new conn " << *c;

                                                conns_.insert(c);
                                                c->msg_cb_ = [this, self] (const auto& c, const auto& msg) {
                                                        NET_TLOG << *self << " got msg from " << c->name();

                                                        if (conn_to_st_.count(c) == 0) {
                                                                return;
                                                        }
                                                        conn_to_st_.at(c)->async_call(
                                                                [this, self, c, msg] {
                                                                        if (msg_cb_) {
                                                                                msg_cb_(c, msg);
                                                                        }
                                                                });
                                                };
                                                c->error_cb_ = [this, self] (const auto& c) {
                                                        NET_TLOG << *self << " got error from " << c->name();

                                                        if (conn_to_st_.count(c) == 0) {
                                                                return;
                                                        }
                                                        conn_to_st_.at(c)->async_call(
                                                                [this, self, c] {
                                                                        if (error_cb_) {
                                                                                error_cb_(c);
                                                                        }
                                                                });
                                                        conns_.erase(c);
                                                        conn_to_st_.erase(c);

                                                        if (stop_.load() && conns_.empty()) {
                                                                new_conn_cb_ = nullptr;
                                                                msg_cb_ = nullptr;
                                                                error_cb_ = nullptr;
                                                                st_->stop();

                                                                if (stopped_cb_) {
                                                                        stopped_cb_();
                                                                }
                                                        }
                                                };
                                                if (new_conn_cb_) {
                                                        auto st = new_conn_cb_(c);
                                                        if (!st) {
                                                                this->disconnect(c);
                                                        } else {
                                                                conn_to_st_[c] = st;
                                                                c->start();
                                                        }
                                                }
                                        };
                                        listeners_.back()->start();
                                        NET_ILOG << *self << " listen " << ip << ":" << port;
                                });
                }
                void broadcast(const shared_ptr<sendstream>& msg) {
                        st_->async_call(
                                [this, self = this->shared_from_this(), msg] {
                                        for (const auto& i : conns_) {
                                                i->send(msg);
                                        }
                                });
                }
                void disconnect(const shared_ptr<WSS_SERVER_CONN>& c) {
                        ADD_TIMER(
                                st_,
                                ([this, self = this->shared_from_this(), c] (auto canceled, const auto& timer) {
                                        if (!canceled) {
                                                if (conns_.count(c) == 0) {
                                                        return;
                                                }
                                                NET_TLOG << "disconnect " << *c;
                                                c->cancel();
                                                conns_.erase(c);
                                                conn_to_st_.erase(c);
                                        }
                                }),
                                1s);
                }
                void send(const shared_ptr<WSS_SERVER_CONN>& c, const shared_ptr<sendstream>& msg) {
                        st_->async_call(
                                [this, self = this->shared_from_this(), c, msg] {
                                        if (conns_.count(c) == 0) {
                                                return;
                                        }
                                        c->send(msg);
                                });
                }
                const string& name() const {
                        return name_;
                }
                friend ostream& operator<<(ostream& os, const server& sv) {
                        return os << sv.name();
                }
                size_t conn_count() const {
                        return conns_.size();
                }

                function<shared_ptr<service_thread> (const shared_ptr<WSS_SERVER_CONN>&)> new_conn_cb_;
                function<void(const shared_ptr<WSS_SERVER_CONN>&, const shared_ptr<recvstream>&)> msg_cb_;
                function<void(const shared_ptr<WSS_SERVER_CONN>&)> error_cb_;
                function<void()> stopped_cb_;
        private:
                const string name_;
                vector<shared_ptr<listener<WSS_SERVER_CONN>>> listeners_;

                set<shared_ptr<WSS_SERVER_CONN>> conns_;
                map<shared_ptr<WSS_SERVER_CONN>, shared_ptr<service_thread>> conn_to_st_;

                shared_ptr<service_thread> st_;
                atomic<bool> stop_;
        };

}
}
