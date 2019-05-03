#include "login_mgr_class.hpp"
#include "login.hpp"
#include "login/log.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/assert.hpp"
#include "utils/game_def.hpp"
#include "proto/config_options.pb.h"
#include "config/options_ptts.hpp"
#include "utils/server_process_mgr.hpp"
#include "gm/yunying_mgr.hpp"
#include "proto/ss_login.pb.h"
#include "admin_mgr_class.hpp"

namespace nora {
        namespace gm {

                using login_mgr = server_process_mgr<login>;

                login_mgr_class::login_mgr_class() {
                        name_ = "login_mgr_class";
                        st_ = make_shared<service_thread>("login_mgr_class");
                        st_->start();
                }

                void login_mgr_class::start() {
                        const auto& ptt = PTTS_GET(options, 1);

                        login_mgr::static_init();
                        login_mgr::instance().init(ptt.gm_info().id());
                }

                void login_mgr_class::add_to_white_list(uint32_t sid, int server_id, const set<string>& ips) {
                        st_->async_call(
                                [this, sid, server_id, ips] {
                                        ASSERT(st_->check_in_thread());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2l_add_to_white_list_req::gatwlr);
                                        req->set_sid(sid);
                                        req->set_server_id(static_cast<uint32_t>(server_id));
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gmd_id(ptt.gm_info().id());
                                        for (const auto& i: ips) {
                                                req->add_ips(i);
                                        }
                                        login_mgr::instance().broadcast_msg(msg);
                                        sids_.insert(sid);
                                });
                }

                pd::white_list_array login_mgr_class::get_white_lists(uint32_t sid, uint32_t server_id) {
                        pd::white_list_array ret;
                        if (sid2white_lists_.count(sid) > 0) {
                                if (server_id != 0) {
                                        set<string> white_lists;
                                        for (const auto& i : sid2white_lists_.at(sid).white_lists()) {
                                                if (white_lists.count(i.ip()) <= 0 && server_id == i.serverid()) {
                                                        white_lists.insert(i.ip());
                                                        auto *white_list = ret.add_white_lists();
                                                        white_list->set_serverid(i.serverid());
                                                        white_list->set_ip(i.ip());
                                                }
                                        }
                                } else {
                                        map<uint32_t, set<string>> serd2whs;
                                        for (const auto& i : sid2white_lists_.at(sid).white_lists()) {
                                                if (serd2whs.count(i.serverid()) > 0) {
                                                        if (serd2whs.at(i.serverid()).count(i.ip()) > 0) {
                                                                continue;
                                                        }
                                                }
                                                serd2whs[i.serverid()].insert(i.ip());
                                                auto *white_list = ret.add_white_lists();
                                                white_list->set_serverid(i.serverid());
                                                white_list->set_ip(i.ip());
                                        }
                                }
                        }
                        return ret;
                }

                void login_mgr_class::add_to_white_list_done(uint32_t sid, pd::result result) {
                        st_->async_call(
                                [this, sid, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (sids_.count(sid) > 0) {
                                                if (sid <= MAX_GMD_SID) {
                                                        yunying_mgr::instance().add_to_white_list_done(sid, result);
                                                } else {
                                                        admin_mgr_class::instance().admin_edit_white_list_done(sid, true, result);
                                                }
                                                sids_.erase(sid);
                                        }
                                });
                }

                void login_mgr_class::login_gonggao(uint32_t sid, const pd::gonggao& gonggao) {
                        st_->async_call(
                                [this, sid, gonggao] {
                                        ASSERT(st_->check_in_thread());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2l_gonggao_req::ggr);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gmd_id(ptt.gm_info().id());
                                        req->set_sid(sid);
                                        *req->mutable_gonggao() = gonggao;
                                        login_mgr::instance().broadcast_msg(msg);
                                        sids_.insert(sid);
                                });
                }

                void login_mgr_class::login_gonggao_done(uint32_t sid) {
                        st_->async_call(
                                [this, sid] {
                                        ASSERT(st_->check_in_thread());
                                        if (sids_.count(sid) > 0) {
                                                yunying_mgr::instance().login_gonggao_done(sid);
                                                sids_.erase(sid);
                                        }
                                });
                }

                void login_mgr_class::remove_from_white_list(uint32_t sid, int server_id, const set<string>& ips) {
                        st_->async_call(
                                [this, sid, server_id, ips] {
                                        ASSERT(st_->check_in_thread());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2l_remove_from_white_list_req::grfwlr);
                                        req->set_sid(sid);
                                        req->set_server_id(static_cast<uint32_t>(server_id));
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gmd_id(ptt.gm_info().id());
                                        for (auto i : ips) {
                                                req->add_ips(i);
                                        }
                                        login_mgr::instance().broadcast_msg(msg);
                                        sids_.insert(sid);
                                });
                }

                void login_mgr_class::remove_from_white_list_done(uint32_t sid, pd::result result) {
                        st_->async_call(
                                [this, sid, result] {
                                        ASSERT(st_->check_in_thread());
                                        if (sids_.count(sid) > 0) {
                                                if (sid <= MAX_GMD_SID) {
                                                        yunying_mgr::instance().remove_from_white_list_done(sid, result);
                                                } else {
                                                        admin_mgr_class::instance().admin_edit_white_list_done(sid, false, result);
                                                }
                                                sids_.erase(sid);
                                        }
                                });
                }

                void login_mgr_class::fetch_white_list(uint32_t sid, int server_id) {
                        st_->async_call(
                                [this, sid, server_id] {
                                        ASSERT(st_->check_in_thread());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2l_fetch_white_list_req::gfwlr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gmd_id(ptt.gm_info().id());
                                        req->set_server_id(server_id);
                                        login_mgr::instance().broadcast_msg(msg);
                                        sids_.insert(sid);

                                        ADD_TIMER(
                                                st_,
                                                ([this, sid, server_id] (auto canceled, const auto& timer) {
                                                        if (!canceled && sids_.count(sid) > 0) {
                                                                if (sid <= MAX_GMD_SID) {
                                                                        yunying_mgr::instance().fetch_white_list_done(sid, pd::OK, this->get_white_lists(sid, server_id));
                                                                } else {
                                                                        admin_mgr_class::instance().admin_fetch_white_list_done(sid, pd::OK, this->get_white_lists(sid, server_id));
                                                                }
                                                                sids_.erase(sid);
                                                                this->clear_returnd_white_lists(sid);
                                                        }
                                                }),
                                                3s);
                                });
                }

                void login_mgr_class::fetch_white_list_done(uint32_t sid, pd::result result, const pd::white_list_array& white_lists) {
                        st_->async_call(
                                [this, sid, result, white_lists] {
                                        ASSERT(st_->check_in_thread());
                                        if (sids_.count(sid) > 0) {
                                                if (sid2white_lists_.count(sid) > 0) {
                                                        for (const auto& i : white_lists.white_lists()) {
                                                                *sid2white_lists_.at(sid).add_white_lists() = i;
                                                        }
                                                } else {
                                                        sid2white_lists_[sid] = white_lists;
                                                }
                                        }
                                });
                }

                void login_mgr_class::fetch_gonggao_list_req(uint32_t conn_id) {
                        st_->async_call(
                                [this, conn_id] {
                                        ASSERT(st_->check_in_thread());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2l_fetch_gonggao_list_req::gfglr);
                                        req->set_aid(conn_id);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gmd_id(ptt.gm_info().id());
                                        login_mgr::instance().random_send_msg(msg);
                                });
                }

                void login_mgr_class::stop() {
                        if (st_) {
                                st_->stop();
                        }
                        LOGIN_DLOG << *this << " stop";
                }

                ostream& operator<<(ostream& os, const login_mgr_class& lm) {
                        return os << lm.name_;
                }
        }
}
