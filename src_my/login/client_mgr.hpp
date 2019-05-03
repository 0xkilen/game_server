#pragma once

#include "client.hpp"
#include "gm_mgr_class.hpp"
#include <map>

using namespace std;
namespace pc = proto::config;
namespace pcs = proto::cs;
namespace pd = proto::data;

namespace nora {
namespace login {

        template <typename T>
        class client_mgr : public enable_shared_from_this<client_mgr<T>> {
        public:
                function<void(uint32_t, uint32_t)> send_gonggao_done_cb_;
                function<void(uint32_t, uint32_t, pd::result)> add_to_white_list_done_cb_;
                function<void(uint32_t, uint32_t, pd::result)> remove_from_white_list_done_cb_;
                function<void(uint32_t, uint32_t, const pd::white_list_array&, pd::result)> fetch_white_list_done_cb_;
                client_mgr(const string& name, const shared_ptr<service_thread>& st) :
                        name_(name),
                        nsv_(make_shared<net::server<T>>("login-clientmgr-netserver")),
                        st_(st) {
                        LOGIN_DLOG << *this << " create";
                }

                void start(const string& ip, unsigned short port) {
                        auto ptt = PTTS_GET_COPY(options, 1u);
                        logindb_ = make_shared<db::connector>(ptt.login_db().ipport(),
                                                              ptt.login_db().user(),
                                                              ptt.login_db().password(),
                                                              ptt.login_db().database());
                        logindb_->start();

                        auto db_msg = make_shared<db::message>("load_gonggao",
                                                               db::message::req_type::rt_select,
                                                               [this, self = this->shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                       auto& results = msg->results();
                                                                       for (const auto& i : results) {
                                                                               pd::gonggao_array ga;
                                                                               ASSERT(i.size() == 1);
                                                                               ga.ParseFromString(boost::any_cast<string>(i[0]));
                                                                               for (const auto& j : ga.gonggao()) {
                                                                                       if (j.has_type()) {
                                                                                               pd::gonggao gg;
                                                                                               gg.set_title(j.title());
                                                                                               gg.set_content(j.content());
                                                                                               gg.set_channel_id(j.channel_id());
                                                                                               if (j.type() == pd::BEFORE_LOGIN_ANNOUNCEMENT) {
                                                                                                       data_.gonggao_.channel_id2before_login_[j.channel_id()] = gg;
                                                                                               } else if (j.type() == pd::SERVER_ANNOUNCEMENT) {
                                                                                                       data_.gonggao_.channel_id2server_[j.channel_id()] = gg;
                                                                                               }
                                                                                       }
                                                                               }
                                                                       }
                                                               });
                        logindb_->push_message(db_msg, st_);

                        db_msg = make_shared<db::message>("load_white_list",
                                                          db::message::req_type::rt_select,
                                                          [this, self = this->shared_from_this()] (const shared_ptr<db::message>& msg) {
                                                                  auto& results = msg->results();
                                                                  if (results.size() <= 0) {
                                                                          return;
                                                                  } else {
                                                                          const auto& result = results.front();
                                                                          ASSERT(result.size() == 1);
                                                                          pd::white_list_array white_lists;
                                                                          white_lists.ParseFromString(boost::any_cast<string>(result[0]));
                                                                          for (auto i : white_lists.white_lists()) {
                                                                                  if (data_.server_id2white_lists_.count(i.serverid()) > 0) {
                                                                                          if (data_.server_id2white_lists_.at(i.serverid()).count(i.ip()) > 0) {
                                                                                                  continue;
                                                                                          } else {
                                                                                                  data_.server_id2white_lists_.at(i.serverid()).insert(i.ip());
                                                                                          }
                                                                                  } else {
                                                                                          set<string> white_list;
                                                                                          white_list.insert(i.ip());
                                                                                          data_.server_id2white_lists_[i.serverid()] = white_list;
                                                                                  }
                                                                          }
                                                                  }
                                                          });
                        logindb_->push_message(db_msg, st_);

                        auto self = this->shared_from_this();
                        nsv_->new_conn_cb_ = [this, self] (const auto& c) -> shared_ptr<service_thread> {
                                CM_DLOG << *this << " got new conn " << *c;
                                auto cl = make_shared<client<T>>("client", nsv_, c, st_);
                                conn2client_[c] = cl;
                                cl->quit_func_ = [this, self, c] () {
                                        ADD_TIMER(
                                                st_,
                                                ([this, self, c] (auto canceled, const auto& timer) {
                                                        nsv_->disconnect(c);
                                                }),
                                                100ms);
                                        conn2client_.erase(c);
                                };
                                cl->server_cb_ = [this, self, c] (auto channel_id) -> pd::gonggao {
                                        if (data_.gonggao_.channel_id2server_.count(channel_id) > 0) {
                                                return data_.gonggao_.channel_id2server_.at(channel_id);
                                        }
                                        return data_.gonggao_.channel_id2server_[0];
                                };
                                cl->before_login_cb_ = [this, self, c] (auto channel_id) -> pd::gonggao {
                                        if (data_.gonggao_.channel_id2before_login_.count(channel_id) > 0) {
                                                return data_.gonggao_.channel_id2before_login_.at(channel_id);
                                        }
                                        return data_.gonggao_.channel_id2before_login_[0];
                                };
                                cl->white_lists_cb_ = [this, self, c] () -> map<uint32_t, set<string>> {
                                        return data_.server_id2white_lists_;
                                };
                                cl->client_fetch_server_ids_func_ = [this, self, c] (const auto& username) -> set<uint32_t> {
                                        ASSERT(client_fetch_server_ids_func_);
                                        return client_fetch_server_ids_func_(username);
                                };
                                return st_;
                        };
                        nsv_->msg_cb_ = [this, self] (const auto& c, const auto& msg) {
                                CM_DLOG << *this << " got msg from conn " << *c;
                                if (conn2client_.count(c) > 0) {
                                        conn2client_.at(c)->process_msg(msg);
                                }
                        };
                        nsv_->error_cb_ = [this, self] (const auto& c) {
                                if (conn2client_.count(c) > 0) {
                                        const auto& client = conn2client_.at(c);
                                        CM_DLOG << *this << " got error from conn " << *client;
                                        client->stop();
                                        conn2client_.erase(c);
                                }
                        };

                        nsv_->start();
                        nsv_->listen(ip, port);

                        CM_ILOG << *this << " start";
                }

                void login_gonggao(uint32_t sid, uint32_t gmd_id, const pd::gonggao& gonggao) {
                        if (gonggao.publish()) {
                                if (gonggao.type() == pd::BEFORE_LOGIN_ANNOUNCEMENT) {
                                        if (gonggao.channel_id() == 0) {
                                                data_.gonggao_.channel_id2before_login_.clear();
                                        }
                                        data_.gonggao_.channel_id2before_login_[gonggao.channel_id()] = gonggao;
                                } else if (gonggao.type() == pd::SERVER_ANNOUNCEMENT) {
                                        if (gonggao.channel_id() == 0) {
                                                data_.gonggao_.channel_id2server_.clear();
                                        }
                                        data_.gonggao_.channel_id2server_[gonggao.channel_id()] = gonggao;
                                }
                        } else {
                                if (gonggao.type() == pd::BEFORE_LOGIN_ANNOUNCEMENT) {
                                        if (data_.gonggao_.channel_id2before_login_.count(gonggao.channel_id()) > 0) {
                                                data_.gonggao_.channel_id2before_login_.erase(gonggao.channel_id());
                                        }
                                } else if (gonggao.type() == pd::SERVER_ANNOUNCEMENT) {
                                        if (data_.gonggao_.channel_id2server_.count(gonggao.channel_id()) > 0) {
                                                data_.gonggao_.channel_id2server_.erase(gonggao.channel_id());
                                        }
                                }
                        }

                        auto db_msg = make_shared<db::message>("update_gonggao",
                                                db::message::req_type::rt_update,
                                                [this, self = this->shared_from_this(), gmd_id, sid] (const shared_ptr<db::message>& msg) {
                                                        this->send_gonggao_done_cb_(sid, gmd_id);
                                                        CM_DLOG << "start_update_gonggao" ;
                        });

                        string str;
                        pd::gonggao_array ga;
                        for (const auto& i : data_.gonggao_.channel_id2server_) {
                                *ga.add_gonggao() = i.second;
                        }
                        for (const auto& i : data_.gonggao_.channel_id2before_login_) {
                                *ga.add_gonggao() = i.second;
                        }
                        ga.SerializeToString(&str);
                        db_msg->push_param(str);
                        logindb_->push_message(db_msg, st_);
                }

                void fetch_gonggao_list(uint32_t aid, uint32_t gmd_id) {
                        st_->async_call(
                                [this, self = this->shared_from_this(), aid, gmd_id] {
                                        ASSERT(st_->check_in_thread());

                                        pd::gonggao_array gonggao_list;
                                        for (const auto& i : data_.gonggao_.channel_id2before_login_) {
                                                *gonggao_list.add_gonggao() = i.second;
                                        }
                                        for (const auto& i : data_.gonggao_.channel_id2server_) {
                                                *gonggao_list.add_gonggao() = i.second;
                                        }
                                        gm_mgr_class::instance().fetch_gonggao_list_done(aid, gmd_id, gonggao_list);
                                });
                }

                void add_to_white_list(uint32_t sid, uint32_t server_id, uint32_t gmd_id, const set<string>& ips) {
                        if (data_.server_id2white_lists_.count(server_id)  > 0) {
                                for (const auto& i : ips) {
                                        if (data_.server_id2white_lists_.at(server_id).count(i) <= 0) {
                                                data_.server_id2white_lists_.at(server_id).insert(i);
                                        } else {
                                                continue;
                                        }
                                }
                        } else {
                                data_.server_id2white_lists_[server_id] = ips;
                        }
                        ASSERT(add_to_white_list_done_cb_);
                        add_to_white_list_done_cb_(sid, gmd_id, pd::OK);
                        update_white_list_db();
                }

                void remove_from_white_list(uint32_t sid, uint32_t gmd_id, uint32_t server_id, const set<string>& ips) {
                        auto result = pd::OK;
                        if (data_.server_id2white_lists_.count(server_id) > 0) {
                                auto old_white_list_size = data_.server_id2white_lists_.at(server_id).size();
                                for (const auto& i : ips) {
                                        if (data_.server_id2white_lists_.at(server_id).count(i) > 0) {
                                                data_.server_id2white_lists_.at(server_id).erase(i);
                                        } else {
                                                continue;
                                        }
                                }
                                if (old_white_list_size == data_.server_id2white_lists_.at(server_id).size()) {
                                        result = pd::NOT_FOUND;
                                }
                                if (data_.server_id2white_lists_.at(server_id).empty()) {
                                        data_.server_id2white_lists_.erase(server_id);
                                }
                                update_white_list_db();
                        } else {
                                result = pd::NOT_FOUND;
                        }
                        ASSERT(remove_from_white_list_done_cb_);
                        remove_from_white_list_done_cb_(sid, gmd_id, result);
                }

                void fetch_white_list(uint32_t sid, uint32_t gmd_id) {
                        pd::white_list_array white_lists;
                        for (const auto& i : data_.server_id2white_lists_) {
                                for (const auto& j : i.second) {
                                        auto *white_list = white_lists.add_white_lists();
                                        white_list->set_serverid(i.first);
                                        white_list->set_ip(j);
                                }
                        }
                        ASSERT(fetch_white_list_done_cb_);
                        fetch_white_list_done_cb_(sid, gmd_id, white_lists, pd::OK);
                }

                void update_white_list_db() {
                        auto db_msg = make_shared<db::message>("update_white_list", db::message::req_type::rt_update);
                        pd::white_list_array white_lists;
                        for (const auto& i : data_.server_id2white_lists_) {
                                for (const auto& j : i.second) {
                                        auto *white_list = white_lists.add_white_lists();
                                        white_list->set_serverid(i.first);
                                        white_list->set_ip(j);
                                }
                        }

                        string str;
                        white_lists.SerializeToString(&str);
                        db_msg->push_param(str);
                        logindb_->push_message(db_msg, st_);
                }

                const pd::white_list_array& white_list() const {
                        pd::white_list_array wla;
                        for (const auto& i : data_.server_id2white_lists_) {
                                for (const auto& j : i.second) {
                                        auto *white_list = wla.add_white_lists();
                                        white_list->set_serverid(i.first);
                                        white_list->set_ip(j);
                                }
                        }
                        return wla;
                }

                void stop() {
                        if (nsv_) {
                                nsv_->stop();
                                nsv_.reset();
                        }
                        auto c2c = conn2client_;
                        for_each(c2c.begin(), c2c.end(),
                                [this] (const auto& i) {
                                        i.second->stop();
                                });
                        c2c.clear();
                        conn2client_.clear();
                        if (logindb_)  {
                                logindb_->stop();
                                logindb_.reset();
                        }
                        CM_DLOG << *this << " stop";
                }
                const string& name() const {
                        return name_;
                }
                friend ostream& operator<<(ostream& os, const client_mgr& cm) {
                        return os << cm.name();
                }

                function<set<uint32_t>(const string&)> client_fetch_server_ids_func_;

        private:
                const string name_;
                map<shared_ptr<T>, shared_ptr<client<T>>> conn2client_;
                shared_ptr<net::server<T>> nsv_;
                shared_ptr<service_thread> st_;
                struct {
                        struct {
                                map<uint32_t, pd::gonggao> channel_id2before_login_;
                                map<uint32_t, pd::gonggao> channel_id2server_;
                        } gonggao_;
                        map<uint32_t, set<string>> server_id2white_lists_;
                } data_;
                shared_ptr<db::connector> logindb_;
        };

}
}
