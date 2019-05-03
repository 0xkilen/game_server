#pragma once

#include "proto/data_base.pb.h"
#include "proto/data_event.pb.h"
#include "proto/config_shop.pb.h"

using namespace std;
namespace pd = proto::data;
namespace pc = proto::config;

namespace nora {
        namespace scene {

                class role;

                pd::result mys_shop_free_refresh_goods(uint32_t pttid, pd::event_res *event_res, role& role);
                pd::result mys_shop_refresh_goods(uint32_t pttid, pd::event_res *event_res, role& role);
                pd::result mys_shop_check_buy(uint32_t pttid, uint32_t good_id, const role& role);
                pd::event_res mys_shop_buy(uint32_t pttid, uint32_t good_id, role& role);
                pd::result shop_check_buy(uint32_t shop, uint32_t good_id, const role& role);
                pd::event_res shop_buy(uint32_t shop, uint32_t good_id, role& role, pd::event_res *merge_res, bool send_bi);

        }
}
