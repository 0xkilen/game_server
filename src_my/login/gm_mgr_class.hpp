#pragma once

#include "net/conn.hpp"
#include "net/stream.hpp"
#include "net/server.hpp"
#include "db/message.hpp"
#include "proto/processor.hpp"
#include "proto/ss_base.pb.h"
#include "proto/data_yunying.pb.h"
#include "gm.hpp"
#include <map>
#include <memory>
#include <boost/core/noncopyable.hpp>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;

namespace nora {
        class service_thread;
        namespace login {

                class gm_mgr_class : private boost::noncopyable {
                public:
                        gm_mgr_class();
                        static gm_mgr_class& instance() {
                                static gm_mgr_class inst;
                                return inst;
                        }
                        void start();
                        void stop();
                        void send_gonggao_done(uint32_t sid, uint32_t gmd_id);
                        void add_to_white_list_done(uint32_t sid, uint32_t gmd_id, pd::result result);
                        void remove_from_white_list_done(uint32_t sid, uint32_t gmd_id, pd::result result);
                        void fetch_white_list_done(uint32_t sid, uint32_t gmd_id, const pd::white_list_array& white_lists, pd::result result);
                        void login_gonggao(uint32_t sid, uint32_t gmd_id, const pd::gonggao& gonggao);
                        void add_to_white_list(uint32_t sid, uint32_t server_id, uint32_t gmd_id, const set<string>& ips);
                        void remove_from_white_list(uint32_t sid, uint32_t gmd_id, uint32_t server_id, const set<string>& ips);
                        void fetch_white_list(uint32_t sid, uint32_t gmd_id);

                        void fetch_gonggao_list(uint32_t sid, uint32_t gmd_id);
                        void fetch_gonggao_list_done(uint32_t aid, uint32_t gmd_id, const pd::gonggao_array& gonggao_list);

                        function<void(uint32_t, uint32_t, const pd::gonggao&)> login_gonggao_func_;
                        function<void(uint32_t, uint32_t)> fetch_gonggao_list_func_;
                        function<void(uint32_t, uint32_t, uint32_t, const set<string>&)> add_to_white_list_func_;
                        function<void(uint32_t, uint32_t, uint32_t, const set<string>&)> remove_from_white_list_func_;
                        function<void(uint32_t, uint32_t)> fetch_white_list_func_;

                        friend ostream& operator<<(ostream& os, const gm_mgr_class& gm);
                private:
                        bool has_gm_conn(uint32_t sid) const;
                        void remove_sid(uint32_t sid);
                        const shared_ptr<gm> get_gm(uint32_t sid);

                        string name_;
                        shared_ptr<service_thread> st_;
                };
        }
}
