#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_player.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/processor.hpp"
#include "utils/client_process_base.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"

namespace pd = proto::data;
namespace ps = proto::ss;

namespace nora {
        namespace common_resource {

                class scene : public proto::processor<scene, ps::base>,
                              public client_process_base,
                              public enable_shared_from_this<scene> {
                public:
                        scene(const string& name);
                        static void static_init(const shared_ptr<service_thread>& st = nullptr);

                        friend ostream& operator<<(ostream& os, const scene& s) {
                                return os << s.name_;
                        }

                        static string base_name();

                        struct fanli {
                                map<uint32_t, uint32_t> recharge_id2count_;
                                shared_ptr<timer_type> lock_timer_;
                                string locked_by_;
                        };
                        static map<string, fanli> username2fanli_;
                        struct back {
                                shared_ptr<timer_type> lock_timer_;
                                string locked_by_;
                        };
                        static map<string, back> come_backs_;
                        static function<void()> done_cb_;
                private:
                        static shared_ptr<db::connector> db_;

                        void process_lock_fanli_req(const ps::base *msg);
                        void process_remove_fanli(const ps::base *msg);
                        void process_lock_come_back_req(const ps::base *msg);
                        void process_remove_come_back(const ps::base *msg);

                        const string name_;
                };

        }
}
