#pragma once

#include "utils/ptts.hpp"
#include "proto/config_plot.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using plot_ptts = ptts<pc::plot>;
                plot_ptts& plot_ptts_instance();
                void plot_ptts_set_funcs();

                using plot_options_ptts = ptts<pc::plot_options>;
                plot_options_ptts& plot_options_ptts_instance();
                void plot_options_ptts_set_funcs();

                using plot_options_group_ptts = ptts<pc::plot_options_group>;
                plot_options_group_ptts& plot_options_group_ptts_instance();
                void plot_options_group_ptts_set_funcs();

        }
}
