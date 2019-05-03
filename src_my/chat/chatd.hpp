#pragma once

#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include <string>
#include <memory>

using namespace std;

namespace nora {
        namespace chat {

                class chatd {
                public:
                        static chatd& instance() {
                                static chatd inst;
                                return inst;
                        }
                        void start(const shared_ptr<service_thread>& st);
                        void stop();
                private:
                        void register_signals();

                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
