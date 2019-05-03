#include "scene_mgr_class.hpp"
#include "scene.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"
#include "utils/assert.hpp"
#include "utils/gid.hpp"
#include "utils/client_process_mgr.hpp"
#include "config/utils.hpp"
#include "config/serverlist_ptts.hpp"
#include "config/options_ptts.hpp"
#include "gm/yunying_mgr.hpp"

namespace nora {
        namespace gm {

                using scene_mgr = client_process_mgr<scene, net::CONN>;

                scene_mgr_class::scene_mgr_class() {
                        name_ = "scene_mgr_class";
                        st_ = make_shared<service_thread>("scene_mgr_class");
                        st_->start();
                }

                void scene_mgr_class::start() {
                        auto ptt = PTTS_GET_COPY(options, 1);
                        scene_mgr::static_init();
			scene_mgr::instance().init(ptt.gm_info().id());
                        scene_mgr::instance().start(ptt.gm_info().scened_ipport().ip(), (unsigned short)(ptt.gm_info().scened_ipport().port()));
                }

                void scene_mgr_class::stop() {
                        if (timer_) {
                                timer_->cancel();
                                timer_.reset();
                        }
                        if (st_) {
                                st_->stop();
                        }
                        GM_DLOG << *this << " stop";
                }

                ostream& operator<<(ostream& os, const scene_mgr_class& sm) {
                        return os << sm.name_;
                }

                void scene_mgr_class::gag_role_by_gid(uint32_t sid, int server_id, uint64_t gid, uint32_t gag_until_time, const string& reason) {
                        st_->async_call(
                                [this, sid, server_id, gid, gag_until_time, reason] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().gag_role_by_gid_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_gag_role_by_gid_req::ggrbgr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        req->set_gag_until_time(gag_until_time);
                                        req->set_reason(reason);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::remove_gag_role_by_gid(uint32_t sid, int server_id, uint64_t gid) {
                        st_->async_call(
                                [this, sid, server_id, gid] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().remove_punish_by_gid_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_remove_punish_by_gid_req::grpbgr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        req->set_type(pd::AERT_GAG);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::ban_role_by_gid(uint32_t sid, int server_id, uint64_t gid, uint32_t ban_until_time, const string& reason) {
                        st_->async_call(
                                [this, sid, server_id, gid, ban_until_time, reason] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().ban_role_by_gid_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_ban_role_by_gid_req::gbrbgr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        req->set_ban_until_time(ban_until_time);
                                        req->set_reason(reason);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::remove_ban_role_by_gid(uint32_t sid, int server_id, uint64_t gid) {
                        st_->async_call(
                                [this, sid, server_id, gid] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().remove_punish_by_gid_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_remove_punish_by_gid_req::grpbgr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        req->set_type(pd::AERT_BAN);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::kick_role_by_gid(uint32_t sid, int server_id, uint64_t gid, const string& reason) {
                        st_->async_call(
                                [this, sid, server_id, gid, reason] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().kick_role_by_gid_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_kick_role_by_gid_req::gkrbgr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        req->set_reason(reason);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_world_chat(uint32_t sid, int server_id) {
                        st_->async_call(
                                [this, sid, server_id] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_world_chat_done(sid, pd::world_chat_array(), pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_world_chat_req::gfwcr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::find_role_by_gid(uint32_t sid, int server_id, uint64_t gid) {
                        st_->async_call(
                                [this, sid, server_id, gid] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().find_role_done(sid, pd::yunying_role_info(), pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_find_role_req::gfrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_gid(gid);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::find_role_by_rolename(uint32_t sid, int server_id, const string& rolename) {
                        st_->async_call(
                                [this, sid, server_id, rolename] {
                                       if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().find_role_done(sid, pd::yunying_role_info(), pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_find_role_req::gfrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_rolename(rolename);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_rank_list(uint32_t sid, int server_id, const string& rank_type, uint32_t page_size) {
                        st_->async_call(
                                [this, sid, server_id, rank_type, page_size] {
                                       if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_rank_list_done(sid, pd::NO_SUCH_SCENE, pd::rank_list_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_rank_list_req::gfrlr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_rank_type(rank_type);
                                        req->set_page_size(page_size);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::internal_recharge(uint32_t sid, int server_id, uint64_t role, uint32_t recharge_id) {
                        st_->async_call(
                                [this, sid, server_id, role, recharge_id] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().recharge_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_internal_recharge_req::girr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_recharge_id(recharge_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::reissue_recharge(uint32_t sid, int server_id, uint64_t role, uint64_t order) {
                        st_->async_call(
                                [this, sid, server_id, role, order] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().recharge_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_reissue_recharge_req::grrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_order(order);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::recharge(uint32_t sid, uint64_t  order, const string& yy_orderno, const string& currency, uint32_t price, uint32_t paytime, const string& product_name, const string& ext_info) {
                        st_->async_call(
                                [this, sid, order, yy_orderno, currency, price, paytime, product_name, ext_info] {
                                        auto server_id = gid::instance().get_server_id(order);
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().recharge_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_recharge_req::grr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_order(order);
                                        req->set_yunying_orderno(yy_orderno);
                                        req->set_currency(currency);
                                        req->set_price(price);
                                        req->set_paytime(paytime);
                                        req->set_product_name(product_name);
                                        req->set_ext_info(ext_info);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::send_mail(uint32_t sid, int server_id, uint32_t channel_id, const pd::gid_array& roles, uint64_t mail_id, const string& title, const string& content, const pd::event_array& events) {
                        st_->async_call(
                                [this, sid, server_id, channel_id, roles, mail_id, title, content, events] {
                                      ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_send_mail_req::gsmr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        *req->mutable_roles() = roles;
                                        req->set_id(mail_id);
                                        req->set_server_id(server_id);
                                        req->set_title(title);
                                        req->set_content(content);
                                        *req->mutable_events() = events;
                                        if (server_id == 0) {
                                                if (channel_id == 0) {
                                                        scene_mgr::instance().broadcast_msg(msg);
                                                } else {
                                                        const auto& server_lists = PTTS_GET_ALL(serverlist);
                                                        bool send = false;
                                                        for (const auto& i : server_lists) {
                                                                for (auto j : i.second.channel_ids()) {
                                                                        if (j == channel_id) {
                                                                                if (scene_mgr::instance().has_client_process(i.second.server_id())) {
                                                                                        scene_mgr::instance().send_msg(i.second.server_id(), msg);
                                                                                        send = true;
                                                                                        break;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                        if (!send) {
                                                                yunying_mgr::instance().send_mail_done(sid, pd::NO_SUCH_SCENE, pd::gid_array());
                                                        }
                                                }
                                        } else {
                                                if (!scene_mgr::instance().has_client_process(server_id) || !PTTS_HAS(serverlist, server_id)) {
                                                        yunying_mgr::instance().send_mail_done(sid, pd::NO_SUCH_SCENE, roles);
                                                        return;
                                                }
                                                const auto& server_list = PTTS_GET(serverlist, server_id);
                                                if (channel_id == 0) {
                                                        scene_mgr::instance().send_msg(server_id, msg);
                                                        return;
                                                }
                                                auto send = false;
                                                for (auto j : server_list.channel_ids()) {
                                                        if (channel_id == j) {
                                                                scene_mgr::instance().send_msg(server_id, msg);
                                                                send = true;
                                                                break;
                                                        }
                                                }
                                                if (!send) {
                                                        yunying_mgr::instance().send_mail_done(sid, pd::NO_SUCH_CHANNEL_ID, roles);
                                                }
                                        }
                                });
                }

                void scene_mgr_class::manage_announcement(uint32_t sid, const pd::announcement& announcement) {
                        st_->async_call(
                                [this, sid, announcement] {
                                        ASSERT(announcement.has_server_id());
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_manage_announcement_req::gmar);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        *req->mutable_announcement() = announcement;
                                        if (announcement.server_id() == 0) {
                                                vector<int> server_ids;
                                                if (announcement.type() == pd::FLOW_LIGHT_ANNOUNCEMENT) {
                                                        if (announcement.channel_id() == 0) {
                                                                scene_mgr::instance().broadcast_msg(msg);
                                                        } else {
                                                                const auto& server_lists = PTTS_GET_ALL(serverlist);
                                                                for (const auto& i : server_lists) {
                                                                        for (auto j : i.second.channel_ids()) {
                                                                                if (j == announcement.channel_id()) {
                                                                                        scene_mgr::instance().send_msg(i.second.server_id(), msg);
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                } else {
                                                        scene_mgr::instance().broadcast_msg(msg);
                                                }
                                                ADD_TIMER(
                                                        st_,
                                                        ([this, sid] (auto canceled, const auto& timer) {
                                                                if (!canceled) {
                                                                        yunying_mgr::instance().manage_announcement_done(sid, pd::OK, vector<int>());
                                                                }
                                                        }),
                                                        3s);
                                        } else {
                                                if (PTTS_HAS(serverlist,  announcement.server_id())) {
                                                        const auto& ptt = PTTS_GET(serverlist, announcement.server_id());
                                                        auto sent = false;
                                                        for (auto i : ptt.channel_ids()) {
                                                                if (announcement.type() == pd::FLOW_LIGHT_ANNOUNCEMENT) {
                                                                        if (i == announcement.channel_id()) {
                                                                                scene_mgr::instance().send_msg(announcement.server_id(), msg);
                                                                                sent = true;
                                                                        }
                                                                } else {
                                                                        scene_mgr::instance().send_msg(announcement.server_id(), msg);
                                                                        sent = true;
                                                                }
                                                        }
                                                        if (!sent) {
                                                                yunying_mgr::instance().manage_announcement_done(sid, pd::NO_SUCH_CHANNEL_ID, vector<int>());
                                                        }
                                                } else {
                                                        yunying_mgr::instance().manage_announcement_done(sid, pd::NO_SUCH_SCENE, vector<int>());
                                                }
                                        }
                                });
                }

                void scene_mgr_class::server_images_announcement(uint32_t sid, uint32_t server_id, const map<string, uint32_t>& announcements) {
                        st_->async_call(
                                [this, sid, server_id, announcements] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().server_images_announcements_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *ia = msg.MutableExtension(ps::g2s_images_announcement::gia);
                                        ia->set_sid(sid);
                                        pd::images_announcement_array images;
                                        for (const auto& i : announcements) {
                                                auto* ima = images.add_images();
                                                ima->set_content(i.first);
                                                ima->set_until_time(i.second);
                                        }
                                        *ia->mutable_announcements() = images;
                                        scene_mgr::instance().send_msg(server_id, msg);
                                        yunying_mgr::instance().server_images_announcements_done(sid, pd::OK);
                                });
                }

                void scene_mgr_class::fetch_log(uint32_t sid, int server_id, uint64_t role, uint32_t item_id, uint32_t start_time, uint32_t end_time, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, role, item_id, start_time, end_time, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_log_done(sid, pd::NO_SUCH_SCENE, pd::yunying_item_change_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_log_req::gflr);
                                        if (item_id != 0) {
                                              req->set_item_id(item_id);
                                        }
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_start_time(start_time);
                                        req->set_end_time(end_time);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_punished(uint32_t sid, int server_id, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, role, start_time, end_time, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_punished_done(sid, pd::NO_SUCH_SCENE,pd::yunying_fetch_punished_info_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_punished_req::gfpr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_start_time(start_time);
                                        req->set_end_time(end_time);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        req->set_server_id(server_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_sent_mail(uint32_t sid, int server_id, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, role, start_time, end_time, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_sent_mail_done(sid, pd::NO_SUCH_SCENE,pd::yunying_fetch_sent_mail_info_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_mail_log_req::gfmlr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_start_time(start_time);
                                        req->set_end_time(end_time);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        req->set_server_id(server_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_login_record(uint32_t sid, int server_id, uint64_t role, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, role, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_login_record_done(sid, pd::NO_SUCH_SCENE,pd::yunying_fetch_login_record_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_login_record_req::gflrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        req->set_server_id(server_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_recharge_record(uint32_t sid, int server_id, uint32_t channel_id, uint64_t order, uint64_t role, int start_time, int end_time, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, channel_id, order, role, start_time, end_time, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_recharge_record_done(sid, pd::NO_SUCH_SCENE,pd::yunying_fetch_recharge_record_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_recharge_record_req::gfrrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_channel(channel_id);
                                        req->set_order(order);
                                        req->set_role(role);
                                        req->set_start_time(start_time);
                                        req->set_end_time(end_time);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        req->set_server_id(server_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_currency_record(uint32_t sid, int server_id, const string& type, uint64_t role, int start_time, int end_time, int page_idx, int page_size) {
                        st_->async_call(
                                [this, sid, server_id, role, type, start_time, end_time, page_idx, page_size] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().fetch_currency_record_done(sid, pd::NO_SUCH_SCENE, pd::yunying_fetch_currency_record_array());
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_currency_record_req::gfcrr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_type(type);
                                        req->set_role(role);
                                        req->set_start_time(start_time);
                                        req->set_end_time(end_time);
                                        req->set_page_idx(page_idx);
                                        req->set_page_size(page_size);
                                        req->set_server_id(server_id);
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::fetch_role_list(uint32_t sid, const string& username) {
                        st_->async_call(
                                [this, sid, username] {
                                        sids_.insert(sid);
                                        client_process_count_ = scene_mgr::instance().client_process_count();
                                        if(client_process_count_ <= 0) {
                                                yunying_mgr::instance().fetch_role_list_done(sid, pd::yunying_role_simple_info_array(), pd::NOT_FOUND);
                                                return;
                                        }

                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_fetch_role_list_req::gfrlr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_username(username);
                                        scene_mgr::instance().broadcast_msg(msg);

                                        timer_ = ADD_TIMER(
                                                st_,
                                                ([this, sid] (auto canceled, const auto& timer) {
                                                        if (timer_ == timer) {
                                                                timer_.reset();
                                                        }
                                                        if (!canceled && this->has_sid(sid)) {
                                                                if (!this->has_role_simple_infos(sid)) {
                                                                        sids_.erase(sid);
                                                                        yunying_mgr::instance().fetch_role_list_done(sid, pd::yunying_role_simple_info_array(), pd::NOT_FOUND);
                                                                        return;
                                                                }
                                                                yunying_mgr::instance().fetch_role_list_done(sid, sid2role_infos_[sid], pd::OK);
                                                                sid2role_infos_.erase(sid);
                                                        }
                                                }),
                                                4s);
                                });
                }

                void scene_mgr_class::add_stuff_by_gid(uint32_t sid, uint32_t server_id, uint64_t role, const pd::event_array& events) {
                        st_->async_call(
                                [this, sid, server_id, role, events] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().add_stuff_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_add_stuff_req::gasr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        *req->mutable_events() = events;
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::add_stuff_by_rolename(uint32_t sid, uint32_t server_id, const string& rolename, const pd::event_array& events) {
                        st_->async_call(
                                [this, sid, server_id, rolename, events] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().add_stuff_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_add_stuff_req::gasr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_rolename(rolename);
                                        *req->mutable_events() = events;
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::dec_stuff_by_gid(uint32_t sid, uint32_t server_id, uint64_t role, const pd::event_array& events) {
                        st_->async_call(
                                [this, sid, server_id, role, events] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().dec_stuff_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_dec_stuff_req::gdsr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_role(role);
                                        *req->mutable_events() = events;
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::operates_activity(uint32_t sid, uint32_t activity, uint32_t start_time, uint32_t duration, pd::admin_activity_type type, const vector<string>& server_ids, bool open_activity) {
                        st_->async_call(
                                [this, sid, activity, start_time, duration, server_ids, type, open_activity] {
                                        for (const auto& i : server_ids) {
                                                if (!scene_mgr::instance().has_client_process(stoul(i))) {
                                                        yunying_mgr::instance().operates_activity_done(sid, pd::NO_SUCH_SCENE);
                                                        return;
                                                }
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_operates_activity_req::goar);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_activity(activity);
                                        req->set_start_time(start_time);
                                        req->set_duration(duration);
                                        req->set_type(type);
                                        req->set_open_activity(open_activity);
                                        for (const auto& i : server_ids) {
                                                scene_mgr::instance().send_msg(stoul(i), msg);
                                        }
                                });
                }

                void scene_mgr_class::dec_stuff_by_rolename(uint32_t sid, uint32_t server_id, const string& rolename, const pd::event_array& events) {
                        st_->async_call(
                                [this, sid, server_id, rolename, events] {
                                        if (!scene_mgr::instance().has_client_process(server_id)) {
                                                yunying_mgr::instance().dec_stuff_done(sid, pd::NO_SUCH_SCENE);
                                                return;
                                        }
                                        ps::base msg;
                                        auto *req = msg.MutableExtension(ps::g2s_dec_stuff_req::gdsr);
                                        req->set_sid(sid);
                                        const auto& ptt = PTTS_GET(options, 1);
                                        req->set_gm_id(ptt.gm_info().id());
                                        req->set_rolename(rolename);
                                        *req->mutable_events() = events;
                                        scene_mgr::instance().send_msg(server_id, msg);
                                });
                }

                void scene_mgr_class::set_role_list(uint32_t sid, const pd::yunying_role_simple_info& yrsi) {
                        st_->async_call(
                                [this, sid, yrsi] {
                                        client_process_count_ -= 1;
                                        if (has_sid(sid)) {
                                                *sid2role_infos_[sid].add_role_simple_infos() = yrsi;
                                        }
                                        if(client_process_count_ <= 0 && has_sid(sid)) {
                                                ASSERT(has_role_simple_infos(sid));
                                                yunying_mgr::instance().fetch_role_list_done(sid, sid2role_infos_[sid], pd::OK);
                                                sids_.erase(sid);
                                                sid2role_infos_.erase(sid);
                                                timer_->cancel();
                                                timer_.reset();
                                        }
                                });
                }
        }
}
