#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"

namespace nora {

        class notice_merger : public merger_base {
        public:
                static notice_merger& instance() {
                        static notice_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_page();
                void on_db_get_notices_done(const shared_ptr<db::message>& msg);

                int cur_page_ = 0;
                int page_size_ = 20;
                int adding_count_ = 0;
        };

}
