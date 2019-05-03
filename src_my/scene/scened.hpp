#pragma once

#include "log_server/log_server.hpp"
//#include "yunying/bi.hpp"
//#include "scene/gm/gm_mgr_class.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include "utils/server_process_mgr.hpp"
#include <memory>

namespace pc = proto::config;
namespace ba = boost::asio;

namespace nora {
        namespace scene {

                class scened {
                public:
                        static scened& instance();
                        int server_id() const;
                        void start(const shared_ptr<service_thread>& st);
                        void stop();

                private:
                        void add_online_count_timer();
                        void add_instance_count_timer();
                        void add_hold_mirror_role_online_timer();
                        void add_sync_data_timer_for_gmd_admin();
                        void register_signals();
                        void system_control();

                        shared_ptr<service_thread> st_;
                        unique_ptr<ba::signal_set> signals_;
                };

        }
}
