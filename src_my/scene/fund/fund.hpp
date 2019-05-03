#pragma once

#include "db/connector.hpp"
#include "utils/assert.hpp"
#include "utils/service_thread.hpp"
#include "proto/data_activity.pb.h"
#include <list>
#include <map>
#include <mutex>
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class fund : public enable_shared_from_this<fund> {
                public:
                        fund(const shared_ptr<service_thread>& st);
                        void start();
                        void stop();
                        uint32_t activity_fund() const;
                        void activity_add_fund(bool additional_count);
                        void update_activity_fund();
                        void gm_activity_add_fund(uint32_t count);
                private:
                        shared_ptr<service_thread> st_;
                        mutable mutex lock_;
                        uint32_t fund_ = 0;
                        shared_ptr<db::connector> gamedb_;
                        shared_ptr<timer_type> timer_;
                };

                inline uint32_t fund::activity_fund() const {
                        lock_guard<mutex> lock(lock_);
                        return fund_;
                }

        }
}
