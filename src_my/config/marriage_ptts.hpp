#pragma once

#include "utils/ptts.hpp"
#include "utils/game_def.hpp"
#include "proto/config_marriage.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using marriage_logic_ptts = ptts<pc::marriage_logic>;
                marriage_logic_ptts& marriage_logic_ptts_instance();
                void marriage_logic_ptts_set_funcs();

                using marriage_question_ptts = ptts<pc::marriage_question>;
                marriage_question_ptts& marriage_question_ptts_instance();
                void marriage_question_ptts_set_funcs();

                using marriage_guesture_ptts = ptts<pc::marriage_guesture>;
                marriage_guesture_ptts& marriage_guesture_ptts_instance();
                void marriage_guesture_ptts_set_funcs();

                using marriage_guesture_pair_ptts = ptts<pc::marriage_guesture_pair>;
                marriage_guesture_pair_ptts& marriage_guesture_pair_ptts_instance();
                void marriage_guesture_pair_ptts_set_funcs();

                using marriage_child_qian_ptts = ptts<pc::marriage_child_qian>;
                marriage_child_qian_ptts& marriage_child_qian_ptts_instance();
                void marriage_child_qian_ptts_set_funcs();

                using marriage_enai_ptts = ptts<pc::marriage_enai>;
                marriage_enai_ptts& marriage_enai_ptts_instance();
                void marriage_enai_ptts_set_funcs();
        }
}
