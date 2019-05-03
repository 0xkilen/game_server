#include "merger_base.hpp"

namespace nora {

        void merger_base::init(const string& from_ipport,
                               const string& from_user,
                               const string& from_password,
                               const string& from_database,
                               const string& to_ipport,
                               const string& to_user,
                               const string& to_password,
                               const string& to_database) {
                st_ = make_shared<service_thread>("merger_base");
                st_->start();
                from_db_ = make_shared<db::connector>(from_ipport,
                                                      from_user,
                                                      from_password,
                                                      from_database);
                from_db_->start();
                to_db_ = make_shared<db::connector>(to_ipport,
                                                    to_user,
                                                    to_password,
                                                    to_database);
                to_db_->start();
        }

        merger_base::~merger_base() {
        }

}
