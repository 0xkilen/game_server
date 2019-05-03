#pragma once

#include "net/client.hpp"
#include "net/conn.hpp"
#include "proto/processor.hpp"
#include "proto/ss_dbcache.pb.h"
#include <string>
#include <memory>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace scene {

                class dbcache : public proto::processor<dbcache, ps::base> {
                public:
                        static dbcache& instance() {
                                static dbcache inst;
                                return inst;
                        }
                        dbcache();
                        dbcache(const shared_ptr<service_thread>& st);
                        static void static_init();
                        void connect(const string& dbcached_ip, unsigned short dbcached_port);
                        void stop();
                        void process_msg(const shared_ptr<net::recvstream>& msg);
                        
                        static shared_ptr<net::sendstream> gen_create_role_req(const string& username, const pd::role& role);
                        static shared_ptr<net::sendstream> gen_find_role_req(const string& username, uint32_t sid, uint32_t gm_id);
                        static shared_ptr<net::sendstream> gen_find_role_by_gid_req(uint64_t gid);
                        static shared_ptr<net::sendstream> gen_update_role_req(const pd::role& role);

                        void send_to_dbcached(const shared_ptr<net::sendstream>& msg, bool queue = false);
                        friend ostream& operator<<(ostream& os, const dbcache& dc);

                        function<void()> init_done_cb_;
                        function<void()> stopped_cb_;
                        function<void(const string&, pd::result)> create_role_done_cb_;
                        function<void(const string&, pd::result, const pd::role&, uint32_t, uint32_t)> find_role_done_cb_;
                        function<void(uint64_t, pd::result, const string&, const pd::role&)> find_role_by_gid_done_cb_;
                        //function<void(uint32_t)> add_gladiators_done_cb_;
                private:
                        void process_create_role_rsp(const ps::base *msg);
                        void process_find_role_rsp(const ps::base *msg);

                        void add_msg_queue_timer();
                        void remove_msg_queue_timer();
                        void push_msg(const shared_ptr<net::sendstream>& msg);
                        shared_ptr<service_thread> st_;
                        shared_ptr<net::client<net::CONN>> ncl_;
                        shared_ptr<net::CONN> conn_;
                        list<shared_ptr<net::sendstream>> msg_queue_;
                        shared_ptr<timer_type> msg_queue_timer_;
                        bool stopped_ = false;
                };

        }
}
