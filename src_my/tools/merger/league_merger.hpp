#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include "proto/data_league.pb.h"

namespace pd = proto::data;

using namespace std;

namespace nora {

        class league_merger : public merger_base {
        public:
                static league_merger& instance() {
                        static league_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_page();
                void fix_to_names();
                void get_cities();
                void merge_cities();
                void on_db_get_leagues_done(const shared_ptr<db::message>& msg);
                void on_db_get_fix_name_to_league_done(const shared_ptr<db::message>& msg);
                void on_db_cities_done(const shared_ptr<db::message>& msg);
                void send_mail(uint64_t role, uint32_t mail_id, const pd::dynamic_data& dynamic_data);
                pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data);

                int cur_page_ = 0;
                int page_size_ = 20;
                int adding_count_ = 0;
                set<uint64_t> fix_name_to_leagues_;

                map<uint64_t, pd::city> from_cities_;
                map<uint64_t, pd::city> to_cities_;
                int waiting_get_cities_ = 0;
                int waiting_update_city_ = 0;
        };
}