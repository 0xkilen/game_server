#pragma once

#include "proto/config_league.pb.h"
#include "proto/config_child.pb.h"
#include "config/drop_ptts.hpp"
#include "utils/assert.hpp"
#include <vector>

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                class role;

                template <typename T>
                struct drop_item {
                        drop_item(T item, int weight) : item_(item), weight_(weight) {
                        }
                        inline int weight() const {
                                return weight_;
                        }
                        inline const T& item() const {
                                return item_;
                        }
                        T item_;
                        int weight_;
                };

                template <typename T>
                vector<T> drop_by_weight(const vector<drop_item<T>>& pool, uint32_t drop_count = 1, bool no_duplicate = true) {
                        if (no_duplicate) {
                                ASSERT(drop_count <= pool.size());
                        }
                        vector<T> ret;

                        int weight_sum = 0;
                        vector<int> weights;
                        for (const auto& i : pool) {
                                weight_sum += i.weight();
                                weights.push_back(i.weight());
                        }
                        if (weight_sum == 0) {
                                return ret;
                        }

                        while (drop_count > 0 && weight_sum > 0) {
                                int num = rand() % weight_sum;
                                int current_weight = 0;
                                for (auto i = 0ul; i < weights.size(); ++i) {
                                        current_weight += weights[i];
                                        if (num < current_weight) {
                                                ret.push_back(pool[i].item());
                                                if (no_duplicate) {
                                                        weight_sum -= weights[i];
                                                        weights[i] = 0;
                                                }
                                                drop_count -= 1;
                                                break;
                                        }
                                }
                        }
                        return ret;
                }

                pd::event_array drop_process(const pc::drop& ptt);
                pd::league_event_array drop_process(const pc::league_drop& ptt);
                pd::child_event_array drop_process(const pc::child_drop& ptt);
        }
}
