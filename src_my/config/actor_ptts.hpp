#pragma once

#include "utils/ptts.hpp"
#include "proto/config_actor.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using actor_ptts = ptts<pc::actor>;
                actor_ptts& actor_ptts_instance();
                void actor_ptts_set_funcs();

                using actor_levelup_ptts = ptts<pc::actor_levelup>;
                actor_levelup_ptts& actor_levelup_ptts_instance();
                void actor_levelup_ptts_set_funcs();

                using actor_add_star_ptts = ptts<pc::actor_add_star>;
                actor_add_star_ptts& actor_add_star_ptts_instance();
                void actor_add_star_ptts_set_funcs();

                using actor_actor_add_star_ptts = ptts<pc::actor_actor_add_star>;
                actor_actor_add_star_ptts& actor_actor_add_star_ptts_instance();
                void actor_actor_add_star_ptts_set_funcs();

                using actor_add_step_ptts = ptts<pc::actor_add_step>;
                actor_add_step_ptts& actor_add_step_ptts_instance();
                void actor_add_step_ptts_set_funcs();

                using actor_add_pin_ptts = ptts<pc::actor_add_pin>;
                actor_add_pin_ptts& actor_add_pin_ptts_instance();
                void actor_add_pin_ptts_set_funcs();

                using actor_intimacy_ptts = ptts<pc::actor_intimacy>;
                actor_intimacy_ptts& actor_intimacy_ptts_instance();
                void actor_intimacy_ptts_set_funcs();

                using actor_united_chance_ptts = ptts<pc::actor_united_chance>;
                actor_united_chance_ptts& actor_united_chance_ptts_instance();
                void actor_united_chance_ptts_set_funcs();

                using actor_voice_ptts = ptts<pc::actor_voice>;
                actor_voice_ptts& actor_voice_ptts_instance();
                void actor_voice_ptts_set_funcs();

                using actor_plot_ptts = ptts<pc::actor_plot>;
                actor_plot_ptts& actor_plot_ptts_instance();
                void actor_plot_ptts_set_funcs();

                using actor_logic_ptts = ptts<pc::actor_logic>;
                actor_logic_ptts& actor_logic_ptts_instance();
                void actor_logic_ptts_set_funcs();

                using actor_bubble_ptts = ptts<pc::actor_bubble>;
                actor_bubble_ptts& actor_bubble_ptts_instance();
                void actor_bubble_ptts_set_funcs();

                using actor_skin_ptts = ptts<pc::actor_skin>;
                actor_skin_ptts& actor_skin_ptts_instance();
                void actor_skin_ptts_set_funcs();

        }
}
