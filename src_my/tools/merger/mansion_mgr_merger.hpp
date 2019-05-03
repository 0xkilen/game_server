#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include "proto/data_mansion.pb.h"
#include <string>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class mansion_mgr_merger : public merger_base {
        public:
                static mansion_mgr_merger& instance() {
                        static mansion_mgr_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void merge();

                unique_ptr<pd::mansion_mgr> from_mansion_mgr_;
                unique_ptr<pd::mansion_mgr> to_mansion_mgr_;
        };

}
