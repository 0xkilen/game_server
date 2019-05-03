#include "gm_mgr_class.hpp"
#include "gm.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_gm.pb.h"
#include "proto/ss_login.pb.h"
#include "utils/assert.hpp"
#include "utils/client_process_mgr.hpp"

namespace nora {
        namespace login {

                using gm_mgr = client_process_mgr<gm>;

                gm_mgr_class::gm_mgr_class() {
                        name_ = "gm_class";
                        st_ = make_shared<service_thread>("login-gm-class");
                        st_->start();
                }

                void gm_mgr_class::start() {
                        auto ptt = PTTS_GET_COPY(options, 1u);
			gm_mgr::instance().init(ptt.login_info().id());
                        gm_mgr::instance().start(ptt.login_info().gmd_ipport().ip(), (unsigned short)(ptt.login_info().gmd_ipport().port()));
                        gm_mgr::instance().static_init();
                }

                void gm_mgr_class::stop() {
                        if (st_) {
                                st_->stop();
                                st_.reset();
                        }
                        LGM_DLOG << *this << " stop";
                }

                void gm_mgr_class::login_gonggao(uint32_t sid, uint32_t gmd_id, const pd::gonggao& gonggao) {
                        st_->async_call([this, sid, gmd_id, gonggao] {
                                this->login_gonggao_func_(sid, gmd_id, gonggao);
                        });
                }

                void gm_mgr_class::fetch_gonggao_list(uint32_t sid, uint32_t gmd_id) {
                        st_->async_call(
                                [this, sid, gmd_id] {
                                       this->fetch_gonggao_list_func_(sid, gmd_id);
                                });
                }

                void gm_mgr_class::add_to_white_list(uint32_t sid, uint32_t server_id, uint32_t gmd_id, const set<string>& ips) {
                        st_->async_call([this, sid, server_id, gmd_id, ips] {
                                this->add_to_white_list_func_(sid, server_id, gmd_id, ips);
                        });
                }

                void gm_mgr_class::remove_from_white_list(uint32_t sid, uint32_t gmd_id, uint32_t server_id, const set<string>& ips) {
                        st_->async_call([this, sid, gmd_id, server_id, ips] {
                                this->remove_from_white_list_func_(sid, gmd_id, server_id, ips);
                        });
                }

                void gm_mgr_class::fetch_white_list(uint32_t sid, uint32_t gmd_id) {
                        st_->async_call([this, sid, gmd_id] {
                                this->fetch_white_list_func_(sid, gmd_id);
                        });
                }

                void gm_mgr_class::send_gonggao_done(uint32_t sid, uint32_t gmd_id) {
                        st_->async_call([this, sid, gmd_id] {
                                if (gm_mgr::instance().has_client_process(gmd_id)) {
                                        ps::base rsp_msg;
                                        auto* rsp = rsp_msg.MutableExtension(ps::l2g_gonggao_rsp::lgr);
                                        rsp->set_sid(sid);
                                        gm_mgr::instance().send_msg(gmd_id, rsp_msg);
                                }
                        });
                }

                void gm_mgr_class::fetch_gonggao_list_done(uint32_t aid, uint32_t gmd_id, const pd::gonggao_array& gonggao_list) {
                        st_->async_call(
                                [this, aid, gmd_id, gonggao_list] {
                                        if (gm_mgr::instance().has_client_process(gmd_id)) {
                                                ps::base rsp_msg;
                                                auto* rsp = rsp_msg.MutableExtension(ps::l2g_fetch_gonggao_list_rsp::lfglr);
                                                rsp->set_aid(aid);
                                                *rsp->mutable_gonggao_list() = gonggao_list;
                                                gm_mgr::instance().send_msg(gmd_id, rsp_msg);
                                        }
                                });
                }

                void gm_mgr_class::add_to_white_list_done(uint32_t sid, uint32_t gmd_id, pd::result result) {
                        st_->async_call([this, sid, gmd_id, result] {
                                if (gm_mgr::instance().has_client_process(gmd_id)) {
                                        ps::base rsp_msg;
                                        auto* rsp = rsp_msg.MutableExtension(ps::l2g_add_to_white_list_rsp::latwlr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        gm_mgr::instance().send_msg(gmd_id, rsp_msg);
                                }
                        });
                }

                void gm_mgr_class::remove_from_white_list_done(uint32_t sid, uint32_t gmd_id, pd::result result) {
                        st_->async_call([this, sid, gmd_id, result] {
                                if (gm_mgr::instance().has_client_process(gmd_id)) {
                                        ps::base rsp_msg;
                                        auto* rsp = rsp_msg.MutableExtension(ps::l2g_remove_from_white_list_rsp::lrfwlr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        gm_mgr::instance().send_msg(gmd_id, rsp_msg);
                                }
                        });
                }

                void gm_mgr_class::fetch_white_list_done(uint32_t sid, uint32_t gmd_id, const pd::white_list_array& white_lists, pd::result result) {
                        st_->async_call([this, sid, white_lists, gmd_id, result] {
                                if (gm_mgr::instance().has_client_process(gmd_id)) {
                                        ps::base rsp_msg;
                                        auto* rsp = rsp_msg.MutableExtension(ps::l2g_fetch_white_list_rsp::lfwlr);
                                        rsp->set_sid(sid);
                                        rsp->set_result(result);
                                        auto *white_list = rsp->mutable_white_lists();
                                        for (const auto& i : white_lists.white_lists()) {
                                                *white_list->add_white_lists() = i;
                                        }
                                        gm_mgr::instance().send_msg(gmd_id, rsp_msg);
                                }
                        });
                }

                ostream& operator<<(ostream& os, const gm_mgr_class& gm_mgr_class) {
                        return os << gm_mgr_class.name_;
                }
        }
}
