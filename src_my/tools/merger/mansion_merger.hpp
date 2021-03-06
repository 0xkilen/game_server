#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"

using namespace std;

namespace nora {

        class mansion_merger : public merger_base {
        public:
                static mansion_merger& instance() {
                        static mansion_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_page();
                void on_db_get_mansions_done(const shared_ptr<db::message>& msg);

                int cur_page_ = 0;
                int page_size_ = 50;
                int adding_count_ = 0;
        };

}
