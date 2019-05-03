#pragma once

#include "utils/service_thread.hpp"
#include "db/connector.hpp"

namespace nora {

        class merger_base {
        public:
                virtual void init(const string& from_ipport,
                                  const string& from_user,
                                  const string& from_password,
                                  const string& from_database,
                                  const string& to_ipport,
                                  const string& to_user,
                                  const string& to_password,
                                  const string& to_database);
                virtual ~merger_base();
                function<void()> done_cb_;
        protected:
                shared_ptr<service_thread> st_;
                shared_ptr<db::connector> from_db_;
                shared_ptr<db::connector> to_db_;
        };

}
