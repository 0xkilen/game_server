#pragma once

#include "proto/data_mail.pb.h"
#include "db/connector.hpp"
#include "utils/service_thread.hpp"
#include <functional>
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class mail : public enable_shared_from_this<mail> {
                public:
                        mail(const shared_ptr<service_thread>& st);
                        void start();
                        void add_mail(uint64_t role, const pd::mail& mail) const;
                        void find_mails(uint64_t role) const;
                        void delete_mail(uint64_t gid) const;
                        void stop();

                        function<void(uint64_t, const pd::mail_array&)> find_mails_done_cb_;
                private:
                        shared_ptr<service_thread> st_;
                        shared_ptr<db::connector> gamedb_;
                };

        }
}
