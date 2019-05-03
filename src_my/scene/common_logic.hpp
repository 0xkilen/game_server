#pragma once

#include "proto/data_actor.pb.h"
#include <map>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                uint32_t calc_zhanli_by_attrs(const map<pd::actor::attr_type, int64_t>& attrs);
                const set<string>& forbid_names();

        }
}
