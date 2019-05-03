#pragma once

#include "merger_base.hpp"
#include "proto/data_guanpin.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class guanpin_merger : public merger_base {
        public:
                static guanpin_merger& instance() {
                        static guanpin_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_rank();
                void merge_rank();
                void load_next_page();
                void on_db_get_guans_done(const shared_ptr<db::message>& msg);
                void send_mail(uint64_t role, uint32_t mail_pttid, const pd::dynamic_data& dynamic_data);
                pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data);

                int cur_page_ = 0;
                int page_size_ = 50;
                int adding_count_ = 0;
                unique_ptr<pd::guanpin> from_guanpin_;
                unique_ptr<pd::guanpin> to_guanpin_;
        };

}
