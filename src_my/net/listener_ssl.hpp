#pragma once

#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/assert.hpp"
#include "config/options_ptts.hpp"
#include <memory>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace net {

                template <>
                class listener<WSS_SERVER_CONN> : public enable_shared_from_this<listener<WSS_SERVER_CONN>>, private boost::noncopyable {
                public:
                        listener(const string& conn_name_prefix,
                                 const string& ip, unsigned short port,
                                 const shared_ptr<service_thread>& st) :
                                name_("netlistenerssl-" + ip),
                                conn_name_prefix_(conn_name_prefix),
                                st_(st),
                                acceptor_(st->get_service()),
                                context_(ba::ssl::context::tlsv12) {

                                ba::ip::tcp::resolver resolver(st->get_service());
                                ba::ip::tcp::endpoint ep = *resolver.resolve({ ip, to_string(static_cast<uint32_t>(port)) });
                                acceptor_.open(ep.protocol());
                                ba::socket_base::reuse_address option(true);
                                acceptor_.set_option(option);
                                acceptor_.bind(ep);

                                auto ptt = PTTS_GET_COPY(options, 1);
                                ASSERT(ptt.has_ssl_files());
                                context_.use_certificate_chain_file(ptt.ssl_files().cert());
                                context_.use_private_key_file(ptt.ssl_files().private_key(), ba::ssl::context::pem);
                                if (ptt.ssl_files().has_verify()) {
                                        context_.load_verify_file(ptt.ssl_files().verify());
                                        context_.set_verify_mode(ba::ssl::verify_peer | ba::ssl::verify_fail_if_no_peer_cert | ba::ssl::verify_client_once);
                                        session_id_context_ = to_string(static_cast<int>(port)) + ":";
                                        session_id_context_.append(ip.rbegin(), ip.rend());
                                        SSL_CTX_set_session_id_context(context_.native_handle(), reinterpret_cast<const unsigned char *>(session_id_context_.data()),
                                                                       min<size_t>(session_id_context_.size(), SSL_MAX_SSL_SESSION_ID_LENGTH));
                                }
                        }
                        void start() {
                                acceptor_.listen();
                                listen();
                        }
                        void listen() {
                                st_->async_call(
                                        [this, self = this->shared_from_this()] {
                                                auto conn = make_shared<WSS_SERVER_CONN>(conn_name_prefix_, st_->get_service(), context_);
                                                acceptor_.async_accept(
                                                        conn->socket().lowest_layer(),
                                                        [this, self, conn] (const auto& ec) {
                                                                if (!ec) {
                                                                        NET_TLOG << *this << " accepted one";
                                                                        ba::ip::tcp::no_delay option(true);
                                                                        conn->socket().lowest_layer().set_option(option);
                                                                        conn->socket().async_handshake(
                                                                                ba::ssl::stream_base::server,
                                                                                [this, self, conn] (const auto& ec) {
                                                                                        if (!ec) {
                                                                                                new_conn_cb_(conn);
                                                                                        }
                                                                                });
                                                                } else if (ec != ba::error::operation_aborted) {
                                                                        NET_ELOG << *this << " accept failed: " << ec.message();
                                                                        return;
                                                                } else {
                                                                        return;
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
                        friend ostream& operator<<(ostream& os, const listener<WSS_SERVER_CONN>& l) {
                                return os << l.name_;
                        }
                        function<void(const shared_ptr<WSS_SERVER_CONN>&)> new_conn_cb_;
                private:
                        const string name_;
                        const string conn_name_prefix_;
                        shared_ptr<service_thread> st_;
                        ba::ip::tcp::acceptor acceptor_;
                        ba::ssl::context context_;
                        string session_id_context_;
                };

        }
}
