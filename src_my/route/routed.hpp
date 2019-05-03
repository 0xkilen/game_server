#pragma once

#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include <string>
#include <memory>

using namespace std;

namespace nora {
        namespace route {

                class routed : public enable_shared_from_this<routed> {
                public:
                        routed(const shared_ptr<service_thread>& st);
                        void start();
                        void stop();
                private:
                        void register_signals();
                        void add_broadcast_route_table_timer();

                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
