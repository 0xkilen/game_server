#include "scene/player/role.hpp"
#include "db_log.hpp"
#include "db/message.hpp"
#include "db/connector.hpp"
#include "utils/assert.hpp"

#define LOG_COMMON(role, type) auto msg = make_shared<db::message>("add_log", db::message::req_type::rt_insert); \
                msg->push_param(role);                            \
                msg->push_param(static_cast<uint32_t>(system_clock::to_time_t(system_clock::now())));                                   \
                msg->push_param(static_cast<uint32_t>(type));

namespace nora {
        namespace scene {

                //payer log
                void db_log::log_login(const role& role, const string& ip) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LOGIN);

                        pd::log log;
                        auto *login = log.mutable_login();
                        login->set_role(role.gid());
                        login->set_rolename(role.name());
                        login->set_ip(ip);
                        login->set_channel(role.yci().channel());
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_logout(const role& role, const string& ip) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LOGOUT);

                        pd::log log;
                        auto *logout = log.mutable_logout();
                        logout->set_role(role.gid());
                        logout->set_rolename(role.name());
                        logout->set_ip(ip);
                        logout->set_channel(role.yci().channel());
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_create_role(const pd::role& data, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CREATE_ROLE);

                        pd::log log;
                        auto *create_role = log.mutable_create_role();
                        *create_role->mutable_data() = data;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_exec_gm(const pd::event_array& ea,
                                 const pd::event_res_array& res_array,
                                 const role& role,
                                 uint32_t pttid,
                                 const pd::league_event_res_array& league_res_array,
                                 const pd::fief_event_res_array& fief_res_array) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::GM_EXEC_EVENT);

                        pd::log log;
                        auto *gee = log.mutable_gm_exec_event();
                        if (ea.events_size() > 0) {
                                *gee->mutable_ea() = ea;
                        }
                        if (pttid > 0) {
                                gee->set_pttid(pttid);
                        }
                        if (res_array.event_reses_size() > 0) {
                                *gee->mutable_res_array() = res_array;
                        }
                        if (league_res_array.league_event_reses_size() > 0) {
                                *gee->mutable_league_res_array() = league_res_array;
                        }
                        if (fief_res_array.fief_event_reses_size() > 0) {
                                *gee->mutable_fief_res_array() = fief_res_array;
                        }

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_sweep_adventure(uint32_t pttid, int times, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::SWEEP_ADVENTURE);

                        pd::log log;
                        auto *sa = log.mutable_sweep_adventure();
                        sa->set_pttid(pttid);
                        sa->set_times(times);
                        *sa->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_collect_adventure_chapter_reward(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::COLLECT_ADVENTURE_CHAPTER_REWARD);

                        pd::log log;
                        auto *cacr = log.mutable_collect_adventure_chapter_reward();
                        cacr->set_pttid(pttid);
                        *cacr->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_challenge_adventure(pd::result result, uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CHALLENGE_ADVENTURE);

                        pd::log log;
                        auto *ca = log.mutable_challenge_adventure();
                        ca->set_pttid(pttid);
                        ca->set_result(result);
                        *ca->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_challenge_lieming(uint32_t pttid, pd::result result, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CHALLENGE_LIEMING);

                        pd::log log;
                        auto *cl = log.mutable_challenge_lieming();
                        cl->set_pttid(pttid);
                        cl->set_result(result);
                        *cl->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_challenge_multi_lieming(uint32_t pttid, pd::result result, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CHALLENGE_MULTI_LIEMING);

                        pd::log log;
                        auto *cml = log.mutable_challenge_multi_lieming();
                        cml->set_pttid(pttid);
                        cml->set_result(result);
                        *cml->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_lottery_draw(const vector<pd::event_res>& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LOTTERY_DRAW);

                        pd::log log;
                        auto *ld = log.mutable_lottery_draw();
                        for (const auto& i : event_res) {
                                *ld->add_event_res() = i;
                        }

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_shop_buy(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::SHOP_BUY);

                        pd::log log;
                        auto *sb = log.mutable_shop_buy();
                        *sb->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_relation_remove_friend(uint64_t gid, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RELATION_REMOVE_FRIEND);

                        pd::log log;
                        auto *rrf = log.mutable_relation_remove_friend();
                        rrf->set_gid(gid);
                        rrf->set_result(pd::OK);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_relation_add_friend(uint64_t gid, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RELATION_ADD_FRIEND);

                        pd::log log;
                        auto *raf = log.mutable_relation_add_friend();
                        raf->set_gid(gid);
                        raf->set_result(pd::OK);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mys_shop_refresh(uint32_t pttid, const pd::event_res& event_res, const pd::mys_shop ms, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MYS_SHOP_REFRESH);

                        pd::log log;
                        auto *msr = log.mutable_mys_shop_refresh();
                        msr->set_pttid(pttid);
                        *msr->mutable_event_res() = event_res;
                        *msr->mutable_ms() = ms;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mys_shop_buy(uint32_t pttid, uint32_t good_id, const pd::event_res* event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MYS_SHOP_BUY);

                        pd::log log;
                        auto *msb = log.mutable_mys_shop_buy();
                        msb->set_pttid(pttid);
                        msb->set_good_id(good_id);
                        *msb->mutable_event_res() = *event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_actor_levelup(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTOR_LEVELUP);

                        pd::log log;
                        auto *al = log.mutable_actor_levelup();
                        *al->mutable_event_res() = event_res;
                        al->set_pttid(pttid);

                        string logstr;
                        log.SerializeToString(&logstr);
                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_actor_add_star(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTOR_ADD_STAR);

                        pd::log log;
                        auto *av = log.mutable_actor_add_star();
                        av->set_pttid(pttid);
                        *av->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_actor_add_step(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTOR_ADD_STEP);

                        pd::log log;
                        auto *aas = log.mutable_actor_add_step();
                        *aas->mutable_event_res() = event_res;
                        aas->set_pttid(pttid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_actor_add_pin(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTOR_ADD_PIN);

                        pd::log log;
                        auto *aap = log.mutable_actor_add_pin();
                        *aap->mutable_event_res() = event_res;
                        aap->set_pttid(pttid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_actor_play_star_plot(uint32_t actor_pttid, uint32_t star, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTOR_PLAY_STAR_PLOT);

                        pd::log log;
                        auto *apsp = log.mutable_actor_play_star_plot();
                        apsp->set_pttid(actor_pttid);
                        apsp->set_star(star);
                        *apsp->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_quest_commit(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::QUEST_COMMIT);

                        pd::log log;
                        auto *qc = log.mutable_quest_commit();
                        *qc->mutable_event_res() = event_res;
                        qc->set_pttid(pttid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_merge(uint64_t gid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_MERGE);

                        pd::log log;
                        auto *em = log.mutable_equipment_merge();
                        *em->mutable_event_res() = event_res;
                        em->set_gid(gid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_add_quality(uint64_t gid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_ADD_QUALITY);

                        pd::log log;
                        auto *eaq = log.mutable_equipment_add_quality();
                        *eaq->mutable_event_res() = event_res;
                        eaq->set_gid(gid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_change_rand_attr(uint64_t gid, int idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_CHANGE_RAND_ATTR);

                        pd::log log;
                        auto *ecra = log.mutable_equipment_change_rand_attr();
                        ecra->set_gid(gid);
                        ecra->set_idx(idx);
                        *ecra->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_decompose(const pd::gid_array& gids, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_DECOMPOSE);

                        pd::log log;
                        auto *ed = log.mutable_equipment_decompose();
                        *ed->mutable_event_res() = event_res;
                        *ed->mutable_gids() = gids;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_compose(uint32_t quality, uint32_t times, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_COMPOSE);

                        pd::log log;
                        auto *ec = log.mutable_equipment_compose();
                        ec->set_quality(quality);
                        ec->set_times(times);
                        *ec->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_xilian(uint64_t gid, const pd::idx_array& lock_idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_XILIAN);

                        pd::log log;
                        auto *ex = log.mutable_equipment_xilian();
                        ex->set_gid(gid);
                        *ex->mutable_lock_idx() = lock_idx;
                        *ex->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_equipment_xilian_confirm(uint64_t gid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::EQUIPMENT_XILIAN_CONFIRM);

                        pd::log log;
                        auto *exc = log.mutable_equipment_xilian_confirm();
                        exc->set_gid(gid);
                        *exc->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_item_use(uint32_t pttid, const string& item_name, int count, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ITEM_USE);

                        pd::log log;
                        auto *item_use = log.mutable_item_use();
                        item_use->set_pttid(pttid);
                        item_use->set_count(count);
                        *item_use->mutable_event_res() = event_res;
                        item_use->set_role(role.gid());
                        item_use->set_time(system_clock::to_time_t(system_clock::now()));
                        item_use->set_name(item_name);
                        item_use->set_role_name(role.name());

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_item_change(uint32_t pttid, const string& item_name, int count, pd::ce_origin origin, const role& role, pd::item_change_type type) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ITEM_CHANGE);

                        pd::log log;
                        auto *ic = log.mutable_item_change();
                        ic->set_role(role.gid());
                        ic->set_role_name(role.name());
                        ic->set_pttid(pttid);
                        ic->set_name(item_name);
                        ic->set_count(count);
                        ic->set_origin(static_cast<int>(origin));
                        ic->set_type(type);
                        ic->set_time(system_clock::to_time_t(system_clock::now()));

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_send_gift(uint64_t gid, uint32_t gift, uint32_t count, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RELATION_SEND_GIFT);

                        pd::log log;
                        auto *sg = log.mutable_send_gift();
                        sg->set_to(gid);
                        sg->set_gift(gift);
                        sg->set_count(count);
                        *sg->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_receive_gift(uint64_t gid, uint32_t gift, uint32_t count, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RELATION_RECEIVE_GIFT);

                        pd::log log;
                        auto *rg = log.mutable_receive_gift();
                        rg->set_from(gid);
                        rg->set_gift(gift);
                        rg->set_count(count);
                        *rg->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_send_mail(uint64_t role, uint64_t mail_gid, uint32_t server_id, const pd::mail& mail) {
                        if (!write_) {
                                return;
                        }
                        LOG_COMMON(role, pd::log::SEND_MAIL);

                        pd::log logs;
                        string logstr;
                        auto *m = logs.mutable_mail();
                        m->set_role(role);
                        auto *mi = m->add_mail_id();
                        mi->set_gid(mail_gid);
                        if (mail.has_dynamic_data() && mail.dynamic_data().has_yunying_mail_id()) {
                                mi->set_id(mail.dynamic_data().yunying_mail_id());
                        } else {
                                mi->set_id(0);
                        }
                        m->set_server_id(server_id);
                        *m->mutable_mail() = mail;
                        logs.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_read_mail(uint64_t role, uint64_t mail_gid, uint32_t server_id, const pd::mail& mail) {
                        if (!write_) {
                                return;
                        }
                        LOG_COMMON(role, pd::log::MAIL_SET_ALREADY_READ);

                        pd::log logs;
                        string logstr;
                        auto *m = logs.mutable_mail();
                        m->set_role(role);
                        auto *mi = m->add_mail_id();
                        mi->set_gid(mail_gid);
                        if (mail.has_dynamic_data() && mail.dynamic_data().has_yunying_mail_id()) {
                                mi->set_id(mail.dynamic_data().yunying_mail_id());
                        } else {
                                mi->set_id(0);
                        }
                        m->set_server_id(server_id);
                        *m->mutable_mail() = mail;
                        logs.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mail_delete(uint64_t role, uint64_t mail_gid, uint32_t server_id, const pd::mail& mail) {
                        if (!write_) {
                                return;
                        }
                        LOG_COMMON(role, pd::log::MAIL_DELETE);

                        pd::log logs;
                        string logstr;
                        auto *m = logs.mutable_mail();
                        m->set_role(role);
                        auto *mi = m->add_mail_id();
                        mi->set_gid(mail_gid);
                        if (mail.has_dynamic_data() && mail.dynamic_data().has_yunying_mail_id()) {
                                mi->set_id(mail.dynamic_data().yunying_mail_id());
                        } else {
                                mi->set_id(0);
                        }
                        m->set_server_id(server_id);
                        *m->mutable_mail() = mail;
                        logs.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mail_fetch(uint64_t role, const map<uint64_t, uint64_t>& mail_ids, uint32_t server_id, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        LOG_COMMON(role, pd::log::MAIL_FETCH);

                        pd::log log;
                        string logstr;
                        auto *mf = log.mutable_mail();
                        mf->set_role(role);
                        for (auto i : mail_ids) {
                                auto *mi = mf->add_mail_id();
                                mi->set_gid(i.first);
                                mi->set_id(i.second);
                        }
                        mf->set_server_id(server_id);
                        *mf->mutable_event_res() = event_res;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                //gm log
                void db_log::log_punish_role_by_gid(const role& role, const string& reason, pd::log::log_type log_type, uint32_t punish_time) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), log_type);

                        pd::log log;
                        auto *punish_role = log.mutable_punish_role();
                        switch(log_type) {
                        case pd::log::PUNISH_ROLE_GAG: {
                                auto *gag_role = punish_role->mutable_gag_role();
                                gag_role->set_role(role.gid());
                                gag_role->set_rolename(role.name());
                                gag_role->set_operate_time(system_clock::to_time_t(system_clock::now()));
                                gag_role->set_reason(reason);
                                gag_role->set_type(1);
                                if (punish_time != 0) {
                                        gag_role->set_punish_time(punish_time);
                                }
                                break;
                        }
                        case pd::log::PUNISH_ROLE_BAN: {
                                auto *ban_role = punish_role->mutable_ban_role();
                                ban_role->set_role(role.gid());
                                ban_role->set_rolename(role.name());
                                ban_role->set_operate_time(system_clock::to_time_t(system_clock::now()));
                                ban_role->set_reason(reason);
                                ban_role->set_type(2);
                                if (punish_time != 0) {
                                        ban_role->set_punish_time(punish_time);
                                }
                                break;
                        }
                        case pd::log::PUNISH_ROLE_KICK: {
                                auto *kick_role = punish_role->mutable_kick_role();
                                kick_role->set_role(role.gid());
                                kick_role->set_rolename(role.name());
                                kick_role->set_operate_time(system_clock::to_time_t(system_clock::now()));
                                kick_role->set_reason(reason);
                                kick_role->set_type(3);
                                break;
                        }
                        default:
                                break;
                        }

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_unpunish_role_by_gid(const role& role, pd::log::log_type log_type) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), log_type);

                        pd::log log;
                        auto *unpunish_role = log.mutable_unpunish_role();
                        unpunish_role->set_role(role.gid());
                        unpunish_role->set_rolename(role.name());
                        unpunish_role->set_operate_time(system_clock::to_time_t(system_clock::now()));
                        unpunish_role->set_type(0);

                        string logstr;
                        log.SerializeToString(&logstr);
                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_internal_recharge(const pd::event_res& event_res, const role& role, uint32_t price) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::INTERNAL_RECHARGE);

                        pd::log log;
                        auto *ir = log.mutable_internal_recharge();
                        ir->set_role(role.gid());
                        ir->set_rolename(role.name());
                        ir->set_price(price);
                        *ir->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_add_stuff(const pd::event_res& event_res, const role& role) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::YUNYING_ADD_STUFF);

                        pd::log log;
                        auto *yas = log.mutable_yunying_add_stuff();
                        yas->set_role(role.gid());
                        *yas->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_dec_stuff(const pd::event_res& event_res, const role& role) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::YUNYING_ADD_STUFF);

                        pd::log log;
                        auto *yas = log.mutable_yunying_dec_stuff();
                        yas->set_role(role.gid());
                        *yas->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_reissue_recharge(uint64_t order, const pd::event_res& event_res, const role& role) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::REISSUE_RECHARGE);

                        pd::log log;
                        auto *rr = log.mutable_reissue_recharge();
                        rr->set_order(order);
                        rr->set_role(role.gid());
                        rr->set_rolename(role.name());
                        *rr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_recharge(uint64_t order, const string& yunying_order, const string& currency, uint32_t price, uint32_t paytime, const string& product_name, const pd::event_res& event_res, const role& role) {
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RECHARGE);

                        pd::log log;
                        auto *recharge = log.mutable_recharge();
                        recharge->set_role(role.gid());
                        recharge->set_rolename(role.name());
                        recharge->set_order(order);
                        recharge->set_price(price);
                        recharge->set_paytime(paytime);
                        *recharge->mutable_event_res() = event_res;
                        recharge->set_yunying_order(yunying_order);
                        recharge->set_currency(currency);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_currency_record(const role& role, pd::resource_type type, pd::ce_origin origin, int cur_count, pd::currency_change_type change_type) {
                        if (role.mirror_role()) {
                                return;
                        }
                        pd::log log;
                        pd::log_currency_change *currency_change;
                        pd::log::log_type log_type;
                        if (type == pd::GOLD) {
                                log_type = pd::log::CURRENCY_GOLD;
                                currency_change = log.mutable_currency_gold();
                        } else {
                                log_type = pd::log::CURRENCY_DIAMOND;
                                currency_change = log.mutable_currency_diamond();
                        }
                        LOG_COMMON(role.gid(), log_type);
                        currency_change->set_role(role.gid());
                        currency_change->set_rolename(role.name());
                        currency_change->set_currency_count(cur_count);
                        currency_change->set_change_type(change_type);
                        currency_change->set_reason(to_string(static_cast<int>(origin)));
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_create_league(const pd::league& league, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CREATE_LEAGUE);

                        pd::log log;
                        auto *cl = log.mutable_create_league();
                        *cl->mutable_league() = league;
                        *cl->mutable_er() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_dismiss_league(const pd::role_league_data& league_data, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::DISMISS_LEAGUE);

                        pd::log log;
                        auto *dl = log.mutable_dismiss_league();
                        *dl->mutable_league_data() = league_data;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_promote_league_vp(uint64_t member, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::PROMOTE_LEAGUE_VP);

                        pd::log log;
                        auto *plv = log.mutable_promote_league_vp();
                        plv->set_member(member);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_depose_league_vp(uint64_t vp, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::DEPOSE_LEAGUE_VP);

                        pd::log log;
                        auto *dlv = log.mutable_depose_league_vp();
                        dlv->set_vp(vp);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_promote_league_president(uint64_t vp, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::PROMOTE_LEAGUE_PRESIDENT);

                        pd::log log;
                        auto *plp = log.mutable_promote_league_president();
                        plp->set_vp(vp);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_kick_league_member(uint64_t member, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::KICK_LEAGUE_MEMBER);

                        pd::log log;
                        auto *klm = log.mutable_kick_league_member();
                        klm->set_member(member);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_impeach(const pd::league_joiners& joiners,  const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_IMPEACH);

                        pd::log log;
                        auto *li = log.mutable_league_impeach();
                        *li->mutable_joiners() = joiners;
                        *li->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_donate(const pd::event_res& event_res, bool boomed, pd::league::donate_type type, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_DONATE);

                        pd::log log;
                        auto *li = log.mutable_league_donate();
                        *li->mutable_event_res() = event_res;
                        li->set_boomed(boomed);
                        li->set_type(type);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_collect_donation_reward(int idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_COLLECT_DONATION_REWARD);

                        pd::log log;
                        auto *lcdr = log.mutable_league_collect_donation_reward();
                        lcdr->set_idx(idx);
                        *lcdr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_help_quest(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_HELP_QUEST);

                        pd::log log;
                        auto *lhq = log.mutable_league_help_quest();
                        *lhq->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_issue_lucky_bag(uint32_t diamond, uint32_t item, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_ISSUE_LUCKY_BAG);

                        pd::log log;
                        auto *lilb = log.mutable_league_issue_lucky_bag();
                        lilb->set_diamond(diamond);
                        lilb->set_item(item);
                        *lilb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_open_lucky_bag(uint32_t diamond, uint32_t item, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_OPEN_LUCKY_BAG);

                        pd::log log;
                        auto *lolb = log.mutable_league_open_lucky_bag();
                        lolb->set_diamond(diamond);
                        lolb->set_item(item);
                        *lolb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_lucky_bag_buy_issue(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_LUCKY_BAG_BUY_ISSUE);

                        pd::log log;
                        auto *llbbi = log.mutable_league_lucky_bag_buy_issue();
                        *llbbi->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_lucky_bag_buy_open(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_LUCKY_BAG_BUY_OPEN);

                        pd::log log;
                        auto *llbbo = log.mutable_league_lucky_bag_buy_open();
                        *llbbo->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_item_apply(uint32_t item, const role& role, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_ITEM_APPLY)

                        pd::log log;
                        auto *lia = log.mutable_league_item_apply();
                        lia->set_item(item);
                        *lia->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_allocate_item(uint32_t item, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_ALLOCATE_ITEM)

                        pd::log log;
                        auto *lai = log.mutable_league_allocate_item();
                        lai->set_item(item);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_mark_campaign_stage(uint32_t stage, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_MARK_CAMPAGIN_STAGE)

                        pd::log log;
                        auto *lmcs = log.mutable_league_mark_campaign_stage();
                        lmcs->set_stage(stage);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_active_campaign_stage(uint32_t stage, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_ACTIVE_CAMPAIGN_STAGE)

                        pd::log log;
                        auto *lacs = log.mutable_league_active_campaign_stage();
                        lacs->set_stage(stage);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_challenge_campaign(uint32_t stage, const pd::battle_team& enemy, uint32_t battle_seed, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_CHALLENGE_CAMPAIGN);

                        pd::log log;
                        auto *lcc = log.mutable_league_challenge_campaign();
                        lcc->set_stage(stage);
                        lcc->set_battle_seed(battle_seed);
                        *lcc->mutable_enemy() = enemy;
                        *lcc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_cancel_item_apply(uint32_t item, const role& role, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::LEAGUE_CANCEL_ITEM_APPLY);

                        pd::log log;
                        auto *lcai = log.mutable_league_cancel_item_apply();
                        lcai->set_item(item);
                        *lcai->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_get_league_war_battle_reward(const role& role, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::LEAGUE_GET_LEAGUE_WAR_BATTLE_REWARD);

                        pd::log log;
                        auto *lcai = log.mutable_league_get_league_war_battle_reward();
                        *lcai->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_get_league_war_rank_reward(const role& role, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::LEAGUE_GET_LEAGUE_WAR_RANK_REWARD);

                        pd::log log;
                        auto *lcai = log.mutable_league_get_league_war_rank_reward();
                        *lcai->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_league_cheer_up_league_war(const role& role, const pd::event_res& event_res) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::LEAGUE_CHEER_UP_LEAGUE_WAR);

                        pd::log log;
                        auto *lcai = log.mutable_league_get_league_war_rank_reward();
                        *lcai->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_create_fief(const pd::fief& data, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CREATE_FIEF);

                        pd::log log;
                        auto *cf = log.mutable_create_fief();
                        *cf->mutable_data() = data;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_trigger_fief_incident(uint32_t incident, const pd::event_res& event_res, const pd::fief_event_res& fief_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::TRIGGER_FIEF_INCIDENT);

                        pd::log log;
                        auto *tfi = log.mutable_trigger_fief_incident();
                        *tfi->mutable_event_res() = event_res;
                        *tfi->mutable_fief_event_res() = fief_event_res;
                        tfi->set_incident(incident);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_building_levelup(pd::fief_building_type type, uint32_t time, const pd::fief_event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_BUILDING_LEVELUP);

                        pd::log log;
                        auto *fbl = log.mutable_fief_building_levelup();
                        fbl->set_type(type);
                        fbl->set_time(time);
                        *fbl->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_building_levelup_speed(pd::fief_building_type type, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_BUILDING_LEVELUP_SPEED);

                        pd::log log;
                        auto *fbls = log.mutable_fief_building_levelup_speed();
                        fbls->set_type(type);
                        *fbls->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_building_cancel_levelup(pd::fief_building_type type, const pd::fief_event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_BUILDING_CANCEL_LEVELUP);

                        pd::log log;
                        auto *fbcl = log.mutable_fief_building_cancel_levelup();
                        fbcl->set_type(type);
                        *fbcl->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_challenge_thief(uint64_t gid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_CHALLENGE_THIEF);

                        pd::log log;
                        auto *fct = log.mutable_fief_challenge_thief();
                        fct->set_gid(gid);
                        *fct->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_boss_challenge(uint64_t fief, const pd::battle_team& boss_team, uint32_t battle_seed, int damage, const pd::battle_damage_info& damage_info, int damage_percent, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_BOSS_CHALLENGE);

                        pd::log log;
                        auto *fbc = log.mutable_fief_boss_challenge();
                        fbc->set_fief(fief);
                        *fbc->mutable_boss_team() = boss_team;
                        fbc->set_battle_seed(battle_seed);
                        fbc->set_damage(damage);
                        *fbc->mutable_damage_info() = damage_info;
                        fbc->set_damage_percent(damage_percent);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_pick_up_grow(pd::fief_building_type building, const pd::fief_event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_PICK_UP_GROW);

                        pd::log log;
                        auto *fpug = log.mutable_fief_pick_up_grow();
                        fpug->set_building(building);
                        *fpug->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_open_area_incident(const pd::fief_incident& incident, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_OPEN_AREA_INCIDENT);

                        pd::log log;
                        auto *foai = log.mutable_fief_open_area_incident();
                        *foai->mutable_incident() = incident;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_fief_join_area_incident(uint64_t to_role, const pd::fief_incident& incident, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FIEF_JOIN_AREA_INCIDENT);

                        pd::log log;
                        auto *fjai = log.mutable_fief_join_area_incident();
                        fjai->set_to_role(to_role);
                        *fjai->mutable_incident() = incident;
                        *fjai->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_create_mansion(const pd::mansion& mansion, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::CREATE_MANSION);

                        pd::log log;
                        auto *cm = log.mutable_create_mansion();
                        *cm->mutable_mansion() = mansion;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_commit_mansion_hall_quest(uint32_t quest, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::COMMIT_MANSION_HALL_QUEST);

                        pd::log log;
                        auto *cmhq = log.mutable_commit_mansion_hall_quest();
                        cmhq->set_quest(quest);
                        *cmhq->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_add_mansion_furniture(const pcs::mansion_add_furniture_array& furniture, const pd::event_res& event_res, uint32_t fancy, bool by_spouse, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ADD_MANSION_FURNITURE);

                        pd::log log;
                        auto *amf = log.mutable_add_mansion_furniture();
                        *amf->mutable_furniture() = furniture;
                        *amf->mutable_event_res() = event_res;
                        amf->set_fancy(fancy);
                        amf->set_by_spouse(by_spouse);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_accept_mansion_quest(uint32_t quest, uint32_t actor, const pd::mansion_quest& accepted_quest, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACCEPT_MANSION_QUEST);

                        pd::log log;
                        auto *amq = log.mutable_accept_mansion_quest();
                        amq->set_quest(quest);
                        amq->set_actor(actor);
                        *amq->mutable_accepted_quest() = accepted_quest;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_help_mansion_quest(uint64_t mansion, const pd::mansion_quest& quest, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::HELP_MANSION_QUEST);

                        pd::log log;
                        auto *hmq = log.mutable_help_mansion_quest();
                        hmq->set_mansion(mansion);
                        *hmq->mutable_quest() = quest;
                        *hmq->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_accept_mansion_other_quest(uint64_t mansion, pd::mansion_building_type building, uint32_t actor, const pd::mansion_quest& accepted_quest, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACCEPT_MANSION_OTHER_QUEST);

                        pd::log log;
                        auto *amoq = log.mutable_accept_mansion_other_quest();
                        amoq->set_mansion(mansion);
                        amoq->set_building(building);
                        amoq->set_actor(actor);
                        *amoq->mutable_accepted_quest() = accepted_quest;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_commit_mansion_other_quest(uint64_t mansion, uint64_t quest, const pd::event_res_array& res_array, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::COMMIT_MANSION_OTHER_QUEST);

                        pd::log log;
                        auto *cmoq = log.mutable_commit_mansion_other_quest();
                        cmoq->set_mansion(mansion);
                        cmoq->set_quest(quest);
                        *cmoq->mutable_res_array() = res_array;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_commit_mansion_quest (uint64_t quest, uint32_t pttid, pd::actor::craft_type craft, uint32_t actor, const pd::event_res_array& res_array, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::COMMIT_MANSION_QUEST);

                        pd::log log;
                        auto *cmq = log.mutable_commit_mansion_quest();
                        cmq->set_quest(quest);
                        cmq->set_pttid(pttid);
                        cmq->set_craft(craft);
                        cmq->set_actor(actor);
                        *cmq->mutable_res_array() = res_array;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_giveup_mansion_quest(uint32_t quest, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::GIVEUP_MANSION_QUEST);

                        pd::log log;
                        auto *gmq = log.mutable_giveup_mansion_quest();
                        gmq->set_quest(quest);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_giveup_mansion_other_quest(uint64_t mansion, uint64_t quest, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::GIVEUP_MANSION_OTHER_QUEST);

                        pd::log log;
                        auto *gmoq = log.mutable_giveup_mansion_other_quest();
                        gmoq->set_mansion(mansion);
                        gmoq->set_quest(quest);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_poison(uint64_t mansion, pd::mansion_building_type building, uint32_t poison, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_POISON);

                        pd::log log;
                        auto *mbp = log.mutable_mansion_banquet_poison();
                        mbp->set_mansion(mansion);
                        mbp->set_building(building);
                        mbp->set_poison(poison);
                        *mbp->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_use_antidote(uint64_t mansion, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_USE_ANTIDOTE);

                        pd::log log;
                        auto *mbua = log.mutable_mansion_banquet_use_antidote();
                        mbua->set_mansion(mansion);
                        *mbua->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_fishing_get_fish(int level, const pd::event_res& cost_event_res, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_FISHING_GET_FISH);

                        pd::log log;
                        auto *mfgf = log.mutable_mansion_fishing_get_fish();
                        mfgf->set_level(level);
                        *mfgf->mutable_cost_event_res() = cost_event_res;
                        *mfgf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_quick_fishing(uint32_t times, uint32_t drop, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_QUICK_FISHING);

                        pd::log log;
                        auto *mqf = log.mutable_mansion_quick_fishing();
                        mqf->set_times(times);
                        mqf->set_drop(drop);
                        *mqf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_visit(uint64_t mansion, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_VISIT);

                        pd::log log;
                        auto *mv = log.mutable_mansion_visit();
                        mv->set_mansion(mansion);
                        *mv->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_start_game(uint64_t other, pd::mansion_game_type game, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_START_GAME);

                        pd::log log;
                        auto *mg = log.mutable_mansion_start_game();
                        mg->set_other(other);
                        mg->set_game(game);
                        *mg->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_switch_banquet_dish(uint32_t new_dish, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_SWITCH_BANQUET_DISH);

                        pd::log log;
                        auto *msbd = log.mutable_mansion_switch_banquet_dish();
                        msbd->set_new_dish(new_dish);
                        *msbd->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_host_banquet(const pd::mansion_banquet& data, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_HOST_BANQUET);

                        pd::log log;
                        auto *mhb = log.mutable_mansion_host_banquet();
                        *mhb->mutable_data() = data;
                        *mhb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_reserve_banquet(uint64_t mansion, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_RESERVE_BANQUET);

                        pd::log log;
                        auto *mrb = log.mutable_mansion_reserve_banquet();
                        mrb->set_mansion(mansion);
                        *mrb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_enter_banquet(uint64_t mansion, pd::mansion_building_type building, const pcs::mansion_banquet_building& banquet_building, bool huanzhuang_level_changed, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_ENTER_BANQUET);

                        pd::log log;
                        auto *meb = log.mutable_mansion_enter_banquet();
                        meb->set_mansion(mansion);
                        meb->set_building(building);
                        meb->set_huanzhuang_level_changed(huanzhuang_level_changed);
                        *meb->mutable_banquet_building() = banquet_building;
                        *meb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_eat_dish(uint64_t mansion, int activity, int atmosphere, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_EAT_DISH);

                        pd::log log;
                        auto *mbed = log.mutable_mansion_banquet_eat_dish();
                        mbed->set_mansion(mansion);
                        mbed->set_activity(activity);
                        mbed->set_atmosphere(atmosphere);
                        *mbed->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_get_coin(uint64_t mansion, const map<uint32_t, uint32_t>& coins, int activity, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_GET_COIN);

                        pd::log log;
                        auto *mbgc = log.mutable_mansion_banquet_get_coin();
                        for (const auto& i : coins) {
                                auto *c = mbgc->add_coins();
                                c->set_coin(i.first);
                                c->set_count(i.second);
                        }
                        mbgc->set_mansion(mansion);
                        mbgc->set_activity(activity);
                        *mbgc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_challenge_thief(uint64_t mansion, int activity, int atmosphere, const pd::battle_damage_info& damage_info, uint32_t battle_seed, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_CHALLENGE_THIEF);

                        pd::log log;
                        auto *mbcf = log.mutable_mansion_banquet_challenge_thief();
                        mbcf->set_mansion(mansion);
                        mbcf->set_activity(activity);
                        mbcf->set_atmosphere(atmosphere);
                        *mbcf->mutable_damage_info() = damage_info;
                        mbcf->set_battle_seed(battle_seed);
                        *mbcf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_open_riddle_box(uint64_t mansion, pd::mansion_building_type building, int activity, int atmosphere, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_OPEN_RIDDLE_BOX);

                        pd::log log;
                        auto *mborb = log.mutable_mansion_banquet_open_riddle_box();
                        mborb->set_mansion(mansion);
                        mborb->set_building(building);
                        mborb->set_activity(activity);
                        mborb->set_atmosphere(atmosphere);
                        *mborb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_challenge_celebrity(uint64_t mansion, uint32_t celebrity, int level, uint32_t activity, int atmosphere, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_CHALLENGE_CELEBRITY);

                        pd::log log;
                        auto *mbcc = log.mutable_mansion_banquet_challenge_celebrity();
                        mbcc->set_mansion(mansion);
                        mbcc->set_celebrity(celebrity);
                        mbcc->set_level(level);
                        mbcc->set_activity(activity);
                        mbcc->set_atmosphere(atmosphere);
                        *mbcc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_exchange_item(uint64_t mansion, uint32_t item_idx, int used_activity, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_EXCHANGE_ITEM);

                        pd::log log;
                        auto *mbei = log.mutable_mansion_banquet_exchange_item();
                        mbei->set_mansion(mansion);
                        mbei->set_used_activity(used_activity);
                        mbei->set_item_idx(item_idx);
                        *mbei->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_banquet_use_firework(uint64_t mansion, uint32_t firework, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_BANQUET_USE_FIREWORK);

                        pd::log log;
                        auto *mbuf = log.mutable_mansion_banquet_use_firework();
                        mbuf->set_mansion(mansion);
                        mbuf->set_firework(firework);
                        *mbuf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_farm_sow(uint32_t land, uint32_t plant, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_FARM_SOW);

                        pd::log log;
                        auto *mfs = log.mutable_mansion_farm_sow();
                        mfs->set_land(land);
                        mfs->set_plant(plant);
                        *mfs->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_farm_harvest(uint64_t mansion, uint32_t land, uint32_t plant, uint32_t count, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_FARM_HARVEST);

                        pd::log log;
                        auto *mfh = log.mutable_mansion_farm_harvest();
                        mfh->set_mansion(mansion);
                        mfh->set_land(land);
                        mfh->set_plant(plant);
                        mfh->set_count(count);
                        *mfh->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_farm_fertilize(uint64_t mansion, uint32_t land, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_FARM_FERTILIZE);

                        pd::log log;
                        auto *mff = log.mutable_mansion_farm_fertilize();
                        mff->set_mansion(mansion);
                        mff->set_land(land);
                        *mff->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_mansion_exchange(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MANSION_EXCHANGE);

                        pd::log log;
                        auto *me = log.mutable_mansion_exchange();
                        me->set_pttid(pttid);
                        *me->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_compose(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_COMPOSE);

                        pd::log log;
                        auto *hc = log.mutable_huanzhuang_compose();
                        hc->set_pttid(pttid);
                        *hc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_pvp_vote(uint64_t vote_to, int votes, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_PVP_VOTE);

                        pd::log log;
                        auto *hpv = log.mutable_huanzhuang_pvp_vote();
                        hpv->set_vote_to(vote_to);
                        hpv->set_votes(votes);
                        *hpv->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_pvp_upvote(uint64_t vote_to, int upvotes, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_PVP_UPVOTE);

                        pd::log log;
                        auto *hpu = log.mutable_huanzhuang_pvp_upvote();
                        hpu->set_vote_to(vote_to);
                        hpu->set_upvotes(upvotes);
                        *hpu->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_arena_challenge(uint32_t rank, uint32_t battle_seed, const pd::battle_team& other_formation, const pd::battle_damage_info& damage_info, const pcs::arena_challenge_record& record, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ARENA_CHALLENGE);

                        pd::log log;
                        auto *ac = log.mutable_arena_challenge();
                        ac->set_rank(rank);
                        ac->set_battle_seed(battle_seed);
                        *ac->mutable_other_formation() = other_formation;
                        *ac->mutable_damage_info() = damage_info;
                        *ac->mutable_record() = record;
                        *ac->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_quest_everyday_reward (uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::QUEST_EVERYDAY_REWARD);

                        pd::log log;
                        auto *qer = log.mutable_quest_everyday_reward();
                        qer->set_pttid(pttid);
                        *qer->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_role_eat(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ROLE_EAT);

                        pd::log log;
                        auto *re = log.mutable_role_eat();
                        *re->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_role_add_fate(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ROLE_ADD_FATE);

                        pd::log log;
                        auto *raf = log.mutable_role_add_fate();
                        *raf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_role_buy_present(uint32_t present, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ROLE_BUY_PRESENT);

                        pd::log log;
                        auto *rbp = log.mutable_role_buy_present();
                        rbp->set_present(present);
                        *rbp->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_upvote(uint64_t upvotee, uint32_t present_count, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::UPVOTE);

                        pd::log log;
                        auto *upvote = log.mutable_upvote();
                        upvote->set_upvotee(upvotee);
                        upvote->set_present_count(present_count);
                        *upvote->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_login_days_reward(uint32_t day, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_LOGIN_DAYS_REWARD);

                        pd::log log;
                        auto *aldr = log.mutable_activity_login_days_reward();
                        aldr->set_day(day);
                        *aldr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_online_time_reward(uint32_t idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_ONLINE_TIME_REWARD);

                        pd::log log;
                        auto *aotr = log.mutable_activity_online_time_reward();
                        aotr->set_idx(idx);
                        *aotr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_seven_days_get_fuli(uint32_t begin_days, uint32_t day, uint32_t idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_SEVEN_DAYS_GET_FULI);

                        pd::log log;
                        auto *asdgf = log.mutable_activity_seven_days_get_fuli();
                        asdgf->set_begin_days(begin_days);
                        asdgf->set_day(day);
                        asdgf->set_idx(idx);
                        *asdgf->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_seven_days_commit_quest(uint32_t begin_days, uint32_t day, uint32_t idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_SEVEN_DAYS_COMMIT_QUEST);

                        pd::log log;
                        auto *asdcq = log.mutable_activity_seven_days_commit_quest();
                        asdcq->set_begin_days(begin_days);
                        asdcq->set_day(day);
                        asdcq->set_idx(idx);
                        *asdcq->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_seven_days_get_reward(uint32_t begin_days, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_SEVEN_DAYS_GET_REWARD);

                        pd::log log;
                        auto *asdgr = log.mutable_activity_seven_days_get_reward();
                        asdgr->set_begin_days(begin_days);
                        *asdgr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_recharge_day_get(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RECHARGE_DAY_GET);

                        pd::log log;
                        auto *rdg = log.mutable_recharge_day_get();
                        rdg->set_pttid(pttid);
                        *rdg->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_recharge_first_get(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_RECHARGE_FIRST_GET);

                        pd::log log;
                        auto *rfg = log.mutable_activity_recharge_first_get();
                        *rfg->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_recharge_day_bufa(const pd::log_recharge_day_bufa_array& recharge_day_bufa_array, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RECHARGE_DAY_BUFA);

                        pd::log log;
                        *log.mutable_recharge_day_bufa() = recharge_day_bufa_array;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_recharge_get_vip_box(uint32_t vip_level, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RECHARGE_GET_VIP_BOX);

                        pd::log log;
                        auto *rgvb = log.mutable_recharge_get_vip_box();
                        rgvb->set_vip_level(vip_level);
                        *rgvb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_yunying_cdkey(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::YUNYING_CDKEY);

                        pd::log log;
                        auto *yc = log.mutable_yunying_cdkey();
                        *yc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_other_accept_propose(uint64_t accepter, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_OTHER_ACCEPT_PROPOSE);

                        pd::log log;
                        auto *moan = log.mutable_marriage_other_accept_propose();
                        moan->set_accepter(accepter);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_recharge_get_vip_buy_box(uint32_t vip_level, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::RECHARGE_GET_VIP_BUY_BOX);

                        pd::log log;
                        auto *rgvbb = log.mutable_recharge_get_vip_buy_box();
                        rgvbb->set_vip_level(vip_level);
                        *rgvbb->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_answer_question(uint32_t question_idx, uint32_t option_idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_ANSWER_QUESTION);

                        pd::log log;
                        auto *maq = log.mutable_marriage_answer_question();
                        maq->set_question_idx(question_idx);
                        maq->set_option_idx(option_idx);
                        *maq->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_start_star_wish(const string& declaration, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_START_STAR_WISH);

                        pd::log log;
                        auto *mssw = log.mutable_marriage_start_star_wish();
                        mssw->set_declaration(declaration);
                        *mssw->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_star_wish_send_gift(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_STAR_WISH_SEND_GIFT);

                        pd::log log;
                        auto *mswsg = log.mutable_marriage_star_wish_send_gift();
                        *mswsg->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_star_wish_select(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_STAR_WISH_SELECT);

                        pd::log log;
                        auto *msws = log.mutable_marriage_star_wish_select();
                        *msws->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_star_wish_selected(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_STAR_WISH_SELECTED);

                        pd::log log;
                        auto *msws = log.mutable_marriage_star_wish_selected();
                        *msws->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_divorce(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_DIVORCE);

                        pd::log log;
                        auto *md = log.mutable_marriage_divorce();
                        *md->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_start_pk(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_START_PK);

                        pd::log log;
                        auto *msp = log.mutable_marriage_start_pk();
                        *msp->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_pk_challenge(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_PK_CHALLENGE);

                        pd::log log;
                        auto *mpc = log.mutable_marriage_pk_challenge();
                        *mpc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_guesture_round_reward(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_GUESTURE_ROUND_REWARD);

                        pd::log log;
                        auto *mgrr = log.mutable_marriage_guesture_round_reward();
                        *mgrr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_pk_select(uint64_t target, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_PK_SELECT);

                        pd::log log;
                        auto *mps = log.mutable_marriage_pk_select();
                        mps->set_target(target);
                        *mps->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_marriage_pk_selected(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::MARRIAGE_PK_SELECTED);

                        pd::log log;
                        auto *msws = log.mutable_marriage_pk_selected();
                        *msws->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_gongdou(pd::gongdou_type type, uint64_t gid, pd::result result, uint64_t target, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::GONGDOU);

                        pd::log log;
                        auto *gongdou = log.mutable_gongdou();
                        gongdou->set_type(type);
                        gongdou->set_gid(gid);
                        gongdou->set_target(target);
                        gongdou->set_result(result);
                        *gongdou->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_feige_get_reward(uint32_t feige, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FEIGE_GET_REWARD);

                        pd::log log;
                        auto *fgr = log.mutable_feige_get_reward();
                        fgr->set_feige(feige);
                        *fgr->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_feige_read_chuanwen(uint32_t chuanwen, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::FEIGE_READ_CHUANWEN);

                        pd::log log;
                        auto *frc = log.mutable_feige_read_chuanwen();
                        frc->set_chuanwen(chuanwen);
                        *frc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_plot_continue(const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::PLOT_CONTINUE);

                        pd::log log;
                        auto *pc = log.mutable_plot_continue();
                        *pc->mutable_event_res() = event_res;
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_xinshou_continue(uint32_t group_pttid, uint32_t pttid, const role& role) {
                        if (!write_) {
                                return;
                        }
                        if (role.mirror_role()) {
                                return;
                        }
                        LOG_COMMON(role.gid(), pd::log::XINSHOU_CONTINUE);

                        pd::log log;
                        auto *xc = log.mutable_xinshou_continue();
                        xc->set_group_pttid(group_pttid);
                        xc->set_pttid(pttid);
                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_challenge(uint32_t target_gpin, uint32_t target_idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_CHALLENGE);
                        pd::log log;
                        auto *gc = log.mutable_guanpin_challenge();
                        gc->set_target_gpin(target_gpin);
                        gc->set_target_idx(target_idx);
                        *gc->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_collect_fenglu(uint32_t gpin, uint32_t idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_COLLECT_FENGLU);
                        pd::log log;
                        auto *cf = log.mutable_guanpin_collect_fenglu();
                        cf->set_gpin(gpin);
                        cf->set_idx(idx);
                        *cf->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_baoming(uint32_t gpin, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_BAOMING);
                        pd::log log;
                        auto *b = log.mutable_guanpin_baoming();
                        b->set_gpin(gpin);
                        *b->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_check_promote(uint32_t gpin, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_CHECK_PROMOTE);
                        pd::log log;
                        auto *cp = log.mutable_guanpin_check_promote();
                        cp->set_gpin(gpin);
                        *cp->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_promote(uint32_t gpin, uint32_t idx, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_PROMOTE);
                        pd::log log;
                        auto *cp = log.mutable_guanpin_promote();
                        cp->set_gpin(gpin);
                        cp->set_idx(idx);
                        *cp->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_guanpin_dianshi_demote(uint32_t gpin, uint32_t idx, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::GUANPIN_DIANSHI_DEMOTE);
                        pd::log log;
                        auto *cp = log.mutable_guanpin_dianshi_demote();
                        cp->set_gpin(gpin);
                        cp->set_idx(idx);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_tower_challenge(uint32_t level, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::TOWER_CHALLENGE);
                        pd::log log;
                        auto *tc = log.mutable_tower_challenge();
                        tc->set_level(level);
                        *tc->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_tower_refresh(uint32_t level, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::TOWER_CHALLENGE);
                        pd::log log;
                        auto *tc = log.mutable_tower_refresh();
                        tc->set_level(level);
                        *tc->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_tower_sweep(uint32_t level, const pd::event_res_array& event_array, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::TOWER_SWEEP);
                        pd::log log;
                        auto *tc = log.mutable_tower_sweep();
                        tc->set_level(level);
                        *tc->mutable_event_array() = event_array;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_vip_qianzhuang_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_VIP_QIANZHUANG_GET_REWARD);
                        pd::log log;
                        auto *avqgr = log.mutable_activity_vip_qianzhuang_get_reward();
                        avqgr->set_pttid(pttid);
                        *avqgr->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }
                void db_log::log_activity_emperor_qianzhuang_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_EMPEROR_QIANZHUANG_GET_REWARD);
                        pd::log log;
                        auto *aeqgr = log.mutable_activity_emperor_qianzhuang_get_reward();
                        aeqgr->set_pttid(pttid);
                        *aeqgr->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_activity_fund_get_reward(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::ACTIVITY_FUND_QIANZHUANG_GET_REWARD);
                        pd::log log;
                        auto *afgr = log.mutable_activity_fund_get_reward();
                        afgr->set_pttid(pttid);
                        *afgr->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_pvp_guess_vote(uint64_t target, int target_votes, int other_votes, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_PVP_GUESS_VOTE);
                        pd::log log;
                        auto *hpgv = log.mutable_huanzhuang_pvp_guess_vote();
                        hpgv->set_target(target);
                        hpgv->set_target_votes(target_votes);
                        hpgv->set_other_votes(other_votes);
                        *hpgv->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_levelup(uint32_t pttid, pd::huanzhuang_part part, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_LEVELUP);
                        pd::log log;
                        auto *hl = log.mutable_huanzhuang_levelup();
                        hl->set_pttid(pttid);
                        hl->set_part(part);
                        *hl->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_inherit(const pd::huanzhuang_item_id& from, const pd::huanzhuang_item_id& to, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_INHERIT);
                        pd::log log;
                        auto *hi = log.mutable_huanzhuang_inherit();
                        *hi->mutable_event_res() = event_res;
                        *hi->mutable_from() = from;
                        *hi->mutable_to() = to;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_change_phase(uint64_t child, const pd::child_event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_CHANGE_PHASE);
                        pd::log log;
                        auto *ccp = log.mutable_child_change_phase();
                        *ccp->mutable_event_res() = event_res;
                        ccp->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_raise(uint64_t child, uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_RAISE);
                        pd::log log;
                        auto *cr = log.mutable_child_raise();
                        *cr->mutable_event_res() = event_res;
                        cr->set_child(child);
                        cr->set_pttid(pttid);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_study(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_STUDY);
                        pd::log log;
                        auto *cs = log.mutable_child_study();
                        *cs->mutable_event_res() = event_res;
                        *cs->mutable_child_event_res() = child_event_res;
                        cs->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_learn_skill(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_LEARN_SKILL);
                        pd::log log;
                        auto *cls = log.mutable_child_learn_skill();
                        *cls->mutable_event_res() = event_res;
                        *cls->mutable_child_event_res() = child_event_res;
                        cls->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_add_skill_exp(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_ADD_SKILL_EXP);
                        pd::log log;
                        auto *cse = log.mutable_child_add_skill_exp();
                        *cse->mutable_event_res() = event_res;
                        *cse->mutable_child_event_res() = child_event_res;
                        cse->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_use_grow_item(uint64_t child, const pd::event_res& event_res, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_USE_GROW_ITEM);
                        pd::log log;
                        auto *cugi = log.mutable_child_use_grow_item();
                        *cugi->mutable_event_res() = event_res;
                        *cugi->mutable_child_event_res() = child_event_res;
                        cugi->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_put_on_jade(uint64_t child, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_PUT_ON_JADE);
                        pd::log log;
                        auto *cpoj = log.mutable_child_put_on_jade();
                        *cpoj->mutable_child_event_res() = child_event_res;
                        cpoj->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_take_off_jade(uint64_t child, const pd::child_event_res& child_event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_TAKE_OFF_JADE);
                        pd::log log;
                        auto *ctoj = log.mutable_child_take_off_jade();
                        *ctoj->mutable_child_event_res() = child_event_res;
                        ctoj->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_change_name(uint64_t child, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_CHANGE_NAME);
                        pd::log log;
                        auto *ccn = log.mutable_child_change_name();
                        *ccn->mutable_event_res() = event_res;
                        ccn->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_change_gender(uint64_t child, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_CHANGE_GENDER);
                        pd::log log;
                        auto *ccg = log.mutable_child_change_gender();
                        *ccg->mutable_event_res() = event_res;
                        ccg->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_child_change_type(uint64_t child, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHILD_CHANGE_TYPE);
                        pd::log log;
                        auto *cct = log.mutable_child_change_type();
                        *cct->mutable_event_res() = event_res;
                        cct->set_child(child);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_chat(const string& content, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::CHAT);
                        pd::log log;
                        auto *chat = log.mutable_chat();
                        *chat->mutable_event_res() = event_res;
                        chat->set_content(content);

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_major_city_bubble_award(uint32_t bubble_pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::MAJOR_CITY_BUBBLE);
                        pd::log log;
                        auto *mmcb = log.mutable_major_city_bubble();
                        mmcb->set_pttid(bubble_pttid);
                        *mmcb->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::log_huanzhuang_get_taozhuang_reward(uint32_t pttid, const pd::event_res& event_res, const role& role) {
                        if (!write_) {
                                return;
                        }

                        if (role.mirror_role()) {
                                return;
                        }

                        LOG_COMMON(role.gid(), pd::log::HUANZHUANG_GET_TAOZHUANG_REWARD);
                        pd::log log;
                        auto *hgtr = log.mutable_huanzhuang_get_taozhuang_reward();
                        hgtr->set_pttid(pttid);
                        *hgtr->mutable_event_res() = event_res;

                        string logstr;
                        log.SerializeToString(&logstr);

                        msg->push_param(logstr);
                        logdb_->push_message(msg);
                }

                void db_log::fetch_log(const string& procedure, uint64_t role, uint32_t start_time, uint32_t end_time, int page_idx, int page_size, const shared_ptr<service_thread>& st, const function<void(const vector<vector<boost::any>>&)>& cb, const vector<pd::log::log_type>& type) {
                        auto db_msg = make_shared<db::message>(procedure,
                                                               db::message::req_type::rt_select,
                                                               [st, cb] (const shared_ptr<db::message>& msg) {
                                                                       st->async_call(
                                                                               [cb, msg] {
                                                                                       cb(msg->results());
                                                                               });
                                                               });

                        auto sum_page_size = page_idx * page_size;
                        if (role != 0) {
                                db_msg->push_param(role);
                        }
                        for (auto i : type) {
                                db_msg->push_param(static_cast<uint32_t>(i));
                        }
                        db_msg->push_param(start_time);
                        db_msg->push_param(end_time);
                        db_msg->push_param(sum_page_size);
                        db_msg->push_param(page_size);
                        logdb_->push_message(db_msg, st);
                }

        }
}
