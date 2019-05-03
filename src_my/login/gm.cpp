#include "gm.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_gm.pb.h"
#include "proto/ss_login.pb.h"
#include "utils/assert.hpp"
#include "gm_mgr_class.hpp"

namespace nora {
        namespace login {

                gm::gm(const string& conn_name) : name_("login-gm" + conn_name) {
                }

                void gm::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::g2l_gonggao_req::descriptor(), &gm::process_gonggao_req);
                        register_proto_handler(ps::g2l_add_to_white_list_req::descriptor(), &gm::process_add_to_white_list_req);
                        register_proto_handler(ps::g2l_remove_from_white_list_req::descriptor(), &gm::process_remove_from_white_list_req);
                        register_proto_handler(ps::g2l_fetch_white_list_req::descriptor(), &gm::process_fetch_white_list_req);
                        register_proto_handler(ps::g2l_fetch_gonggao_list_req::descriptor(), &gm::process_fetch_gonggao_list_req);
                }

                string gm::base_name() {
                        return "gm";
                }

                void gm::process_gonggao_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2l_gonggao_req::ggr);
                        gm_mgr_class::instance().login_gonggao(req.sid(), req.gmd_id(), req.gonggao());
                }

                void gm::process_add_to_white_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2l_add_to_white_list_req::gatwlr);
                        set<string> ips;
                        for (const auto& i : req.ips()) {
                                ips.insert(i);
                        }
                        gm_mgr_class::instance().add_to_white_list(req.sid(), req.server_id(), req.gmd_id(), ips);
                }

                void gm::process_remove_from_white_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2l_remove_from_white_list_req::grfwlr);
                        set<string> ips;
                        for (const auto& i : req.ips()) {
                                ips.insert(i);
                        }
                        gm_mgr_class::instance().remove_from_white_list(req.sid(), req.gmd_id(), req.server_id(), ips);
                }

                void gm::process_fetch_white_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2l_fetch_white_list_req::gfwlr);
                        gm_mgr_class::instance().fetch_white_list(req.sid(), req.gmd_id());
                }

                void gm::process_fetch_gonggao_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2l_fetch_gonggao_list_req::gfglr);
                        gm_mgr_class::instance().fetch_gonggao_list(req.aid(), req.gmd_id());
                }

                ostream& operator<<(ostream& os, const gm& g) {
                        return os << g.name_;
                }
        }
}
