#include "utils.hpp"
#include "actor_ptts.hpp"
#include "skill_ptts.hpp"
#include "item_ptts.hpp"
#include "plot_ptts.hpp"
#include "growth_ptts.hpp"
#include "chat_ptts.hpp"
#include "equipment_ptts.hpp"
#include "utils/game_def.hpp"
#include <set>

using namespace std;

namespace pc = proto::config;
namespace pd = proto::data;

namespace nora {
        namespace config {

                actor_ptts& actor_ptts_instance() {
                        static actor_ptts inst;
                        return inst;
                }

                void actor_ptts_set_funcs() {
                        actor_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.icon().empty()) {
                                        CONFIG_ELOG << ptt.id() << " no icon";
                                }
                                if (ptt.quality() > 5 || ptt.quality() < 3) {
                                        CONFIG_ELOG << ptt.id() << " invalid quality " << ptt.quality();
                                }
                                if (!check_conditions(ptt.reset_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check reset conditions failed";
                                }
                        };
                        actor_ptts_instance().modify_func_ = [] (auto& ptt) {
                                actor_ptts_instance().modify_func_ = [] (auto& ptt) {
                                        modify_events_by_conditions(ptt.reset_conditions(), *ptt.mutable__reset_events());
                                };
                        };
                        actor_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.icon().empty()) {
                                        CONFIG_ELOG << "actor config " << ptt.id() << " icon is empty" ;
                                }
                                if (ptt.quality() > 5 || ptt.quality() < 1) {
                                        CONFIG_ELOG << "actor quality error" << ptt.DebugString();
                                }
                                map<uint32_t, uint32_t> slot2skill;
                                set<uint32_t> xinwu;
                                for (auto i: ptt.skill()) {
                                        if (!PTTS_HAS(skill, i)) {
                                                CONFIG_ELOG << ptt.id() << " actor skill not exist " << i;
                                        } else {
                                                const auto& skill_ptt = PTTS_GET(skill, i);
                                                if (skill_ptt.has_slot()) {
                                                        if (ptt.type() != pd::actor::ZHU && slot2skill.count(skill_ptt.slot()) > 0) {
                                                                CONFIG_ELOG << ptt.id() << " found same slot skills " << slot2skill.at(skill_ptt.slot()) << " and " << i;
                                                        }
                                                        slot2skill[skill_ptt.slot()] = i;
                                                }
                                        }
                                }
                                for (auto i : ptt.final_skill()) {
                                        if (!PTTS_HAS(skill, i)) {
                                                CONFIG_ELOG << ptt.id() << " actor final skill not exist " << i;
                                        }
                                }
                                if (ptt.has_xinwu()) {
                                        if (!PTTS_HAS(item, ptt.xinwu())) {
                                                CONFIG_ELOG << ptt.id() << " xinwu not exist " << ptt.xinwu();
                                        } else {
                                                auto& item_ptt = PTTS_GET(item, ptt.xinwu());
                                                item_ptt.set__xinwu_actor(ptt.id());
                                        }
                                        if (xinwu.count(ptt.xinwu()) > 0) {
                                                CONFIG_ELOG << ptt.id() << " xinwu duplicate" << ptt.xinwu();
                                        }
                                        xinwu.insert(ptt.xinwu());
                                }
                                for (auto i : ptt.levelup_growth()) {
                                        if (!PTTS_HAS(growth, i)) {
                                                CONFIG_ELOG << ptt.id() << " levelup growth not exist " << i;
                                        }
                                }
                                if (ptt.has_add_pin_growth()) {
                                        if (!PTTS_HAS(not_portional_growth, ptt.add_pin_growth())) {
                                                CONFIG_ELOG << ptt.id() << " add_pin_growth not exist " << ptt.add_pin_growth();
                                        } else {
                                                const auto& add_pin_growth_ptt = PTTS_GET(not_portional_growth, ptt.add_pin_growth());
                                                if (add_pin_growth_ptt.decos().size() < static_cast<int>(ACTOR_MAX_PIN)) {
                                                        CONFIG_ELOG << ptt.id() << " add_pin_growth attribute_decos size less than " << ACTOR_MAX_PIN << " " << ptt.add_pin_growth();
                                                }
                                        }
                                }
                                if (ptt.has_add_step_growth()) {
                                        if (!PTTS_HAS(not_portional_growth, ptt.add_step_growth())) {
                                                CONFIG_ELOG << ptt.id() << " add_step_growth not exist " << ptt.add_step_growth();
                                        }
                                }
                                if (ptt.has_add_star_craft_growth()) {
                                        if (!PTTS_HAS(craft_growth, ptt.add_star_craft_growth())) {
                                                CONFIG_ELOG << ptt.id() << " add_star_craft_growth not exist " << ptt.add_star_craft_growth();
                                        }
                                }
                                if (ptt.has_voice_pttid()) {
                                        if (!PTTS_HAS(actor_voice, ptt.voice_pttid())) {
                                                CONFIG_ELOG << ptt.id() << " voice_pttid not exist " << ptt.voice_pttid();
                                        }
                                }
                                if (ptt.has_plot_pttid()) {
                                        if (!PTTS_HAS(actor_plot, ptt.plot_pttid())) {
                                                CONFIG_ELOG << ptt.id() << " plot_pttid not exist " << ptt.plot_pttid();
                                        }
                                }
                                if (ptt.has_add_pin()) {
                                        if (!PTTS_HAS(actor_add_pin, ptt.add_pin())) {
                                                CONFIG_ELOG << ptt.id() << " add_pin not exist " << ptt.add_pin();
                                        }
                                }
                                if (ptt.has_bubble_pttid()) {
                                        if (!PTTS_HAS(actor_bubble, ptt.bubble_pttid())) {
                                                CONFIG_ELOG << ptt.id() << " bubble_pttid not exist " << ptt.bubble_pttid();
                                        }
                                }
                                if (ptt.has_add_step_cost()) {
                                        if (!PTTS_HAS(actor_add_step, ptt.add_step_cost())) {
                                                CONFIG_ELOG << ptt.id() << " add_step_cost not exist " << ptt.add_step_cost();
                                        }
                                }
                                for (auto i : ptt.skins()) {
                                        if (!PTTS_HAS(actor_skin, i)) {
                                                CONFIG_ELOG << ptt.id() << " actor skin not exist " << i;
                                        }

                                        auto& skin_ptt = PTTS_GET(actor_skin, i);
                                        if (skin_ptt.has__actor()) {
                                                CONFIG_ELOG << ptt.id() << " actor skin duplicate " << i << " skin " << skin_ptt.id();
                                                continue;
                                        }
                                        skin_ptt.set__actor(ptt.id());
                                }
                                for (auto i : ptt.equipment_suites()) {
                                        if (!PTTS_HAS(equipment_suite, i)) {
                                                CONFIG_ELOG << ptt.id() << " actor equipment suite not exist " << i;
                                        }
                                }

                                if (ptt.type() == pd::actor::ZHU) {
                                        set<uint32_t> skills;
                                        for (auto i : ptt.skill()) {
                                                if (!PTTS_HAS(skill, i)) {
                                                        CONFIG_ELOG << ptt.id() << " actor skill not exist " << i;
                                                } else {
                                                        if (skills.count(i) > 0) {
                                                                CONFIG_ELOG << ptt.id() << " actor skill duplicate in skills and unlocksills " << i;
                                                        } else {
                                                                skills.insert(i);
                                                        }
                                                        const auto& skill_ptt = PTTS_GET(skill, i);
                                                        auto find = false;
                                                        for (auto j : skill_ptt.tags()) {
                                                                if (j == pc::skill::ATTACK || j == pc::skill::DEFENCE || j == pc::skill::ASSIST) {
                                                                        if (find) {
                                                                                CONFIG_ELOG << ptt.id() << " actor skill " << skill_ptt.id() << " tag is dupicate ";
                                                                        }
                                                                        find = true;
                                                                }
                                                        }
                                                        if (!find) {
                                                                CONFIG_ELOG << ptt.id() << " actor zhu skill " << skill_ptt.id() << " tag is not find in attack or defence or assist ";
                                                        }
                                                }
                                        }
                                        for (auto i : ptt.lock_skill()) {
                                                if (!PTTS_HAS(skill, i)) {
                                                        CONFIG_ELOG << ptt.id() << " actor skill not exist " << i;
                                                } else {
                                                        if (skills.count(i) > 0) {
                                                                CONFIG_ELOG << ptt.id() << " actor skill duplicate in skills and unlocksills " << i;
                                                        } else {
                                                                skills.insert(i);
                                                        }
                                                        const auto& skill_ptt = PTTS_GET(skill, i);
                                                        auto find = false;
                                                        for (auto j : skill_ptt.tags()) {
                                                                if (j == pc::skill::ATTACK || j == pc::skill::DEFENCE || j == pc::skill::ASSIST) {
                                                                        if (find) {
                                                                                CONFIG_ELOG << ptt.id() << " actor skill " << skill_ptt.id() << " tag is dupicate ";
                                                                        }
                                                                        find = true;
                                                                }
                                                        }
                                                        if (!find) {
                                                                CONFIG_ELOG << ptt.id() << " actor zhu skill " << skill_ptt.id() << " tag is not find in attack or defence or assist ";
                                                        }
                                                }
                                        }
                                }
                                if (ptt.has_add_star_pttid() && !PTTS_HAS(actor_add_star, ptt.add_star_pttid())) {
                                        CONFIG_ELOG << ptt.id() << " actor add star not exist " << ptt.add_star_pttid();
                                }
                                if (ptt.has_levelup_pttid() && !PTTS_HAS(actor_levelup, ptt.levelup_pttid())) {
                                        CONFIG_ELOG << ptt.id() << " actor levelup pttid not exist " << ptt.levelup_pttid();
                                }
                                if (!verify_conditions(ptt.reset_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify reset conditions failed";
                                }
                                if (!verify_events(ptt._reset_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify reset events failed";
                                }
                        };
                }

                actor_levelup_ptts& actor_levelup_ptts_instance() {
                        static actor_levelup_ptts inst;
                        return inst;
                }

                void actor_levelup_ptts_set_funcs() {
                        actor_levelup_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.levels()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check conditions failed";
                                        }
                                        if (!check_child_conditions(i.child_conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check child conditions failed";
                                        }
                                }
                        };
                        actor_levelup_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_levels()) {
                                        auto *conditions = i.mutable_conditions();
                                        auto *c = conditions->add_conditions();
                                        c->set_type(pd::condition::LEVEL_GREATER_THAN_ACTOR_LEVEL);

                                        modify_events_by_conditions(*conditions, *i.mutable__events());
                                }
                        };
                        actor_levelup_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.levels()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                        }
                                        if (!verify_events(i._events())) {
                                                CONFIG_ELOG << ptt.id() << " verify _events failed";
                                        }
                                }
                        };
                }

                actor_add_star_ptts& actor_add_star_ptts_instance() {
                        static actor_add_star_ptts inst;
                        return inst;
                }

                void actor_add_star_ptts_set_funcs() {
                        actor_add_star_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.stars()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check role conditions failed";
                                        }
                                        auto found_cost_item = false;
                                        for (const auto& j : i.conditions().conditions()) {
                                                if (j.type() == pd::condition::COST_ITEM) {
                                                        if (found_cost_item) {
                                                                CONFIG_ELOG << ptt.id() << " conditions has more than 1 cost item";
                                                        } else {
                                                                found_cost_item = true;
                                                                if (j.arg_size() != 1) {
                                                                        CONFIG_ELOG << ptt.id() << " conditions cost item should only has 1 arg";
                                                                }
                                                        }
                                                }
                                        }
                                }
                        };
                        actor_add_star_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_stars()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable__events());
                                }
                        };
                        actor_add_star_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.stars()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                        }
                                        if (!verify_events(i._events())) {
                                                CONFIG_ELOG << ptt.id() << " verify _events failed";
                                        }
                                }
                        };
                }

                actor_actor_add_star_ptts& actor_actor_add_star_ptts_instance() {
                        static actor_actor_add_star_ptts inst;
                        return inst;
                }

                void actor_actor_add_star_ptts_set_funcs() {
                }

                actor_add_step_ptts& actor_add_step_ptts_instance() {
                        static actor_add_step_ptts inst;
                        return inst;
                }

                void actor_add_step_ptts_set_funcs() {
                        actor_add_step_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.steps()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check conditions failed";
                                        }
                                }
                        };
                        actor_add_step_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_steps()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable__events());
                                }
                        };
                        actor_add_step_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.steps()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                        }
                                }
                                for (const auto& i : ptt.steps()) {
                                        if (!verify_events(i._events())) {
                                                CONFIG_ELOG << ptt.id() << " verify _events failed";
                                        }
                                }
                        };
                }

                actor_add_pin_ptts& actor_add_pin_ptts_instance() {
                        static actor_add_pin_ptts inst;
                        return inst;
                }

                void actor_add_pin_ptts_set_funcs() {
                        actor_add_pin_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.pin_size() != ACTOR_MAX_PIN) {
                                        CONFIG_ELOG << ptt.id() << " need " << ACTOR_MAX_PIN << " condevents got " << ptt.pin_size();
                                }
                                for (auto& i : *ptt.mutable_pin()) {
                                        modify_events_by_conditions(i.conditions(), *i.mutable_events());
                                }
                        };
                }

                actor_intimacy_ptts& actor_intimacy_ptts_instance() {
                        static actor_intimacy_ptts inst;
                        return inst;
                }

                void actor_intimacy_ptts_set_funcs() {
                        actor_intimacy_ptts_instance().check_func_ = [] (const auto& ptt) {
                                const auto& i = ptt.intimacy();
                                if (i.actor_a() == i.actor_b()) {
                                        CONFIG_ELOG << ptt.id() << " two actors are the same";
                                }
                        };
                        actor_intimacy_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                const auto& i = ptt.intimacy();
                                if (!PTTS_HAS(actor, i.actor_a())) {
                                        CONFIG_ELOG << ptt.id() << " actor not exist " << i.actor_a();
                                }
                                if (!PTTS_HAS(actor, i.actor_b())) {
                                        CONFIG_ELOG << ptt.id() << " actor not exist " << i.actor_b();
                                }
                        };
                }

                actor_united_chance_ptts& actor_united_chance_ptts_instance() {
                        static actor_united_chance_ptts inst;
                        return inst;
                }
                void actor_united_chance_ptts_set_funcs() {
                        actor_united_chance_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                const auto& i = ptt.intimacy_level();
                                if (!PTTS_HAS(actor_actor_add_star, i)) {
                                        CONFIG_ELOG << "actor_actor_add_star table not exist " << i;
                                }
                        };
                }

                actor_voice_ptts& actor_voice_ptts_instance() {
                        static actor_voice_ptts inst;
                        return inst;
                }
                void actor_voice_ptts_set_funcs() {
                }

                actor_plot_ptts& actor_plot_ptts_instance() {
                        static actor_plot_ptts inst;
                        return inst;
                }
                void actor_plot_ptts_set_funcs() {
                        actor_plot_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.visit_plot()) {
                                        for (auto j : i.plot()) {
                                                if (!PTTS_HAS(plot, j)) {
                                                        CONFIG_ELOG << ptt.id() << " visit plot not exist " << j;
                                                }
                                        }
                                        for (auto j : i.thank_plot()) {
                                                if (!PTTS_HAS(plot, j)) {
                                                        CONFIG_ELOG << ptt.id() << " visit thank plot not exist " << j;
                                                }
                                        }
                                        for (auto j : i.head_plot()) {
                                                if (!PTTS_HAS(plot, j)) {
                                                        CONFIG_ELOG << ptt.id() << " visit head plot not exist " << j;
                                                }
                                        }
                                        for (auto j : i.chest_plot()) {
                                                if (!PTTS_HAS(plot, j)) {
                                                        CONFIG_ELOG << ptt.id() << " visit chest plot not exist " << j;
                                                }
                                        }
                                        for (auto j : i.legs_plot()) {
                                                if (!PTTS_HAS(plot, j)) {
                                                        CONFIG_ELOG << ptt.id() << " visit legs plot not exist " << j;
                                                }
                                        }
                                }

                                for (const auto& i : ptt.star_plot()) {
                                        if (!PTTS_HAS(plot, i.plot())) {
                                                CONFIG_ELOG << ptt.id() << " star plot not exist " << i.plot();
                                        }
                                }
                        };
                }

                actor_logic_ptts& actor_logic_ptts_instance() {
                        static actor_logic_ptts inst;
                        return inst;
                }

                void actor_logic_ptts_set_funcs() {
                        actor_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.exchange_xinwu_count() <= 0) {
                                        CONFIG_ELOG << " exchange xinwu count <= 0";
                                }
                                if (!check_conditions(ptt.equipment_conditions())) {
                                        CONFIG_ELOG << " check equipment_conditions failed";
                                }
                                if (!check_conditions(ptt.aura_equipment_conditions())) {
                                        CONFIG_ELOG << " check aura equipment_conditions failed";
                                }
                                if (!check_conditions(ptt.add_pin_conditions())) {
                                        CONFIG_ELOG << " check add_pin_conditions failed";
                                }
                                if (!check_conditions(ptt.add_step_conditions())) {
                                        CONFIG_ELOG << " check add_step_conditions failed";
                                }
                                if (!check_conditions(ptt.skill_conditions())) {
                                        CONFIG_ELOG << " check skill_conditions failed";
                                }
                        };
                        actor_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.equipment_conditions())) {
                                        CONFIG_ELOG << " verify equipment_conditions failed";
                                }
                                if (!verify_conditions(ptt.aura_equipment_conditions())) {
                                        CONFIG_ELOG << " verify aura equipment_conditions failed";
                                }
                                if (!verify_conditions(ptt.add_pin_conditions())) {
                                        CONFIG_ELOG << " verify add_pin_conditions failed";
                                }
                                if (!verify_conditions(ptt.add_step_conditions())) {
                                        CONFIG_ELOG << " verify add_step_conditions failed";
                                }
                                if (!verify_conditions(ptt.skill_conditions())) {
                                        CONFIG_ELOG << " verify skill_conditions failed";
                                }
                                if (!PTTS_HAS(system_chat, ptt.lottery_system_chat())) {
                                        CONFIG_ELOG << ptt.id() << " lottery_system_chat not exist " << ptt.lottery_system_chat();
                                }
                                for (auto i : ptt.star_system_chat()) {
                                        if (!PTTS_HAS(system_chat, i.system_chat())) {
                                                CONFIG_ELOG << ptt.id() << " system_chat not exist " << i.system_chat();
                                        }
                                }
                                for (auto i : ptt.pin_system_chat()) {
                                        if (!PTTS_HAS(system_chat, i.system_chat())) {
                                                CONFIG_ELOG << ptt.id() << " system_chat not exist " << i.system_chat();
                                        }
                                }
                        };
                }

                actor_bubble_ptts& actor_bubble_ptts_instance() {
                        static actor_bubble_ptts inst;
                        return inst;
                }

                void actor_bubble_ptts_set_funcs() {

                }

                actor_skin_ptts& actor_skin_ptts_instance() {
                        static actor_skin_ptts inst;
                        return inst;
                }

                void actor_skin_ptts_set_funcs() {
                }

        }
}
