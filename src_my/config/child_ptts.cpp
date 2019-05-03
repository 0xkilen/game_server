#include "child_ptts.hpp"
#include "plot_ptts.hpp"
#include "actor_ptts.hpp"
#include "skill_ptts.hpp"
#include "item_ptts.hpp"
#include "chat_ptts.hpp"
#include "utils.hpp"
#include "utils/time_utils.hpp"

namespace nora {
        namespace config {

                child_phase_ptts& child_phase_ptts_instance() {
                        static child_phase_ptts inst;
                        return inst;
                }

                void child_phase_ptts_set_funcs() {
                        child_phase_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_child_conditions(ptt.next_phase_conditions())) {
                                        CONFIG_ELOG << pd::child_phase_Name(ptt.phase()) << " check next phase conditions failed";
                                }
                        };
                        child_phase_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_child_conditions(ptt.next_phase_conditions())) {
                                        CONFIG_ELOG << pd::child_phase_Name(ptt.phase()) << " verify next phase conditions failed";
                                }
                                for (auto i : ptt.actors()) {
                                        if (!PTTS_HAS(actor, i)) {
                                                CONFIG_ELOG << pd::child_phase_Name(ptt.phase()) << " actor not exist " << i;
                                        }
                                }
                        };
                }

                child_logic_ptts& child_logic_ptts_instance() {
                        static child_logic_ptts inst;
                        return inst;
                }

                void child_logic_ptts_set_funcs() {
                        child_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.grow_interval() <= 0) {
                                        CONFIG_ELOG << "grow interval must greater than 0 ";
                                }
                                if (!check_child_conditions(ptt.normal_grow_conditions())) {
                                        CONFIG_ELOG  << " check normal grow conditions failed";
                                }
                                if (!check_child_conditions(ptt.slow_grow_conditions())) {
                                        CONFIG_ELOG  << " check slow grow conditions failed";
                                }
                                if (!check_child_conditions(ptt.grow_item_conditions())) {
                                        CONFIG_ELOG << " check grow item child conditions failed";
                                }
                                if (!check_child_events(ptt.grow_item_events())) {
                                        CONFIG_ELOG << " check grow item child events failed";
                                }
                                if (!check_child_events(ptt.noon_events())) {
                                        CONFIG_ELOG << " check noon events failed";
                                }
                                if (!check_conditions(ptt.change_name_conditions())) {
                                        CONFIG_ELOG << " check change name conditions failed";
                                }
                                if (!check_conditions(ptt.change_gender_conditions())) {
                                        CONFIG_ELOG << " check change gender conditions failed";
                                }
                                if (!check_conditions(ptt.change_type_conditions())) {
                                        CONFIG_ELOG << " check change type conditions failed";
                                }
                        };
                        child_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.change_name_conditions(), *ptt.mutable__change_name_events());
                                modify_events_by_conditions(ptt.change_gender_conditions(), *ptt.mutable__change_gender_events());
                                modify_events_by_conditions(ptt.change_type_conditions(), *ptt.mutable__change_type_events());

                                auto *c = ptt.mutable__grow_item_conditions()->add_conditions();
                                c->set_type(pd::condition::COST_ITEM);
                                c->add_arg(to_string(ptt.grow_item()));
                                c->add_arg("1");

                                modify_events_by_conditions(ptt._grow_item_conditions(), *ptt.mutable__grow_item_events());
                        };
                        child_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(plot, ptt.read_plot())) {
                                        CONFIG_ELOG << "read plot not exist " << ptt.read_plot();
                                }
                                if (!verify_child_conditions(ptt.normal_grow_conditions())) {
                                        CONFIG_ELOG << " verify normal grow conditions failed";
                                }
                                if (!verify_child_conditions(ptt.slow_grow_conditions())) {
                                        CONFIG_ELOG << " verify slow grow conditions failed";
                                }
                                if (!PTTS_HAS(item, ptt.grow_item())) {
                                        CONFIG_ELOG << " grow item not exist " << ptt.grow_item();
                                }
                                if (!verify_conditions(ptt._grow_item_conditions())) {
                                        CONFIG_ELOG << " verify grow item conditions failed";
                                }
                                if (!verify_conditions(ptt._grow_item_conditions())) {
                                        CONFIG_ELOG << " verify grow item conditions failed";
                                }
                                if (!verify_events(ptt._grow_item_events())) {
                                        CONFIG_ELOG << " verify grow item events failed";
                                }
                                if (!verify_child_events(ptt.grow_item_events())) {
                                        CONFIG_ELOG << " verify grow item child events failed";
                                }
                                if (!verify_child_events(ptt.noon_events())) {
                                        CONFIG_ELOG << " verify noon events failed";
                                }
                                if (!verify_conditions(ptt.change_name_conditions())) {
                                        CONFIG_ELOG << " verify change name conditions failed";
                                }
                                if (!verify_events(ptt._change_name_events())) {
                                        CONFIG_ELOG << " verify change name events failed";
                                }
                                if (!verify_events(ptt._change_gender_events())) {
                                        CONFIG_ELOG << " verify change gender events failed";
                                }
                                if (!verify_events(ptt._change_type_events())) {
                                        CONFIG_ELOG << " verify change type events failed";
                                }
                                if (!PTTS_HAS(system_chat, ptt.raise_system_chat())) {
                                        CONFIG_ELOG << " verify system chat failed, " << ptt.raise_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.study_system_chat())) {
                                        CONFIG_ELOG << " verify system chat failed, " << ptt.study_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.add_skill_exp_system_chat())) {
                                        CONFIG_ELOG << " verify system chat failed, " << ptt.add_skill_exp_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.put_on_jade_system_chat())) {
                                        CONFIG_ELOG << " verify system chat failed, " << ptt.put_on_jade_system_chat();
                                }
                                if (!PTTS_HAS(system_chat, ptt.new_child_system_chat())) {
                                        CONFIG_ELOG << " verify system chat failed, " << ptt.new_child_system_chat();
                                }
                                for (const auto& i : ptt.gender_pairs()) {
                                        if (!PTTS_HAS(actor, i.male())) {
                                                CONFIG_ELOG << " gender pair actor not exist " << i.male();
                                        }
                                        if (!PTTS_HAS(actor, i.female())) {
                                                CONFIG_ELOG << " gender pair actor not exist " << i.female();
                                        }
                                        const auto& male_ptt = PTTS_GET(actor, i.male());
                                        if (male_ptt.gender() != pd::MALE) {
                                                CONFIG_ELOG << " gender pair gender not right " << i.male();
                                        }
                                        const auto& female_ptt = PTTS_GET(actor, i.female());
                                        if (female_ptt.gender() != pd::FEMALE) {
                                                CONFIG_ELOG << " gender pair actor gender not right " << i.female();
                                        }
                                        if (male_ptt.type() != female_ptt.type()) {
                                                CONFIG_ELOG << " gender pair actor not the same type " << i.male() << " " << i.female();
                                        }
                                }
                                for (const auto& i : ptt.type_groups()) {
                                        if (!PTTS_HAS(actor, i.tank())) {
                                                CONFIG_ELOG << " type groups actor not exist " << i.tank();
                                        }
                                        if (!PTTS_HAS(actor, i.dps())) {
                                                CONFIG_ELOG << " type groups actor not exist " << i.dps();
                                        }
                                        if (!PTTS_HAS(actor, i.assist())) {
                                                CONFIG_ELOG << " type groups actor not exist " << i.assist();
                                        }
                                        const auto& tank_ptt = PTTS_GET(actor, i.tank());
                                        if (tank_ptt.type() != pd::actor::TANK) {
                                                CONFIG_ELOG << " type group actor type not right " << i.tank();
                                        }
                                        const auto& dps_ptt = PTTS_GET(actor, i.dps());
                                        if (dps_ptt.type() != pd::actor::DPS) {
                                                CONFIG_ELOG << " type group actor type not right " << i.dps();
                                        }
                                        const auto& assist_ptt = PTTS_GET(actor, i.assist());
                                        if (assist_ptt.type() != pd::actor::ASSIST) {
                                                CONFIG_ELOG << " type group actor type not right " << i.assist();
                                        }
                                        if (tank_ptt.gender() != dps_ptt.gender() || dps_ptt.gender() != assist_ptt.gender()) {
                                                CONFIG_ELOG << " type group actor not the same gender " << i.tank() << " " << i.dps() << " " << i.assist();
                                        }

                                }
                        };
                }

                child_value_ptts& child_value_ptts_instance() {
                        static child_value_ptts inst;
                        return inst;
                }

                void child_value_ptts_set_funcs() {
                        child_value_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.dec_interval() > 0) {
                                        if (ptt.dec_count() <= 0) {
                                                CONFIG_ELOG << "dev interval greater than 0 but dev count not " << ptt.dec_count();
                                        }
                                        if (ptt.dec_odds() <= 0) {
                                                CONFIG_ELOG << "dev interval greater than 0 but dev odds not " << ptt.dec_odds();
                                        }
                                }
                        };
                }

                child_raise_ptts& child_raise_ptts_instance() {
                        static child_raise_ptts inst;
                        return inst;
                }

                void child_raise_ptts_set_funcs() {
                        child_raise_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (!check_child_conditions(ptt.child_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check child conditions failed";
                                }
                                if (!check_child_events(ptt.child_events())) {
                                        CONFIG_ELOG << ptt.id() << " check child events failed";
                                }
                                if (!check_conditions(ptt.pay_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check pay conditions failed";
                                }
                                if (!check_child_conditions(ptt.pay_child_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check pay child conditions failed";
                                }
                                if (!check_child_events(ptt.pay_child_events())) {
                                        CONFIG_ELOG << ptt.id() << " check pay child events failed";
                                }
                        };
                        child_raise_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.has_item()) {
                                        auto *c = ptt.mutable_conditions()->add_conditions();
                                        c->set_type(pd::condition::COST_ITEM);
                                        c->add_arg(to_string(ptt.item()));
                                        c->add_arg("1");
                                }

                                modify_events_by_conditions(ptt.conditions(), *ptt.mutable__events());
                                if (ptt.has_pay_conditions()) {
                                        modify_events_by_conditions(ptt.pay_conditions(), *ptt.mutable__pay_events());
                                }
                        };
                        child_raise_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_item() && !PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG << "item not exist " << ptt.item();
                                }
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                                if (!verify_child_conditions(ptt.child_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify child conditions failed";
                                }
                                if (!verify_child_events(ptt.child_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify child events failed";
                                }
                                if (!verify_conditions(ptt.pay_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify pay conditions failed";
                                }
                                if (!verify_events(ptt._pay_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify pay events failed";
                                }
                                if (!verify_child_conditions(ptt.pay_child_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify pay child conditions failed";
                                }
                                if (!verify_child_events(ptt.pay_child_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify pay child events failed";
                                }
                        };
                }

                child_study_ptts& child_study_ptts_instance() {
                        static child_study_ptts inst;
                        return inst;
                }

                void child_study_ptts_set_funcs() {
                        child_study_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.levels()) {
                                        if (!check_child_conditions(i.conditions())) {
                                                CONFIG_ELOG << "check conditions failed";
                                        }
                                }
                        };
                        child_study_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_levels()) {
                                        auto *e = i.mutable__events()->add_events();
                                        e->set_type(pd::CET_STUDY_LEVELUP);
                                        e->add_arg(pd::actor_attr_type_Name(ptt.attr_type()));
                                }
                        };
                        child_study_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.levels()) {
                                        if (!verify_child_conditions(i.conditions())) {
                                                CONFIG_ELOG << "verify conditions failed";
                                        }
                                }
                        };
                }

                child_study_item_ptts& child_study_item_ptts_instance() {
                        static child_study_item_ptts inst;
                        return inst;
                }

                void child_study_item_ptts_set_funcs() {
                        child_study_item_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.item() << " check conditions failed";
                                }
                                if (!check_child_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.item() << " check events failed";
                                }
                        };
                        child_study_item_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *c = ptt.mutable__conditions()->add_conditions();
                                c->set_type(pd::condition::COST_ITEM);
                                c->add_arg(to_string(ptt.item()));
                                c->add_arg("1");

                                modify_events_by_conditions(ptt._conditions(), *ptt.mutable__events());

                                auto found = false;
                                for (const auto& i : ptt.events().events()) {
                                        if (i.type() != pd::CET_CHANGE_VALUE) {
                                                continue;
                                        }
                                        pd::child_value_type type;
                                        pd::child_value_type_Parse(i.arg(0), &type);
                                        switch (type) {
                                        case pd::CVT_HP_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::HP);
                                                found = true;
                                                break;
                                        case pd::CVT_ATTACK_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::ATTACK);
                                                found = true;
                                                break;
                                        case pd::CVT_DEFENCE_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::PHYSICAL_DEFENCE);
                                                found = true;
                                                break;
                                        case pd::CVT_BOOM_RATE_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::BOOM_RATE);
                                                found = true;
                                                break;
                                        case pd::CVT_HIT_RATE_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::HIT_RATE);
                                                found = true;
                                                break;
                                        case pd::CVT_ANTI_BOOM_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::ANTI_BOOM);
                                                found = true;
                                                break;
                                        case pd::CVT_DODGE_RATE_EXP:
                                                ptt.set__exp_value_type(type);
                                                ptt.set__attr_type(pd::actor::DODGE_RATE);
                                                found = true;
                                                break;
                                        default:
                                                break;
                                        }
                                        if (found) {
                                                break;
                                        }
                                }
                                if (!found) {
                                        CONFIG_ELOG << ptt.item() << " no add exp value";
                                }
                        };
                        child_study_item_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG << ptt.item() << " item not exist";
                                }
                                if (!verify_conditions(ptt._conditions())) {
                                        CONFIG_ELOG << ptt.item() << " verify _conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << ptt.item() << " verify _events failed";
                                }
                                if (!verify_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.item() << " verify conditions failed";
                                }
                                if (!verify_child_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.item() << " verify events failed";
                                }
                        };
                }

                child_skill_ptts& child_skill_ptts_instance() {
                        static child_skill_ptts inst;
                        return inst;
                }

                void child_skill_ptts_set_funcs() {
                        child_skill_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " check conditions failed";
                                }
                                if (!check_conditions(ptt.change_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " check change conditions failed";
                                }
                                if (!check_child_conditions(ptt.child_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " check events failed";
                                }
                                if (!check_child_conditions(ptt.levelup_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " check levelup conditions failed";
                                }
                        };
                        child_skill_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *e = ptt.mutable__child_events()->add_events();
                                e->set_type(pd::CET_LEARN_SKILL);
                                e->add_arg(to_string(ptt.skill()));

                                e = ptt.mutable__levelup_events()->add_events();
                                e->set_type(pd::CET_LEVELUP_SKILL);
                                e->add_arg(to_string(ptt.skill()));

                                modify_events_by_conditions(ptt.change_conditions(), *ptt.mutable__change_events());
                        };
                        child_skill_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(skill, ptt.skill())) {
                                        CONFIG_ELOG << ptt.skill()<< " skill not exist " << ptt.skill();
                                }
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify conditions failed";
                                }
                                if (!verify_conditions(ptt.change_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify change conditions failed";
                                }
                                if (!verify_events(ptt._change_events())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify change events failed";
                                }
                                if (!verify_child_conditions(ptt.child_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify child conditions failed";
                                }
                                if (!verify_child_events(ptt._child_events())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify child events failed";
                                }
                                if (!verify_child_conditions(ptt.levelup_conditions())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify levelup conditions failed";
                                }
                                if (!verify_child_events(ptt._levelup_events())) {
                                        CONFIG_ELOG << ptt.skill()<< " verify levelup events failed";
                                }
                        };
                }

                child_learn_skill_actor_ptts& child_learn_skill_actor_ptts_instance() {
                        static child_learn_skill_actor_ptts inst;
                        return inst;
                }

                void child_learn_skill_actor_ptts_set_funcs() {
                        child_learn_skill_actor_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << ptt.actor() << " actor not exist " << ptt.actor();
                                }
                                for (auto i : ptt.skills()) {
                                        if (!PTTS_HAS(child_skill, i)) {
                                                CONFIG_ELOG << ptt.actor() << " skill not exist " << i;
                                        }
                                }
                        };
                }

                child_add_skill_exp_item_ptts& child_add_skill_exp_item_ptts_instance() {
                        static child_add_skill_exp_item_ptts inst;
                        return inst;
                }

                void child_add_skill_exp_item_ptts_set_funcs() {
                        child_add_skill_exp_item_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.item() << " check conditions failed";
                                }
                                if (!check_child_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.item() << " check events failed";
                                }
                        };
                        child_add_skill_exp_item_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *c = ptt.mutable__conditions()->add_conditions();
                                c->set_type(pd::condition::COST_ITEM);
                                c->add_arg(to_string(ptt.item()));
                                c->add_arg("1");

                                modify_events_by_conditions(ptt._conditions(), *ptt.mutable__events());

                                auto found = false;
                                for (const auto& i : ptt.events().events()) {
                                        if (i.type() != pd::CET_CHANGE_VALUE) {
                                                continue;
                                        }
                                        pd::child_value_type type;
                                        pd::child_value_type_Parse(i.arg(0), &type);
                                        switch (type) {
                                        case pd::CVT_SKILL_EXP:
                                                ptt.set__skill_slot(2);
                                                ptt.set__skill_exp_type(type);
                                                found = true;
                                                break;
                                        default:
                                                break;
                                        }
                                        if (found) {
                                                break;
                                        }
                                }
                                if (!found) {
                                        CONFIG_ELOG << ptt.item() << " no add skill exp value";
                                }
                        };
                        child_add_skill_exp_item_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(item, ptt.item())) {
                                        CONFIG_ELOG << ptt.item() << " item not exist";
                                }
                                if (!verify_conditions(ptt._conditions())) {
                                        CONFIG_ELOG << ptt.item() << " verify _conditions failed";
                                }
                                if (!verify_events(ptt._events())) {
                                        CONFIG_ELOG << ptt.item() << " verify _events failed";
                                }
                                if (!verify_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.item() << " verify conditions failed";
                                }
                                if (!verify_child_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.item() << " verify events failed";
                                }
                        };
                }

                child_drop_ptts& child_drop_ptts_instance() {
                        static child_drop_ptts inst;
                        return inst;
                }

                void child_drop_ptts_set_funcs() {
                        child_drop_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!check_child_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check events failed";
                                        }
                                }
                        };
                        child_drop_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.events()) {
                                        if (!verify_child_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify events failed";
                                        }
                                }
                        };
                }

                child_buff_ptts& child_buff_ptts_instance() {
                        static child_buff_ptts inst;
                        return inst;
                }

                void child_buff_ptts_set_funcs() {
                        child_buff_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.buff() << " check conditions failed";
                                }
                        };
                        child_buff_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(buff, ptt.buff())) {
                                        CONFIG_ELOG << ptt.buff() << " buff not exist " << ptt.buff();
                                }
                                if (!verify_child_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.buff() << " verify conditions failed";
                                }
                        };
                }

        }
}
