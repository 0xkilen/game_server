#include "scene.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "utils/game_def.hpp"
#include "config/options_ptts.hpp"
#include "proto/data_yunying.pb.h"
#include "utils/assert.hpp"
#include "yunying_mgr.hpp"
#include "gm/scene_mgr_class.hpp"
#include "gm/admin_mgr_class.hpp"

namespace nora {
        namespace gm {

                scene::scene(const string& conn_name) : name_("gm-scene-" + conn_name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::s2g_gag_role_by_gid_rsp::descriptor(), &scene::process_gag_role_by_gid_rsp);
                        register_proto_handler(ps::s2g_ban_role_by_gid_rsp::descriptor(), &scene::process_ban_role_by_gid_rsp);
                        register_proto_handler(ps::s2g_remove_punish_by_gid_rsp::descriptor(), &scene::process_remove_punish_by_gid_rsp);
                        register_proto_handler(ps::s2g_kick_role_by_gid_rsp::descriptor(), &scene::process_kick_role_by_gid_rsp);
                        register_proto_handler(ps::s2g_fetch_world_chat_rsp::descriptor(), &scene::process_fetch_world_chat_rsp);
                        register_proto_handler(ps::s2g_find_role_rsp::descriptor(), &scene::process_find_role_rsp);
                        register_proto_handler(ps::s2g_recharge_rsp::descriptor(), &scene::process_recharge_rsp);
                        register_proto_handler(ps::s2g_fetch_rank_list_rsp::descriptor(), &scene::process_fetch_rank_list_rsp);
                        register_proto_handler(ps::s2g_send_mail_rsp::descriptor(), &scene::process_send_mail_rsp);
                        register_proto_handler(ps::s2g_manage_announcement_rsp::descriptor(), &scene::process_manage_announcement_rsp);
                        register_proto_handler(ps::s2g_fetch_log_rsp::descriptor(), &scene::process_fetch_log_rsp);
                        register_proto_handler(ps::s2g_fetch_punished_rsp::descriptor(), &scene::process_fetch_punished_rsp);
                        register_proto_handler(ps::s2g_fetch_sent_mail_rsp::descriptor(), &scene::process_fetch_sent_mail_rsp);
                        register_proto_handler(ps::s2g_fetch_login_record_rsp::descriptor(), &scene::process_fetch_login_record_rsp);
                        register_proto_handler(ps::s2g_fetch_recharge_record_rsp::descriptor(), &scene::process_fetch_recharge_record_rsp);
                        register_proto_handler(ps::s2g_fetch_currency_record_rsp::descriptor(), &scene::process_fetch_currency_record_rsp);
                        register_proto_handler(ps::s2g_fetch_role_list_rsp::descriptor(), &scene::process_fetch_role_list_rsp);
                        register_proto_handler(ps::s2g_add_stuff_rsp::descriptor(), &scene::process_add_stuff_rsp);
                        register_proto_handler(ps::s2g_dec_stuff_rsp::descriptor(), &scene::process_dec_stuff_rsp);
                        register_proto_handler(ps::s2g_operates_activity_rsp::descriptor(), &scene::process_operates_activity_rsp);

                        register_proto_handler(ps::s2g_admin_role_online_notice::descriptor(), &scene::process_admin_role_online_notice);
                        register_proto_handler(ps::s2g_admin_role_offline_notice::descriptor(), &scene::process_admin_role_offline_notice);
                        register_proto_handler(ps::s2g_admin_role_register_count_update_notice::descriptor(), &scene::process_admin_role_register_count_update_notice);

                }

                string scene::base_name() {
                        return "scene";
                }

                void scene::process_gag_role_by_gid_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_gag_role_by_gid_rsp::sgrbgr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().gag_role_by_gid_done(rsp.sid(), rsp.result());
                        } else {
                                admin_mgr_class::instance().admin_edit_role_by_gid_done(rsp.sid(), rsp.result(), pd::AERT_GAG);
                        }
                }

                void scene::process_ban_role_by_gid_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_ban_role_by_gid_rsp::sbrbgr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().ban_role_by_gid_done(rsp.sid(), rsp.result());
                        } else {
                                admin_mgr_class::instance().admin_edit_role_by_gid_done(rsp.sid(), rsp.result(), pd::AERT_BAN);
                        }
                }

                void scene::process_remove_punish_by_gid_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_remove_punish_by_gid_rsp::srpbgr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().remove_punish_by_gid_done(rsp.sid(), rsp.result());
                        } else {
                                admin_mgr_class::instance().admin_edit_role_rollback_by_gid_done(rsp.sid(), rsp.result(), rsp.type());
                        }
                }

                void scene::process_kick_role_by_gid_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_kick_role_by_gid_rsp::skrbgr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().kick_role_by_gid_done(rsp.sid(), rsp.result());
                        } else {
                                admin_mgr_class::instance().admin_edit_role_by_gid_done(rsp.sid(), rsp.result(), pd::AERT_KICK);
                        }
                }

                void scene::process_fetch_world_chat_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_world_chat_rsp::sfwcr);
                        yunying_mgr::instance().fetch_world_chat_done(rsp.sid(), rsp.world_chat_array(), rsp.result());
                }

                void scene::process_find_role_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_find_role_rsp::sfrr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().find_role_done(rsp.sid(), rsp.role(), rsp.result());
                        } else {
                                admin_mgr_class::instance().get_role_info_done(rsp.sid(), rsp.server_id(), rsp.role_data(), rsp.result());
                        }

                }

                void scene::process_fetch_rank_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_rank_list_rsp::sfrlr);
                        yunying_mgr::instance().fetch_rank_list_done(rsp.sid(), rsp.result(), rsp.rank_list_array());
                }

                void scene::process_recharge_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_recharge_rsp::srr);
                        yunying_mgr::instance().recharge_done(rsp.sid(), rsp.result());
                }

                void scene::process_send_mail_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_send_mail_rsp::ssmr);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().send_mail_done(rsp.sid(), rsp.result(), rsp.roles());
                        } else {
                                admin_mgr_class::instance().admin_send_mail_done(rsp.sid(), rsp.result());
                        }
                }

                void scene::process_manage_announcement_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_manage_announcement_rsp::smar);
                        yunying_mgr::instance().manage_announcement_done(rsp.sid(), rsp.result(), vector<int>());
                }

                void scene::process_fetch_log_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_log_rsp::sflr);
                        yunying_mgr::instance().fetch_log_done(rsp.sid(), rsp.result(), rsp.item_changes());
                }

                void scene::process_fetch_punished_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_punished_rsp::sfpr);
                        yunying_mgr::instance().fetch_punished_done(rsp.sid(), rsp.result(), rsp.fetch_punished_infos());
                }

                void scene::process_fetch_sent_mail_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_sent_mail_rsp::sfsmr);
                        yunying_mgr::instance().fetch_sent_mail_done(rsp.sid(), rsp.result(), rsp.fetch_sent_mail_infos());
                }

                void scene::process_fetch_login_record_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_login_record_rsp::sflrr);
                        yunying_mgr::instance().fetch_login_record_done(rsp.sid(), rsp.result(), rsp.fetch_login_records());
                }

                void scene::process_fetch_recharge_record_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_recharge_record_rsp::sfrrr);
                        yunying_mgr::instance().fetch_recharge_record_done(rsp.sid(), rsp.result(), rsp.fetch_recharge_records());
                }

                void scene::process_fetch_currency_record_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_currency_record_rsp::sfcrr);
                        yunying_mgr::instance().fetch_currency_record_done(rsp.sid(), rsp.result(), rsp.fetch_currency_records());
                }

                void scene::process_fetch_role_list_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_fetch_role_list_rsp::sfrlr);
                        pd::yunying_role_simple_info yrsi;
                        if (rsp.has_role_simple_info()) {
                                yrsi = rsp.role_simple_info();
                        }
                        scene_mgr_class::instance().set_role_list(rsp.sid(), yrsi);
                }

                void scene::process_add_stuff_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_add_stuff_rsp::sasr);
                        yunying_mgr::instance().add_stuff_done(rsp.sid(), rsp.result());
                }

                void scene::process_dec_stuff_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_dec_stuff_rsp::sdsr);
                        yunying_mgr::instance().dec_stuff_done(rsp.sid(), rsp.result());
                }

                void scene::process_operates_activity_rsp(const ps::base *msg) {
                        const auto& rsp = msg->GetExtension(ps::s2g_operates_activity_rsp::soar);
                        if (rsp.sid() <= MAX_GMD_SID) {
                                yunying_mgr::instance().operates_activity_done(rsp.sid(), rsp.result());
                        } else {
                                admin_mgr_class::instance().operates_activity_done(rsp.sid(), rsp.result());
                        }
                }

                void scene::process_admin_role_online_notice(const ps::base *msg) {
                        const auto& notice = msg->GetExtension(ps::s2g_admin_role_online_notice::saron);
                        admin_mgr_class::instance().player_online(notice.server_id(), notice.role_gid());
                }

                void scene::process_admin_role_offline_notice(const ps::base *msg) {
                        const auto& notice = msg->GetExtension(ps::s2g_admin_role_offline_notice::saron);
                        admin_mgr_class::instance().player_offline(notice.server_id(), notice.role_gid());
                }

                void scene::process_admin_role_register_count_update_notice(const ps::base *msg) {
                        const auto& notice = msg->GetExtension(ps::s2g_admin_role_register_count_update_notice::sarrcun);
                        admin_mgr_class::instance().player_register_count_update(notice.server_id(), notice.register_count());
                }

                ostream& operator<<(ostream& os, const scene& s) {
                        return os << s.name_;
                }

        }
}
