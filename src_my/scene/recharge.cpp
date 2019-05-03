#include "recharge.hpp"
#include "scene/player/role.hpp"
#include "scene/event.hpp"
#include "scene/condition.hpp"
#include "config/recharge_ptts.hpp"

namespace nora {
        namespace scene {

                pd::result recharge_check(uint64_t order, bool check_price, uint32_t price, const role& role) {
                        if (!role.has_recharge_order(order)) {
                                if (role.recharge_order_recharged(order)) {
                                        return pd::RECHARGE_ORDER_RECHARGED;
                                } else {
                                        return pd::RECHARGE_NO_SUCH_ORDER;
                                }
                        }
                        auto pttid = role.recharge_order_pttid(order);
                        const auto& ptt = PTTS_GET(recharge, pttid);
                        if (check_price && ptt.price() != price) {
                                return pd::RECHARGE_ORDER_PRICE_MISMATCH;
                        }

                        return pd::OK;
                }

                pd::result check_recharge_day(uint32_t pttid, const role& role) {
                        if (!PTTS_HAS(recharge, pttid)) {
                                return pd::RECHARGE_INVALID_PTTID;
                        }
                        const auto& ptt = PTTS_GET(recharge, pttid);
                        if (ptt.days() <= 0) {
                                return pd::RECHARGE_INVALID_PTTID;
                        }
                        if (!role.has_recharge_day(pttid)) {
                                return pd::RECHARGE_NOT_RECHARGE;
                        }
                        if (role.recharge_day(pttid) <= refresh_day()) {
                                return pd::RECHARGE_DAYS_NO_MORE;
                        }
                        if (role.recharge_last_process_day(pttid) >= refresh_day()) {
                                return pd::RECHARGE_DAY_ALREADY_GOT;
                        }
                        return pd::OK;
                }

                pd::event_res recharge_process(uint64_t order, uint32_t price, role& role) {
                        auto pttid = role.recharge_order_pttid(order);
                        const auto& ptt = PTTS_GET(recharge, pttid);
                        auto events = ptt.events();
                        auto charged = role.has_recharged(pttid);
                        if (ptt.has_first_events() && !charged) {
                                event_merge(events, ptt.first_events());
                        }
                        pd::ce_env ce;
                        if (ptt.has__activity_everyday_libao()) {
                                ce.set_origin(pd::CO_ACTIVITY_EVERYDAY_LIBAO);
                        } else if (ptt.has__activity_chaozhi_resource_libao() || ptt._activity_chaozhi_huanzhuang_libao()){
                                ce.set_origin(pd::CO_ACTIVITY_LIBAO_CHAOZHI);
                        } else if (ptt.has__activity_limit_libao()) {
                                role.activity_add_limit_libao_bought(ptt._activity_limit_libao());
                                ce.set_origin(pd::CO_ACTIVITY_LIMIT_LIBAO);
                        } else if (ptt.has__shop() && ptt.has__shop_good()) {
                                ce.set_origin(pd::CO_SHOP_BUY);
                                if (refresh_day() >= role.activity_discount_goods_start_day() && refresh_day() <= role.activity_discount_goods_start_day() + role.activity_discount_goods_duration()) {
                                        ce.set_origin(pd::CO_ACTIVITY_DISCOUNT_GOODS);
                                }
                                if (refresh_day() >= role.activity_festival_start_day() && refresh_day() <= role.activity_festival_start_day() + role.activity_festival_duration() - 1) {
                                        ce.set_origin(pd::CO_ACTIVITY_FESTIVAL_SHOP_BUY);
                                }
                        } else {
                                ce.set_origin(pd::CO_YUNYING_RECHARGE);
                        }

                        if (role.activity_leiji_recharge_start_day() != 0) {
                                role.activity_update_leiji_recharge(price);
                        }
                        role.recharge_update_paid_money(price);
                        role.recharge_order_done(order);
                        if (ptt.days() > 0) {
                                role.update_recharge_day(pttid, ptt.days());
                                if (ptt._type() == pc::RT_GENERAL_MONTH_CARD) {
                                        ce.set_origin(pd::CO_GENERAL_MONTH_CARD);
                                } else if (ptt._type() == pc::RT_GOLD_MONTH_CARD) {
                                        ce.set_origin(pd::CO_GOLD_MONTH_CARD);
                                }
                        }

                        auto start_day = role.activity_continue_recharge_start_day();
                        auto duration = role.activity_continue_recharge_duration();
                        if (refresh_day() >= start_day && refresh_day() <= start_day + duration - 1) {
                                role.activity_continue_recharge_update_dayrecharge(price);
                        }

                        return event_process(events, role, ce);
                }

                pd::result recharge_check_get_vip_box(uint32_t vip_level, const role& role) {
                        if (!PTTS_HAS(vip, vip_level)) {
                                return pd::RECHARGE_INVAILID_VIP_LEVEL;
                        }
                        if (vip_level > role.vip_level()) {
                                return pd::RECHARGE_VIP_LEVEL_NOT_ENOUGH;
                        }
                        if (role.recharge_got_vip_box(vip_level)) {
                                return pd::RECHARGE_ALREADY_GOT_THIS_VIP_BOX;
                        }
                        const auto& ptt = PTTS_GET(vip, vip_level);
                        if (!ptt.has_box()) {
                                return pd::RECHARGE_NO_SUCH_VIP_BOX;
                        }
                        return condition_check(ptt.box().conditions(), role);
                }

                pd::event_res recharge_get_vip_box(uint32_t vip_level, role& role) {
                        const auto& ptt = PTTS_GET(vip, vip_level);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_VIP_BOX);
                        auto ret = event_process(ptt.box().events(), role, ce);
                        role.recharge_set_got_vip_box(vip_level);
                        return ret;
                }

                pd::result recharge_check_get_vip_buy_box(uint32_t vip_level, const role& role) {
                        if (!PTTS_HAS(vip, vip_level)) {
                                return pd::RECHARGE_INVAILID_VIP_LEVEL;
                        }
                        if (vip_level > role.vip_level()) {
                                return pd::RECHARGE_VIP_LEVEL_NOT_ENOUGH;
                        }
                        if (role.recharge_got_vip_buy_box(vip_level)) {
                                return pd::RECHARGE_ALREADY_GOT_THIS_VIP_BUY_BOX;
                        }
                        const auto& ptt = PTTS_GET(vip, vip_level);
                        if (!ptt.has_buy_box()) {
                                return pd::RECHARGE_NO_SUCH_VIP_BUY_BOX;
                        }
                        return condition_check(ptt.buy_box().conditions(), role);
                }

                pd::event_res recharge_get_vip_buy_box(uint32_t vip_level, role& role) {
                        const auto& ptt = PTTS_GET(vip, vip_level);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_VIP_BUY_BOX);
                        auto ret = event_process(ptt.buy_box().events(), role, ce);
                        role.recharge_set_got_vip_buy_box(vip_level);
                        return ret;
                }

                uint32_t vip_exp_to_level(uint32_t exp) {
                        uint32_t ret = 0;
                        const auto& ptts = PTTS_GET_ALL(vip);
                        for (const auto& i : ptts) {
                                if (exp >= i.second.need_exp()) {
                                        ret = i.first;
                                } else {
                                        break;
                                }
                        }
                        return ret;
                }

        }
}
