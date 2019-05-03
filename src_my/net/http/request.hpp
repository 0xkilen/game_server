#pragma once

#include "header.hpp"
#include "proto/ys_base.pb.h"
#include <string>
#include <vector>

using namespace std;
namespace py = proto::ys;

namespace nora {
        namespace net {
                namespace http {

                        struct request {
                                string method;
                                string uri;
                                int http_version_major;
                                int http_version_minor;
                                vector<header> headers;
                                string content;
                                int content_length = 0;

                                void serialize(py::base& req);
                                void serialize(ostream& req_str);
                        };

                }
        }
}
