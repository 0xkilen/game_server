#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include "proto/data_server.pb.h"

using namespace std;
namespace pd = proto::data;

namespace nora {

        class server_merger : public merger_base {
        public:
                static server_merger& instance() {
                        static server_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void merge();

                unique_ptr<pd::server> from_server_;
                unique_ptr<pd::server> to_server_;
        };

}
