#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class activity_mgr_merger : public merger_base {
        public:
                static activity_mgr_merger& instance() {
                        static activity_mgr_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_mgr();
                void merge_mgr();
                void on_db_save_activity_mgr_done(const shared_ptr<db::message>& msg);

                int adding_count_ = 0;
                unique_ptr<pd::activity_mgr> from_activity_mgr_;
                unique_ptr<pd::activity_mgr> to_activity_mgr_;
        };

}
