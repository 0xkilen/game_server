#pragma once

#include "proto/data_base.pb.h"
#include <string>
#include <vector>

using namespace std;
namespace pd = proto::data;

namespace nora {

        bool is_char(int c);
        bool is_digit(int c);
        bool is_eng_char(int c);

        size_t string_length(const string& str);

        pd::result check_name(const string& name, const set<string>& forbid_names, bool only_chinese = true);
        pd::result check_chinese_name(const string& name);
        pd::result check_formation_name(const string& name);
        pd::result check_content(const string& content, bool no_dirty = true);

        vector<string> split_string(const string& str, char delim = ' ');
        bool replace_string(string& str, const string& from, const string& to);
        bool str_compare_value(const string& op, int v1, int v2);

        string bytes_to_string(size_t bytes);

}
