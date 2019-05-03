#include "player_ptts.hpp"
#include "actor_ptts.hpp"
#include "item_ptts.hpp"
#include "title_ptts.hpp"
#include "quest_ptts.hpp"
#include "equipment_ptts.hpp"
#include "mail_ptts.hpp"
#include "drop_ptts.hpp"
#include "mansion_ptts.hpp"
#include "utils.hpp"
#include "utils/dirty_word_filter.hpp"


namespace pd = proto::data;
namespace nora {
        namespace config {

                create_role_ptts& create_role_ptts_instance() {
                        static create_role_ptts inst;
                        return inst;
                }

                void create_role_ptts_set_funcs() {
                        create_role_ptts_instance().verify_func_ = [] (const pc::create_role& ptt) {
                                for (auto i : ptt.actors()) {
                                        if (!PTTS_HAS(actor, i)) {
                                                CONFIG_ELOG << " actor not exist " << i;
                                        }
                                }
                                for (auto i : ptt.grids()) {
                                        if (!PTTS_HAS(actor, i.actor())) {
                                                CONFIG_ELOG << " actor not exist " << i.actor();
                                        } else {
                                                const auto& actor_ptt = PTTS_GET(actor, i.actor());
                                                if (actor_ptt.type() == pd::actor::ZHU && (ptt.id() == 1 || ptt.id() == 2)) {
                                                        if (i.actor() != ptt.id()) {
                                                                CONFIG_ELOG << " zhujue actor id error " << i.actor();
                                                        }
                                                }
                                        }
                                }
                                for (const auto& i : ptt.items()) {
                                        if (!PTTS_HAS(item, i.pttid())) {
                                                CONFIG_ELOG << " item not exist " << i.pttid();
                                        }
                                }
                                for (auto i : ptt.quests()) {
                                        if (!PTTS_HAS(quest, i)) {
                                                CONFIG_ELOG << " quests not exist " << i;
                                        }
                                }
                                for (const auto& i : ptt.spine_selection()) {
                                        if (!SPINE_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " spine not exist " << pd::spine_part_Name(i.part()) << " " << i.pttid();
                                        }
                                }
                                for (const auto& i : ptt.spine_collection()) {
                                        if (!SPINE_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " spine not exist " << pd::spine_part_Name(i.part()) << " " << i.pttid();
                                        }
                                }
                                for (const auto& i : ptt.huanzhuang_selection()) {
                                        if (!HUANZHUANG_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " huanzhuang not exist " << pd::huanzhuang_part_Name(i.part()) << " " << i.pttid();
                                        }
                                }
                                for (const auto& i : ptt.huanzhuang_collection()) {
                                        if (!HUANZHUANG_PTTS_HAS(i.part(), i.pttid())) {
                                                CONFIG_ELOG << ptt.id() << " huanzhuang not exist " << pd::huanzhuang_part_Name(i.part()) << " " << i.pttid();
                                        }
                                }
                                for (const auto& i : ptt.equipments()) {
                                        if (!PTTS_HAS(equipment, i.pttid())) {
                                                CONFIG_ELOG << " equipment not exist " << i.pttid();
                                        }
                                }
                                for (auto i : ptt.titles()) {
                                        if (!PTTS_HAS(title, i)) {
                                                CONFIG_ELOG << " title not exist " << i;
                                                continue;
                                        }
                                        const auto& title_ptt = PTTS_GET(title, i);
                                        if (title_ptt.days() > 0) {
                                                CONFIG_ELOG << " title has days " << i;
                                        }
                                }
                                for (auto i : ptt.mansion_cook_recipes()) {
                                        if (!PTTS_HAS(mansion_cook_recipe, i)) {
                                                CONFIG_ELOG << " mansion cook recipe not exist " << i;
                                        }
                                }
                        };
                }

                role_levelup_ptts& role_levelup_ptts_instance() {
                        static role_levelup_ptts inst;
                        return inst;
                }

                void role_levelup_ptts_set_funcs() {
                        role_levelup_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.level() << " check conditions failed";
                                }
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.level() << " check events failed";
                                }
                        };
                        role_levelup_ptts_instance().modify_func_ = [] (auto& ptt) {
                                pd::event_array events;
                                auto *e = events.add_events();
                                e->set_type(pd::event::LEVELUP);
                                e = events.add_events();
                                e->set_type(pd::event::ACTOR_LEVELUP);
                                for (const auto& i : ptt.events().events()) {
                                        *events.add_events() = i;
                                }

                                modify_events_by_conditions(ptt.conditions(), events);
                                *ptt.mutable_events() = events;
                        };
                        role_levelup_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.level() << " verify conditions failed";
                                }
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.level() << " verify events failed";
                                }
                                if (ptt.has_mail() && !PTTS_HAS(mail, ptt.mail())) {
                                        CONFIG_ELOG << ptt.level() << " mail not exist " << ptt.mail();
                                }
                        };
                }

                role_add_fate_ptts& role_add_fate_ptts_instance() {
                        static role_add_fate_ptts inst;
                        return inst;
                }

                void role_add_fate_ptts_set_funcs() {
                        role_add_fate_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.fate() << " check events failed";
                                }
                        };

                        role_add_fate_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *events = ptt.mutable_events();
                                auto *e = events->add_events();
                                e->set_type(pd::event::ROLE_ADD_FATE);

                                modify_events_by_conditions(ptt.conditions(), *events);
                        };

                        role_add_fate_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.fate() << " verify events failed";
                                }
                        };
                }

                role_logic_ptts& role_logic_ptts_instance() {
                        static role_logic_ptts inst;
                        return inst;
                }

                void role_logic_ptts_set_funcs() {
                        role_logic_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.eat_events())) {
                                        CONFIG_ELOG << " check eat events failed";
                                }
                                if (!check_events(ptt.share_events())) {
                                        CONFIG_ELOG << " check share events failed";
                                }
                                if (!check_conditions(ptt.add_fate_conditions())) {
                                        CONFIG_ELOG << " check add fate conditions failed";
                                }
                                if (!check_conditions(ptt.customize_icon_conditions())) {
                                        CONFIG_ELOG << " check customize icon conditions failed";
                                }
                                if (!check_conditions(ptt.rank_like_conditions())) {
                                        CONFIG_ELOG << " check rank_like_conditions failed";
                                }
                                if (!check_conditions(ptt.mirror_unlock_conditions())) {
                                        CONFIG_ELOG << " check mirror_unlock_conditions failed";
                                }
                        };
                        role_logic_ptts_instance().modify_func_ = [] (auto& ptt) {
                                modify_events_by_conditions(ptt.change_name_conditions(), *ptt.mutable__change_name_events());
                                modify_events_by_conditions(ptt.rank_like_conditions(), *ptt.mutable_rank_like_events());
                        };
                        role_logic_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.eat_events())) {
                                        CONFIG_ELOG << " verify eat events failed";
                                }
                                if (!verify_events(ptt.share_events())) {
                                        CONFIG_ELOG << " verify share events failed";
                                }
                                if (!verify_conditions(ptt.add_fate_conditions())) {
                                        CONFIG_ELOG << " verify add_fate_conditions failed";
                                }
                                if (!verify_conditions(ptt.customize_icon_conditions())) {
                                        CONFIG_ELOG << " verify customize icon conditions failed";
                                }
                                if (!verify_conditions(ptt.rank_like_conditions())) {
                                        CONFIG_ELOG << " verify rank_like_conditions failed";
                                }
                                if (!verify_events(ptt.rank_like_events())) {
                                        CONFIG_ELOG << " verify rank_like_events failed";
                                }
                                if (!verify_conditions(ptt.mirror_unlock_conditions())) {
                                        CONFIG_ELOG << " check mirror_unlock_conditions failed";
                                }
                        };
                }

                role_present_ptts& role_present_ptts_instance() {
                        static role_present_ptts inst;
                        return inst;
                }

                void role_present_ptts_set_funcs() {
                        role_present_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.buy_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check buy conditions failed";
                                }
                        };

                        role_present_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *e = ptt.mutable__buy_events()->add_events();
                                e->set_type(pd::event::ADD_PRESENT);
                                e->add_arg(to_string(ptt.id()));
                                e->add_arg(to_string(10));
                                modify_events_by_conditions(ptt.buy_conditions(), *ptt.mutable__buy_events());
                        };

                        role_present_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_conditions(ptt.buy_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify buy conditions failed";
                                }
                                if (!verify_events(ptt._buy_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify buy events failed";
                                }
                                if (!PTTS_HAS(drop, ptt.drop())) {
                                        CONFIG_ELOG << ptt.id() << " drop not exist " << ptt.drop();
                                }
                        };
                }

                robot_ptts& robot_ptts_instance() {
                        static robot_ptts inst;
                        return inst;
                }

                void robot_ptts_set_funcs() {
                        robot_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!dirty_word_filter::instance().check(ptt.username())) {
                                        CONFIG_ELOG << ptt.id() << " username has dirty word";
                                }
                        };
                }

                gm_data_ptts& gm_data_ptts_instance() {
                        static gm_data_ptts inst;
                        return inst;
                }

                void gm_data_ptts_set_funcs() {
                        gm_data_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed";
                                }
                        };

                        gm_data_ptts_instance().verify_func_ = [] (auto& ptt) {
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                        };
                }

                gm_sequence_ptts& gm_sequence_ptts_instance() {
                        static gm_sequence_ptts inst;
                        return inst;
                }

                void gm_sequence_ptts_set_funcs() {
                        gm_sequence_ptts_instance().verify_func_ = [] (auto& ptt) {
                                for (auto i : ptt.sequences()) {
                                        if (!PTTS_HAS(gm_data, i)) {
                                                CONFIG_ELOG << ptt.id() << " has not exist in gm_data " << i;
                                        }
                                }
                        };
                }

        }
}
