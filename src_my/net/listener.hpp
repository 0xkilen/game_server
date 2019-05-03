#pragma once

#include "log.hpp"
#include "utils/service_thread.hpp"
#include <memory>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace net {

                template <typename T>
                class listener : public enable_shared_from_this<listener<T>>, private boost::noncopyable {
                public:
                        listener(const string& conn_name_prefix,
                                 const string& ip, unsigned short port,
                                 const shared_ptr<service_thread>& st) :
                                name_("netlistener-" + ip),
                                conn_name_prefix_(conn_name_prefix),
                                st_(st),
                                acceptor_(st->get_service()) {
                                ba::ip::tcp::resolver resolver(st->get_service());
                                ba::ip::tcp::endpoint ep = *resolver.resolve({ ip, to_string(static_cast<uint32_t>(port)) });
                                acceptor_.open(ep.protocol());
                                ba::socket_base::reuse_address option(true);
                                acceptor_.set_option(option);
                                acceptor_.bind(ep);
                        }
                        void start() {
                                acceptor_.listen();
                                listen();
                        }
                        void listen() {
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                static uint32_t idx = 0;
                                                auto conn = make_shared<T>(conn_name_prefix_ + to_string(idx), st_->get_service());
                                                idx += 1;
                                                acceptor_.async_accept(
                                                        conn->socket(),
                                                        [this, self, conn] (const auto& ec) {
                                                                if (!ec) {
                                                                        NET_TLOG << *this << " accepted one";
                                                                        ba::ip::tcp::no_delay option(true);
                                                                        conn->socket().lowest_layer().set_option(option);
                                                                        new_conn_cb_(conn);
                                                                } else if (ec != ba::error::operation_aborted) {
                                                                        return;
                                                                } else {
                                                                        NET_ELOG << *this << " accept failed: " << ec.message();
                                                                }
                                                                this->listen();
                                                        });
                                        });
                        }
                        void stop() {
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                new_conn_cb_ = nullptr;
                                                acceptor_.close();
                                        });
                        }
                        friend ostream& operator<<(ostream& os, const listener<T>& l) {
                                return os << l.name_;
                        }
                        function<void(const shared_ptr<T>&)> new_conn_cb_;
                protected:
                        const string name_;
                        const string conn_name_prefix_;
                        shared_ptr<service_thread> st_;
                        ba::ip::tcp::acceptor acceptor_;
                };

        }
}
