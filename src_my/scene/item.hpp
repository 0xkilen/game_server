#pragma once

#include "utils/assert.hpp"
#include "utils/instance_counter.hpp"
#include "proto/data_item.pb.h"

namespace pd = proto::data;

namespace nora {
        namespace scene {

                class item : private instance_countee {
                public:
                        item();
                        item(uint32_t pttid, int count);
                        void serialize(proto::data::item *item) const;
                        void parse(const proto::data::item *item);
                        uint32_t pttid() const;
                        int count() const;
                        int change_count(int value);
                        bool confirmed() const;
                        void set_confirmed(bool confirmed);
                private:
                        uint32_t pttid_ = 0;
                        int count_ = 0;
                        bool confirmed_ = false;
                };

                inline uint32_t item::pttid() const {
                        return pttid_;
                }

                inline int item::count() const {
                        return count_;
                }

                inline int item::change_count(int value) {
                        count_ = max(0, count_ + value);
                        return count_;
                }

                inline bool item::confirmed() const {
                        return confirmed_;
                }

                inline void item::set_confirmed(bool confirmed) {
                        confirmed_ = confirmed;
                }

        }
}
