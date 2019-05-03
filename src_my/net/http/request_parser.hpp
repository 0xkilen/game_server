#pragma once

#include "request.hpp"
#include <tuple>

using namespace std;

namespace nora {
        namespace net {
                namespace http {

                        struct request;

                        // Parser for incoming requests.
                        class request_parser {
                        public:
                                // Construct ready to parse the request method.
                                request_parser();

                                // Reset to initial parser state.
                                void reset();

                                // Result of parse.
                                enum result_type { good, bad, indeterminate };

                                // Parse some data. The enum return value is good when a complete request has
                                // been parsed, bad if the data is invalid, indeterminate when more data is
                                // required. The InputIterator return value indicates how much of the input
                                // has been consumed.
                                template <typename InputIterator>
                                tuple<result_type, InputIterator> parse(request& req,
                                                                        InputIterator begin, InputIterator end) {
                                        while (begin != end) {
                                                result_type result = consume(req, *begin++);
                                                if (result == bad) {
                                                        return make_tuple(result, begin);
                                                }
                                                if (result == good) {
                                                        auto uri = req.uri;
                                                        if (!url_decode(uri, req.uri)) {
                                                                return make_tuple(bad, begin);
                                                        }
                                                        if (req.uri.empty() || req.uri[0] != '/' || req.uri.find("..") != string::npos) {
                                                                return make_tuple(bad, begin);
                                                        }
                                                        return make_tuple(good, begin);
                                                }
                                        }
                                        return make_tuple(indeterminate, begin);
                                }

                        private:
                                // Handle the next character of input.
                                result_type consume(request& req, char input);

                                // Check if a byte is an HTTP character.
                                static bool is_char(int c);

                                // Check if a byte is an HTTP control character.
                                static bool is_ctl(int c);

                                // Check if a byte is defined as an HTTP tspecial character.
                                static bool is_tspecial(int c);

                                // Check if a byte is a digit.
                                static bool is_digit(int c);

                                static bool url_decode(const string& in, string& out);

                                // The current state of the parser.
                                enum state {
                                        method_start,
                                        method,
                                        uri,
                                        http_version_h,
                                        http_version_t_1,
                                        http_version_t_2,
                                        http_version_p,
                                        http_version_slash,
                                        http_version_major_start,
                                        http_version_major,
                                        http_version_minor_start,
                                        http_version_minor,
                                        expecting_newline_1,
                                        header_line_start,
                                        header_lws,
                                        header_name,
                                        space_before_header_value,
                                        header_value,
                                        expecting_newline_2,
                                        expecting_newline_3,
                                        header_end,
                                        expecting_newline_4,
                                        content,
                                } state_;
                        };

                }
        }
}
