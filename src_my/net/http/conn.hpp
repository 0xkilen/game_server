#pragma once

#include "reply.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "net/stream.hpp"
#include "utils/instance_counter.hpp"
#include <array>
#include <memory>
#include <boost/asio.hpp>

using namespace std;
namespace bs = boost::system;

namespace nora {
        namespace net {
                namespace http {

                        template <typename T>
                        class conn : public enable_shared_from_this<conn<T>>,
                                     private instance_countee {
                        public:
                                template <typename... Args>
                                conn(const string& name_prefix, Args &&... args) :
                                        instance_countee(ICT_HTTP_CONN),
                                        socket_(new T(forward<Args>(args)...)),
                                        name_(name_prefix) {
                                }
                                void start() {
                                        do_read();
                                }
                                void cancel() {
                                        socket_->lowest_layer().cancel();
                                }
                                void stop() {
                                        cancel();
                                }
                                void send(const shared_ptr<sendstream>& msg) {
                                        istream is(&msg->streambuf_);
                                        reply_.parse(is);
                                        do_write();
                                }
                                const string& name() const {
                                        return name_;
                                }
                                string remote_ip() const {
                                        try {
                                                return socket_->lowest_layer().remote_endpoint().address().to_string();
                                        } catch (const bs::system_error& e) {
                                                return "no ip";
                                        }
                                }
                                T& socket() {
                                        return *socket_;
                                }
                                friend ostream& operator<<(ostream& os, const conn& c) {
                                        return os << c.name_ << " " << c.remote_ip();
                                }

                                function<void(const shared_ptr<conn>&, const shared_ptr<recvstream>&)> msg_cb_;
                                function<void(const shared_ptr<conn>&)> error_cb_;
                        private:
                                void do_read() {
                                        socket_->async_read_some(boost::asio::buffer(buffer_),
                                                                 [this, self = this->shared_from_this()] (const auto& ec, auto bytes_transferred) {
                                                                         if (!ec) {
                                                                                 request_parser::result_type result;
                                                                                 tie(result, ignore) = request_parser_.parse(
                                                                                         request_, buffer_.data(), buffer_.data() + bytes_transferred);

                                                                                 if (result == request_parser::good) {
                                                                                         auto msg = make_shared<recvstream>();
                                                                                         ostream os(&msg->streambuf_);
                                                                                         request_.serialize(os);

                                                                                         msg_cb_(self, msg);
                                                                                 } else if (result == request_parser::bad) {
                                                                                         reply_ = reply::stock_reply(reply::bad_request);
                                                                                         this->do_write();
                                                                                 } else {
                                                                                         this->do_read();
                                                                                 }
                                                                         } else if (ec != ba::error::operation_aborted) {
                                                                                 this->cancel();
                                                                                 error_cb_(self);
                                                                         }
                                                                 });
                                }
                                void do_write() {
                                        ba::async_write(*socket_, reply_.to_buffers(),
                                                        [this, self = this->shared_from_this()](bs::error_code ec, size_t) {
                                                                if (!ec) {
                                                                        bs::error_code ignored_ec;
                                                                        socket_->lowest_layer().shutdown(ba::ip::tcp::socket::shutdown_both,
                                                                                         ignored_ec);
                                                                } else if (ec != ba::error::operation_aborted) {
                                                                        cancel();
                                                                        error_cb_(self);
                                                                }
                                                        });
                                }

                                static thread_local uint64_t conn_idx;
                                unique_ptr<T> socket_;
                                const string name_;
                                array<char, 8192> buffer_;
                                request request_;
                                request_parser request_parser_;
                                reply reply_;
                        };

                }
        }
}
