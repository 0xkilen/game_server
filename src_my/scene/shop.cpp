#include "shop.hpp"
#include "scene/player/role.hpp"
#include "scene/player/player_mgr.hpp"
#include "scene/drop.hpp"
#include "scene/event.hpp"
#include "scene/condition.hpp"
#include "scene/condevent.hpp"
#include "scene/activity/activity_logic.hpp"
#include "utils/time_utils.hpp"
#include "config/shop_ptts.hpp"

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                namespace {
                        void mys_shop_refresh_goods_process(const pc::mys_shop& ptt, pd::mys_shop& mys_shop, pd::event_res& res, role& role, bool cost_free = false) {
                                pd::ce_env ce;
                                ce.set_cost_percent(cost_free ? 0 : 1000);
                                ce.set_origin(pd::CO_SHOP_REFRESH_MYS_SHOP);
                                res = event_process(ptt._refresh_events(), role, ce);

                                vector<drop_item<uint32_t>> dis;
                                for (const auto& i : ptt.goods()) {
                                        dis.emplace_back(i.id(), i.weight());
                                }
                                auto dropped = drop_by_weight(dis, 8);
                                ASSERT(dropped.size() == 8);
                                for (const auto& i : dropped) {
                                        auto *good = mys_shop.add_goods();
                                        good->set_id(i);
                                        good->set_bought(false);
                                }
                        }
                }

                pd::result mys_shop_free_refresh_goods(uint32_t pttid, pd::event_res *event_res, role& role) {
                        if (!PTTS_HAS(mys_shop, pttid)) {
                                return pd::MYS_SHOP_INVALID_MYS_SHOP;
                        }
                        const auto& ptt = PTTS_GET(mys_shop, pttid);

                        pd::mys_shop mys_shop;
                        mys_shop.set_pttid(ptt.id());

                        auto refreshed_time = role.mys_shop_free_refreshed_time(ptt.id());
                        if (system_clock::now() - system_clock::from_time_t(refreshed_time) < 2h) {
                                return pd::MYS_SHOP_REFRESH_GOODS_WRONG_TIME;
                        }

                        auto refreshed_tp = system_clock::from_time_t(refreshed_time);
                        refreshed_time = system_clock::to_time_t(refreshed_tp + (system_clock::now() - refreshed_tp) / 2h * 2h);

                        pd::ce_env ce;
                        ce.set_cost_percent(0);
                        auto result = condition_check(ptt.conditions(), role, ce);
                        if (result != pd::OK) {
                                return result;
                        }
                        result = condition_check(ptt.refresh_conditions(), role, ce);
                        if (result != pd::OK) {
                                return result;
                        }

                        mys_shop.set_free_refreshed_time(refreshed_time);

                        mys_shop_refresh_goods_process(ptt, mys_shop, *event_res, role, true);

                        role.update_mys_shop(mys_shop);
                        role.on_event(ECE_MYS_SHOP_REFRESH, { pttid });
                        return result;
                }

                pd::result mys_shop_refresh_goods(uint32_t pttid, pd::event_res *event_res, role& role) {
                        if (!PTTS_HAS(mys_shop, pttid)) {
                                return pd::MYS_SHOP_INVALID_MYS_SHOP;
                        }
                        const auto& ptt = PTTS_GET(mys_shop, pttid);

                        pd::mys_shop mys_shop;
                        mys_shop.set_pttid(ptt.id());

                        auto result = condition_check(ptt.conditions(), role);
                        if (result != pd::OK) {
                                return result;
                        }
                        result = condition_check(ptt.refresh_conditions(), role);
                        if (result != pd::OK) {
                                return result;
                        }

                        mys_shop.set_free_refreshed_time(role.mys_shop_free_refreshed_time(ptt.id()));

                        mys_shop_refresh_goods_process(ptt, mys_shop, *event_res, role);

                        role.update_mys_shop(mys_shop);
                        role.on_event(ECE_MYS_SHOP_REFRESH, { pttid });
                        return result;
                }

                pd::result mys_shop_check_buy(uint32_t pttid, uint32_t good_id, const role& role) {
                        if (!PTTS_HAS(mys_shop, pttid)) {
                                return pd::MYS_SHOP_INVALID_MYS_SHOP;
                        }
                        const auto& ptt = PTTS_GET(mys_shop, pttid);
                        auto result = condition_check(ptt.conditions(), role, pd::ce_env());
                        if (result != pd::OK) {
                                return result;
                        }

                        if (!role.has_mys_shop_good(ptt.id(), good_id)) {
                                return pd::MYS_SHOP_NO_SUCH_GOOD;
                        }

                        for (const auto& i : ptt.goods()) {
                                if (i.id() == good_id) {
                                        pd::ce_env ce;
                                        if (i.discount() > 0) {
                                                ce.set_cost_percent(i.discount() * 1000 / 100);
                                        }
                                        return condition_check(ptt.conditions(), role, ce);
                                }
                        }

                        ASSERT(false);
                        return pd::NOT_FOUND;
                }

                pd::event_res mys_shop_buy(uint32_t pttid, uint32_t good_id, role& role) {
                        const auto& ptt = PTTS_GET(mys_shop, pttid);
                        role.bought_mys_shop_good(ptt.id(), good_id);
                        for (const auto& i : ptt.goods()) {
                                if (i.id() == good_id) {
                                        pd::ce_env ce;
                                        ce.set_origin(pd::CO_SHOP_BUY_MYS_SHOP);
                                        if (i.discount() > 0) {
                                                ce.set_cost_percent(i.discount() * 1000 / 100);
                                        }
                                        return event_process(i.events(), role, ce);
                                }
                        }

                        ASSERT(false);
                        return pd::event_res();
                }

                pd::result shop_check_buy(uint32_t shop, uint32_t good_id, const role& role) {
                        if (!PTTS_HAS(shop, shop)) {
                                return pd::SHOP_NO_SUCH_SHOP;
                        }

                        auto ptt = PTTS_GET_COPY(shop, shop);
                        auto ret = condition_check(ptt.conditions(), role);
                        if (ret != pd::OK) {
                                return ret;
                        }

                        const pc::good *good = nullptr;
                        for (const auto& i : ptt.goods()) {
                                if (i.id() == good_id) {
                                        good = &i;
                                        break;
                                }
                        }
                        if (!good) {
                                return pd::SHOP_NO_SUCH_GOOD;
                        } else if (good->withdraw()) {
                                return pd::SHOP_GOOD_WITHDRAW;
                        }

                        ret = condition_check(good->visible_conditions(), role);
                        if (ret != pd::OK) {
                                return ret;
                        }

                        if (good->server_open_day_ranges_size() > 0) {
                                ret = pd::SERVER_OPEN_DAY_MISMATCH;
                                auto day = refresh_day() - player_mgr_class::instance().server_open_day();
                                for (const auto& i : good->server_open_day_ranges()) {
                                        if (day >= i.begin() && day <= i.end()) {
                                                ret = pd::OK;
                                                break;
                                        }
                                }
                        }
                        if (ret != pd::OK) {
                                return ret;
                        }

                        if (good->has_all_time_limit()) {
                                auto all_times = role.shop_buy_all_times(shop, good_id);
                                if (all_times >= good->all_time_limit()) {
                                        return pd::SHOP_GOOD_REACH_ALL_TIMES_LIMIT;
                                }
                        }

                        if (ptt.has__activity_festival()) {
                                auto start_day = role.activity_festival_start_day();
                                auto duration = role.activity_festival_duration();
                                auto festival_ptt = PTTS_GET(activity_festival, ptt._activity_festival());
                                if (start_day == 0 || start_day + duration < refresh_day()) {
                                        return pd::SHOP_NOT_ACRIVITY_FESTIVAL_TIME;
                                }
                                if (good->has_all_time_limit() && role.shop_record_times(festival_ptt.id(), good_id) + 1 > good->all_time_limit()) {
                                        return pd::ACTIVITY_FESTIVAL_SHOP_TIMES_NOT_ENOUGH;
                                }
                        }

                        if (ptt.has__discount_activity_id()) {
                                ret = role.check_activity_discount_goods(ptt._discount_activity_id(), *good);
                                if (ret != pd::OK) {
                                        return ret;
                                }
                        }

                        if (good->has__activity_seven_days_begin_day()) {
                                ret = activity_check_seven_days_buy_good(
                                        good->_activity_seven_days_begin_day(),
                                        good->_activity_seven_days_day(),
                                        role);
                                if (ret != pd::OK) {
                                        return ret;
                                }
                        }

                        if (good->vip_conditions_size() > 0) {
                                if (good->vip_conditions_size() == 1) {
                                        ret = condition_check(good->vip_conditions(0).conditions(), role);
                                        if (ret != pd::OK) {
                                                return ret;
                                        }
                                } else {
                                        auto vip_level = role.vip_level();
                                        ASSERT(vip_level < static_cast<uint32_t>(good->vip_conditions_size()));
                                        ASSERT(vip_level == good->vip_conditions(vip_level).vip_level());
                                        ret = condition_check(good->vip_conditions(vip_level).conditions(), role);
                                        if (ret != pd::OK) {
                                                return ret;
                                        }
                                }
                        }

                        if (good->type() != pc::good::RECHARGE) {
                                ret = condevent_check(good->_condevents(), role);
                                if (ret != pd::OK) {
                                        return ret;
                                }
                        }

                        return ret;
                }

                pd::event_res shop_buy(uint32_t shop, uint32_t good_id, role& role, pd::event_res *merge_res, bool send_bi) {
                        auto ptt = PTTS_GET_COPY(shop, shop);
                        const pc::good *good = nullptr;
                        for (const auto& i : ptt.goods()) {
                                if (i.id() == good_id) {
                                        good = &i;
                                        break;
                                }
                        }
                        ASSERT(good);
                        pd::event_res res;
                        if (merge_res) {
                                res = *merge_res;
                        }

                        pd::ce_env ce;
                        ce.set_origin(pd::CO_SHOP_BUY);
                        ASSERT(condevent_check(good->_condevents(), role, ce) == pd::OK);
                        res = condevent_process(good->_condevents(), role, ce, &res, send_bi);
                        role.on_event(ECE_SHOP_BUY, { good_id, 1u , shop});
                        return res;
                }

        }
}
