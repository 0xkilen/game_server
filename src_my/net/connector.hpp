#pragma once

#include "log.hpp"
#include "utils/exception.hpp"
#include "utils/service_thread.hpp"
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <functional>
#include <thread>
#include <set>
#include <boost/core/noncopyable.hpp>

using namespace std;
using boost::asio::io_service;

namespace nora {
        class service_thread;
        namespace net {

                template <typename T>
                class connector : public enable_shared_from_this<connector<T>>, private boost::noncopyable {
                public:
                        connector(const string& name, const string& conn_name_prefix, const shared_ptr<service_thread>& st) :
                                name_(name), conn_name_prefix_(conn_name_prefix), st_(st), stopped_(false) {
                        }
                        void connect(const string& ip, uint16_t port) {
                                st_->async_call(
                                        [this, self = this->shared_from_this(), ip, port] {
                                                if (stopped_) {
                                                        return;
                                                }

                                                ba::ip::tcp::resolver resolver(st_->get_service());
                                                NET_DLOG << *this << " connect " << ip << ":" << port;

                                                auto ep = ba::ip::tcp::endpoint(ba::ip::address::from_string(ip.c_str()), port);
                                                string ip_str(ip.c_str());
                                                if (ip_str.find_first_not_of("0123456789.") == string::npos) {
                                                        static uint32_t idx = 0;
                                                        auto conn = make_shared<T>(conn_name_prefix_ + to_string(idx), st_->get_service());
                                                        idx += 1;
                                                        conn->socket().lowest_layer().async_connect(ep,
                                                                                                    [this, self, conn, ip, port] (auto ec) {
                                                                                                            if (!ec) {
                                                                                                                    NET_TLOG << *this << " connected " << ip << ":" << port;
                                                                                                                    new_conn_cb_(conn);
                                                                                                            } else if (ec == ba::error::try_again) {
                                                                                                                    this->connect(ip, port);
                                                                                                            } else if (ec != ba::error::operation_aborted) {
                                                                                                                    NET_ELOG << *this << " connect " << ip << ":" << port << " failed: " << ec.message();
                                                                                                                    connect_failed_cb_(ip, port);
                                                                                                            }
                                                                                                    });
                                                } else {
                                                        auto iter = resolver.resolve(ep);
                                                        static uint32_t idx = 0;
                                                        auto conn = make_shared<T>(conn_name_prefix_ + to_string(idx), st_->get_service());
                                                        idx += 1;
                                                        ba::async_connect(conn->socket(),
                                                                          iter,
                                                                          [this, self, conn, ip, port](auto ec, auto i) {
                                                                                  if (!ec) {
                                                                                          NET_TLOG << *this << " connected " << ip << ":" << port;
                                                                                          new_conn_cb_(conn);
                                                                                  } else if (ec == ba::error::try_again) {
                                                                                          this->connect(ip, port);
                                                                                  } else if (ec != ba::error::operation_aborted) {
                                                                                          NET_ELOG << *this << " connect " << ip << ":" << port << " failed: " << ec.message();
                                                                                          connect_failed_cb_(ip, port);
                                                                                  }
                                                                          });
                                                }
                                        });
                        }
                        void stop() {
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                stopped_ = true;
                                                new_conn_cb_ = nullptr;
                                        });
                        }
                        const string& name() const {
                                return name_;
                        }
                        friend ostream& operator<<(ostream& os, const connector& c) {
                                return os << c.name();
                        }
                        function<void(const shared_ptr<T>&)> new_conn_cb_;
                        function<void(const string&, uint16_t)> connect_failed_cb_;
                private:
                        int connecting_count_;
                        const string name_;
                        const string conn_name_prefix_;
                        shared_ptr<service_thread> st_;
                        bool stopped_;
                };

        }
}

