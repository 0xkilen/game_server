#include "utils.hpp"
#include "utils/time_utils.hpp"
#include "utils/dirty_word_filter.hpp"
#include "mansion_ptts.hpp"
#include "plot_ptts.hpp"
#include "actor_ptts.hpp"
#include "item_ptts.hpp"
#include "drop_ptts.hpp"
#include "chat_ptts.hpp"
#include "huanzhuang_ptts.hpp"
#include "proto/config_mansion.pb.h"

namespace pc = proto::config;

namespace nora {
        namespace config {

                namespace {

                        void check_mansion_building_common(uint32_t level, const pc::mansion_building_common& common) {
                                if (level == 1 && common.need_fancy() > 0) {
                                        CONFIG_ELOG << level << " need fancy should be 0, got " << common.need_fancy();
                                }
                                if (common.random_plot_size() == 0) {
                                        CONFIG_ELOG << level << " need visit random plot";
                                }
                        }

                        void verify_mansion_building_common(uint32_t level, const pc::mansion_building_common& common) {
                                for (auto i : common.quests()) {
                                        if (!PTTS_HAS(mansion_quest, i)) {
                                                CONFIG_ELOG << level << " quest not exist " << i;
                                        }
                                }
                                if (common.has_other_quest() && !PTTS_HAS(mansion_quest, common.other_quest())) {
                                        CONFIG_ELOG << level << " quest not exist " << common.other_quest();
                                }
                        }

                }

                mansion_furniture_group_ptts& mansion_furniture_group_ptts_instance() {
                        static mansion_furniture_group_ptts inst;
                        return inst;
                }

                void mansion_furniture_group_ptts_set_funcs() {
                        mansion_furniture_group_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto i : ptt.furniture()) {
                                        if (!PTTS_HAS(mansion_furniture, i)) {
                                                continue;
                                        }

                                        auto& furniture_ptt = PTTS_GET(mansion_furniture, i);
                                        furniture_ptt.set__group(ptt.id());

                                        for (const auto& j : ptt.conditions().conditions()) {
                                                *furniture_ptt.mutable_unlock_conditions()->add_conditions() = j;
                                                *furniture_ptt.mutable_buy_conditions()->add_conditions() = j;
                                        }
                                }
                        };
                        mansion_furniture_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.furniture()) {
                                        if (!PTTS_HAS(mansion_furniture, i)) {
                                                CONFIG_ELOG << ptt.id() << " furniture not exist " << i;
                                        }
                                }
                        };
                }

                mansion_furniture_type_ptts& mansion_furniture_type_ptts_instance() {
                        static mansion_furniture_type_ptts inst;
                        return inst;
                }

                void mansion_furniture_type_ptts_set_funcs() {
                }

                mansion_furniture_collision_group_ptts& mansion_furniture_collision_group_ptts_instance() {
                        static mansion_furniture_collision_group_ptts inst;
                        return inst;
                }

                void mansion_furniture_collision_group_ptts_set_funcs() {
                }

                mansion_area_ptts& mansion_area_ptts_instance() {
                        static mansion_area_ptts inst;
                        return inst;
                }

                void mansion_area_ptts_set_funcs() {
                }

                mansion_furniture_ptts& mansion_furniture_ptts_instance() {
                        static mansion_furniture_ptts inst;
                        return inst;
                }

                void mansion_furniture_ptts_set_funcs() {
                        mansion_furniture_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.buildings_size() == 0) {
                                        CONFIG_ELOG << ptt.id() << " need at least 1 buildings";
                                }
                        };
                        mansion_furniture_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.unlock_conditions(), *ptt.mutable__unlock_events());
                                modify_events_by_conditions(ptt.buy_conditions(), *ptt.mutable__buy_events());
                        };
                        mansion_furniture_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!ptt.has_appearance()) {
                                        if (ptt.sub_furniture_size() == 0) {
                                                CONFIG_ELOG << ptt.id() << " no appearance and no sub furniture";
                                        } else {
                                                if (!PTTS_HAS(mansion_furniture_type, ptt.type())) {
                                                        CONFIG_ELOG << ptt.id() << " furniture type not fixed " << ptt.type();
                                                } else {
                                                        const auto& type_ptt = PTTS_GET(mansion_furniture_type, ptt.type());
                                                        if (!type_ptt.pos_fixed()) {
                                                                CONFIG_ELOG << ptt.id() << " has sub furniture but type pos not fixed";
                                                        }
                                                }
                                        }
                                }
                                for (auto i : ptt.sub_furniture()) {
                                        if (!PTTS_HAS(mansion_furniture, i)) {
                                                CONFIG_ELOG << ptt.id() << " sub furniture not exist " << i;
                                                continue;
                                        }
                                        const auto& sub_ptt = PTTS_GET(mansion_furniture, i);
                                        if (!sub_ptt.has_appearance()) {
                                                CONFIG_ELOG << ptt.id() << " sub furniture no appearance " << i;
                                        }
                                        if (sub_ptt.sub_furniture_size() > 0) {
                                                CONFIG_ELOG << ptt.id() << " sub furniture has sub furniture " << i;
                                        }
                                        if (sub_ptt.buildings_size() != 1) {
                                                CONFIG_ELOG << ptt.id() << " sub furniture should has 1 buildings, got " << sub_ptt.buildings_size();
                                                continue;
                                        }
                                        if (!sub_ptt.buildings(0).has_init_pos()) {
                                                CONFIG_ELOG << ptt.id() << " sub furniture no init pos " << i;
                                        }
                                }
                                const auto& logic_ptt = PTTS_GET(mansion_logic, 1);
                                for (const auto& i : ptt.buildings()) {
                                        for (const auto& j : logic_ptt.buildings()) {
                                                if (i.building() == j.building()) {
                                                        for (auto k : i.allowed_areas()) {
                                                                auto has = false;
                                                                for (auto l : j.areas()) {
                                                                        if (k == l) {
                                                                                has = true;
                                                                                break;
                                                                        }
                                                                }
                                                                if (!has) {
                                                                        CONFIG_ELOG << ptt.id() << " alowed area not exist in building " << k;
                                                                }
                                                        }
                                                        const auto& type_ptt = PTTS_GET(mansion_furniture_type, ptt.type());
                                                        if (type_ptt.pos_fixed() && i.allowed_areas_size() > 0) {
                                                                if (i.allowed_areas_size() != 1) {
                                                                        CONFIG_ELOG << ptt.id() << " pos fixed and has allowed areas need 1 allow areas, got " << i.allowed_areas_size();
                                                                }
                                                        }
                                                }
                                        }
                                }

                                if (ptt.has_type()) {
                                        const auto& type_ptt = PTTS_GET(mansion_furniture_type, ptt.type());
                                        if (type_ptt.pos_fixed()) {
                                                for (const auto& i : ptt.buildings()) {
                                                        if (!i.has_init_pos()) {
                                                                CONFIG_ELOG << ptt.id()
                                                                            << " type pos fixed but no init pos in building "
                                                                            << pd::mansion_building_type_Name(i.building());
                                                        }
                                                }
                                        }
                                }

                                if (ptt.has_collision_group() && !PTTS_HAS(mansion_furniture_collision_group, ptt.collision_group())) {
                                        CONFIG_ELOG << ptt.id() << " collision_group not exist " << ptt.collision_group();
                                }
                        };
                }

                mansion_hall_ptts& mansion_hall_ptts_instance() {
                        static mansion_hall_ptts inst;
                        return inst;
                }

                void mansion_hall_ptts_set_funcs() {
                        mansion_hall_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_mansion_building_common(ptt.level(), ptt.common());
                        };
                        mansion_hall_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.common().quests()) {
                                        if (!PTTS_HAS(mansion_hall_quest, i)) {
                                                CONFIG_ELOG << ptt.level() << " quest not exist " << i;
                                        }
                                }
                        };
                }

                mansion_hall_quest_ptts& mansion_hall_quest_ptts_instance() {
                        static mansion_hall_quest_ptts inst;
                        return inst;
                }

                void mansion_hall_quest_ptts_set_funcs() {
                        mansion_hall_quest_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(plot, ptt.plot())) {
                                        CONFIG_ELOG << ptt.id() << " plot not exist " << ptt.plot();
                                }
                                if (!PTTS_HAS(plot, ptt.lucky_plot())) {
                                        CONFIG_ELOG << ptt.id() << " plot not exist " << ptt.lucky_plot();
                                }
                        };
                }

                mansion_quest_ptts& mansion_quest_ptts_instance() {
                        static mansion_quest_ptts inst;
                        return inst;
                }

                void mansion_quest_ptts_set_funcs() {
                        mansion_quest_ptts_instance().check_func_ = [] (const auto& ptt) {
                        };
                        mansion_quest_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(drop, ptt.base_drop())) {
                                        CONFIG_ELOG << ptt.id() << " base drop not exist " << ptt.base_drop();
                                }
                                if (ptt.craft_drops().times_ranges_size() != 6) {
                                        CONFIG_ELOG << ptt.id() << " need 6 times ranges, got " << ptt.craft_drops().times_ranges_size();
                                }
                                if (ptt.craft_drops().craft_drops_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " need 3 craft drops, got " << ptt.craft_drops().craft_drops_size();
                                }
                                for (const auto& i : ptt.craft_drops().craft_drops()) {
                                        if (i.drops_size() != 1) {
                                                CONFIG_ELOG << ptt.id() << " need 1 craft drops drops, got " << i.drops_size();
                                        }
                                        for (auto j : i.drops()) {
                                                if (!PTTS_HAS(drop, j)) {
                                                        CONFIG_ELOG << ptt.id() << " craft drop drop not exist " << j;
                                                }
                                        }
                                }
                        };
                }

                mansion_incident_ptts& mansion_incident_ptts_instance() {
                        static mansion_incident_ptts inst;
                        return inst;
                }

                void mansion_incident_ptts_set_funcs() {

                }

                mansion_craft_type_summary_ptts& mansion_craft_type_summary_ptts_instance() {
                        static mansion_craft_type_summary_ptts inst;
                        return inst;
                }

                void mansion_craft_type_summary_ptts_set_funcs() {
                        mansion_craft_type_summary_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.fishes()) {
                                        if (!PTTS_HAS(item, i)) {
                                                CONFIG_ELOG << pd::actor::craft_type_Name(ptt.craft()) << " fish item not exist " << i;
                                        }
                                }
                                for (auto i : ptt.vegetables()) {
                                        if (!PTTS_HAS(item, i)) {
                                                CONFIG_ELOG << pd::actor::craft_type_Name(ptt.craft()) << " vegetable item not exist " << i;
                                        }
                                }
                                for (auto i : ptt.meats()) {
                                        if (!PTTS_HAS(item, i)) {
                                                CONFIG_ELOG << pd::actor::craft_type_Name(ptt.craft()) << " meat item not exist " << i;
                                        }
                                }
                        };
                }

                mansion_pond_ptts& mansion_pond_ptts_instance() {
                        static mansion_pond_ptts inst;
                        return inst;
                }

                void mansion_pond_ptts_set_funcs() {
                        mansion_pond_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.fish_drops().craft_drops()) {
                                        if (i.drops_size() != 4) {
                                                CONFIG_ELOG << ptt.level() << " need 4 fish_drop drop, got " << i.drops_size();
                                        }
                                }
                                check_mansion_building_common(ptt.level(), ptt.common());
                        };
                        mansion_pond_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_mansion_building_common(ptt.level(), ptt.common());
                        };
                }

                mansion_kitchen_ptts& mansion_kitchen_ptts_instance() {
                        static mansion_kitchen_ptts inst;
                        return inst;
                }

                void mansion_kitchen_ptts_set_funcs() {
                        mansion_kitchen_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_mansion_building_common(ptt.level(), ptt.common());
                        };
                        mansion_kitchen_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_mansion_building_common(ptt.level(), ptt.common());
                        };
                }

                mansion_bedroom_ptts& mansion_bedroom_ptts_instance() {
                        static mansion_bedroom_ptts inst;
                        return inst;
                }

                void mansion_bedroom_ptts_set_funcs() {
                        mansion_bedroom_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_mansion_building_common(ptt.level(), ptt.common());
                        };
                        mansion_bedroom_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_mansion_building_common(ptt.level(), ptt.common());
                        };
                }

                mansion_vegetable_plot_ptts& mansion_vegetable_plot_ptts_instance() {
                        static mansion_vegetable_plot_ptts inst;
                        return inst;
                }

                void mansion_vegetable_plot_ptts_set_funcs() {
                        mansion_vegetable_plot_ptts_instance().check_func_ = [] (const auto& ptt) {
                                check_mansion_building_common(ptt.level(), ptt.common());
                        };
                        mansion_vegetable_plot_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                verify_mansion_building_common(ptt.level(), ptt.common());
                        };
                }

                mansion_bedroom_tool_ptts& mansion_bedroom_tool_ptts_instance() {
                        static mansion_bedroom_tool_ptts inst;
                        return inst;
                }

                void mansion_bedroom_tool_ptts_set_funcs() {
                }

                mansion_logic_ptts& mansion_logic_ptts_instance() {
                        static mansion_logic_ptts inst;
                        return inst;
                }

                void mansion_logic_ptts_set_funcs() {
                        mansion_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.create_conditions())) {
                                        CONFIG_ELOG << " check create conditions failed";
                                }

                                if (!check_conditions(ptt.housekeeper_conditions())) {
                                        CONFIG_ELOG << " check housekeeper conditions failed";
                                }

                                for (const auto& i : ptt.banquet().switch_dish_condevents().condevents()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << " check banquet switch dish conditions failed";
                                        }
                                }
                                if (ptt.banquet().dish().points_size() <= 0) {
                                        CONFIG_ELOG << "need at least one points, got " << ptt.banquet().dish().points_size();
                                } else if (ptt.banquet().dish().points(0).combo_times() != 0) {
                                        CONFIG_ELOG << "first points should set combo times as 0, but is " << ptt.banquet().dish().points(0).combo_times();
                                }
                                if (ptt.banquet().dish().level_points_size() != 4) {
                                        CONFIG_ELOG << "need 4 level points, got " << ptt.banquet().dish().level_points_size();
                                }
                                for (auto i : ptt.banquet().dish().level_points()) {
                                        if (i.point() < ptt.banquet().dish().basic_points() || i.point() > ptt.banquet().dish().max_points()) {
                                                CONFIG_ELOG << "level point overflow " << i.point();
                                        }
                                }
                                if (ptt.banquet().dish().basic_points() > ptt.banquet().dish().max_points()) {
                                        CONFIG_ELOG << "dish basic points is greater than max ponits";
                                }
                                for (const auto& i : ptt.banquet().weilie_game_cfg().refresh_data()) {
                                        if (!PTTS_HAS(mansion_banquet_weilie_game, i.pttid())) {
                                                CONFIG_ELOG << "mansion_banquet_weilie_game table not exist " << i.pttid();
                                        }
                                }
                                if (ptt.create_hall_quest_count() <= 0) {
                                        CONFIG_ELOG << "create hall quest count is 0";
                                }

                                if (!check_events(ptt.help().events())) {
                                        CONFIG_ELOG << "check help events failed";
                                }

                                if (!check_events(ptt.visit().events())) {
                                        CONFIG_ELOG << "check visit events failed";
                                }

                                if (!check_conditions(ptt.free_fishing_conditions())) {
                                        CONFIG_ELOG << " check free_fishing_conditions failed";
                                }
                                if (!check_events(ptt.free_fishing_events())) {
                                        CONFIG_ELOG << "check free_fishing_events failed";
                                }

                                if (!check_conditions(ptt.free_water_conditions())) {
                                        CONFIG_ELOG << " check free_water_conditions failed";
                                }
                                if (!check_events(ptt.free_water_events())) {
                                        CONFIG_ELOG << "check free_water_events failed";
                                }

                                if (!check_conditions(ptt.free_disinsect_conditions())) {
                                        CONFIG_ELOG << " check free_disinsect_conditions failed";
                                }
                                if (!check_events(ptt.free_disinsect_events())) {
                                        CONFIG_ELOG << "check free_disinsect_events failed";
                                }

                                if (!check_conditions(ptt.free_steal_plant_conditions())) {
                                        CONFIG_ELOG << " check free_steal_plant_conditions failed";
                                }
                                if (!check_events(ptt.free_steal_plant_events())) {
                                        CONFIG_ELOG << "check free_steal_plant_events failed";
                                }

                        };
                        mansion_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *banquet = ptt.mutable_banquet();
                                for (auto& i : *banquet->mutable_switch_dish_condevents()->mutable_condevents()) {
                                        auto *events = i.mutable__events();
                                        modify_events_by_conditions(i.conditions(), *events);
                                }
                                for (const auto& i : banquet->banquet_period()) {
                                        auto *ep = banquet->add__banquet_enter_period();
                                        *ep->mutable_begin() = min_before_time_point(i.begin(), 5);
                                        *ep->mutable_end() = i.end();
                                }

                                auto *c = ptt.mutable_farm()->mutable__fertilize_conditions()->add_conditions();
                                c->set_type(pd::condition::COST_ITEM);
                                c->add_arg(to_string(ptt.farm().fertilizer_item()));
                                c->add_arg("1");

                                modify_events_by_conditions(ptt.farm()._fertilize_conditions(), *ptt.mutable_farm()->mutable__fertilize_events());
                                modify_events_by_conditions(ptt.farm().steal_conditions(), *ptt.mutable_farm()->mutable__steal_events());
                                modify_events_by_conditions(ptt.free_fishing_conditions(), *ptt.mutable_free_fishing_events());
                                modify_events_by_conditions(ptt.free_water_conditions(), *ptt.mutable_free_water_events());
                                modify_events_by_conditions(ptt.free_disinsect_conditions(), *ptt.mutable_free_disinsect_events());
                                modify_events_by_conditions(ptt.free_steal_plant_conditions(), *ptt.mutable_free_steal_plant_events());
                        };
                        mansion_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.create_conditions())) {
                                        CONFIG_ELOG << " verify create conditions failed";
                                }

                                if (!verify_conditions(ptt.housekeeper_conditions())) {
                                        CONFIG_ELOG << " verify housekeeper conditions failed";
                                }

                                set<uint32_t> init_pos_types;
                                for (const auto& i : ptt.buildings()) {
                                        init_pos_types.clear();
                                        for (auto j : i.furniture()) {
                                                if (!PTTS_HAS(mansion_furniture, j)) {
                                                        CONFIG_ELOG << ptt.id() << " building " << pd::mansion_building_type_Name(i.building()) << " furniture " << j << " not exist";
                                                        continue;
                                                }

                                                const auto& furniture_ptt = PTTS_GET(mansion_furniture, j);
                                                auto found = false;
                                                for (const auto& k : furniture_ptt.buildings()) {
                                                        if (k.building() == i.building()) {
                                                                found = true;
                                                                const auto& type_ptt = PTTS_GET(mansion_furniture_type, furniture_ptt.type());
                                                                if (type_ptt.pos_fixed() && init_pos_types.count(furniture_ptt.type()) > 0) {
                                                                        CONFIG_ELOG << ptt.id() << " building has more than 1 furniture with the same fixed pos type " << furniture_ptt.type();
                                                                        init_pos_types.insert(furniture_ptt.type());
                                                                }
                                                                break;
                                                        }
                                                }
                                                if (!found) {
                                                        CONFIG_ELOG << ptt.id() << " building has furniture, but furniture no such building " << j;
                                                }
                                        }
                                }

                                const auto& mansion_hall_ptt = PTTS_GET(mansion_hall, 1);
                                if (mansion_hall_ptt.common().quests_size() < ptt.create_hall_quest_count()) {
                                        CONFIG_ELOG << "hall quest not enough for create";
                                }

                                if (!verify_events(ptt.help().events())) {
                                        CONFIG_ELOG << "verify help events failed";
                                }

                                if (!verify_events(ptt.visit().events())) {
                                        CONFIG_ELOG << "verify visit events failed";
                                }

                                for (auto i : ptt.visit().plot()) {
                                        if (!PTTS_HAS(plot, i)) {
                                                CONFIG_ELOG << "visit plot not exist " << i;
                                        }
                                }

                                const auto& banquet = ptt.banquet();
                                for (const auto& i : ptt.banquet().switch_dish_condevents().condevents()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << " verify banquet switch dish conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.banquet().switch_dish_condevents().condevents()) {
                                        if (!verify_events(i._events())) {
                                                CONFIG_ELOG << " verify banquet switch dish events failed";
                                        }
                                }
                                if (static_cast<int>(banquet.switch_dish_times_limit()) != banquet.switch_dish_condevents().condevents_size()) {
                                        CONFIG_ELOG << "banquet switch dishes times limit does not match switch dishes condevents";
                                }

                                if (!verify_events(ptt.farm().first_help_events())) {
                                        CONFIG_ELOG << "verify farm first help events failed";
                                }
                                if (!verify_events(ptt.farm().help_events())) {
                                        CONFIG_ELOG << "verify farm help events failed";
                                }
                                if (!PTTS_HAS(item, ptt.farm().fertilizer_item())) {
                                        CONFIG_ELOG << "fertilizer item not exist " << ptt.farm().fertilizer_item();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().start_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().start_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().question_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().question_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().dish_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().dish_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().riddle_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().riddle_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().thief_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().thief_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().celebrity_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().celebrity_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().coins_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().coins_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().prepare_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().prepare_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.banquet().prepare_host_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.banquet().prepare_host_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.update_furniture_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.update_furniture_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.change_setting_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.change_setting_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.set_housekeeper_system_chat())) {
                                        CONFIG_ELOG << "system chat not exist " << ptt.set_housekeeper_system_chat();
                                }
                                if (!PTTS_HAS(plot, ptt.open_plot())) {
                                        CONFIG_ELOG << "open plot not exist " << ptt.open_plot();
                                }
                        };
                }

                mansion_banquet_poison_ptts& mansion_banquet_poison_ptts_instance() {
                        static mansion_banquet_poison_ptts inst;
                        return inst;
                }

                void mansion_banquet_poison_ptts_set_funcs() {

                }

                mansion_banquet_firework_ptts& mansion_banquet_firework_ptts_instance() {
                        static mansion_banquet_firework_ptts inst;
                        return inst;
                }

                void mansion_banquet_firework_ptts_set_funcs() {
                        mansion_banquet_firework_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG << ptt.id() << " item not exist " << ptt.item();
                                }
                        };
                }

                mansion_banquet_dish_ptts& mansion_banquet_dish_ptts_instance() {
                        static mansion_banquet_dish_ptts inst;
                        return inst;
                }

                void mansion_banquet_dish_ptts_set_funcs() {

                }

                mansion_banquet_celebrity_ptts& mansion_banquet_celebrity_ptts_instance() {
                        static mansion_banquet_celebrity_ptts inst;
                        return inst;
                }

                void mansion_banquet_celebrity_ptts_set_funcs() {
                        mansion_banquet_celebrity_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.invite_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check invite conditions failed";
                                }
                                if (ptt.host_challenge_events_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " need 3 host challenge events, got " << ptt.host_challenge_events_size();
                                }
                                for (const auto& i : ptt.host_challenge_events()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check host challenge reward events failed";
                                        }
                                }
                                if (ptt.guest_challenge_events_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " need 3 guest challenge events, got " << ptt.guest_challenge_events_size();
                                }
                                for (const auto& i : ptt.guest_challenge_events()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check guest challenge reward events failed";
                                        }
                                }
                        };
                        mansion_banquet_celebrity_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.invite_conditions(), *ptt.mutable__invite_events());
                        };
                        mansion_banquet_celebrity_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << ptt.id() << " actor not exist " << ptt.actor();
                                }
                                if (ptt.huanzhuang_size() == 0) {
                                        CONFIG_ELOG << ptt.id() << " no huanzhuang";
                                }
                                for (auto i : ptt.huanzhuang()) {
                                        if (!PTTS_HAS(huanzhuang, i)) {
                                                CONFIG_ELOG << ptt.id() << " huanzhuang not exist " << i;
                                        }
                                }
                                if (!verify_conditions(ptt.invite_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify invite conditions failed";
                                }
                                for (const auto& i : ptt.host_challenge_events()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify host challenge reward events failed";
                                        }
                                }
                                for (const auto& i : ptt.guest_challenge_events()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify guest challenge reward events failed";
                                        }
                                }
                        };
                }

                mansion_banquet_celebrity_group_ptts& mansion_banquet_celebrity_group_ptts_instance() {
                        static mansion_banquet_celebrity_group_ptts inst;
                        return inst;
                }

                void mansion_banquet_celebrity_group_ptts_set_funcs() {
                        mansion_banquet_celebrity_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.celebrities()) {
                                        if (!PTTS_HAS(mansion_banquet_celebrity, i)) {
                                                CONFIG_ELOG << ptt.id() << " celebrity not exist " << i;
                                        }
                                }
                        };
                }

                mansion_banquet_prestige_ptts& mansion_banquet_prestige_ptts_instance() {
                        static mansion_banquet_prestige_ptts inst;
                        return inst;
                }

                void mansion_banquet_prestige_ptts_set_funcs() {

                }

                mansion_banquet_riddle_ptts& mansion_banquet_riddle_ptts_instance() {
                        static mansion_banquet_riddle_ptts inst;
                        return inst;
                }

                void mansion_banquet_riddle_ptts_set_funcs() {
                        mansion_banquet_riddle_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.boxes_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " boxes need 3, got " << ptt.boxes_size();
                                }
                        };
                }

                mansion_banquet_question_ptts& mansion_banquet_question_ptts_instance() {
                        static mansion_banquet_question_ptts inst;
                        return inst;
                }

                void mansion_banquet_question_ptts_set_funcs() {
                        mansion_banquet_question_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.answers_size() == 0) {
                                        CONFIG_ELOG << ptt.id() << " no answers";
                                }
                        };
                        mansion_banquet_question_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.answers()) {
                                        if (!dirty_word_filter::instance().check(i.content())) {
                                                CONFIG_ELOG << ptt.id() << " answer has dirty word";
                                        }
                                }
                        };
                }

                mansion_banquet_question_group_ptts& mansion_banquet_question_group_ptts_instance() {
                        static mansion_banquet_question_group_ptts inst;
                        return inst;
                }

                void mansion_banquet_question_group_ptts_set_funcs() {
                        mansion_banquet_question_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.questions()) {
                                        if (!PTTS_HAS(mansion_banquet_question, i)) {
                                                CONFIG_ELOG << ptt.id() << " question not exist " << i;
                                        }
                                }
                        };
                }

                mansion_banquet_thief_ptts& mansion_banquet_thief_ptts_instance() {
                        static mansion_banquet_thief_ptts inst;
                        return inst;
                }

                void mansion_banquet_thief_ptts_set_funcs() {

                }

                mansion_banquet_pintu_game_ptts& mansion_banquet_pintu_game_ptts_instance() {
                        static mansion_banquet_pintu_game_ptts inst;
                        return inst;
                }

                void mansion_banquet_pintu_game_ptts_set_funcs() {
                        mansion_banquet_pintu_game_ptts_instance().check_func_ = [] (const auto& ptt) {};
                }

                mansion_banquet_weilie_game_ptts& mansion_banquet_weilie_game_ptts_instance() {
                        static mansion_banquet_weilie_game_ptts inst;
                        return inst;
                }

                void mansion_banquet_weilie_game_ptts_set_funcs() {

                }

                mansion_banquet_link_game_ptts& mansion_banquet_link_game_ptts_instance() {
                        static mansion_banquet_link_game_ptts inst;
                        return inst;
                }

                void mansion_banquet_link_game_ptts_set_funcs() {

                }

                mansion_banquet_rhythm_master_game_ptts& mansion_banquet_rhythm_master_game_ptts_instance() {
                        static mansion_banquet_rhythm_master_game_ptts inst;
                        return inst;
                }

                void mansion_banquet_rhythm_master_game_ptts_set_funcs() {

                }

                mansion_banquet_coin_ptts& mansion_banquet_coin_ptts_instance() {
                        static mansion_banquet_coin_ptts inst;
                        return inst;
                }

                void mansion_banquet_coin_ptts_set_funcs() {
                        mansion_banquet_coin_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed";
                                }
                        };
                        mansion_banquet_coin_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                        };
                }

                mansion_banquet_coin_stage_ptts& mansion_banquet_coin_stage_ptts_instance() {
                        static mansion_banquet_coin_stage_ptts inst;
                        return inst;
                }

                void mansion_banquet_coin_stage_ptts_set_funcs() {
                        mansion_banquet_coin_stage_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.coins()) {
                                        if (!PTTS_HAS(mansion_banquet_coin, i.coin())) {
                                                CONFIG_ELOG << ptt.level() << " coin not exist " << i.coin();
                                        }
                                }
                        };
                }

                mansion_banquet_ptts& mansion_banquet_ptts_instance() {
                        static mansion_banquet_ptts inst;
                        return inst;
                }

                void mansion_banquet_ptts_set_funcs() {
                        mansion_banquet_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.host_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check host conditions failed";
                                }
                                if (!check_conditions(ptt.reserve_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check reserve conditions failed";
                                }
                                if (!check_events(ptt.guest_events())) {
                                        CONFIG_ELOG << ptt.id() << " check init events failed";
                                }
                        };
                        mansion_banquet_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.reserve_conditions(), *ptt.mutable__reserve_events());
                        };
                        mansion_banquet_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.host_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify host conditions failed";
                                }
                                for (auto i : ptt.dishes()) {
                                        if (!PTTS_HAS(mansion_banquet_dish, i)) {
                                                CONFIG_ELOG << ptt.id() << " dish not exist " << i;
                                        }
                                }
                                if (!verify_conditions(ptt.reserve_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify reserve conditions failed";
                                }
                                if (!verify_events(ptt._reserve_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify reserve events failed";
                                }
                                if (!verify_events(ptt.guest_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify init events failed";
                                }
                                if (!PTTS_HAS(mansion_banquet_question_group, ptt.question_group())) {
                                        CONFIG_ELOG << ptt.id() << " question group not exist " << ptt.question_group();
                                }
                                if (!PTTS_HAS(mansion_banquet_celebrity_group, ptt.celebrity_group())) {
                                        CONFIG_ELOG << ptt.id() << " celebrity group not exist " << ptt.celebrity_group();
                                }
                        };
                }

                mansion_farm_plant_ptts& mansion_farm_plant_ptts_instance() {
                        static mansion_farm_plant_ptts inst;
                        return inst;
                }

                void mansion_farm_plant_ptts_set_funcs() {
                        mansion_farm_plant_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.harvests_size() != 6) {
                                        CONFIG_ELOG << ptt.id() << " harvests need size 6, got size " << ptt.harvests_size();
                                }
                                if (!check_events(ptt.harvest_events())) {
                                        CONFIG_ELOG << ptt.id() << " check harvest events failed";
                                }
                                auto need_minutes = 0u;
                                for (const auto& i : ptt.growths()) {
                                        need_minutes += i.minutes();
                                }
                                if (need_minutes <= 0) {
                                        CONFIG_ELOG << ptt.id() << " need growths minutes, got minutes " << need_minutes;
                                }
                        };
                        mansion_farm_plant_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *c = ptt.mutable_sow_conditions()->add_conditions();
                                c->set_type(pd::condition::COST_ITEM);
                                c->add_arg(to_string(ptt.seed_item()));
                                c->add_arg(to_string(ptt.sow_seed_count()));

                                modify_events_by_conditions(ptt.sow_conditions(), *ptt.mutable__sow_events());
                        };
                        mansion_farm_plant_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.seed_item())) {
                                        CONFIG_ELOG << ptt.id() << " seed item not exist " << ptt.seed_item();
                                }

                                if (!verify_events(ptt.harvest_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify harvest events failed";
                                }

                                if (!verify_conditions(ptt.sow_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify sow conditions failed";
                                }
                                if (!verify_events(ptt._sow_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify sow events failed";
                                }
                        };
                }

                mansion_cook_recipe_ptts& mansion_cook_recipe_ptts_instance() {
                        static mansion_cook_recipe_ptts inst;
                        return inst;
                }

                void mansion_cook_recipe_ptts_set_funcs() {
                        mansion_cook_recipe_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                for (const auto& i : ptt.unlock()) {
                                        if (i.level_times_size() != 3) {
                                                CONFIG_ELOG << ptt.id() << " need 3 unlock level_times, got " << i.level_times_size();
                                        }
                                }
                                if (ptt.level_events_size() != 3) {
                                        CONFIG_ELOG << ptt.id() << " need 3 level events, got " << ptt.level_events_size();
                                }
                                for (const auto& i : ptt.level_events()) {
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check level events failed";
                                        }
                                }
                        };
                        mansion_cook_recipe_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__events());
                        };
                        mansion_cook_recipe_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << ptt.id() << " verify _events failed";
                                }
                                for (const auto& i : ptt.unlock()) {
                                        if (!PTTS_HAS(mansion_cook_recipe, i.recipe())) {
                                                CONFIG_ELOG << ptt.id() << " need 3 unlock recipe not exist " << i.recipe();
                                        }
                                }
                                for (const auto& i : ptt.level_events()) {
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify level_events failed";
                                        }
                                }
                        };
                }

                mansion_game_ptts& mansion_game_ptts_instance() {
                        static mansion_game_ptts inst;
                        return inst;
                }

                void mansion_game_ptts_set_funcs() {
                        mansion_game_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::mansion_game_type_Name(ptt.type()) << " check conditions failed";
                                }
                        };
                        mansion_game_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__events());
                        };
                        mansion_game_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << pd::mansion_game_type_Name(ptt.type()) << " verify conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << pd::mansion_game_type_Name(ptt.type()) << " verify _events failed";
                                }
                        };
                }

                mansion_exchange_ptts& mansion_exchange_ptts_instance() {
                        static mansion_exchange_ptts inst;
                        return inst;
                }

                void mansion_exchange_ptts_set_funcs() {
                        mansion_exchange_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed";
                                }

                        };
                        mansion_exchange_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable_events());
                        };
                        mansion_exchange_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                        };
                }

                mansion_game_fish_capper_ptts& mansion_game_fish_capper_ptts_instance() {
                        static mansion_game_fish_capper_ptts inst;
                        return inst;
                }
                void mansion_game_fish_capper_ptts_set_funcs() {
                        mansion_game_fish_capper_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (ptt.capper_drops_size() != 4) {
                                        CONFIG_ELOG << ptt.id() << " need capper_drops size 4, got " << ptt.capper_drops_size();
                                }
                        };
                        mansion_game_fish_capper_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__cost_events());
                        };
                        mansion_game_fish_capper_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.id())) {
                                        CONFIG_ELOG << "item_table not exist " << ptt.id();
                                }

                                for (const auto& i : ptt.capper_drops()) {
                                        if (!PTTS_HAS(drop, i)) {
                                                CONFIG_ELOG << ptt.id() << " drop_table not exist " << i;
                                        }
                                }
                        };
                }

                mansion_banquet_wedding_ball_ptts& mansion_banquet_wedding_ball_ptts_instance() {
                        static mansion_banquet_wedding_ball_ptts inst;
                        return inst;
                }
                void mansion_banquet_wedding_ball_ptts_set_funcs() {
                        mansion_banquet_wedding_ball_ptts_instance().check_func_ = [] (const auto& ptt) {

                        };
                        mansion_banquet_wedding_ball_ptts_instance().modify_func_ = [] (auto& ptt) {

                        };
                        mansion_banquet_wedding_ball_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(system_chat, ptt.host_system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " system_chat_table not exist " << ptt.host_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.guest_system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " system_chat_table not exist " << ptt.guest_system_chat();
                                }
                        };
                }

                mansion_banquet_wedding_candy_ptts& mansion_banquet_wedding_candy_ptts_instance() {
                        static mansion_banquet_wedding_candy_ptts inst;
                        return inst;
                }
                void mansion_banquet_wedding_candy_ptts_set_funcs() {
                        mansion_banquet_wedding_candy_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (!check_events(ptt.candy_events())) {
                                        CONFIG_ELOG << ptt.id() << " check candy_events failed";
                                }
                        };
                        mansion_banquet_wedding_candy_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__cost_events());
                        };
                        mansion_banquet_wedding_candy_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(system_chat, ptt.system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " system_chat_table not exist " << ptt.system_chat();
                                }
                        };
                }

        }
}
