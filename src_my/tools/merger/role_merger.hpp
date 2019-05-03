#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"

namespace nora {

        class role_merger : public merger_base {
        public:
                static role_merger& instance() {
                        static role_merger inst;
                        return inst;
                }
                void start();
        private:
                void load_next_page();
                void on_db_get_roles_done(const shared_ptr<db::message>& msg);
                void on_db_find_fix_name_to_role_done(const shared_ptr<db::message>& msg);
                void fix_to_names();
                void send_mail(uint64_t role, uint32_t mail_id);
                pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data);

                int cur_page_ = 0;
                int page_size_ = 20;
                int adding_count_ = 0;
                set<uint64_t> fix_name_to_roles_;
        };

}
