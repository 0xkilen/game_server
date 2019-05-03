#pragma once

#include "stream.hpp"
#include "log.hpp"
#include "utils/instance_counter.hpp"
#include "utils/assert.hpp"
#include <memory>
#include <functional>
#include <list>
#include <limits>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ba = boost::asio;
namespace bs = boost::system;

namespace nora {
        namespace net {

                template <typename T>
                class conn : public enable_shared_from_this<conn<T>>,
                             private boost::noncopyable,
                             private instance_countee {
                public:
                        template <typename... Args>
                        conn(const string& name_prefix, Args &&... args) :
                                instance_countee(ICT_CONN),
                                socket_(new T(forward<Args>(args)...)),
                                strand_(socket_->get_io_service()),
                                name_(name_prefix),
                                stop_(false) {
                        }
                        void start() {
                                read_header();
                        }
                        void cancel() {
                                socket_->cancel();
                        }
                        void stop() {
                                stop_.store(true);
                        }
                        void send(const shared_ptr<sendstream>& msg) {
                                auto header = make_shared<sendstream>();

                                ASSERT(msg->size() <= numeric_limits<uint32_t>::max());
                                auto size = ba::detail::socket_ops::host_to_network_long(static_cast<uint32_t>(msg->size()));
                                header->put(((char *)&size)[0]);
                                header->put(((char *)&size)[1]);
                                header->put(((char *)&size)[2]);
                                header->put(((char *)&size)[3]);

                                auto self = this->shared_from_this();
                                strand_.post(
                                        [this, self, header, msg] {
                                                send_queue_.emplace_back(header, msg);
                                                if (send_queue_.size() == 1) {
                                                        send_from_queue();
                                                }
                                        });
                        }
                        const string& name() const {
                                return name_;
                        }
                        string remote_ip() const {
                                try {
                                        return socket_->remote_endpoint().address().to_string();
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
                        void send_from_queue() {
                                strand_.post(
                                        [this, self = this->shared_from_this()] {
                                                ba::async_write(*socket_, send_queue_.begin()->header_->streambuf_,
                                                                strand_.wrap(
                                                                        [this, self] (const auto& ec, auto length) {
                                                                                if (!ec) {
                                                                                        ba::async_write(*socket_, send_queue_.begin()->msg_->streambuf_,
                                                                                                        strand_.wrap(
                                                                                                                [this, self] (const auto& ec, auto length) {
                                                                                                                        auto send_queued = send_queue_.begin();
                                                                                                                        if (!ec) {
                                                                                                                                send_queue_.erase(send_queued);
                                                                                                                                if (send_queue_.size() > 0) {
                                                                                                                                        this->send_from_queue();
                                                                                                                                } else if (stop_.load()) {
                                                                                                                                        this->cancel();
                                                                                                                                        error_cb_(self);
                                                                                                                                }
                                                                                                                        } else if (ec != ba::error::operation_aborted) {
                                                                                                                                NET_TLOG << *self << " write error: " << ec.message();
                                                                                                                                this->cancel();
                                                                                                                                error_cb_(self);
                                                                                                                        }
                                                                                                                }));
                                                                                } else if (ec != ba::error::operation_aborted) {
                                                                                        NET_TLOG << *self << " write error: " << ec.message();
                                                                                        this->cancel();
                                                                                        error_cb_(self);
                                                                                }
                                                                        }));
                                        });
                        }
                        void read_header() {
                                auto header = make_shared<recvstream>();
                                auto self = this->shared_from_this();
                                ba::async_read(*socket_, header->streambuf_, ba::transfer_exactly(4),
                                               [this, self, header] (const auto& ec, auto length) {
                                                       if (!ec) {
                                                               uint32_t size;
                                                               header->get(((char *)&size)[0]);
                                                               header->get(((char *)&size)[1]);
                                                               header->get(((char *)&size)[2]);
                                                               header->get(((char *)&size)[3]);
                                                               size = ba::detail::socket_ops::network_to_host_long(size);
                                                               this->read_msg(size);
                                                       } else if (ec != ba::error::operation_aborted) {
                                                               NET_TLOG << *this << " read error: " << ec.message();
                                                               this->cancel();
                                                               error_cb_(self);
                                                       }
                                               });
                        }
                        void read_msg(uint32_t size) {
                                auto msg = make_shared<recvstream>();
                                ba::async_read(*socket_, msg->streambuf_,
                                               ba::transfer_exactly(size),
                                               [this, self = this->shared_from_this(), msg] (const auto& ec, auto length) {
                                                       if (!ec) {
                                                               msg_cb_(self, msg);
                                                               this->read_header();
                                                       } else if (ec != ba::error::operation_aborted) {
                                                               NET_TLOG << *this << " read error: " << ec.message();
                                                               this->cancel();
                                                               error_cb_(self);
                                                       }
                                               });
                        }

                        static thread_local uint64_t conn_idx;
                        unique_ptr<T> socket_;
                        ba::strand strand_;
                        const string name_;
                        list<senddata> send_queue_;
                        atomic<bool> stop_;
                };

        }
}
