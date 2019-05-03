#pragma once

#include "proto/data_fief.pb.h"
#include "proto/data_plot.pb.h"
#include "proto/data_league.pb.h"

namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;

                pd::event_array drop_to_event(const pd::event_array& a);
                void event_merge(pd::event_array& a, const pd::event_array& b);
                void event_adopt_env(pd::event_array& ea, const pd::ce_env& ce);

                pd::event_res event_process(const pd::event_array& ea, role& role, const pd::ce_env& ce, const pd::event_res *merge_res = nullptr, bool send_bi = true);

                void event_revert(const pd::event_res& er, role& role, pd::ce_origin ce_type);
                void event_revert(const pd::event_array& events, role& role, pd::ce_origin ce_type);
                pd::event_array event_revert_events(const pd::event_array& events);

                int event_add_resource_count(const pd::event_array& events, pd::resource_type type);

                void fief_event_merge(pd::fief_event_array& a, const pd::fief_event_array& b);

                void plot_event_merge(pd::plot_event_array& a, const pd::plot_event_array& b);

                void league_event_merge(pd::league_event_array& a, const pd::league_event_array& b);
                void send_event_res_to_bi(const role& role, const pd::event_res& res, pd::ce_origin origin);

        }
}
