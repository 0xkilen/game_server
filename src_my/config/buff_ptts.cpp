#include "buff_ptts.hpp"
#include "proto/config_actor.pb.h"
#include "actor_ptts.hpp"
#include "skill_ptts.hpp"
#include <map>
#include <set>


namespace pc = proto::config;

namespace nora {
        namespace config {

                buff_ptts& buff_ptts_instance() {
                        static buff_ptts inst;
                        return inst;
                }

                struct buff_effect_info {
                        buff_effect_info(int arg_size, int role_size) : arg_size_(arg_size), role_size_(role_size) {}
                        int arg_size_;
                        int role_size_;
                };
                map<pc::buff_effect_condition::buff_effect_condition_type, buff_effect_info> effect_condition_info_map = {
                        { pc::buff_effect_condition::HP, buff_effect_info(2, 2)},
                        { pc::buff_effect_condition::HP_PERCENT, buff_effect_info(2, 1)},
                        { pc::buff_effect_condition::ACTOR_LEVEL, buff_effect_info(2, 2)},
                        { pc::buff_effect_condition::ACTOR_TAG, buff_effect_info(1, 1)},
                        { pc::buff_effect_condition::NO_ACTOR_TAG, buff_effect_info(1, 1)},
                        { pc::buff_effect_condition::BUFF_LAYER, buff_effect_info(3, 1)},
                        { pc::buff_effect_condition::RANDOM_NUMBER, buff_effect_info(1, 0)},
                        { pc::buff_effect_condition::TARGET_ALIVE, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_DEAD, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::COMPARE_ACTOR_PTTID, buff_effect_info(1, 1)},
                        { pc::buff_effect_condition::TARGET_IS_FEMALE, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_IS_MALE, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_TYPE_IS_ZHU, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_TYPE_IS_DPS, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_TYPE_IS_TANK, buff_effect_info(0, 1)},
                        { pc::buff_effect_condition::TARGET_TYPE_IS_ASSIST, buff_effect_info(0, 1)}
                };
                map<pc::buff_effect_effect::buff_effect_type, buff_effect_info> effect_effect_info_map{
                        { pc::buff_effect_effect::DAMAGE, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::MAX_HP_PERCENT_DAMAGE, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::CURRENT_HP_PERCENT_DAMAGE, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::DAMAGEOUT, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::DAMAGEOUT_HEAL, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::ATTACK, buff_effect_info(2, 2)},
                        { pc::buff_effect_effect::ATTACK_BY_SKILL, buff_effect_info(0, 2)},
                        { pc::buff_effect_effect::HEAL, buff_effect_info(0, 2)},
                        { pc::buff_effect_effect::MINUS_HP, buff_effect_info(1, 2)},
                        { pc::buff_effect_effect::TARGETER_NUM, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::HP_PERCENT_ATTACK, buff_effect_info(2, 2)},
                        { pc::buff_effect_effect::HP_PERCENT_HEAL, buff_effect_info(2, 2)},
                        { pc::buff_effect_effect::FIX_HEAL, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::MAX_HP_PERCENT_HEAL, buff_effect_info(1, 2)},
                        //{ pc::buff_effect_effect::SOUL_BONDS, buff_effect_info(0, 0)},
                        { pc::buff_effect_effect::FREE_ACTION, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::VALUE_DESCREASE, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::POISON_DAMAGE, buff_effect_info(2, 2)},
                        { pc::buff_effect_effect::TAUNT, buff_effect_info(0, 2)},
                        { pc::buff_effect_effect::CAST_SKILL, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::CHANGE_VALUE, buff_effect_info(2, 0)},
                        { pc::buff_effect_effect::CHANGE_VALUE_PERCENT, buff_effect_info(2, 0)},
                        { pc::buff_effect_effect::SCALE_TO, buff_effect_info(0, 0)},
                        { pc::buff_effect_effect::ADD_BUFF, buff_effect_info(2, 2)},
                        { pc::buff_effect_effect::REMOVE_BUFF, buff_effect_info(1, 1)},
                        { pc::buff_effect_effect::REMOVE_ALL_BUFF, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::REMOVE_ALL_DEBUFF, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::REMOVE_RANDOM_BUFF, buff_effect_info(1, 1)},
                        { pc::buff_effect_effect::REMOVE_RANDOM_DEBUFF, buff_effect_info(1, 1)},
                        //{ pc::buff_effect_effect::CHANGE_SPINE, buff_effect_info(0, 0)},
                        { pc::buff_effect_effect::DISABLE_SKILL, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::DISABLE_ALL_SKILL, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::ATTACK_BACK, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::DAMAGE_CHANGE_BY_HP_PERCENT, buff_effect_info(2, 1)},
                        // { pc::buff_effect_effect::ADD_BUFF_BY_PASSIVE, buff_effect_info(2, 0)},
                        { pc::buff_effect_effect::REFRESH_SKILLCD, buff_effect_info(0, 1)},
                        { pc::buff_effect_effect::SHARE_HEAL, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::HEMOPHAGIA, buff_effect_info(1, 1)},
                        { pc::buff_effect_effect::DEATH_INCREASE_DAMAGE, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::PARTTAKE_DAMAGE, buff_effect_info(1, 1)},
                        { pc::buff_effect_effect::ADD_BUFF_RANDOM, buff_effect_info(3, 2)},
                        { pc::buff_effect_effect::EACH_TIME_CHANGE_VALUE, buff_effect_info(3, 0)},
                        { pc::buff_effect_effect::SAME_TARGET, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::PLAY_PLOT, buff_effect_info(1, 0)},
                        { pc::buff_effect_effect::ADD_SHIELD, buff_effect_info(2, 2)},
                };

                const static set<string> change_value_types {
                        {"max_hp"},
                        {"attack"},
                        {"physical_defence"},
                        {"magical_defence"},
                        {"dodge_rate"},
                        {"hit_rate"},
                        {"boom_rate"},
                        {"anti_boom"},
                        {"boom_damage"},
                        {"dec_boom_damage"},
                        {"block_rate"},
                        {"anti_block_rate"},
                        {"in_damage_percent"},
                        {"in_heal_percent"},
                        {"out_heal_percent"},
                        {"out_damage_percent"},
                        {"add_pvp_damage_percent"},
                        {"dec_pvp_damage_percent"},
                };

                void buff_ptts_set_funcs() {
                        buff_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.name().empty()) {
                                        CONFIG_ELOG << ptt.id() << " name is empty";
                                }
                                if (ptt.time() < -1) {
                                        CONFIG_ELOG << ptt.id() << " time less than -1";
                                }
                                if (ptt.on_add_size() > 0) {
                                        for (const auto& i : ptt.on_add()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_remove_size() > 0) {
                                        for (const auto& i : ptt.on_remove()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_skillin_size() > 0) {
                                        for (const auto& i : ptt.on_skillin()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_skillout_size() > 0) {
                                        for (const auto& i : ptt.on_skillout()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_damagein_size() > 0) {
                                        for (const auto& i : ptt.on_damagein()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_damageout_size() > 0) {
                                        for (const auto& i : ptt.on_damageout()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_healin_size() > 0) {
                                        for (const auto& i : ptt.on_healin()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_attackin_size() > 0) {
                                        for (const auto& i : ptt.on_attackin()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_buffin_size() > 0) {
                                        for (const auto& i : ptt.on_buffin()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_turn_size() > 0) {
                                        for (const auto& i : ptt.on_turn()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_turn_end_size() > 0) {
                                        for (const auto& i : ptt.on_turn_end()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }
                                }
                                if (ptt.on_turn_before_size() > 0) {
                                        for (const auto& i : ptt.on_turn_before()){
                                                check_buff_effect_condition_size(ptt, i);
                                                check_buff_effect_effect_size(ptt, i);
                                        }      
                                }
                        };
                        buff_ptts_instance().modify_func_ = [] (const auto& ptt) {};
                        buff_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.on_add_size() > 0) {
                                        for (const auto& i : ptt.on_add()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_remove_size() > 0) {
                                        for (const auto& i : ptt.on_remove()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_skillin_size() > 0) {
                                        for (const auto& i : ptt.on_skillin()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_skillout_size() > 0) {
                                        for (const auto& i : ptt.on_skillout()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_damagein_size() > 0) {
                                        for (const auto& i : ptt.on_damagein()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_damageout_size() > 0) {
                                        for (const auto& i : ptt.on_damageout()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_healin_size() > 0) {
                                        for (const auto& i : ptt.on_healin()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_attackin_size() > 0) {
                                        for (const auto& i : ptt.on_attackin()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_buffin_size() > 0) {
                                        for (const auto& i : ptt.on_buffin()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_turn_size() > 0) {
                                        for (const auto& i : ptt.on_turn()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_turn_end_size() > 0) {
                                        for (const auto& i : ptt.on_turn_end()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                                if (ptt.on_skillout_size() > 0) {
                                        for (const auto& i : ptt.on_skillout()){
                                                verify_buff_effect_effect(ptt, i);
                                        }
                                }
                        };
                }

                void check_buff_effect_condition_size(const proto::config::buff& ptt, const pc::buff_effect& bf) {
                        for (const auto& ec : bf.condition()) {
                                if (effect_condition_info_map.count(ec.type()) <= 0) {
                                        continue;
                                }
                                if (effect_condition_info_map.at(ec.type()).arg_size_ != ec.arg_size()) {
                                        CONFIG_ELOG << "type=" << buff_effect_condition_buff_effect_condition_type_Name(ec.type()) <<"    buff condition.arg_size() !="<< effect_condition_info_map.at(ec.type()).arg_size_  <<"   arg_size()=" << ec.arg_size() << "\n" << ptt.DebugString();
                                }
                                if (ec.type() == pc::buff_effect_condition::HP_PERCENT) {
                                        if (effect_condition_info_map.at(ec.type()).role_size_ > ec.role_size()) {
                                                CONFIG_ELOG << "type=" << buff_effect_condition_buff_effect_condition_type_Name(ec.type()) <<"    buff condition.role_size() < "<< effect_condition_info_map.at(ec.type()).role_size_  <<"   role_size()=" << ec.role_size() << "\n" << ptt.DebugString();
                                        }
                                        continue;
                                }
                                if (effect_condition_info_map.at(ec.type()).role_size_ != ec.role_size() + ec.find_role_size()) {
                                        CONFIG_ELOG << "type=" << buff_effect_condition_buff_effect_condition_type_Name(ec.type()) <<"    buff condition.role_size() !="<< effect_condition_info_map.at(ec.type()).role_size_  <<"   role_size()=" << ec.role_size() << "\n" << ptt.DebugString();
                                }
                        }
                }

                void check_buff_effect_effect_size(const proto::config::buff& ptt, const pc::buff_effect& bf) {
                        for (const auto& ee : bf.effect()) {
                                if (effect_effect_info_map.count(ee.type()) <= 0) {
                                        continue;
                                }

                                if (ee.type() == pc::buff_effect_effect::DISABLE_SKILL) {
                                        if (ee.arg_size() <= effect_effect_info_map.at(ee.type()).arg_size_) {
                                                CONFIG_ELOG << "type=" << buff_effect_effect_buff_effect_type_Name(ee.type()) <<"    buff effect.arg_size() <="<< effect_effect_info_map.at(ee.type()).arg_size_  <<"   arg_size()=" << ee.arg_size() << "\n" << ptt.DebugString();
                                        }
                                } else {
                                        if (effect_effect_info_map.at(ee.type()).arg_size_ != ee.arg_size()) {
                                                CONFIG_ELOG << "type=" << buff_effect_effect_buff_effect_type_Name(ee.type()) <<"    buff effect.arg_size() !="<< effect_effect_info_map.at(ee.type()).arg_size_  <<"   arg_size()=" << ee.arg_size() << "\n" << ptt.DebugString();
                                        }
                                }

                                if (ee.type() == pc::buff_effect_effect::CHANGE_VALUE ||
                                    ee.type() == pc::buff_effect_effect::CHANGE_VALUE_PERCENT) {
                                        if (change_value_types.count(ee.arg(0)) == 0) {
                                                CONFIG_ELOG << "type=" << buff_effect_effect_buff_effect_type_Name(ee.type()) << " arg value " << ee.arg(0) << " not exist";
                                        }
                                }

                                if (ee.type() == pc::buff_effect_effect::CHANGE_VALUE ||
                                    ee.type() == pc::buff_effect_effect::CHANGE_VALUE_PERCENT ||
                                    ee.type() == pc::buff_effect_effect::CAST_SKILL) {
                                        if (ee.role_size() <= effect_effect_info_map.at(ee.type()).role_size_) {
                                                CONFIG_ELOG << "type=" << buff_effect_effect_buff_effect_type_Name(ee.type()) <<"    buff effect.role_size() <="<< effect_effect_info_map.at(ee.type()).role_size_  <<"   role_size()=" << ee.role_size() << "\n" << ptt.DebugString();
                                        }
                                } else {
                                        if (effect_effect_info_map.at(ee.type()).role_size_ != ee.role_size() + ee.find_role_size()) {
                                                CONFIG_ELOG << "type=" << buff_effect_effect_buff_effect_type_Name(ee.type()) <<"    buff effect.role_size() !="<< effect_effect_info_map.at(ee.type()).role_size_  <<"   role_size()=" << ee.role_size() << "\n" << ptt.DebugString();
                                        }
                                }
                        }
                }

                void verify_buff_effect_effect(const proto::config::buff& ptt, const pc::buff_effect& bf) {
                        for (const auto& ee : bf.effect()) {
                                if (ee.type() == pc::buff_effect_effect::CAST_SKILL) {
                                        auto skill_id = stoi(ee.arg(0));
                                        if(!PTTS_HAS(skill, skill_id)) {
                                                CONFIG_ELOG << "type="<<buff_effect_effect_buff_effect_type_Name(ee.type()) <<"     cast_skill of buff table not in skill table, id=" << ptt.id() << " skill=" << skill_id;
                                        }
                                }
                                if (ee.type() == pc::buff_effect_effect::ADD_BUFF_BY_PASSIVE) {
                                        auto buff_id = stoi(ee.arg(0));
                                        if(!PTTS_HAS(buff, buff_id)) {
                                                CONFIG_ELOG << "type="<<buff_effect_effect_buff_effect_type_Name(ee.type()) <<"     add_buff_by_passive of buff table not in buff table, id=" << ptt.id() << " buff=" << buff_id;
                                        }
                                }
                        }
                }

                void verify_buff_actor_effect_effect(const proto::config::buff& ptt, const pc::buff_effect& bf) {
                        for (const auto& ee : bf.effect()) {
                                if (ee.type() == pc::buff_effect_effect::CAST_SKILL) {
                                        auto skill_id = stoi(ee.arg(0));
                                        try {
                                                PTTS_GET_ALL(actor);
                                        } catch(const exception& e) {
                                                CONFIG_ELOG << "actor table is empty";
                                        }
                                        map<uint32_t, set<uint32_t>> skill_to_actor;
                                        auto actors_ptt = PTTS_GET_ALL(actor);
                                        for(auto& i : actors_ptt) {
                                                auto actor = i.second;
                                                if(actor.skill_size() > 0) {
                                                        for (auto j: actor.skill()) {
                                                                if (skill_to_actor.count(j) <= 0) {
                                                                        set<uint32_t> s = {i.first};
                                                                        skill_to_actor[j] = s;
                                                                } else {
                                                                        skill_to_actor[j].insert(i.first);
                                                                }
                                                        }
                                                }
                                        }
                                        if (!skill_to_actor.empty()) {
                                                if (skill_to_actor.count(skill_id) <= 0) {
                                                        CONFIG_ELOG << "type="<<buff_effect_effect_buff_effect_type_Name(ee.type()) << "     cast_skill of buff table not in actor table, id=" << ptt.id() << " cast_skill=" << skill_id;
                                                }
                                        } else {
                                                CONFIG_ELOG << "actor table not set skill";
                                        }
                                }
                        }
                }

        }
}

