#pragma once

#include "proto/data_base.pb.h"
#include <string>

using namespace std;
namespace pd = proto::data;

namespace nora {
        pd::result parse_str_version_to_vector(const string& version, vector<uint32_t>& version_vec);
        int32_t compare_client_version(const vector<uint32_t> version_a, const vector<uint32_t> version_b);
}
