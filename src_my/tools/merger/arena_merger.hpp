#pragma once

#include "merger_base.hpp"
#include "proto/data_arena.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class arena_merger : public merger_base {
        public:
                static arena_merger& instance() {
                        static arena_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_rank();
                void merge_rank();
                void load_next_page();
                void load_to_db_next_page();
                void on_db_get_gladiators_done(const shared_ptr<db::message>& msg);
                void on_db_get_to_db_gladiators_done(const shared_ptr<db::message>& msg);

                int cur_page_ = 0;
                int page_size_ = 50;
                int adding_count_ = 0;
                unique_ptr<pd::arena_rank> from_arena_;
                unique_ptr<pd::arena_rank> to_arena_;
        };

}
