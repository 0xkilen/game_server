#pragma once

#include "merger_base.hpp"
#include "proto/data_league.pb.h"
#include "utils/service_thread.hpp"
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {

        class league_war_merger : public merger_base {
        public:
                static league_war_merger& instance() {
                        static league_war_merger inst;
                        return inst;
                }
                void start();
        private:
                void load();
                void merge();

                unique_ptr<pd::league_war> from_league_war_;
                unique_ptr<pd::league_war> to_league_war_;
        };

}