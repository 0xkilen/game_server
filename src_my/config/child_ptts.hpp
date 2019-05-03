#pragma once

#include "utils/ptts.hpp"
#include "proto/config_child.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using child_phase_ptts = ptts<pc::child_phase>;
                child_phase_ptts& child_phase_ptts_instance();
                void child_phase_ptts_set_funcs();

                using child_logic_ptts = ptts<pc::child_logic>;
                child_logic_ptts& child_logic_ptts_instance();
                void child_logic_ptts_set_funcs();

                using child_value_ptts = ptts<pc::child_value>;
                child_value_ptts& child_value_ptts_instance();
                void child_value_ptts_set_funcs();

                using child_raise_ptts = ptts<pc::child_raise>;
                child_raise_ptts& child_raise_ptts_instance();
                void child_raise_ptts_set_funcs();

                using child_study_ptts = ptts<pc::child_study>;
                child_study_ptts& child_study_ptts_instance();
                void child_study_ptts_set_funcs();

                using child_study_item_ptts = ptts<pc::child_study_item>;
                child_study_item_ptts& child_study_item_ptts_instance();
                void child_study_item_ptts_set_funcs();

                using child_skill_ptts = ptts<pc::child_skill>;
                child_skill_ptts& child_skill_ptts_instance();
                void child_skill_ptts_set_funcs();

                using child_learn_skill_actor_ptts = ptts<pc::child_learn_skill_actor>;
                child_learn_skill_actor_ptts& child_learn_skill_actor_ptts_instance();
                void child_learn_skill_actor_ptts_set_funcs();

                using child_add_skill_exp_item_ptts = ptts<pc::child_add_skill_exp_item>;
                child_add_skill_exp_item_ptts& child_add_skill_exp_item_ptts_instance();
                void child_add_skill_exp_item_ptts_set_funcs();

                using child_drop_ptts = ptts<pc::child_drop>;
                child_drop_ptts& child_drop_ptts_instance();
                void child_drop_ptts_set_funcs();

                using child_buff_ptts = ptts<pc::child_buff>;
                child_buff_ptts& child_buff_ptts_instance();
                void child_buff_ptts_set_funcs();

        }
}
