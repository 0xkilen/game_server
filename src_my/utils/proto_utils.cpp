#include "utils/proto_utils.hpp"
#include "utils/assert.hpp"

namespace nora {

        const pc::mansion_craft_drop& pick_craft_drop(const pc::mansion_craft_drop_array& drops, pd::actor::craft_type craft) {
                for (const auto& i : drops.craft_drops()) {
                        if (i.craft() == craft) {
                                return i;
                        }
                }
                ASSERT(false);
                return drops.craft_drops(0);
        }

        void merge_mansion_furniture(pd::mansion_all_furniture& a, const pd::mansion_all_furniture& b) {
                for (const auto& i : b.furniture()) {
                        *a.add_furniture() = i;
                }
        }

}
