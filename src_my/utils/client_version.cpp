#include "string_utils.hpp"
#include "client_version.hpp"

namespace pd = proto::data;

namespace nora {
        pd::result parse_str_version_to_vector(const string& version, vector<uint32_t>& version_vec) {
        	auto split_vec = split_string(version, '.');
        	if (split_vec.size() != 3) {
        		return pd::CLIENT_VERSION_INVAILD;
        	}
        	for (auto i : split_vec) {
        		version_vec.push_back(stoul(i));
        	}
        	return pd::OK;
        }

        int32_t compare_client_version(const vector<uint32_t> version_a, const vector<uint32_t> version_b) {
        	for (uint32_t i = 0; i < version_b.size(); ++i)
        	{
        		auto small_a = version_a[i];
        		auto small_b = version_b[i];
        		if (small_a == small_b) {
        			continue;
        		}
        		if (small_a > small_b) {
        			return 1;
        		}
        		else if (small_b > small_a) {
        			return -1;
        		}
        	}
        	return 0;
        }

}
