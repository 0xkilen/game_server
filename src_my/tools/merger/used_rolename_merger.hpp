#pragma once

#include "merger_base.hpp"
#include "utils/service_thread.hpp"
#include <string>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class used_rolename_merger : public merger_base {
        public:
                static used_rolename_merger& instance() {
                        static used_rolename_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void on_db_load_done(const shared_ptr<db::message>& msg);

                set<string> used_rolenames_;
                int loading_count_ = 0;
        };

}
