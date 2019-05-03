#include "shop_ptts.hpp"
#include "utils/log.hpp"
#include "utils.hpp"
#include "item_ptts.hpp"
#include "equipment_ptts.hpp"
#include "actor_ptts.hpp"
#include "spine_ptts.hpp"
#include "huanzhuang_ptts.hpp"
#include "recharge_ptts.hpp"
#include "utils/assert.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                shop_ptts& shop_ptts_instance() {
                        static shop_ptts inst;
                        return inst;
                }

                void shop_ptts_set_funcs() {
                        shop_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.goods()) {
                                        if (i.prices_size() <= 0) {
                                                if (i.type() != pc::good::RECHARGE) {
                                                        CONFIG_ELOG << ptt.id() << " good " << i.id() << " no price";
                                                }
                                        } else {
                                                if (i.prices(0).start() != 1) {
                                                        CONFIG_ELOG << ptt.id() << " good " << i.id() << " first price start not 1 is " << i.prices(0).start();
                                                }
                                                for (const auto& j : i.prices()) {
                                                        if (j.price() < 0) {
                                                                CONFIG_ELOG << ptt.id() << " price < 0";
                                                        }
                                                }
                                        }
                                        if (i.count() < 10 && i.count() >= 100) {
                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " invalid count " << i.count();
                                        }
                                        if (i.vip_conditions_size() != 0 && i.vip_conditions_size() != 1 && i.vip_conditions_size() != 17) {
                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " need 0 or 1 or 17 vip conditions, got " << i.vip_conditions_size();
                                        }
                                }
                        };
                        shop_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_goods()) {
                                        for (auto& j : *i.mutable_vip_conditions()) {
                                                for (auto& k : *j.mutable_conditions()->mutable_conditions()) {
                                                        if (k.type() == pd::condition::BUY_GOOD_TIMES_LESS) {
                                                                ASSERT(k.arg_size() >= 1);
                                                                auto args = k.arg();
                                                                k.clear_arg();
                                                                k.add_arg(to_string(ptt.id()));
                                                                k.add_arg(to_string(i.id()));
                                                                for (auto l : args) {
                                                                        k.add_arg(l);
                                                                }
                                                        }
                                                }
                                        }

                                        const auto& price_type_str = pd::resource_type_Name(i.price_type());
                                        for (auto j = 0; j < i.prices_size(); ++j) {
                                                const auto& p = i.prices(j);
                                                auto next_start = (j == i.prices_size() - 1) ? numeric_limits<int>::max() : i.prices(j + 1).start();
                                                auto *ce = i.mutable__condevents()->add_condevents();
                                                auto *cond = ce->mutable_conditions()->add_conditions();
                                                cond->set_type(pd::condition::BUY_GOOD_TIMES_LESS);
                                                cond->add_arg(to_string(ptt.id()));
                                                cond->add_arg(to_string(i.id()));
                                                cond->add_arg(to_string(next_start - 1));

                                                if (i.type() != pc::good::RECHARGE) {
                                                        cond = ce->mutable_conditions()->add_conditions();
                                                        cond->set_type(pd::condition::COST_RESOURCE);
                                                        cond->add_arg(price_type_str);
                                                        if (i.has_discount()) {
                                                                ASSERT(i.discount() < 100);
                                                                cond->add_arg(to_string(p.price() * i.discount() / 100));
                                                        } else {
                                                                cond->add_arg(to_string(p.price()));
                                                        }
                                                }

                                                if (i.type() == pc::good::SPINE) {
                                                        auto *cond = ce->mutable_conditions()->add_conditions();
                                                        cond->set_type(pd::condition::NO_SPINE_COLLECTION);
                                                        cond->add_arg(i.arg(0));
                                                        cond->add_arg(i.arg(1));
                                                }

                                                if (i.type() == pc::good::HUANZHUANG) {
                                                        auto *cond = ce->mutable_conditions()->add_conditions();
                                                        cond->set_type(pd::condition::NO_HUANZHUANG_COLLECTION);
                                                        cond->add_arg(i.arg(0));
                                                        cond->add_arg(i.arg(1));
                                                }

                                                auto *events = ce->mutable_events();
                                                auto *e = events->add_events();
                                                e->set_type(pd::event::ADD_BUY_GOOD_TIMES);
                                                e->add_arg(to_string(ptt.id()));
                                                e->add_arg(to_string(i.id()));

                                                modify_events_by_conditions(ce->conditions(), *events);

                                                // add event by good
                                                switch (i.type()) {
                                                case pc::good::ITEM: {
                                                        if (i.arg_size() != 1) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " ITEM need 1 arg";
                                                                break;
                                                        }
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::ADD_ITEM);
                                                        event->add_arg(i.arg(0));
                                                        event->add_arg(to_string(i.count()));
                                                        break;
                                                }
                                                case pc::good::EQUIPMENT: {
                                                        if (i.arg_size() != 1) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " EQUIPMENT need 1 arg";
                                                                break;
                                                        }
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::ADD_EQUIPMENT);
                                                        event->add_arg(i.arg(0));
                                                        event->add_arg(to_string(i.count()));
                                                        break;
                                                }
                                                case pc::good::RESOURCE: {
                                                        if (i.arg_size() != 1) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " RESOURCE need 1 arg";
                                                                break;
                                                        }
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::ADD_RESOURCE);
                                                        event->add_arg(i.arg(0));
                                                        event->add_arg(to_string(i.count()));
                                                        break;
                                                }
                                                case pc::good::SPINE: {
                                                        if (i.arg_size() != 2) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " SPINE need 2 arg";
                                                                break;
                                                        }
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::ADD_SPINE_COLLECTION);
                                                        event->add_arg(i.arg(0));
                                                        event->add_arg(i.arg(1));
                                                        break;
                                                }
                                                case pc::good::HUANZHUANG: {
                                                        if (i.arg_size() != 2 && i.arg_size() != 4) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " HUANZHUANG need 2 or 4 arg";
                                                                break;
                                                        }
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::ADD_HUANZHUANG_COLLECTION);
                                                        event->add_arg(i.arg(0));
                                                        event->add_arg(i.arg(1));
                                                        if (i.arg_size() == 4) {
                                                                event->add_arg(i.arg(2));
                                                                event->add_arg(i.arg(3));
                                                        }
                                                        break;
                                                }
                                                case pc::good::MANSION_HALL_QUEST: {
                                                        auto *event = events->add_events();
                                                        event->set_type(pd::event::MANSION_ADD_HALL_QUEST);
                                                        break;
                                                }
                                                case pc::good::RECHARGE: {
                                                        if (i.arg_size() != 1) {
                                                                CONFIG_ELOG << ptt.id() << " good " << i.id() << " RECHARGE need 1 arg";
                                                        }
                                                        break;
                                                }
                                                }
                                                modify_conditions_by_events(ce->events(), *ce->mutable_conditions());
                                        }
                                }

                                for (const auto& i : ptt.goods()) {
                                        for (const auto& j : i.vip_conditions()) {
                                                if (!check_conditions(j.conditions())) {
                                                        CONFIG_ELOG << ptt.id() << " good " << i.id() << " check vip conditions failed";
                                                }
                                        }
                                }
                        };
                        shop_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }

                                for (const auto& i : ptt.goods()) {
                                        for (const auto& j : i.vip_conditions()) {
                                                if (!verify_conditions(j.conditions())) {
                                                        CONFIG_ELOG << ptt.id() << " good " << i.id() << " verify vip conditions failed";
                                                }
                                        }

                                        switch(i.type()) {
                                        case pc::good::ITEM: {
                                                if (i.arg_size() != 1) {
                                                        continue;
                                                }
                                                if (!PTTS_HAS(item, stoul(i.arg(0)))) {
                                                        CONFIG_ELOG << ptt.id() << " item not exist " << i.arg(0);
                                                }
                                                break;
                                        }
                                        case pc::good::EQUIPMENT: {
                                                if (i.arg_size() != 1) {
                                                        continue;
                                                }
                                                if (!PTTS_HAS(equipment, stoul(i.arg(0)))) {
                                                        CONFIG_ELOG << ptt.id() << " equipment not exist " << i.arg(0);
                                                }
                                                break;
                                        }
                                        case pc::good::RESOURCE: {
                                                if (i.arg_size() != 1) {
                                                        continue;
                                                }

                                                pd::resource_type type;
                                                if (!pd::resource_type_Parse(i.arg(0), &type)) {
                                                        CONFIG_ELOG << ptt.id() << " invalid resource " << i.arg(0);
                                                }
                                                break;
                                        }
                                        case pc::good::SPINE: {
                                                if (i.arg_size() != 2 && i.arg_size() != 4) {
                                                        continue;
                                                }

                                                pd::spine_part part;
                                                if (!pd::spine_part_Parse(i.arg(0), &part)) {
                                                        CONFIG_ELOG << ptt.id() << " invalid spine type " << i.arg(0);
                                                } else {
                                                        if (!SPINE_PTTS_HAS(part, stoul(i.arg(1)))) {
                                                                CONFIG_ELOG << ptt.id() << " " << pd::spine_part_Name(part) << " not exist " << i.arg(1);
                                                        }
                                                }
                                                if (i.arg_size() == 4) {
                                                        if (!pd::spine_part_Parse(i.arg(2), &part)) {
                                                                CONFIG_ELOG << ptt.id() << " invalid spine type " << i.arg(2);
                                                        } else {
                                                                if (!SPINE_PTTS_HAS(part, stoul(i.arg(3)))) {
                                                                        CONFIG_ELOG << ptt.id() << " " << pd::spine_part_Name(part) << " not exist " << i.arg(3);
                                                                }
                                                        }
                                                }
                                                break;
                                        }
                                        case pc::good::HUANZHUANG: {
                                                if (i.arg_size() != 2 && i.arg_size() != 4) {
                                                        continue;
                                                }

                                                pd::huanzhuang_part part;
                                                if (!pd::huanzhuang_part_Parse(i.arg(0), &part)) {
                                                        CONFIG_ELOG << ptt.id() << " invalid spine type " << i.arg(0);
                                                } else {
                                                        if (!HUANZHUANG_PTTS_HAS(part, stoul(i.arg(1)))) {
                                                                CONFIG_ELOG << ptt.id() << " " << huanzhuang_part_Name(part) << " not exist " << i.arg(1);
                                                        }
                                                }
                                                if (i.arg_size() == 4) {
                                                        if (!pd::huanzhuang_part_Parse(i.arg(2), &part)) {
                                                                CONFIG_ELOG << ptt.id() << " invalid spine type " << i.arg(2);
                                                        } else {
                                                                if (!HUANZHUANG_PTTS_HAS(part, stoul(i.arg(3)))) {
                                                                        CONFIG_ELOG << ptt.id() << " " << huanzhuang_part_Name(part) << " not exist " << i.arg(3);
                                                                }
                                                        }
                                                }
                                                break;
                                        }
                                        case pc::good::RECHARGE: {
                                                if (i.arg_size() != 1) {
                                                        continue;
                                                }
                                                auto recharge_pttid = stoul(i.arg(0));
                                                if (!PTTS_HAS(recharge, recharge_pttid)) {
                                                        CONFIG_ELOG << ptt.id() << " recharge not exist " << recharge_pttid;
                                                        continue;
                                                }
                                                auto& recharge_ptt = PTTS_GET(recharge, recharge_pttid);
                                                recharge_ptt.set__shop(ptt.id());
                                                recharge_ptt.set__shop_good(i.id());
                                                break;
                                        }
                                        default:
                                        break;
                                        }
                                }
                        };
                }

                mys_shop_ptts& mys_shop_ptts_instance() {
                        static mys_shop_ptts inst;
                        return inst;
                }

                void mys_shop_ptts_set_funcs() {
                        mys_shop_ptts_instance().check_func_ = [] (auto& ptt) {
                                if (!check_conditions(ptt.refresh_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check refresh conditions failed";
                                }
                                for (const auto& i : ptt.goods()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check good conditions failed";
                                        }
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check good events failed";
                                        }
                                        if (i.discount() > 0 && i.discount() > 100) {
                                                CONFIG_ELOG << ptt.id() << " discount should be smaller than 100";
                                        }
                                }
                        };
                        mys_shop_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.refresh_conditions(), *(ptt.mutable__refresh_events()));
                                for (auto& i : *ptt.mutable_goods()) {
                                        modify_events_by_conditions(i.conditions(), *(i.mutable_events()));
                                }
                        };
                        mys_shop_ptts_instance().verify_func_ = [] (auto& ptt) {
                                if (!verify_conditions(ptt.refresh_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify refresh conditions failed";
                                }
                                for (const auto& i : ptt.goods()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify good conditions failed";
                                        }
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify good events failed";
                                        }
                                }
                        };
                }

        }
}
