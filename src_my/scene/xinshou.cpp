#include "xinshou.hpp"
#include "scene/player/role.hpp"
#include "scene/plot.hpp"
#include "scene/event.hpp"
#include "scene/condition.hpp"
#include "config/plot_ptts.hpp"
#include "config/xinshou_ptts.hpp"

namespace nora {
        namespace scene {

                pd::result xinshou_continue(uint32_t group_pttid, uint32_t pttid, role& role) {
                        if (!PTTS_HAS(xinshou_group, group_pttid)) {
                                return pd::XINSHOU_INVALID_GROUP;
                        }
                        if (!PTTS_HAS(xinshou, pttid)) {
                                return pd::XINSHOU_INVALID_XINSHOU;
                        }
                        const auto& group_ptt = PTTS_GET(xinshou_group, group_pttid);
                        auto found = false;
                        auto last = false;
                        for (auto i = 0; i < group_ptt.xinshou_size(); ++i) {
                                if (group_ptt.xinshou(i) == pttid) {
                                        found = true;
                                        if (i == group_ptt.xinshou_size() - 1) {
                                                last = true;
                                        }
                                        break;
                                }
                        }
                        if (!found) {
                                return pd::XINSHOU_PTTID_AND_GROUP_MISMATCH;
                        }
                        if (last || pttid == group_ptt.save_xinshou()) {
                                role.xinshou_add_passed_group(group_pttid);
                        }
                        return pd::OK;
                }

        }
}
