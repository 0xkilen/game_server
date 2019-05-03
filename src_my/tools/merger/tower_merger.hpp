#pragma once

#include "merger_base.hpp"
#include "proto/data_tower.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class tower_merger : public merger_base {
        public:
                static tower_merger& instance() {
                        static tower_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_level();
                void merge();

                int cur_level_ = 1;
                bool from_tower_end_ = false;
                bool to_tower_end_ = false;
                
                unique_ptr<pd::tower_record_array> from_tower_;
                unique_ptr<pd::tower_record_array> to_tower_;
        };

}