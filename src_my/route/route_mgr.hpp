#pragma once

#include "proto/data_route.pb.h"
#include "utils/server_process_mgr.hpp"
#include <mutex>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        namespace route {

                class route_mgr {
                public:
                        static route_mgr& instance() {
                                static route_mgr inst;
                                return inst;
                        }
                        void start(pd::route_client_type type);
                        void init(uint32_t id);
                        void stop();
                        void update_route_table(const pd::route_table& route_table);
                        pd::route_client_type type() const;
                        pd::route_info route_info() const;
                        pd::route_table route_table() const;
                        friend ostream& operator<<(ostream& os, const route_mgr& rm) {
                                return os << "route_mgr";
                        }

                        function<void()> init_done_cb_;
                        function<void(const pd::route_table&)> route_table_updated_cb_;
                private:
                        mutable mutex lock_;
                        shared_ptr<service_thread> st_;
                        pd::route_client_type type_;
                        pd::route_info route_info_;
                        pd::route_table route_table_;
                        uint32_t id_ = 0;
                        bool stopped_ = true;
                };

                inline void route_mgr::init(uint32_t id) {
                        id_ = id;
                }

                inline pd::route_client_type route_mgr::type() const {
                        return type_;
                }

                inline pd::route_info route_mgr::route_info() const {
                        lock_guard<mutex> lock(lock_);
                        return route_info_;
                }

                inline pd::route_table route_mgr::route_table() const {
                        lock_guard<mutex> lock(lock_);
                        return route_table_;
                }

        }
}
