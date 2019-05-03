#pragma once

#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>

using namespace std;

namespace nora {
        namespace common_resource {

                class common_resourced {
                public:
                        static common_resourced& instance () {
                                static common_resourced inst;
                                return inst;
                        }
                        void start();
                        void stop();
                private:
                        void register_signals();

                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
