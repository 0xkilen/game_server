#include "request_parser.hpp"
#include <sstream>

namespace nora {
        namespace net {
                namespace http {

                        request_parser::request_parser()
                                : state_(method_start) {
                        }

                        void request_parser::reset() {
                                state_ = method_start;
                        }

                        request_parser::result_type request_parser::consume(request& req, char input) {
                                switch (state_) {
                                case method_start:
                                        if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                                                return bad;
                                        } else {
                                                state_ = method;
                                                req.method.push_back(input);
                                                return indeterminate;
                                        }
                                case method:
                                        if (input == ' ') {
                                                state_ = uri;
                                                return indeterminate;
                                        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                                                return bad;
                                        } else {
                                                req.method.push_back(input);
                                                return indeterminate;
                                        }
                                case uri:
                                        if (input == ' ') {
                                                state_ = http_version_h;
                                                return indeterminate;
                                        } else if (is_ctl(input)) {
                                                return bad;
                                        } else {
                                                req.uri.push_back(input);
                                                return indeterminate;
                                        }
                                case http_version_h:
                                        if (input == 'H') {
                                                state_ = http_version_t_1;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_t_1:
                                        if (input == 'T') {
                                                state_ = http_version_t_2;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_t_2:
                                        if (input == 'T') {
                                                state_ = http_version_p;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_p:
                                        if (input == 'P') {
                                                state_ = http_version_slash;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_slash:
                                        if (input == '/') {
                                                req.http_version_major = 0;
                                                req.http_version_minor = 0;
                                                state_ = http_version_major_start;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_major_start:
                                        if (is_digit(input)) {
                                                req.http_version_major = req.http_version_major * 10 + input - '0';
                                                state_ = http_version_major;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_major:
                                        if (input == '.') {
                                                state_ = http_version_minor_start;
                                                return indeterminate;
                                        } else if (is_digit(input)) {
                                                req.http_version_major = req.http_version_major * 10 + input - '0';
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_minor_start:
                                        if (is_digit(input)) {
                                                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                                                state_ = http_version_minor;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case http_version_minor:
                                        if (input == '\r') {
                                                state_ = expecting_newline_1;
                                                return indeterminate;
                                        } else if (is_digit(input)) {
                                                req.http_version_minor = req.http_version_minor * 10 + input - '0';
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case expecting_newline_1:
                                        if (input == '\n') {
                                                state_ = header_line_start;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case header_line_start:
                                        if (input == '\r') {
                                                state_ = expecting_newline_3;
                                                return indeterminate;
                                        } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
                                                state_ = header_lws;
                                                return indeterminate;
                                        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                                                return bad;
                                        } else {
                                                req.headers.push_back(header());
                                                req.headers.back().name.push_back(input);
                                                state_ = header_name;
                                                return indeterminate;
                                        }
                                case header_lws:
                                        if (input == '\r') {
                                                state_ = expecting_newline_2;
                                                return indeterminate;
                                        } else if (input == ' ' || input == '\t') {
                                                return indeterminate;
                                        } else if (is_ctl(input)) {
                                                return bad;
                                        } else {
                                                state_ = header_value;
                                                req.headers.back().value.push_back(input);
                                                return indeterminate;
                                        }
                                case header_name:
                                        if (input == ':') {
                                                state_ = space_before_header_value;
                                                return indeterminate;
                                        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
                                                return bad;
                                        } else {
                                                req.headers.back().name.push_back(input);
                                                return indeterminate;
                                        }
                                case space_before_header_value:
                                        if (input == ' ') {
                                                state_ = header_value;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case header_value:
                                        if (input == '\r') {
                                                state_ = expecting_newline_2;
                                                return indeterminate;
                                        } else if (is_ctl(input)) {
                                                return bad;
                                        } else {
                                                req.headers.back().value.push_back(input);
                                                return indeterminate;
                                        }
                                case expecting_newline_2:
                                        if (input == '\n') {
                                                state_ = header_line_start;
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case expecting_newline_3:
                                        if (input == '\n') {
                                                state_ = content;
                                                for (const auto& i : req.headers) {
                                                        if (i.name == "Content-Length") {
                                                                req.content_length = stoi(i.value);
                                                        }
                                                }
                                                if (req.content_length == 0) {
                                                        return good;
                                                } else if (req.content_length < 0) {
                                                        return bad;
                                                } else {
                                                        return indeterminate;
                                                }
                                                return indeterminate;
                                        } else {
                                                return bad;
                                        }
                                case content:
                                        req.content.push_back(input);
                                        if (static_cast<int>(req.content.size()) >= req.content_length) {
                                                auto content = req.content;
                                                url_decode(content, req.content);
                                                return good;
                                        }
                                        return indeterminate;
                                default:
                                        return bad;
                                }
                        }

                        bool request_parser::is_char(int c) {
                                return c >= 0 && c <= 127;
                        }

                        bool request_parser::is_ctl(int c) {
                                return (c >= 0 && c <= 31) || (c == 127);
                        }

                        bool request_parser::is_tspecial(int c) {
                                switch (c) {
                                case '(': case ')': case '<': case '>': case '@':
                                case ',': case ';': case ':': case '\\': case '"':
                                case '/': case '[': case ']': case '?': case '=':
                                case '{': case '}': case ' ': case '\t':
                                        return true;
                                default:
                                        return false;
                                }
                        }

                        bool request_parser::is_digit(int c) {
                                return c >= '0' && c <= '9';
                        }

                        bool request_parser::url_decode(const string& in, string& out) {
                                out.clear();
                                out.reserve(in.size());
                                for (size_t i = 0; i < in.size(); ++i) {
                                        if (in[i] == '%') {
                                                if (i + 3 <= in.size()) {
                                                        int value = 0;
                                                        istringstream is(in.substr(i + 1, 2));
                                                        if (is >> hex >> value) {
                                                                out += static_cast<char>(value);
                                                                i += 2;
                                                        } else {
                                                                return false;
                                                        }
                                                } else {
                                                        return false;
                                                }
                                        } else if (in[i] == '+') {
                                                out += ' ';
                                        } else {
                                                out += in[i];
                                        }
                                }
                                return true;
                        }

                }
        }
}
