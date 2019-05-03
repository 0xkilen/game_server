#include "item.hpp"

namespace nora {
        namespace scene {

                item::item() : instance_countee(ICT_ITEM), count_(0) {
                }

                item::item(uint32_t pttid, int count) :
                        instance_countee(ICT_ITEM), pttid_(pttid), count_(count) {
                }

                void item::serialize(proto::data::item *item) const {
                        item->set_pttid(pttid_);
                        item->set_count(count_);
                        item->set_confirmed(confirmed_);
                }

                void item::parse(const proto::data::item *item) {
                        pttid_ = item->pttid();
                        count_ = item->count();
                        confirmed_ = item->confirmed();
                }

        }
}
