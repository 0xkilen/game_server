#include "request.hpp"
#include "utils/string_utils.hpp"

namespace nora {
        namespace net {
                namespace http {

                        void request::serialize(py::base& msg) {
                                auto *req = msg.MutableExtension(py::request::req);
                                req->set_method(method);
                                req->set_uri(uri);
                                req->set_http_version_major(http_version_major);
                                req->set_http_version_minor(http_version_minor);
                                for (const auto& i : headers) {
                                        auto *h = req->add_headers();
                                        h->set_name(i.name);
                                        h->set_value(i.value);
                                }
                                auto param_pairs = split_string(content, '&');
                                for (const auto& i : param_pairs) {
                                        if (i.find('=') == string::npos) {
                                                continue;
                                        }
                                        auto param_pair = split_string(i, '=');
                                        if (param_pair.size() == 1) {
                                                auto *p = req->add_params();
                                                p->set_name(param_pair[0]);
                                        } else if (param_pair.size() == 2) {
                                                auto *p = req->add_params();
                                                p->set_name(param_pair[0]);
                                                p->set_value(param_pair[1]);
                                        } else if (param_pair.size() > 2) {
                                                auto *p = req->add_params();
                                                p->set_name(param_pair[0]);

                                                string value(param_pair[1]);
                                                for (auto i = 2u; i < param_pair.size(); ++i) {
                                                        value += '=';
                                                        value += param_pair[i];
                                                }
                                                p->set_value(value);
                                        }
                                }
                        }

                        void request::serialize(ostream& os) {
                                py::base msg;
                                serialize(msg);
                                msg.SerializeToOstream(&os);
                        }

                }
        }
}
