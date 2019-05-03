#include "gm.hpp"
#include "log.hpp"
#include "scene/scened.hpp"
#include "scene/recharge.hpp"
#include "scene/db_log.hpp"
#include "net/stream.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/yunying.hpp"
#include "config/serverlist_ptts.hpp"
#include "config/spine_ptts.hpp"
#include "config/huanzhuang_ptts.hpp"
#include "config/resource_ptts.hpp"
#include "config/item_ptts.hpp"
#include "scene/player/player_mgr.hpp"
#include "scene/activity/activity_mgr.hpp"

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                gm::gm(const string& name) : name_(name) {
                }

                void gm::static_init() {
                        register_proto_handler(ps::g2s_gag_role_by_gid_req::descriptor(), &gm::process_gag_role_by_gid_req);
                        register_proto_handler(ps::g2s_ban_role_by_gid_req::descriptor(), &gm::process_ban_role_by_gid_req);
                        register_proto_handler(ps::g2s_remove_punish_by_gid_req::descriptor(), &gm::process_remove_punish_by_gid_req);
                        register_proto_handler(ps::g2s_kick_role_by_gid_req::descriptor(), &gm::process_kick_role_by_gid_req);
                        register_proto_handler(ps::g2s_fetch_world_chat_req::descriptor(), &gm::process_fetch_world_chat_req);
                        register_proto_handler(ps::g2s_find_role_req::descriptor(), &gm::process_find_role_req);
                        register_proto_handler(ps::g2s_recharge_req::descriptor(), &gm::process_recharge_req);
                        register_proto_handler(ps::g2s_internal_recharge_req::descriptor(), &gm::process_internal_recharge_req);
                        register_proto_handler(ps::g2s_fetch_rank_list_req::descriptor(), &gm::process_fetch_rank_list_req);
                        register_proto_handler(ps::g2s_send_mail_req::descriptor(), &gm::process_send_mail_req);
                        register_proto_handler(ps::g2s_manage_announcement_req::descriptor(), &gm::process_manage_announcement_req);
                        register_proto_handler(ps::g2s_images_announcement::descriptor(), &gm::process_images_announcement);
                        register_proto_handler(ps::g2s_reissue_recharge_req::descriptor(), &gm::process_reissue_recharge_req);
                        register_proto_handler(ps::g2s_fetch_log_req::descriptor(), &gm::process_fetch_log_req);
                        register_proto_handler(ps::g2s_fetch_punished_req::descriptor(), &gm::process_fetch_punished_req);
                        register_proto_handler(ps::g2s_fetch_mail_log_req::descriptor(), &gm::process_fetch_mail_log_req);
                        register_proto_handler(ps::g2s_fetch_login_record_req::descriptor(), &gm::process_fetch_login_record_req);
                        register_proto_handler(ps::g2s_fetch_recharge_record_req::descriptor(), &gm::process_fetch_recharge_record_req);
                        register_proto_handler(ps::g2s_fetch_currency_record_req::descriptor(), &gm::process_fetch_currency_record_req);
                        register_proto_handler(ps::g2s_fetch_role_list_req::descriptor(), &gm::process_fetch_role_list_req);
                        register_proto_handler(ps::g2s_add_stuff_req::descriptor(), &gm::process_add_stuff_req);
                        register_proto_handler(ps::g2s_dec_stuff_req::descriptor(), &gm::process_dec_stuff_req);
                        register_proto_handler(ps::g2s_operates_activity_req::descriptor(), &gm::process_operates_activity_req);
                }

                void gm::process_gag_role_by_gid_req (const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_gag_role_by_gid_req::ggrbgr);
                        player_mgr_class::instance().gag_role_by_gid(req.sid(), req.gm_id(), req.gid(), req.gag_until_time(), req.reason());
                }

                void gm::process_ban_role_by_gid_req (const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_ban_role_by_gid_req::gbrbgr);
                        player_mgr_class::instance().ban_role_by_gid(req.sid(), req.gm_id(), req.gid(), req.ban_until_time(), req.reason());
                }

                void gm::process_remove_punish_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_remove_punish_by_gid_req::grpbgr);
                        if (req.type() == pd::AERT_GAG) {
                                player_mgr_class::instance().remove_gag_role_by_gid(req.sid(), req.gm_id(), req.gid());
                        } else if(req.type() == pd::AERT_BAN) {
                                player_mgr_class::instance().remove_ban_role_by_gid(req.sid(), req.gm_id(), req.gid());
                        }
                }

                void gm::process_kick_role_by_gid_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_kick_role_by_gid_req::gkrbgr);
                        player_mgr_class::instance().kick_role_by_gid(req.sid(), req.gm_id(), req.gid(), req.reason());
                }

                void gm::process_fetch_world_chat_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_world_chat_req::gfwcr);
                        player_mgr_class::instance().fetch_world_chat(req.sid(), req.gm_id());
                }

                void gm::process_find_role_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_find_role_req::gfrr);
                        if (req.has_gid()) {
                                player_mgr_class::instance().yunying_find_role_by_gid(req.sid(), req.gm_id(), req.gid());
                        } else {
                                player_mgr_class::instance().yunying_find_role_by_rolename(req.sid(), req.gm_id(), req.rolename());
                        }
                }

                void gm::process_fetch_rank_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_rank_list_req::gfrlr);
                        gm_mgr_class::instance().fetch_ranK_list(req.sid(), req.gm_id(), req.rank_type(), req.page_size());
                }

                void gm::process_internal_recharge_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_internal_recharge_req::girr);
                        player_mgr_class::instance().yunying_internal_recharge(req.sid(), req.gm_id(), req.role(), req.recharge_id());
                }

                void gm::process_send_mail_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_send_mail_req::gsmr);
                        player_mgr_class::instance().yunying_send_mail(req.sid(), req.gm_id(), req.server_id(), req.roles(), req.id(), req.title(), req.content(), req.events());
                }

                void gm::process_reissue_recharge_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_reissue_recharge_req::grrr);
                        player_mgr_class::instance().yunying_reissue_recharge(req.sid(), req.gm_id(), req.role(), req.order());
                }

                void gm::process_recharge_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_recharge_req::grr);
                        player_mgr_class::instance().yunying_recharge(req.sid(), req.gm_id(), req.order(), req.yunying_orderno(), req.currency(), req.price(), req.paytime(), req.product_name(), req.ext_info());
                }

                void gm::process_fetch_log_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_log_req::gflr);
                        gm_mgr_class::instance().fetch_log(req.sid(), req.gm_id(), "find_logs_by_gid_and_one_type", req.role(), req.item_id(), req.start_time(), req.end_time(), req.page_idx(), req.page_size());
                }

                void gm::process_manage_announcement_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_manage_announcement_req::gmar);
                        gm_mgr_class::instance().manage_announcement_done(req.sid(), req.gm_id(), req.announcement());
                }

                void gm::process_images_announcement(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_images_announcement::gia);
                        gm_mgr_class::instance().add_images_announcement(req.announcements());
                }

                void gm::process_fetch_punished_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_punished_req::gfpr);
                        string procedure;
                        if (req.role() == 0) {
                                procedure = "find_logs_by_five_type";
                        } else {
                                procedure = "find_logs_by_gid_and_five_type";
                        }
                        gm_mgr_class::instance().fetch_punished_log(req.sid(), req.gm_id(), procedure, req.role(), req.server_id(), req.start_time(), req.end_time(), req.page_idx(), req.page_size());
                }

                void gm::process_fetch_mail_log_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_mail_log_req::gfmlr);
                        string procedure = "find_mail_log_by_mail_gid";
                        if (req.role() == 0) {
                                procedure = "find_mail_log";
                        }
                        gm_mgr_class::instance().fetch_mail_log(req.sid(), req.gm_id(), procedure, req.role(), req.server_id(), req.start_time(), req.end_time(), req.page_idx(), req.page_size());
                }

                void gm::process_fetch_login_record_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_login_record_req::gflrr);
                        gm_mgr_class::instance().fetch_login_record(req.sid(), req.gm_id(), "find_logs_by_gid_and_two_type", req.role(), req.server_id(), 0, system_clock::to_time_t(system_clock::now()), req.page_idx(), req.page_size());
                }

                void gm::process_fetch_recharge_record_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_recharge_record_req::gfrrr);
                        gm_mgr_class::instance().fetch_recharge_record(req.sid(), req.gm_id(), "find_logs_by_gid_and_three_type", req.role(), req.server_id(), req.start_time(), req.end_time(), req.page_idx(), req.page_size());
                }

                void gm::process_fetch_currency_record_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_currency_record_req::gfcrr);
                        auto types = split_string(req.type(), ',');
                        vector<pd::log::log_type> currency_types;
                        for (auto i : types) {
                                pd::log::log_type log_type;
                                pd::log_log_type_Parse(i, &log_type);
                                currency_types.push_back(log_type);
                        }
                        string procedure = "find_logs_by_gid_and_one_type";
                        if (types.size() == 2) {
                                procedure = "find_logs_by_gid_and_two_type";
                        }
                        gm_mgr_class::instance().fetch_currency_record(req.sid(), req.gm_id(), procedure, req.role(), currency_types, req.server_id(), req.start_time(), req.end_time(), req.page_idx(), req.page_size());
                }

                void gm::process_fetch_role_list_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_fetch_role_list_req::gfrlr);
                        auto username = req.username();
                        auto split_username = split_string(req.username(), '_');
                        if (split_username.size() == 1) {
                                const auto& ptt = PTTS_GET(options, 1);
                                username = split_username[0] + "_" + to_string(ptt.scene_info().server_id());
                        }
                        player_mgr_class::instance().yunying_fetch_role_list(req.sid(), req.gm_id(), username);
                }

                void gm::process_add_stuff_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_add_stuff_req::gasr);
                        if (req.has_role()) {
                                player_mgr_class::instance().yunying_add_stuff_by_gid(req.sid(), req.gm_id(), req.role(), req.events());
                        } else {
                                player_mgr_class::instance().yunying_add_stuff_by_rolename(req.sid(), req.gm_id(), req.rolename(), req.events());
                        }
                }

                void gm::process_dec_stuff_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_dec_stuff_req::gdsr);
                        if (req.has_role()) {
                                player_mgr_class::instance().yunying_dec_stuff_by_gid(req.sid(), req.gm_id(), req.role(), req.events());
                        } else {
                                player_mgr_class::instance().yunying_dec_stuff_by_rolename(req.sid(), req.gm_id(), req.rolename(), req.events());
                        }
                }

                void gm::process_operates_activity_req(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::g2s_operates_activity_req::goar);
                        switch(req.type()) {
                        case pd::AAT_PRIZE_WHEEL:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_prize_wheel(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_prize_wheel(req.sid(), req.gm_id());
                                }
                                break;
                        case pd::AAT_LIMIT_PLAY:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_limit_play(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_limit_play(req.sid(), req.gm_id(), req.activity());
                                }
                                break;
                        case pd::AAT_CONTINUE_RECHARGE:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_continue_recharge(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_continue_recharge(req.sid(), req.gm_id(), req.activity());
                                }
                                break;
                        case pd::AAT_DISCOUNT_GOODS:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_discount_goods(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_discount_goods(req.sid(), req.gm_id());
                                }
                                break;
                        case pd::AAT_RECHARGE_RANK:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_recharge_rank(req.sid(), req.gm_id(), req.start_time(), req.duration());
                                } else {
                                        activity_mgr::instance().stop_recharge_rank(req.sid(), req.gm_id());
                                }
                                break;
                        case pd::AAT_LEIJI_RECHARGE:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_leiji_recharge(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_leiji_recharge(req.sid(), req.gm_id(), req.activity());
                                }
                                break;
                        case pd::AAT_LEIJI_CONSUME:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_festival(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_festival(req.sid(), req.gm_id(), req.activity());
                                }
                                break;
                        case pd::AAT_TEA_PARTY:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_tea_party(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_tea_party(req.sid(), req.gm_id());
                                }
                                break;
                        case pd::AAT_FESTIVAL:
                                if (req.open_activity()) {
                                        activity_mgr::instance().start_festival(req.sid(), req.gm_id(), req.start_time(), req.duration(), req.activity());
                                } else {
                                        activity_mgr::instance().stop_festival(req.sid(), req.gm_id(), req.activity());
                                }
                                break;
                        default:
                                break;
                        }
                }
        }
}
