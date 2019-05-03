#pragma once

#include "utils.hpp"
#include "utils/ptts.hpp"
#include "proto/config_feige.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using feige_message_ptts = ptts<pc::feige_message>;
                feige_message_ptts& feige_message_ptts_instance();
                void feige_message_ptts_set_funcs();

                using feige_ptts = ptts<pc::feige>;
                feige_ptts& feige_ptts_instance();
                void feige_ptts_set_funcs();

                using feige_chuanwen_ptts = ptts<pc::feige_chuanwen>;
                feige_chuanwen_ptts& feige_chuanwen_ptts_instance();
                void feige_chuanwen_ptts_set_funcs();

                using feige_chuanwen_group_ptts = ptts<pc::feige_chuanwen_group>;
                feige_chuanwen_group_ptts& feige_chuanwen_group_ptts_instance();
                void feige_chuanwen_group_ptts_set_funcs();

                using feige_cg_ptts = ptts<pc::feige_cg>;
                feige_cg_ptts& feige_cg_ptts_instance();
                void feige_cg_ptts_set_funcs();

                using major_city_bubble_ptts = ptts<pc::major_city_bubble>;
                major_city_bubble_ptts& major_city_bubble_ptts_instance();
                void major_city_bubble_ptts_set_funcs();

        }
}
