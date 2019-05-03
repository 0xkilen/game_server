#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_plot.pb.h"

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;
                pd::result xinshou_continue(uint32_t group_pttid, uint32_t pttid, role& role);

        }
}
