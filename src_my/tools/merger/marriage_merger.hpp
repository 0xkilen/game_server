#pragma once

#include "merger_base.hpp"
#include "proto/data_marriage_data.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class marriage_merger : public merger_base {
        public:
                static marriage_merger& instance() {
                        static marriage_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void merge();

                unique_ptr<pd::marriage_data> from_marriage_;
                unique_ptr<pd::marriage_data> to_marriage_;
        };

}
