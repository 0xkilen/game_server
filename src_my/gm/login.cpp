#include "login.hpp"
#include "login/log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "utils/game_def.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_gm.pb.h"
#include "proto/ss_login.pb.h"
#include "utils/assert.hpp"
#include "gm/login_mgr_class.hpp"
#include "admin_mgr_class.hpp"

namespace nora {
        namespace gm {

                login::login(const string& conn_name) : name_("" + conn_name) {
                }

                void login::static_init() {
                        login::register_proto_handler(ps::l2g_gonggao_rsp::descriptor(), &login::process_gonggao_rsp);
                        login::register_proto_handler(ps::l2g_add_to_white_list_rsp::descriptor(), &login::process_add_to_white_list_rsp);
                        login::register_proto_handler(ps::l2g_remove_from_white_list_rsp::descriptor(), &login::process_remove_from_white_list_rsp);
                        login::register_proto_handler(ps::l2g_fetch_white_list_rsp::descriptor(), &login::process_fetch_white_list_rsp);
                        login::register_proto_handler(ps::l2g_fetch_gonggao_list_rsp::descriptor(), &login::process_fetch_gonggao_list_rsp);
                }

                string login::base_name() {
                        return "login";
                }

                void login::process_gonggao_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::l2g_gonggao_rsp::lgr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                login_mgr_class::instance().login_gonggao_done(rsp.sid());
                        } else {
                                admin_mgr_class::instance().admin_edit_gonggao_done(rsp.sid(), pd::OK);
                        }
                }

                void login::process_fetch_gonggao_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::l2g_fetch_gonggao_list_rsp::lfglr);
                        admin_mgr_class::instance().admin_fetch_gonggao_list_done(rsp.aid(), rsp.gonggao_list());
                }

                void login::process_add_to_white_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::l2g_add_to_white_list_rsp::latwlr);
                        login_mgr_class::instance().add_to_white_list_done(rsp.sid(), rsp.result());
                }

                void login::process_remove_from_white_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::l2g_remove_from_white_list_rsp::lrfwlr);
                        login_mgr_class::instance().remove_from_white_list_done(rsp.sid(), rsp.result());
                }

                void login::process_fetch_white_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::l2g_fetch_white_list_rsp::lfwlr);
                        login_mgr_class::instance().fetch_white_list_done(rsp.sid(), rsp.result(), rsp.white_lists());
                }

                ostream& operator<<(ostream& os, const login& lg) {
                        return os << lg.name_;
                }
        }
}
