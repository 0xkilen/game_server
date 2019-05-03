#pragma once

#include "scene.hpp"
#include "config/options_ptts.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include <boost/asio.hpp>
#include <string>
#include <memory>

using namespace std;
namespace ba = boost::asio;

namespace nora {
        namespace dbcache {

                class dbcached {
                public:
                        dbcached(const shared_ptr<service_thread>& st);
                        void start();
                        void stop();
                private:
                        void register_signals();
                        void add_instance_count_timer();
                        shared_ptr<service_thread> st_;
                        shared_ptr<scene> scene_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
