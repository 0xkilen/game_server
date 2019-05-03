#pragma once

#include "header.hpp"
#include "proto/ys_base.pb.h"
#include <string>
#include <vector>
#include <boost/asio.hpp>

using namespace std;
namespace ba = boost::asio;
namespace py = proto::ys;

namespace nora {
        namespace net {
                namespace http {

                        // A reply to be sent to a client.
                        struct reply {
                                // The status of the reply.
                                enum status_type {
                                        ok = 200,
                                        created = 201,
                                        accepted = 202,
                                        no_content = 204,
                                        multiple_choices = 300,
                                        moved_permanently = 301,
                                        moved_temporarily = 302,
                                        not_modified = 304,
                                        bad_request = 400,
                                        unauthorized = 401,
                                        forbidden = 403,
                                        not_found = 404,
                                        internal_server_error = 500,
                                        not_implemented = 501,
                                        bad_gateway = 502,
                                        service_unavailable = 503
                                } status;

                                /// The headers to be included in the reply.
                                vector<header> headers;

                                /// The content to be sent in the reply.
                                string content;

                                /// Convert the reply into a vector of buffers. The buffers do not own the
                                /// underlying memory blocks, therefore the reply object must remain valid and
                                /// not be changed until the write operation has completed.
                                vector<ba::const_buffer> to_buffers();

                                void parse(const py::base& msg);
                                void parse(istream& is);

                                /// Get a stock reply.
                                static reply stock_reply(status_type status);
                        };
                }
        }
}
