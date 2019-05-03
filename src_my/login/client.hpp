#pragma once

#include "log.hpp"
#include "utils/exception.hpp"
#include "utils/service_thread.hpp"
#include "utils/client_version.hpp"
#include "proto/data_base.pb.h"
#include "proto/cs_login.pb.h"
#include "proto/ss_base.pb.h"
#include "proto/cs_base.pb.h"
#include "proto/data_yunying.pb.h"
#include "proto/processor.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "config/serverlist_ptts.hpp"
#include "config/channel_specify_ptts.hpp"
#include "login/scene.hpp"
#include "net/server.hpp"
#include "net/stream.hpp"
#include "db/connector.hpp"
#include <memory>
#include <set>

using namespace std;
namespace pc = proto::config;
namespace pcs = proto::cs;
namespace pd = proto::data;

namespace nora {
        namespace login {

                using scene_mgr = client_process_mgr<scene>;

                template <typename T>
                class client : public enable_shared_from_this<client<T>>,
                               public proto::processor<client<T>, pcs::base> {
                public:
                        function<void()> quit_func_;
                        function<pd::gonggao(uint32_t)> before_login_cb_;
                        function<pd::gonggao(uint32_t)> server_cb_;
                        function<map<uint32_t, set<string>>()> white_lists_cb_;

                        client(const string& name, const shared_ptr<net::server<T>>& nsv, const shared_ptr<T>& c, const shared_ptr<service_thread>& st) :
                                name_(name),
                                nsv_(nsv),
                                conn_(c),
                                st_(st) {
                                this->register_proto_handler(pcs::c2l_fetch_info_req::descriptor(), &client::process_fetch_info_req);
                                ip_ = c->remote_ip();
                        }

                        void process_fetch_info_req(const pcs::base *msg) {
                                auto req = msg->GetExtension(pcs::c2l_fetch_info_req::cfir);
                                pcs::base rsp_msg;
                                auto *rsp = rsp_msg.MutableExtension(pcs::l2c_fetch_info_rsp::lfir);
                                if (req.has_username()) {
                                        ASSERT(client_fetch_server_ids_func_);
                                        auto server_ids = client_fetch_server_ids_func_(req.username());
                                        for (auto i : server_ids) {
                                                rsp->add_server_ids(i);
                                        }
                                }

                                auto gonggao_before_login = before_login_cb_(req.channel_id());
                                auto gonggao_server = server_cb_(req.channel_id());
                                auto server_id2white_list = white_lists_cb_();
                                auto serverlist_ptts = PTTS_GET_ALL_COPY(serverlist);
                                bool in_white_list = false;
                                for (const auto& i : server_id2white_list) {
                                        if (i.second.count(ip_) > 0) {
                                                in_white_list = true;
                                                if (serverlist_ptts.count(i.first) > 0) {
                                                        serverlist_ptts.at(i.first).set__in_white_list(true);
                                                }
                                        }
                                }

                                auto use_specific_list = false;
                                auto no_hot_update_url = false;
                                if (req.client_version_size() == 3) {
                                        vector<uint32_t> client_version_vec;
                                        for (auto small_version : req.client_version()) {
                                                client_version_vec.push_back(small_version);
                                        }
                                        auto all_specific_server_list = PTTS_GET_ALL_COPY(specific_server_list);
                                        for (auto& i : all_specific_server_list) {
                                                auto& server_list = i.second;
                                                for (auto j : server_list.client_versions()) {
                                                        vector<uint32_t> specific_version_vec;
                                                        if (parse_str_version_to_vector(j, specific_version_vec) == pd::OK) {
                                                                if (compare_client_version(client_version_vec, specific_version_vec) == 0) {
                                                                        use_specific_list = true;
                                                                }
                                                                break;
                                                        }
                                                }

                                                if (use_specific_list) {
                                                        for (auto j : server_list.server_ids()) {
                                                                if (PTTS_HAS(serverlist, j)) {
                                                                        ASSERT(serverlist_ptts.count(j) > 0);
                                                                        *rsp->add_servers() = serverlist_ptts.at(j);
                                                                }
                                                        }
                                                        break;
                                                }
                                        }

                                        auto ptt = PTTS_GET_COPY(options, 1);
                                        vector<uint32_t> hot_update_version_vec;
                                        if (parse_str_version_to_vector(ptt.login_info().hot_update_client_version(), hot_update_version_vec) == pd::OK) {
                                                if (compare_client_version(client_version_vec, hot_update_version_vec) < 0) {
                                                        no_hot_update_url = true;
                                                }
                                        }
                                }

                                if (!use_specific_list) {
                                        if (server_id2white_list.empty()) {
                                                for (const auto& i : serverlist_ptts) {
                                                        if (i.second.status() != pc::ZHUNBEI) {
                                                                for (auto j : i.second.channel_ids()) {
                                                                        if (j == req.channel_id()) {
                                                                                *rsp->add_servers() = i.second;
                                                                                break;
                                                                        }
                                                                }
                                                        }
                                                }
                                        } else {
                                                for (const auto& i : serverlist_ptts) {
                                                        for (auto j : i.second.channel_ids()) {
                                                                if (j == req.channel_id()) {
                                                                        if (i.second.status() == pc::ZHUNBEI) {
                                                                                if (i.second._in_white_list()) {
                                                                                        *rsp->add_servers() = i.second;
                                                                                }
                                                                        } else {
                                                                                *rsp->add_servers() = i.second;
                                                                        }
                                                                        break;
                                                                }
                                                        }
                                                }
                                        }
                                }

                                if (!no_hot_update_url) {
                                        auto channel_specify_ptt = PTTS_GET_ALL_COPY(channel_specify);
                                        for (const auto& i : channel_specify_ptt) {
                                                for (auto j : i.second.channel_id()) {
                                                        if (req.channel_id() == j) {
                                                                if (in_white_list) {
                                                                        rsp->set_hot_update_url(i.second.white_list_url() + i.second.white_list_version() + '/');
                                                                } else {
                                                                        rsp->set_hot_update_url(i.second.hot_update_root_url() + i.second.client_version() + '/');
                                                                }
                                                        }
                                                }
                                        }
                                }

                                if (gonggao_before_login.has_title() && gonggao_before_login.has_content()) {
                                        *rsp->mutable_before_login() = gonggao_before_login;
                                }
                                if (gonggao_server.has_title() && gonggao_server.has_content()) {
                                        *rsp->mutable_server() = gonggao_server;
                                }
                                send_to_client(rsp_msg);
                        }

                        void process_msg(const shared_ptr<net::recvstream>& msg) {
                                ASSERT(st_->check_in_thread());
                                pcs::base cs_msg;
                                ASSERT(cs_msg.ParseFromIstream(msg.get()));
                                CLIENT_TLOG << *this << " process msg\n" << cs_msg.DebugString();
                                this->process(this, &cs_msg);
                        }

                        void send_to_client(const pcs::base& msg) {
                                ASSERT(st_->check_in_thread());
                                CLIENT_TLOG << *this << " send " << msg.DebugString();
                                auto ss = make_shared<net::sendstream>();
                                msg.SerializeToOstream(ss.get());
                                nsv_->send(conn_, ss);
                        }

                        void stop() {
                                if (nsv_ && conn_) {
                                        nsv_->disconnect(conn_);
                                        conn_.reset();
                                }
                        }
                        const string& name() const {
                                return name_;
                        }
                        friend ostream& operator<<(ostream& os, const client& c) {
                                return os << c.name();
                        }
                        
                        function<set<uint32_t>(const string&)> client_fetch_server_ids_func_;

                private:
                        const string name_;
                        shared_ptr<net::server<T>> nsv_;
                        shared_ptr<T> conn_;
                        shared_ptr<service_thread> st_;
                        string ip_;
                };

        }
}
