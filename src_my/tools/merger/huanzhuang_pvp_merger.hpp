#pragma once

#include "merger_base.hpp"
#include "proto/data_huanzhuang_pvp.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class huanzhuang_pvp_merger : public merger_base {
        public:
                static huanzhuang_pvp_merger& instance() {
                        static huanzhuang_pvp_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void merge();

                unique_ptr<pd::huanzhuang_pvp> from_huanzhuang_pvp_;
                unique_ptr<pd::huanzhuang_pvp> to_huanzhuang_pvp_;
        };

}
