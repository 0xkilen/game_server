#pragma once

#include "common.hpp"
#include "net/stream.hpp"
#include "utils/instance_counter.hpp"
#include "utils/crypto.hpp"
#include <memory>
#include <functional>
#include <list>
#include <string>
#include <map>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace net {
                namespace websocket {

                        template<typename T>
                        class server_conn : public enable_shared_from_this<server_conn<T>>,
                                            private boost::noncopyable,
                                            private instance_countee {
                        public:
                                template <typename... Args>
                                server_conn(const string& name_prefix, Args &&... args) :
                                        instance_countee(ICT_WS_CONN),
                                        socket_(new T(forward<Args>(args)...)),
                                        strand_(socket_->get_io_service()),
                                        name_(name_prefix),
                                        closed_(false) {
                                }
                                void start() {
                                        NET_TLOG << *this << " start";
                                        read_handshake();
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
                                friend ostream& operator<<(ostream& os, const server_conn& c) {
                                        return os << c.name_ << " " << c.remote_ip();
                                }

                                function<void(const shared_ptr<server_conn>&, const shared_ptr<recvstream>&)> msg_cb_;
                                function<void(const shared_ptr<server_conn>&)> error_cb_;
                        private:
                                // fin_rsv_opcode: 129=one fragment, text, 130=one fragment, binary, 136=close connection.
                                // see http://tools.ietf.org/html/rfc6455#section-5.2 for more information
                                void send(const shared_ptr<sendstream>& msg, unsigned char fin_rsv_opcode) {
                                        auto header = make_shared<sendstream>();
                                        auto size = msg->size();
                                        header->put(fin_rsv_opcode);

                                        if (size >= 126) {
                                                int byte_count;
                                                if(size > 0xffff) {
                                                        byte_count = 8;
                                                        header->put(127);
                                                } else {
                                                        byte_count = 2;
                                                        header->put(126);
                                                }
                                                for(int c = byte_count - 1; c >= 0; c--) {
                                                        header->put((size >> (8 * c)) % 256);
                                                }
                                        } else {
                                                header->put((unsigned char)(size));
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
                                                                                [this, self] (bs::error_code ec, size_t length) {
                                                                                        if(!ec) {
                                                                                                ba::async_write(*socket_, send_queue_.begin()->msg_->streambuf_,
                                                                                                                strand_.wrap(
                                                                                                                        [this, self] (bs::error_code ec, size_t length) {
                                                                                                                                auto send_queued = send_queue_.begin();
                                                                                                                                if (!ec) {
                                                                                                                                        send_queue_.erase(send_queued);
                                                                                                                                        if (send_queue_.size() > 0)
                                                                                                                                                send_from_queue();
                                                                                                                                } else if (ec != ba::error::operation_aborted) {
                                                                                                                                        NET_TLOG << *this << " write error: " << ec.message();
                                                                                                                                        cancel();
                                                                                                                                        error_cb_(self);
                                                                                                                                }
                                                                                                                        }));
                                                                                        } else if (ec != ba::error::operation_aborted) {
                                                                                                NET_TLOG << *this << " write error: " << ec.message();
                                                                                                cancel();
                                                                                                error_cb_(self);
                                                                                        }
                                                                                }));
                                                });
                                }
                                void read_handshake() {
                                        NET_TLOG << *this << " read handshake";

                                        auto read_buffer = make_shared<ba::streambuf>();
                                        ba::async_read_until(*socket_, *read_buffer, "\r\n\r\n",
                                                             [this, self = this->shared_from_this(), read_buffer] (bs::error_code ec, size_t length) {
                                                                     if (!ec) {
                                                                             NET_TLOG << *this << " read handshake length: " << length;
                                                                             istream is(read_buffer.get());
                                                                             parse_handshake(is);
                                                                             write_handshake(read_buffer);
                                                                     } else if (ec != ba::error::operation_aborted) {
                                                                             NET_TLOG << *this << " read handshake error: " << ec.message();
                                                                             socket_->lowest_layer().close();
                                                                             error_cb_(self);
                                                                     }
                                                             });
                                }
                                void parse_handshake(istream& is) {
                                        NET_TLOG << *this << " parse handshake";

                                        string line;
                                        getline(is, line);
                                        size_t method_end;
                                        if ((method_end = line.find(' ')) != string::npos) {
                                                size_t path_end;
                                                if ((path_end = line.find(' ', method_end + 1)) != string::npos) {
                                                        method_ = line.substr(0, method_end);
                                                        NET_TLOG << "method: " << method_;
                                                        path_ = line.substr(method_end + 1, path_end - method_end - 1);
                                                        NET_TLOG << "path: " << path_;
                                                        if ((path_end + 6) < line.size()) {
                                                                http_version_ = line.substr(path_end + 6, line.size() - (path_end + 6) - 1);
                                                        } else {
                                                                http_version_ = "1.1";
                                                        }
                                                        NET_TLOG << "http_version: " << http_version_;

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
                                                }
                                        }
                                }
                                bool generate_handshake(ostream& handshake) const {
                                        NET_TLOG << *this << " generate handshake";

                                        if (headers_.count("Sec-WebSocket-Key") == 0) {
                                                return false;
                                        }

                                        auto sha1 = SHA1(headers_.at("Sec-WebSocket-Key") + ws_magic_string);

                                        handshake << "HTTP/1.1 101 Web Socket Protocol Handshake\r\n";
                                        handshake << "Upgrade: websocket\r\n";
                                        handshake << "Connection: Upgrade\r\n";
                                        handshake << "Sec-WebSocket-Accept: " << Base64::encode(sha1) << "\r\n";
                                        handshake << "\r\n";

                                        return true;
                                }
                                void write_handshake(const shared_ptr<ba::streambuf>& read_buffer) {
                                        NET_TLOG << *this << " write handshake";

                                        //Find path- and method-match, and generate response
                                        auto write_buffer = make_shared<ba::streambuf>();
                                        ostream handshake(write_buffer.get());

                                        if (generate_handshake(handshake)) {
                                                ba::async_write(*socket_, *write_buffer,
                                                                [this, self = this->shared_from_this(), write_buffer, read_buffer] (const auto& ec, auto length) {
                                                                        if (!ec) {
                                                                                NET_TLOG << *this << " write handshake length: " << length;
                                                                                this->read_header(read_buffer);
                                                                        } else if (ec != ba::error::operation_aborted) {
                                                                                NET_TLOG << *this << " write handshake error: " << ec.message();
                                                                                this->cancel();
                                                                                error_cb_(self);
                                                                        }
                                                                });
                                                handshaked_ = true;
                                                send_from_queue();
                                        } else {
                                                NET_TLOG << *this << " generate handshake failed";
                                                cancel();
                                                error_cb_(this->shared_from_this());
                                        }
                                }
                                void read_header(const shared_ptr<ba::streambuf>& read_buffer) {
                                        ba::async_read(*socket_, *read_buffer, ba::transfer_exactly(2),
                                                       [this, self = this->shared_from_this(), read_buffer] (const auto& ec, auto length) mutable {
                                                               if (!ec) {
                                                                       NET_TLOG << *this << " read first header length: " << length;
                                                                       if (length == 0) {
                                                                               this->read_header(read_buffer);
                                                                               return;
                                                                       }
                                                                       istream is(read_buffer.get());

                                                                       array<unsigned char, 2> header;
                                                                       is.read((char *)header.data(), 2);
                                                                       unsigned char fin_rsv_opcode = header[0];

                                                                       // close connection if unmasked message from client (protocol error)
                                                                       if (header[1] < 128) {
                                                                               const string reason("message from client not masked");
                                                                               this->send_close(1002, reason);
                                                                               error_cb_(self);
                                                                               return;
                                                                       }

                                                                       size_t length = (header[1] & 127);

                                                                       if (length == 126) {
                                                                               // 2 next bytes is the size of content
                                                                               ba::async_read(*socket_, *read_buffer, boost::asio::transfer_exactly(2),
                                                                                              [this, self, read_buffer, fin_rsv_opcode] (const auto& ec, auto length) mutable {
                                                                                                      if(!ec) {
                                                                                                              NET_TLOG << *this << " read second header length: " << length;
                                                                                                              istream is(read_buffer.get());
                                                                                                              array<unsigned char, 2> length_bytes;
                                                                                                              is.read((char *)length_bytes.data(), 2);
                                                                                                              size_t length = 0;
                                                                                                              auto byte_count = 2;
                                                                                                              for(auto c = 0; c < byte_count; c++) {
                                                                                                                      length += length_bytes[c] << (8 * (byte_count - 1 - c));
                                                                                                              }
                                                                                                              NET_TLOG << *this << " msg lenth: " << length;
                                                                                                              this->read_msg(read_buffer, length, fin_rsv_opcode);
                                                                                                      } else if (ec != ba::error::operation_aborted) {
                                                                                                              NET_TLOG << *this << " read header error: " << ec.message();
                                                                                                              this->cancel();
                                                                                                              error_cb_(self);
                                                                                                      };
                                                                                              });
                                                                       } else if (length == 127) {
                                                                               //8 next bytes is the size of content
                                                                               ba::async_read(*socket_, *read_buffer, boost::asio::transfer_exactly(8),
                                                                                              [this, self, read_buffer, fin_rsv_opcode] (const auto& ec, auto length) mutable {
                                                                                                      if(!ec) {
                                                                                                              NET_TLOG << *this << " read second header length: " << length;
                                                                                                              istream is(read_buffer.get());
                                                                                                              array<unsigned char, 8> length_bytes;
                                                                                                              is.read((char *)length_bytes.data(), 8);

                                                                                                              size_t length = 0;
                                                                                                              auto byte_count = 8;
                                                                                                              for (auto c = 0; c < byte_count; c++) {
                                                                                                                      length += length_bytes[c] << (8 * (byte_count - 1 - c));
                                                                                                              }
                                                                                                              NET_TLOG << *this << " msg length: " << length;
                                                                                                              this->read_msg(read_buffer, length, fin_rsv_opcode);
                                                                                                      } else if (ec != ba::error::operation_aborted) {
                                                                                                              NET_TLOG << *this << " read second header error: " << ec.message();
                                                                                                              this->cancel();
                                                                                                              error_cb_(self);
                                                                                                      };
                                                                                              });
                                                                       } else {
                                                                               this->read_msg(read_buffer, length, fin_rsv_opcode);
                                                                       }
                                                               } else if (ec != ba::error::operation_aborted) {
                                                                       NET_TLOG << *this << " read first header error: " << ec.message();
                                                                       this->cancel();
                                                                       error_cb_(self);
                                                               }
                                                       });
                                }
                                void read_msg(const shared_ptr<ba::streambuf>& read_buffer, size_t size, unsigned char fin_rsv_opcode) {
                                        ba::async_read(*socket_, *read_buffer, ba::transfer_exactly(4 + size),
                                                       [this, self = this->shared_from_this(), read_buffer, size, fin_rsv_opcode] (const boost::system::error_code& ec, size_t length) {
                                                               if(!ec) {
                                                                       NET_TLOG << *this << " read msg length: " << length;
                                                                       istream raw_message_data(read_buffer.get());

                                                                       array<unsigned char, 4> mask;
                                                                       raw_message_data.read((char *)mask.data(), 4);

                                                                       auto msg = make_shared<recvstream>();
                                                                       ostream message_data_out_stream(&msg->streambuf_);
                                                                       for (auto c = 0u; c < size; c++) {
                                                                               message_data_out_stream.put(raw_message_data.get()^mask[c % 4]);
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
                                                                                       send(empty_send_stream, fin_rsv_opcode+1);
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
                                list<senddata> send_queue_;
                                atomic<bool> closed_;
                                bool handshaked_ = false;
                        };

                }
        }
}
