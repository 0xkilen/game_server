#pragma once

#include "proto/data_battle.pb.h"
#include "db/connector.hpp"
#include "utils/assert.hpp"
#include <list>
#include <map>
#include <mutex>
#include <memory>

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                class conflict_battle_records {
                public:
                        static conflict_battle_records& instance() {
                                static conflict_battle_records inst;
                                return inst;
                        }

                        conflict_battle_records();
                        void record(uint64_t gid, const function<void(pd::result, uint64_t, const pd::conflict_battle_record&)>&, const shared_ptr<service_thread>& st) const;
                        void add_record(const pd::conflict_battle_record& record);
                private:
                        mutable mutex lock_;
                        shared_ptr<db::connector> gamedb_;
                };

        }
}
