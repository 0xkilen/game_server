#include "admin.hpp"
#include "route/route_mgr.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "scene_mgr_class.hpp"
#include "admin_mgr_class.hpp"
#include "login_mgr_class.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_gm.pb.h"
#include "utils/time_utils.hpp"

namespace nora {
        namespace gm {

                admin::admin(const string& conn_name) : name_("gm-client-" + conn_name) {
                }

                void admin::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::c2g_login::descriptor(), &admin::process_login);
                        register_proto_handler(ps::c2g_get_role_info_by_gid_req::descriptor(), &admin::process_get_role_info_by_gid_req);
                        register_proto_handler(ps::c2g_get_role_info_by_name_req::descriptor(), &admin::process_get_role_info_by_name_req);
                        register_proto_handler(ps::c2g_admin_start_activity_req::descriptor(), &admin::process_admin_start_activity_req);
                        register_proto_handler(ps::c2g_admin_edit_role_by_gid_req::descriptor(), &admin::process_admin_edit_role_by_gid_req);
                        register_proto_handler(ps::c2g_admin_edit_role_rollback_by_gid_req::descriptor(), &admin::process_admin_edit_role_rollback_by_gid_req);
                        register_proto_handler(ps::c2g_admin_send_mail_req::descriptor(), &admin::process_admin_send_mail_req);
                        register_proto_handler(ps::c2g_admin_fetch_gonggao_list_req::descriptor(), &admin::process_admin_fetch_gonggao_list_req);
                        register_proto_handler(ps::c2g_admin_edit_gonggao_req::descriptor(), &admin::process_admin_edit_gonggao_req);
                        register_proto_handler(ps::c2g_admin_fetch_white_list_req::descriptor(), &admin::process_admin_fetch_white_list_req);
                        register_proto_handler(ps::c2g_admin_edit_white_list_req::descriptor(), &admin::process_admin_edit_white_list_req);
                }

                string admin::base_name() {
                        return "admin";
                }

                void admin::process_login(const ps::base *msg) {
                        ADMIN_TLOG << *this << " login admin";
                        const auto& login = msg->GetExtension(ps::c2g_login::cl);
                        admin_mgr_class::instance().login_done(id(), login.username(), login.password());
                }

                void admin::process_get_role_info_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_get_role_info_by_gid_req::cgribgr);
                        if (req.server_ids_size() == 0 || req.gid() == 0) {
                                return;
                        }
                        ADMIN_TLOG << *this << " get role by gid " << req.gid();
                        for (const auto& i : req.server_ids()) {
                                scene_mgr_class::instance().find_role_by_gid(id(), i, req.gid());
                        }
                }

                void admin::process_get_role_info_by_name_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_get_role_info_by_name_req::cgribnr);
                        if (req.server_ids_size() == 0 || req.name() == "") {
                                return;
                        }
                        ADMIN_TLOG << *this << " get role by name " << req.name();
                        for (const auto& i : req.server_ids()) {
                                scene_mgr_class::instance().find_role_by_rolename(id(), i, req.name());
                        }
                }

                void admin::process_admin_start_activity_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_start_activity_req::casar);
                        ADMIN_TLOG << *this << " start activity, type " << pd::admin_activity_type_Name(req.activity_type()) << " activity_id " << req.activity_id();
                        vector<string> server_ids;
                        for (const auto& i : req.server_ids()) {
                                server_ids.push_back(to_string(i));
                        }
                        scene_mgr_class::instance().operates_activity(id(), req.activity_id(), refresh_day_from_time_t(req.start_time()), req.duration(), req.activity_type(), server_ids, req.opened());
                }

                void admin::process_admin_edit_role_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_edit_role_by_gid_req::caerbgr);
                        if (req.gid() == 0 || req.server_id() == 0) {
                                return;
                        }

                        ADMIN_TLOG << *this << " edit role by gid " << req.gid();
                        auto end_time = system_clock::to_time_t(system_clock::now()) + req.until_time() * MINUTE;
                        switch (req.edit_type()) {
                        case pd::AERT_GAG:
                                scene_mgr_class::instance().gag_role_by_gid(id(), req.server_id(), req.gid(), end_time, "gag");
                                break;
                        case pd::AERT_BAN:
                                scene_mgr_class::instance().ban_role_by_gid(id(), req.server_id(), req.gid(), end_time, "ban");
                                break;
                        case pd::AERT_KICK:
                                scene_mgr_class::instance().kick_role_by_gid(id(), req.server_id(), req.gid(), "kick");
                                break;
                        default:
                                break;
                        }
                }

                void admin::process_admin_edit_role_rollback_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_edit_role_rollback_by_gid_req::caerrbgr);
                        if (req.gid() == 0 || req.server_id() == 0) {
                                return;
                        }

                        ADMIN_TLOG << *this << " edit role rollback by gid " << req.gid();
                        switch (req.edit_type()) {
                        case pd::AERT_GAG:
                                scene_mgr_class::instance().remove_gag_role_by_gid(id(), req.server_id(), req.gid());
                                break;
                        case pd::AERT_BAN:
                                scene_mgr_class::instance().remove_ban_role_by_gid(id(), req.server_id(), req.gid());
                                break;
                        default:
                                break;
                        }
                }

                void admin::process_admin_send_mail_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_send_mail_req::casmr);
                        ADMIN_TLOG << *this << " admin send mail";
                        if (req.gids_size() == 0) {
                                return;
                        }

                        pd::gid_array gids;
                        for (const auto& i : req.gids()) {
                                gids.add_gids(i);
                        }
                        scene_mgr_class::instance().send_mail(id(), req.server_id(), 0, gids, req.mail_id(), req.title(), req.content(), req.events());
                }

                void admin::process_admin_fetch_gonggao_list_req(const ps::base *msg) {
                        msg->GetExtension(ps::c2g_admin_fetch_gonggao_list_req::cafglr);
                        login_mgr_class::instance().fetch_gonggao_list_req(id());
                }

                void admin::process_admin_edit_gonggao_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_edit_gonggao_req::caegr);

                        pd::gonggao gonggao;
                        gonggao.set_publish(req.publish());
                        gonggao.set_type(req.type());
                        gonggao.set_system_id(req.system_id());
                        gonggao.set_title(req.title());
                        gonggao.set_content(req.content());
                        gonggao.set_channel_id(req.channel_id());

                        login_mgr_class::instance().login_gonggao(id(), gonggao);
                }

                void admin::process_admin_fetch_white_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_fetch_white_list_req::cafwlr);
                        if (req.server_id() == 0) {
                                return;
                        }

                        login_mgr_class::instance().fetch_white_list(id(), req.server_id());
                }

                void admin::process_admin_edit_white_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::c2g_admin_edit_white_list_req::caewlr);
                        if (req.server_id() == 0 || req.white_ips_size() == 0) {
                                return;
                        }

                        set<string> ips;
                        for (const auto& i : req.white_ips()) {
                                ips.insert(i);
                        }
                        if (req.publish()) {
                                login_mgr_class::instance().add_to_white_list(id(), req.server_id(), ips);
                        } else {
                                login_mgr_class::instance().remove_from_white_list(id(), req.server_id(), ips);
                        }
                }

        }
}
