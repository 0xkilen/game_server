#pragma once

#include "merger_base.hpp"
#include "proto/data_rank.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class rank_merger : public merger_base {
        public:
                static rank_merger& instance() {
                        static rank_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_rank(pd::rank_type type);
                void merge(pd::rank_type type);
                void normal_merge(pd::rank_type type);
                void chaotang_merge(pd::rank_type type);

                uint64_t rank_gid_ = 0;
                unique_ptr<pd::rank> from_rank_;
                unique_ptr<pd::rank> to_rank_;
        };

}
