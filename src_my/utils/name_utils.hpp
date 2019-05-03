#pragma once

#include "config/utils.hpp"
#include "config/common_ptts.hpp"
#include <string>

using namespace std;

namespace nora {

        template <typename T>
        string random_name(uint32_t start_name_pool, const T& exclude_names) {
                string ret;

                const auto& first = PTTS_GET(name_pool, start_name_pool);
                const auto& second = PTTS_GET(name_pool, start_name_pool + 1);
                const auto& third = PTTS_GET(name_pool, start_name_pool + 2);

                while (true) {
                        ret.clear();
                        ret += first.chars(rand() % first.chars_size());
                        ret += second.chars(rand() % second.chars_size());
                        ret += third.chars(rand() % third.chars_size());
                        if (exclude_names.count(ret) == 0) {
                                break;
                        }
                }

                return ret;
        }

}
