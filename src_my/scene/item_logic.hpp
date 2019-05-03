#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_event.pb.h"
#include "proto/data_item.pb.h"

namespace pd = proto::data;

namespace nora {
        namespace scene {

                class role;

                pd::result item_check_use(uint32_t pttid, int count, const role& role);
                pd::event_res item_use(uint32_t pttid, int count, role& role);
                pd::result item_check_xinwu_compose(uint32_t xinwu, uint32_t count, const role& role);
                pd::event_res item_xinwu_compose(uint32_t xinwu, uint32_t count, role& role);
                pd::result item_check_xinwu_decompose(uint32_t xinwu, uint32_t count, const role& role);
                pd::event_res item_xinwu_decompose(uint32_t xinwu, uint32_t count, role& role);

        }

}
