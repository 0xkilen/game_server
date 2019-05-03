#pragma once

#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include <string>
#include <memory>

using namespace std;

namespace nora {
        namespace mirror {

                class mirrord : public enable_shared_from_this<mirrord> {
                public:
                        mirrord(const shared_ptr<service_thread>& st);
                        void start();
                        void stop();
                private:
                        void register_signals();

                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
