#include "string_utils.hpp"
#include "common.hpp"
#include "utils/dirty_word_filter.hpp"
#include <sstream>
#include <mutex>

namespace nora {

        bool is_char(int c) {
                return c >= 0 && c <= 127;
        }

        bool is_digit(int c) {
                return c >= '0' && c <= '9';
        }

        bool is_eng_char(int c) {
                return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        }

        size_t string_length(const string& str) {
                size_t ret = 0;
                for (auto i = 0u; i < str.length(); ++i) {
                        auto c = static_cast<unsigned char>(str[i]);
                        if (c >= 0 && c <= 127) {
                                ret += 1;
                        } else if ((c & 0xE0) == 0xC0) {
                                ret += 2;
                                i += 1;
                        } else if ((c & 0xF0) == 0xE0) {
                                ret += 2;
                                i += 2;
                        } else if ((c & 0xF8) == 0xF0) {
                                ret += 2;
                                i += 3;
                        } else {
                                return 0;
                        }
                }
                return ret;
        }

        pd::result check_chinese_name(const string& str) {  
                unsigned char utf[4] = {0};  
                unsigned char unicode[3] = {0};  
                pd::result res = pd::OK;  
                for (auto i = 0u; i < str.length(); i++) {  
                        if ((str[i] & 0x80) == 0) {
                                res = pd::ONLY_CHINESE_CAN_BE_USED;
                                break;
                        } else {
                                utf[0] = str[i];
                                utf[1] = str[i + 1];
                                utf[2] = str[i + 2];
                                i++;
                                i++;
                                unicode[0] = ((utf[0] & 0x0F) << 4) | ((utf[1] & 0x3C) >> 2);
                                unicode[1] = ((utf[1] & 0x03) << 6) | (utf[2] & 0x3F);
                                if(unicode[0] >= 0x4e && unicode[0] <= 0x9f){
                                        if (unicode[0] == 0x9f && unicode[1] > 0xa5) {
                                                res = pd::ONLY_CHINESE_CAN_BE_USED;
                                                break;
                                        } else {
                                                res = pd::OK;
                                        }
                                }  else {
                                        res = pd::ONLY_CHINESE_CAN_BE_USED;
                                        break;
                                }
                        }
                }
                return res;
        }

        pd::result check_name(const string& name, const set<string>& forbid_names, bool only_chinese) {
                if (only_chinese) {
                        auto result = check_chinese_name(name);
                        if (result != pd::OK) {
                                return result;
                        }
                }
                for (auto i : name) {
                        if (!is_char(i)) {
                                continue;
                        }
                        if (!is_eng_char(i) && !is_digit(i)) {
                                return pd::NAME_INVALID_CHAR;
                        }
                }
                auto length = string_length(name);
                if (length < 4) {
                        return pd::NAME_TOO_SHORT;
                }
                if (length > 12) {
                        return pd::NAME_TOO_LONG;
                }
                if (!dirty_word_filter::instance().check(name)) {
                        return pd::HAS_DIRTY_WORD;
                }
                if (forbid_names.count(name) > 0) {
                        return pd::FORBID_NAME;
                }
                return pd::OK;
        }

        pd::result check_formation_name(const string& name) {
                for (auto i : name) {
                        if (!is_char(i)) {
                                continue;
                        }
                        if (!is_eng_char(i) && !is_digit(i)) {
                                return pd::NAME_INVALID_CHAR;
                        }
                }
                auto length = string_length(name);
                if (length <= 0) {
                        return pd::NAME_TOO_SHORT;
                }
                if (length > 8) {
                        return pd::NAME_TOO_LONG;
                }
                if (!dirty_word_filter::instance().check(name)) {
                        return pd::HAS_DIRTY_WORD;
                }
                return pd::OK;
        }

        pd::result check_content(const string& content, bool no_dirty) {
                auto length = string_length(content);
                if (length > 64) {
                        return pd::CONTENT_TOO_LONG;
                }
                if (no_dirty && !dirty_word_filter::instance().check(content)) {
                        return pd::HAS_DIRTY_WORD;
                }
                return pd::OK;
        }

        vector<string> split_string(const string& str, char delim) {
                vector<string> elems;
                stringstream ss;
                ss.str(str);
                string item;
                auto inserter = back_inserter(elems);
                while (getline(ss, item, delim)) {
                        *(inserter++) = item;
                }
                return elems;
        }

        bool replace_string(string& str, const string& from, const string& to) {
                auto start_pos = str.find(from);
                if(start_pos == std::string::npos) {
                        return false;
                }
                str.replace(start_pos, from.length(), to);
                return true;
        }

        bool str_compare_value(const string& op, int v1, int v2) {
                if (op == "<") {
                        return v1 < v2;
                } else if (op == ">") {
                        return v1 > v2;
                } else if (op == "<=") {
                        return v1 <= v2;
                } else if (op == ">=") {
                        return v1 >= v2;
                } else if (op == "!=") {
                        return v1 != v2;
                } else if (op == "=" || op == "==") {
                        return v1 == v2;
                }
                return false;
        }

        string bytes_to_string(size_t bytes) {
                if (bytes >= MB) {
                        return to_string(bytes / MB) + "MB";
                } else if (bytes >= KB) {
                        return to_string(bytes / KB) + "KB";
                } else {
                        return to_string(bytes) + "B";
                }
        }

}
