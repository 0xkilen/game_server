#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/ss_base.pb.h"
#include "net/conn.hpp"
#include "net/client.hpp"
#include "db/message.hpp"
#include "login.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>
#include "proto/processor.hpp"
#include "proto/ss_gm.pb.h"

using namespace std;
namespace pd = proto::data;
namespace ps = proto::ss;
namespace pc = proto::config;

namespace nora {
        class service_thread;
        namespace gm {

                class login_mgr_class : private boost::noncopyable {
                public:
                        login_mgr_class();
                        static login_mgr_class& instance() {
                                static login_mgr_class inst;
                                return inst;
                        }
                        void start();
                        void login_gonggao(uint32_t sid, const pd::gonggao& gonggao);
                        void fetch_gonggao_list_req(uint32_t conn_id);
                        void add_to_white_list(uint32_t sid, int server_id, const set<string>& ips);
                        void remove_from_white_list(uint32_t sid, int server_id, const set<string>& ips);
                        void fetch_white_list(uint32_t sid, int server_id);
                        void login_gonggao_done(uint32_t sid);
                        void add_to_white_list_done(uint32_t sid, pd::result result);
                        void remove_from_white_list_done(uint32_t sid, pd::result result);
                        void fetch_white_list_done(uint32_t sid, pd::result result, const pd::white_list_array& white_lists);
                        void stop();
                        pd::white_list_array get_white_lists(uint32_t sid, uint32_t server_id);
                        void clear_returnd_white_lists(uint32_t sid);

                        friend ostream& operator<<(ostream& os, const login_mgr_class& sm);

                private:
                        string name_;
                        shared_ptr<service_thread> st_;
                        set<uint32_t> sids_;
                        map<uint32_t, pd::white_list_array> sid2white_lists_;
                };

                inline void login_mgr_class::clear_returnd_white_lists(uint32_t sid) {
                        if (sid2white_lists_.count(sid) > 0 && sid2white_lists_.at(sid).white_lists_size() > 0) {
                                sid2white_lists_.at(sid).clear_white_lists();
                        }
                }

        }
}
