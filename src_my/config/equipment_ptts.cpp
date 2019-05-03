#include "utils.hpp"
#include "equipment_ptts.hpp"
#include "growth_ptts.hpp"
#include "origin_ptts.hpp"
#include "chat_ptts.hpp"

namespace nora {
        namespace config {

                equipment_ptts& equipment_ptts_instance() {
                        static equipment_ptts inst;
                        return inst;
                }

                void equipment_ptts_set_funcs() {
                        equipment_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(not_portional_growth, ptt.fix_attr().levelup_growth())) {
                                        CONFIG_ELOG << ptt.id() << " levelup growth not exist " << ptt.fix_attr().levelup_growth();
                                }
                                if (!PTTS_HAS(equipment_levelup, ptt.levelup_pttid())) {
                                        CONFIG_ELOG << ptt.id() << " levelup pttid not exist " << ptt.levelup_pttid();
                                }
                                if (!PTTS_HAS(equipment_add_quality, ptt.add_quality_pttid())) {
                                        CONFIG_ELOG << ptt.id() << " add quality pttid not exist " << ptt.add_quality_pttid();
                                }
                                for (auto i : ptt.origins()) {
                                        if (!PTTS_HAS(origin, i)) {
                                                CONFIG_ELOG << ptt.id() << " origin not exist " << i;
                                        }
                                }
                        };
                }

                equipment_levelup_ptts& equipment_levelup_ptts_instance() {
                        static equipment_levelup_ptts inst;
                        return inst;
                }

                void equipment_levelup_ptts_set_funcs() {
                        equipment_levelup_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_condevents()->mutable_condevents()) {
                                        auto *events = i.mutable__events();
                                        auto *e = events->add_events();
                                        e->set_type(pd::event::EQUIPMENT_LEVELUP);

                                        modify_events_by_conditions(i.conditions(), *events);
                                }
                        };
                }

                equipment_suite_ptts& equipment_suite_ptts_instance() {
                        static equipment_suite_ptts inst;
                        return inst;
                }

                void equipment_suite_ptts_set_funcs() {
                        equipment_suite_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto i : ptt.equipments()) {
                                        auto& equipment_ptt = PTTS_GET(equipment, i);
                                        equipment_ptt.set__suite(ptt.id());
                                }
                        };
                        equipment_suite_ptts_instance().verify_func_ = [] (auto& ptt) {
                                for (auto i : ptt.equipments()) {
                                        if (!PTTS_HAS(equipment, i)) {
                                                CONFIG_ELOG << ptt.id() << " got not exist equipment " << i;
                                        }
                                }
                        };
                }

                equipment_rand_attr_pool_ptts& equipment_rand_attr_pool_ptts_instance() {
                        static equipment_rand_attr_pool_ptts inst;
                        return inst;
                }

                void equipment_rand_attr_pool_ptts_set_funcs() {
                }

                equipment_rand_attr_quality_ptts& equipment_rand_attr_quality_ptts_instance() {
                        static equipment_rand_attr_quality_ptts inst;
                        return inst;
                }

                void equipment_rand_attr_quality_ptts_set_funcs() {
                }

                equipment_add_quality_ptts& equipment_add_quality_ptts_instance() {
                        static equipment_add_quality_ptts inst;
                        return inst;
                }

                void equipment_add_quality_ptts_set_funcs() {
                        equipment_add_quality_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_condevents()->mutable_condevents()) {
                                        auto *events = i.mutable_events();
                                        auto *e = events->add_events();
                                        e->set_type(pd::event::EQUIPMENT_ADD_QUALITY);

                                        modify_events_by_conditions(i.conditions(), *events);
                                }
                        };
                }
                equipment_decompose_ptts& equipment_decompose_ptts_instance() {
                        static equipment_decompose_ptts inst;
                        return inst;
                }

                void equipment_decompose_ptts_set_funcs() {
                        equipment_decompose_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.quality_to_point_size() != 5) {
                                        CONFIG_ELOG << ptt.id() << " need 5 quality_to_point got " << ptt.quality_to_point_size();
                                }
                        };
                }

                equipment_compose_ptts& equipment_compose_ptts_instance() {
                        static equipment_compose_ptts inst;
                        return inst;
                }

                void equipment_compose_ptts_set_funcs() {
                        equipment_compose_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.quality_size() != 4) {
                                        CONFIG_ELOG << "need 4 quality, got " << ptt.quality_size();
                                }
                                for (const auto& i : ptt.quality()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << "check quality conditions failed";
                                        }
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << "check quality events failed";
                                        }
                                }
                        };
                        equipment_compose_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_quality()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable_events());
                                }
                        };
                        equipment_compose_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.quality()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << " verify quality conditions failed";
                                        }
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << " verify quality events failed";
                                        }
                                }
                        };
                }

                equipment_logic_ptts& equipment_logic_ptts_instance() {
                        static equipment_logic_ptts inst;
                        return inst;
                }

                void equipment_logic_ptts_set_funcs() {
                        equipment_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.star_fragment_conditions())) {
                                        CONFIG_ELOG << "check star fragment conditions failed";
                                }
                                if (!check_events(ptt.star_fragment_events())) {
                                        CONFIG_ELOG << "check star fragment events failed";
                                }
                                if (!check_conditions(ptt.star_soul_conditions())) {
                                        CONFIG_ELOG << "check star soul conditions failed";
                                }
                                if (!check_events(ptt.star_soul_events())) {
                                        CONFIG_ELOG << "check star soul events failed";
                                }
                                if (!check_conditions(ptt.change_rand_attr_conditions())) {
                                        CONFIG_ELOG << "check change rand attr conditions failed";
                                }
                                if (!check_conditions(ptt.decompose_conditions())) {
                                        CONFIG_ELOG << "check decompose conditions failed";
                                }
                                if (!check_conditions(ptt.xilian_conditions())) {
                                        CONFIG_ELOG << "check xilian conditions failed";
                                }
                                if (!check_conditions(ptt.xilian_lock_conditions())) {
                                        CONFIG_ELOG << "check xilian lock conditions failed";
                                }
                        };
                        equipment_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.change_rand_attr_conditions(), *ptt.mutable__change_rand_attr_events());
                                modify_events_by_conditions(ptt.star_fragment_conditions(), *ptt.mutable_star_fragment_events());
                                modify_events_by_conditions(ptt.star_soul_conditions(), *ptt.mutable_star_soul_events());
                                modify_events_by_conditions(ptt.xilian_conditions(), *ptt.mutable__xilian_events());
                                modify_events_by_conditions(ptt.xilian_lock_conditions(), *ptt.mutable__xilian_lock_events());
                        };
                        equipment_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(system_chat, ptt.system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " system chat not exit " << ptt.system_chat();
                                }
                                for (auto i : ptt.exp_equipments()) {
                                        if (!PTTS_HAS(equipment, i)) {
                                                CONFIG_ELOG << ptt.id() << " exp equipment not exist " << i;
                                        }
                                }
                                if (!verify_conditions(ptt.star_fragment_conditions())) {
                                        CONFIG_ELOG << "verify star fragment conditions failed";
                                }
                                if (!verify_events(ptt.star_fragment_events())) {
                                        CONFIG_ELOG << "verify star fragment events failed";
                                }
                                if (!verify_conditions(ptt.star_soul_conditions())) {
                                        CONFIG_ELOG << "verify star soul conditions failed";
                                }
                                if (!verify_events(ptt.star_soul_events())) {
                                        CONFIG_ELOG << "verify star soul events failed";
                                }
                                if (!verify_conditions(ptt.change_rand_attr_conditions())) {
                                        CONFIG_ELOG << "verify change rand attr conditions failed";
                                }
                                if (!verify_events(ptt._change_rand_attr_events())) {
                                        CONFIG_ELOG << "verify change rand attr events failed";
                                }
                                if (!verify_conditions(ptt.decompose_conditions())) {
                                        CONFIG_ELOG << "verify decompose conditions failed";
                                }
                                if (!verify_conditions(ptt.xilian_conditions())) {
                                        CONFIG_ELOG << "verify xilian conditions failed";
                                }
                                if (!verify_events(ptt._xilian_events())) {
                                        CONFIG_ELOG << "verify xilian events failed";
                                }
                                if (!verify_conditions(ptt.xilian_lock_conditions())) {
                                        CONFIG_ELOG << "verify xilian lock conditions failed";
                                }
                                if (!verify_events(ptt._xilian_lock_events())) {
                                        CONFIG_ELOG << "verify xilian lock events failed";
                                }
                        };
                }

        }
}
