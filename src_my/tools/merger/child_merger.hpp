#pragma once

#include "merger_base.hpp"
#include "proto/data_child.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class child_merger : public merger_base {
        public:
                static child_merger& instance() {
                        static child_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_mgr();
                void merge_mgr();
                void load_next_page();
                void on_db_get_children_done(const shared_ptr<db::message>& msg);

                int cur_page_ = 0;
                int page_size_ = 50;
                int adding_count_ = 0;
                unique_ptr<pd::child_mgr> from_child_mgr_;
                unique_ptr<pd::child_mgr> to_child_mgr_;
        };

}
