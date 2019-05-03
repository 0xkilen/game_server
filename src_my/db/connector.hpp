#pragma once

#include "utils/log.hpp"
#include "utils/service_thread.hpp"
#include "message.hpp"
#include <mysql_driver.h>
#include <string>
#include <memory>
#include <atomic>
#include <boost/core/noncopyable.hpp>

using namespace std;

namespace nora {
namespace db {

        class connector : private boost::noncopyable {
        public:
                connector(const string& ipport,
                          const string& user,
                          const string& passwd,
                          const string& database);
                ~connector();
                void start();
                void connect();
                void push_message(const shared_ptr<message>& msg, const shared_ptr<service_thread>& st = nullptr);
                void stop();
        private:
                unique_ptr<sql::Connection> conn_;
                shared_ptr<service_thread> st_;
                const string ipport_;
                const string user_;
                const string passwd_;
                const string database_;
                map<string, shared_ptr<sql::PreparedStatement>> pstmts_;
                bool stopped_ = false;
        };

}
}
