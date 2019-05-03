#pragma once

#include "common.hpp"
#include "net/log.hpp"
#include "net/stream.hpp"
#include "utils/instance_counter.hpp"
#include "utils/crypto.hpp"
#include <memory>
#include <functional>
#include <list>
#include <string>
#include <map>
#include <random>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ba = boost::asio;
namespace bs = boost::system;

namespace nora {
        namespace net {
                namespace websocket {

                        template <typename T>
                        class client_conn : public enable_shared_from_this<client_conn<T>>,
                                            private boost::noncopyable,
                                            private instance_countee {
                        public:
                                template <typename... Args>
                                client_conn(const string& name_prefix, Args &&... args) :
                                        instance_countee(ICT_WS_CONN),
                                        socket_(new T(forward<Args>(args)...)),
                                        strand_(socket_->get_io_service()),
                                        name_(name_prefix),
                                        closed_(false) {
                                }
                                void start() {
                                        NET_TLOG << *this << " start";
                                        write_handshake();
                                }
                                void cancel() {
                                        socket_->lowest_layer().cancel();
                                }
                                void stop() {
                                        cancel();
                                }
                                void send(const shared_ptr<sendstream>& msg) {
                                        send(msg, 130);
                                }
                                T& socket() {
                                        return *socket_;
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
                                friend ostream& operator<<(ostream& os, const client_conn& c) {
                                        return os << c.name_ << " " << c.remote_ip();
                                }

                                function<void(const shared_ptr<client_conn>&, const shared_ptr<recvstream>&)> msg_cb_;
                                function<void(const shared_ptr<client_conn>&)> error_cb_;
                        private:
                                // fin_rsv_opcode: 129=one fragment, text, 130=one fragment, binary, 136=close connection.
                                // see http://tools.ietf.org/html/rfc6455#section-5.2 for more information
                                void send(const shared_ptr<sendstream>& msg, unsigned char fin_rsv_opcode) {
                                        vector<unsigned char> mask;
                                        mask.resize(4);
                                        uniform_int_distribution<unsigned short> dist(0, 255);
                                        random_device rd;
                                        for(size_t c = 0; c < 4; c++) {
                                                mask[c] = static_cast<unsigned char>(dist(rd));
                                        }

                                        auto header = make_shared<sendstream>();
                                        auto size = msg->size();
                                        header->put(fin_rsv_opcode);

                                        if (size >= 126) {
                                                int byte_count;
                                                if(size > 0xffff) {
                                                        byte_count = 8;
                                                        header->put(127 + 128);
                                                } else {
                                                        byte_count = 2;
                                                        header->put(126 + 128);
                                                }
                                                for(int c = byte_count - 1; c >= 0; c--) {
                                                        header->put((size >> (8 * c)) % 256);
                                                }
                                        } else {
                                                header->put((unsigned char)(size + 128));
                                        }

                                        for (auto c = 0; c < 4; c++) {
                                                header->put(static_cast<char>(mask[c]));
                                        }

                                        auto length = msg->size();
                                        for (auto c = 0u; c < length; c++) {
                                                msg->put(msg->get() ^ mask[c % 4]);
                                        }

                                        strand_.post(
                                                [this, self = this->shared_from_this(), header, msg] {
                                                        send_queue_.emplace_back(header, msg);
                                                        if (send_queue_.size() == 1) {
                                                                send_from_queue();
                                                        }
                                                });
                                }
                                void send_close(int status, const string& reason) {
                                        // send close only once (in case close is initiated by server)
                                        if (closed_.load()) {
                                                return;
                                        }
                                        closed_.store(true);

                                        auto ss = make_shared<sendstream>();

                                        ss->put(status >> 8);
                                        ss->put(status % 256);
                                        *ss << reason;

                                        // fin_rsv_opcode=136: message close
                                        send(ss, 136);
                                }
                                void send_from_queue() {
                                        if (send_queue_.empty() || !handshaked_) {
                                                return;
                                        }

                                        strand_.post(
                                                [this, self = this->shared_from_this()] {
                                                        ba::async_write(*socket_, send_queue_.begin()->header_->streambuf_,
                                                                        strand_.wrap(
                                                                                [this, self] (auto ec, auto length) {
                                                                                        if(!ec) {
                                                                                                ba::async_write(*socket_, send_queue_.begin()->msg_->streambuf_,
                                                                                                                strand_.wrap(
                                                                                                                        [this, self] (auto ec, auto length) {
                                                                                                                                auto send_queued = send_queue_.begin();
                                                                                                                                if (!ec) {
                                                                                                                                        send_queue_.erase(send_queued);
                                                                                                                                        if (send_queue_.size() > 0)
                                                                                                                                                this->send_from_queue();
                                                                                                                                } else if (ec != ba::error::operation_aborted) {
                                                                                                                                        NET_TLOG << *this << " write error: " << ec.message();
                                                                                                                                        this->cancel();
                                                                                                                                        error_cb_(self);
                                                                                                                                }
                                                                                                                        }));
                                                                                        } else if (ec != ba::error::operation_aborted) {
                                                                                                NET_TLOG << *this << " write error: " << ec.message();
                                                                                                this->cancel();
                                                                                                error_cb_(self);
                                                                                        }
                                                                                }));
                                                });
                                }
                                void write_handshake() {
                                        NET_TLOG << *this << " write handshake";

                                        auto write_buffer = make_shared<ba::streambuf>();
                                        ostream handshake(write_buffer.get());

                                        if (generate_handshake(handshake)) {
                                                ba::async_write(*socket_, *write_buffer,
                                                                [this, self = this->shared_from_this(), write_buffer] (auto ec, auto length) {
                                                                        if (!ec) {
                                                                                NET_TLOG << *this << " handshake length: " << length;
                                                                                this->read_handshake();
                                                                        } else if (ec != ba::error::operation_aborted) {
                                                                                NET_TLOG << *this << " handshake error: " << ec.message();
                                                                                this->cancel();
                                                                                error_cb_(self);
                                                                        }
                                                                });
                                        } else {
                                                NET_TLOG << *this << " generate handshake failed";
                                                cancel();
                                                error_cb_(this->shared_from_this());
                                        }
                                }
                                void read_handshake() {
                                        NET_TLOG << *this << " read handshake";

                                        auto read_buffer = make_shared<ba::streambuf>();
                                        ba::async_read_until(*socket_, *read_buffer, "\r\n\r\n",
                                                             [this, self = this->shared_from_this(), read_buffer] (auto ec, auto length) {
                                                                     if (!ec) {
                                                                             NET_TLOG << *this << " read handshake length: " << length;
                                                                             istream is(read_buffer.get());
                                                                             if (!this->parse_handshake(is)) {
                                                                                     socket_->lowest_layer().close();
                                                                                     error_cb_(self);
                                                                                     return;
                                                                             }

                                                                             if (status_.empty() || status_.compare(0, 4, "101 ") != 0) {
                                                                                     NET_TLOG << *this << " read handshake error: status not 101";
                                                                                     socket_->lowest_layer().close();
                                                                                     error_cb_(self);
                                                                                     return;
                                                                             }

                                                                             if (headers_.count("Sec-WebSocket-Accept") == 0 ||
                                                                                 Base64::decode(headers_.at("Sec-WebSocket-Accept")) != SHA1(nonce_ + ws_magic_string)) {
                                                                                     NET_TLOG << *this << " read handshake error: check Sec-WebSocket-Accept failed";
                                                                                     socket_->lowest_layer().close();
                                                                                     error_cb_(self);
                                                                                     return;
                                                                             }

                                                                             handshaked_ = true;
                                                                             this->read_header(read_buffer);
                                                                             this->send_from_queue();
                                                                     } else if (ec != ba::error::operation_aborted) {
                                                                             NET_TLOG << *this << " read handshake error: " << ec.message();
                                                                             socket_->lowest_layer().close();
                                                                             error_cb_(self);
                                                                     }
                                                             });
                                }
                                bool parse_handshake(istream& is) {
                                        NET_TLOG << *this << " parse handshake";

                                        string line;
                                        getline(is, line);
                                        auto version_end = line.find(' ');
                                        if (version_end != string::npos) {
                                                if (5 < line.size()) {
                                                        http_version_ = line.substr(5, version_end - 5);
                                                        NET_TLOG << "http_version: " << http_version_;
                                                } else {
                                                        NET_ELOG << "no http_version";
                                                        return false;
                                                }
                                                if ((version_end + 1) < line.size()) {
                                                        status_ = line.substr(version_end + 1, line.size() - (version_end + 1) - 1);
                                                        NET_TLOG << "status: " << status_;
                                                } else {
                                                        NET_ELOG << "no status";
                                                        return false;
                                                }

                                                getline(is, line);
                                                size_t param_end;
                                                while ((param_end = line.find(':')) != string::npos) {
                                                        size_t value_start = param_end + 1;
                                                        if ((value_start) < line.size()) {
                                                                if (line[value_start] == ' ') {
                                                                        value_start++;
                                                                }
                                                                if (value_start < line.size()) {
                                                                        auto key = line.substr(0, param_end);
                                                                        auto value = line.substr(value_start, line.size() - value_start - 1);
                                                                        headers_.insert(make_pair(key, value));
                                                                        NET_TLOG << "header key: " << key << " header value: " << value;
                                                                }
                                                        }
                                                        getline(is, line);
                                                }
                                        } else {
                                                NET_ELOG << "no http_version";
                                                return false;
                                        }

                                        return true;
                                }
                                bool generate_handshake(ostream& handshake) {
                                        NET_TLOG << *this << " generate handshake";

                                        handshake << "GET " << "jxwy"  << " HTTP/1.1\r\n";
                                        handshake << "Host: " << "jxwy" << "\r\n";
                                        handshake << "Upgrade: websocket\r\n";
                                        handshake << "Connection: Upgrade\r\n";

                                        // Make random 16-byte nonce
                                        nonce_.resize(16);
                                        uniform_int_distribution<unsigned short> dist(0, 255);
                                        random_device rd;
                                        for (auto c = 0; c < 16; c++) {
                                                nonce_[c] = static_cast<char>(dist(rd));
                                        }

                                        nonce_ = Base64::encode(nonce_);
                                        handshake << "Sec-WebSocket-Key: " << nonce_ << "\r\n";
                                        handshake << "Sec-WebSocket-Version: 13\r\n";
                                        handshake << "\r\n";

                                        return true;
                                }
                                void read_header(const shared_ptr<ba::streambuf>& read_buffer) {
                                        ba::async_read(*socket_, *read_buffer, ba::transfer_exactly(2),
                                                       [this, self = this->shared_from_this(), read_buffer] (bs::error_code ec, size_t length) mutable {
                                                               if (!ec) {
                                                                       NET_TLOG << *this << " read first header length: " << length;
                                                                       if (length == 0) {
                                                                               read_header(read_buffer);
                                                                               return;
                                                                       }
                                                                       istream is(read_buffer.get());

                                                                       array<unsigned char, 2> header;
                                                                       is.read((char *)header.data(), 2);
                                                                       unsigned char fin_rsv_opcode = header[0];

                                                                       // close connection if masked message from server (protocol error)
                                                                       if (header[1] >= 128) {
                                                                               const string reason("message from server masked");
                                                                               send_close(1002, reason);
                                                                               error_cb_(self);
                                                                               return;
                                                                       }

                                                                       size_t length = (header[1] & 127);

                                                                       if (length == 126) {
                                                                               // 2 next bytes is the size of content
                                                                               ba::async_read(*socket_, *read_buffer, boost::asio::transfer_exactly(2),
                                                                                              [this, self, read_buffer, fin_rsv_opcode] (bs::error_code ec, size_t length) mutable {
                                                                                                      if (!ec) {
                                                                                                              NET_TLOG << *this << " read second header length: " << length;
                                                                                                              istream is(read_buffer.get());
                                                                                                              array<unsigned char, 2> length_bytes;
                                                                                                              is.read((char *)length_bytes.data(), 2);
                                                                                                              size_t length = 0;
                                                                                                              int byte_count = 2;
                                                                                                              for (int c = 0; c < byte_count; c++) {
                                                                                                                      length += length_bytes[c] << (8 * (byte_count - 1 - c));
                                                                                                              }
                                                                                                              NET_TLOG << *this << " msg lenth: " << length;
                                                                                                              read_msg(read_buffer, length, fin_rsv_opcode);
                                                                                                      } else if (ec != ba::error::operation_aborted) {
                                                                                                              NET_TLOG << *this << " read header error: " << ec.message();
                                                                                                              cancel();
                                                                                                              error_cb_(self);
                                                                                                      };
                                                                                              });
                                                                       } else if (length == 127) {
                                                                               // 8 next bytes is the size of content
                                                                               ba::async_read(*socket_, *read_buffer, boost::asio::transfer_exactly(8),
                                                                                              [this, self, read_buffer, fin_rsv_opcode] (bs::error_code ec, size_t length) mutable {
                                                                                                      if (!ec) {
                                                                                                              NET_TLOG << *this << " read second header length: " << length;
                                                                                                              istream is(read_buffer.get());
                                                                                                              array<unsigned char, 8> length_bytes;
                                                                                                              is.read((char *)length_bytes.data(), 8);

                                                                                                              size_t length = 0;
                                                                                                              int byte_count = 8;
                                                                                                              for (int c = 0; c < byte_count; c++) {
                                                                                                                      length += length_bytes[c] << (8 * (byte_count - 1 - c));
                                                                                                              }
                                                                                                              NET_TLOG << *this << " msg length: " << length;
                                                                                                              read_msg(read_buffer, length, fin_rsv_opcode);
                                                                                                      } else if (ec != ba::error::operation_aborted) {
                                                                                                              NET_TLOG << *this << " read second header error: " << ec.message();
                                                                                                              cancel();
                                                                                                              error_cb_(self);
                                                                                                      };
                                                                                              });
                                                                       } else {
                                                                               read_msg(read_buffer, length, fin_rsv_opcode);
                                                                       }
                                                               } else if (ec != ba::error::operation_aborted) {
                                                                       NET_TLOG << *this << " read first header error: " << ec.message();
                                                                       cancel();
                                                                       error_cb_(self);
                                                               }
                                                       });
                                }
                                void read_msg(const shared_ptr<ba::streambuf>& read_buffer, size_t size, unsigned char fin_rsv_opcode) {
                                        ba::async_read(*socket_, *read_buffer, ba::transfer_exactly(size),
                                                       [this, self = this->shared_from_this(), read_buffer, size, fin_rsv_opcode] (const boost::system::error_code& ec, size_t length) {
                                                               if(!ec) {
                                                                       NET_TLOG << *this << " read msg length: " << length;
                                                                       istream raw_message_data(read_buffer.get());

                                                                       auto msg = make_shared<recvstream>();
                                                                       ostream message_data_out_stream(&msg->streambuf_);
                                                                       for (auto c = 0u; c < size; c++) {
                                                                               message_data_out_stream.put(raw_message_data.get());
                                                                       }
                                                                       // if connection close
                                                                       if ((fin_rsv_opcode & 0x0f) == 8) {
                                                                               NET_TLOG << *this << " msg is close";
                                                                               int status = 0;
                                                                               if (size >= 2) {
                                                                                       unsigned char byte1 = msg->get();
                                                                                       unsigned char byte2 = msg->get();
                                                                                       status = (byte1 << 8) + byte2;
                                                                               }

                                                                               ostringstream oss;
                                                                               oss << msg->rdbuf();
                                                                               send_close(status, oss.str());
                                                                               error_cb_(self);
                                                                               return;
                                                                       } else {
                                                                               // if ping
                                                                               if ((fin_rsv_opcode & 0x0f) == 9) {
                                                                                       NET_TLOG << *this << " msg is ping";
                                                                                       // send pong
                                                                                       auto empty_send_stream = make_shared<sendstream>();
                                                                                       send(empty_send_stream, fin_rsv_opcode + 1);
                                                                               } else {
                                                                                       NET_TLOG << *this << " msg is data";
                                                                                       msg_cb_(self, msg);
                                                                               }

                                                                               // next message
                                                                               read_header(read_buffer);
                                                                       }
                                                               } else if (ec != ba::error::operation_aborted) {
                                                                       NET_TLOG << *this << " read error: " << ec.message();
                                                                       cancel();
                                                                       error_cb_(self);
                                                               }
                                                       });
                                }

                                static thread_local uint64_t conn_idx;
                                unique_ptr<T> socket_;
                                ba::strand strand_;
                                const string name_;
                                string method_;
                                string path_;
                                map<string, string> headers_;
                                string http_version_;
                                string status_;
                                list<senddata> send_queue_;
                                atomic<bool> closed_;
                                string nonce_;
                                bool handshaked_ = false;
                        };

                }
        }
}
