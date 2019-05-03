#pragma once

#include "log.hpp"
#include <stdexcept>
#include <string>

using namespace std;

#define DECLARE_BORING_EXCEPTION(class_name) class class_name : public runtime_error { \
public: \
explicit class_name(string&& file, int line) : \
        runtime_error(move(file) + string(":") + to_string(line)) {} \
explicit class_name(string&& message, string&& file, int line) : \
        runtime_error(move(file) + string(":") + to_string(line) + string(" ") + move(message)) {} \
explicit class_name(const string& message, string&& file, int line) : \
        runtime_error(move(file) + string(":") + to_string(line) + string(" ") + message) {} \
explicit class_name(string&& message) : \
        runtime_error(move(message)) {} \
explicit class_name(const string& message) : \
        runtime_error(message) {} \
};

#define THROW_MSG(e, msg) throw e(msg, string(__FILE__),  __LINE__)
#define THROW(e) throw e(string(__FILE__),  __LINE__)

namespace nora {

//-----------------------------common----------------------------
        DECLARE_BORING_EXCEPTION(conflict);
        DECLARE_BORING_EXCEPTION(not_inited);
        DECLARE_BORING_EXCEPTION(init_failed);
        DECLARE_BORING_EXCEPTION(invalid_param);
        DECLARE_BORING_EXCEPTION(not_found);
        DECLARE_BORING_EXCEPTION(bad_num_format);
        DECLARE_BORING_EXCEPTION(bad_bool_format);
        DECLARE_BORING_EXCEPTION(bad_config_format);
        DECLARE_BORING_EXCEPTION(out_of_range);
        DECLARE_BORING_EXCEPTION(got_duplicate);
        DECLARE_BORING_EXCEPTION(not_enough);
        DECLARE_BORING_EXCEPTION(forbidden);
        DECLARE_BORING_EXCEPTION(assert_failed);
        DECLARE_BORING_EXCEPTION(path_not_exist);

//-----------------------------player----------------------------
        DECLARE_BORING_EXCEPTION(bad_db_player_format);
        DECLARE_BORING_EXCEPTION(bad_db_player_data);
        DECLARE_BORING_EXCEPTION(exp_not_enough);
        DECLARE_BORING_EXCEPTION(gold_not_enough);
        DECLARE_BORING_EXCEPTION(level_not_enough);

//-----------------------------actor----------------------------
        DECLARE_BORING_EXCEPTION(already_equip_xequip);
        DECLARE_BORING_EXCEPTION(not_found_slot);
        DECLARE_BORING_EXCEPTION(no_item);
        DECLARE_BORING_EXCEPTION(need_equip_equip);
        DECLARE_BORING_EXCEPTION(no_xequip_slot);
        DECLARE_BORING_EXCEPTION(summon_lottery_error);

//-----------------------------adventure----------------------------
        DECLARE_BORING_EXCEPTION(grid_no_npc);
        DECLARE_BORING_EXCEPTION(grid_no_event);
        DECLARE_BORING_EXCEPTION(won_npc_enough_today);
        DECLARE_BORING_EXCEPTION(not_enough_power);
        DECLARE_BORING_EXCEPTION(grid_is_block);
        DECLARE_BORING_EXCEPTION(invalid_grid);
        DECLARE_BORING_EXCEPTION(challenge_before_move);
        DECLARE_BORING_EXCEPTION(invalid_move);
        DECLARE_BORING_EXCEPTION(jump_adventure);
        DECLARE_BORING_EXCEPTION(have_not_passed);
        DECLARE_BORING_EXCEPTION(already_stay_adventure);
        DECLARE_BORING_EXCEPTION(stay_event_lottery_error);
        DECLARE_BORING_EXCEPTION(giveup_event_no_npc);
        DECLARE_BORING_EXCEPTION(no_npc_grid);
        DECLARE_BORING_EXCEPTION(no_event_grid);
        DECLARE_BORING_EXCEPTION(grid_already_opened);
        DECLARE_BORING_EXCEPTION(today_have_not_reset);
        DECLARE_BORING_EXCEPTION(brand_new_adventure);
        DECLARE_BORING_EXCEPTION(reset_times_ran_out);
        
//-----------------------------skyladder----------------------------
        DECLARE_BORING_EXCEPTION(challenge_defeated_defender);
        DECLARE_BORING_EXCEPTION(defeat_times_reached_max);
        DECLARE_BORING_EXCEPTION(no_more_get_defenders_times);

//-----------------------------lottery----------------------------
        DECLARE_BORING_EXCEPTION(event_has_challenge_npc);

//-----------------------------league------------------------------
        DECLARE_BORING_EXCEPTION(name_exist);
        DECLARE_BORING_EXCEPTION(bad_db_league_format);
        DECLARE_BORING_EXCEPTION(join_before_quit);
        DECLARE_BORING_EXCEPTION(no_such_league);
        DECLARE_BORING_EXCEPTION(not_in_league);
        DECLARE_BORING_EXCEPTION(in_league);
        DECLARE_BORING_EXCEPTION(empty_name);
        DECLARE_BORING_EXCEPTION(no_such_joiner);
        DECLARE_BORING_EXCEPTION(not_a_leader);

//-----------------------------elua------------------------------
        DECLARE_BORING_EXCEPTION(elua_error);

//-----------------------------pb------------------------------
        DECLARE_BORING_EXCEPTION(pb_error);

}
