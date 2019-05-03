#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"

using namespace std;

namespace nora {

        class fief_merger : public merger_base {
        public:
                static fief_merger& instance() {
                        static fief_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_page();
                void on_db_get_fiefs_done(const shared_ptr<db::message>& msg);

                int cur_page_ = 0;
                int page_size_ = 50;
                int adding_count_ = 0;
        };

}
