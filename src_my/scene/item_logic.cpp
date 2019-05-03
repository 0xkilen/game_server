#include "item_logic.hpp"
#include "scene/condition.hpp"
#include "scene/event.hpp"
#include "scene/player/role.hpp"
#include "scene/item.hpp"
#include "config/item_ptts.hpp"

namespace nora {
        namespace scene {

                pd::result item_check_use(uint32_t pttid, int count, const role& role) {
                        if (!role.has_item(pttid)) {
                                return pd::ROLE_NO_SUCH_ITEM;
                        }
                        auto item = role.get_item(pttid);
                        if (item->count() < count) {
                                return pd::ROLE_ITEM_NOT_ENOUGH;
                        }
                        const auto& ptt = PTTS_GET(item, item->pttid());
                        if (count > ptt.max_use_count()) {
                                return pd::ITEM_USE_TOO_MANY;
                        }
                        auto ret = condition_check(ptt.use_conditions(), role);
                        if (ret != pd::OK) {
                                return ret;
                        }
                        if (!ptt.direct_use()) {
                                return pd::ITEM_CANNOT_USE_DIRECTLY;
                        }
                        if (ptt.has_everyday_use_count()) {
                                if (role.get_item_everyday_use_count(pttid) + count > ptt.everyday_use_count()) {
                                        return pd::ITEM_USE_EXCEED_EVERYDAY_COUNT_LIMIT;
                                }
                        }
                        return pd::OK;
                }

                pd::event_res item_use(uint32_t pttid, int count, role& role) {
                        auto item = role.get_item(pttid);
                        ASSERT(item->count() >= count);
                        role.on_event(ECE_COST_ITEM, { pttid, count });
                        const auto& ptt = PTTS_GET(item, item->pttid());
                        pd::event_res ret;
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ITEM_USE);
                        bool send_bi = false;
                        for (auto i = 0; i < count; ++i) {
                                ret = event_process(ptt.use_events(), role, ce, &ret, false);
                                send_bi = true;
                        }
                        if (ptt.has_everyday_use_count()) {
                                role.update_item_everyday_use_count(pttid, count);
                        }
                        if (send_bi) {
                                send_event_res_to_bi(role, ret, ce.origin());
                        }
                        return ret;
                }

                pd::result item_check_xinwu_compose(uint32_t xinwu, uint32_t count, const role& role) {
                        if (!PTTS_HAS(item_xinwu_exchange, xinwu)) {
                                return pd::ITEM_XINWU_COMPOSE_INVALID_XINWU;
                        }
                        if (count == 0) {
                                return pd::ITEM_XINWU_COMPOSE_INVALID_COUNT;
                        }
                        const auto& ptt = PTTS_GET(item_xinwu_exchange, xinwu);
                        pd::ce_env ce;
                        ce.set_cost_percent(1000 * count);
                        return condition_check(ptt.compose_conditions(), role);
                }

                pd::event_res item_xinwu_compose(uint32_t xinwu, uint32_t count, role& role) {
                        const auto& ptt = PTTS_GET(item_xinwu_exchange, xinwu);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ITEM_XINWU_COMPOSE);
                        ce.set_cost_percent(1000 * count);
                        ce.set_add_percent(1000 * count);
                        return event_process(ptt.compose_events(), role, ce);
                }

                pd::result item_check_xinwu_decompose(uint32_t xinwu, uint32_t count, const role& role) {
                        if (!PTTS_HAS(item_xinwu_exchange, xinwu)) {
                                return pd::ITEM_XINWU_DECOMPOSE_INVALID_XINWU;
                        }
                        if (count == 0) {
                                return pd::ITEM_XINWU_DECOMPOSE_INVALID_COUNT;
                        }
                        const auto& ptt = PTTS_GET(item_xinwu_exchange, xinwu);
                        pd::ce_env ce;
                        ce.set_cost_percent(1000 * count);
                        return condition_check(ptt.decompose_conditions(), role);
                }

                pd::event_res item_xinwu_decompose(uint32_t xinwu, uint32_t count, role& role) {
                        const auto& ptt = PTTS_GET(item_xinwu_exchange, xinwu);
                        pd::ce_env ce;
                        ce.set_origin(pd::CO_ITEM_XINWU_DECOMPOSE);
                        ce.set_cost_percent(1000 * count);
                        ce.set_add_percent(1000 * count);
                        return event_process(ptt.decompose_events(), role, ce);
                }

        }

}
