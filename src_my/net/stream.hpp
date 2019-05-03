#pragma once

#include <iostream>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace net {

                class sendstream : public iostream {
                public:
                        sendstream() : iostream(&streambuf_) {}
                        size_t size() {
                                return streambuf_.size();
                        }
                        ba::streambuf streambuf_;
                };

                class recvstream : public istream {
                public:
                        recvstream() : istream(&streambuf_) {}
                        recvstream(const char *) : istream(&streambuf_) {}
                        size_t size() {
                                return streambuf_.size();
                        }
                        ba::streambuf streambuf_;
                };

                class senddata {
                public:
                        senddata(shared_ptr<sendstream> header, shared_ptr<sendstream> msg) :
                                header_(header), msg_(msg) {}
                        shared_ptr<sendstream> header_;
                        shared_ptr<sendstream> msg_;
                };

        }
}
