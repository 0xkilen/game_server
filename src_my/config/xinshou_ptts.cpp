#include "utils.hpp"
#include "xinshou_ptts.hpp"
#include "plot_ptts.hpp"
#include "actor_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                xinshou_ptts& xinshou_ptts_instance() {
                        static xinshou_ptts inst;
                        return inst;
                }

                void xinshou_ptts_set_funcs() {
                        xinshou_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.need_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check need conditions failed";
                                }
                        };
                        xinshou_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                auto guide_count = static_cast<int>(PTTS_GET_ALL(xinshou_guide).size());
                                if (ptt.plot_size() < guide_count) {
                                        CONFIG_ELOG << ptt.id() << " need " << guide_count << " plot, got " << ptt.plot_size();
                                }
                                for (auto i : ptt.plot()) {
                                        if (!PTTS_HAS(plot, i)) {
                                                CONFIG_ELOG << ptt.id() << " plot not exist " << i;
                                        }
                                        const auto& plot_ptt = PTTS_GET(plot, i);
                                        if (plot_ptt.has_next_plot()) {
                                                CONFIG_ELOG << ptt.id() << " plot " << plot_ptt.id() << " has next plot";
                                        }
                                }
                                if (!verify_conditions(ptt.need_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify need conditions failed";
                                }
                        };
                }

                xinshou_group_ptts& xinshou_group_ptts_instance() {
                        static xinshou_group_ptts inst;
                        return inst;
                }

                void xinshou_group_ptts_set_funcs() {
                        xinshou_group_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check conditions failed";
                                }
                                if (!check_conditions(ptt.finish_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " check finish conditions failed";
                                }
                                if (ptt.has_save_xinshou()) {
                                        auto found = false;
                                        for (auto i : ptt.xinshou()) {
                                                if (i == ptt.save_xinshou()) {
                                                        found = true;
                                                        break;
                                                }
                                        }
                                        if (!found) {
                                                CONFIG_ELOG << ptt.id() << " save xinshou " << ptt.save_xinshou() << " not in xinshou";
                                        }
                                }
                        };
                        xinshou_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.xinshou()) {
                                        if (!PTTS_HAS(xinshou, i)) {
                                                CONFIG_ELOG << ptt.id() << " xinshou not exist " << i;
                                        }
                                }
                                if (!verify_conditions(ptt.conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                }
                                if (!verify_conditions(ptt.finish_conditions())) {
                                        CONFIG_ELOG << ptt.id() << " verify finish conditions failed";
                                }
                                if (ptt.has_resume_group()) {
                                        if (!PTTS_HAS(xinshou_group, ptt.resume_group())) {
                                                CONFIG_ELOG << ptt.id() << " resume group not exist";
                                        } else {
                                                auto& group_ptt = PTTS_GET(xinshou_group, ptt.resume_group());
                                                group_ptt.set__resume(true);
                                                for (auto i : group_ptt.xinshou()) {
                                                        if (!PTTS_HAS(xinshou, i)) {
                                                                continue;
                                                        }
                                                        auto& resume_ptt = PTTS_GET(xinshou_group, ptt.resume_group());
                                                        resume_ptt.set__resume(true);
                                                }
                                        }
                                }
                        };
                }

                xinshou_guide_ptts& xinshou_guide_ptts_instance() {
                        static xinshou_guide_ptts inst;
                        return inst;
                }

                void xinshou_guide_ptts_set_funcs() {
                        xinshou_guide_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!PTTS_HAS(actor, ptt.actor())) {
                                        CONFIG_ELOG << ptt.id() << " guide actor not exist " << ptt.actor();
                                }
                        };
                }

                module_introduce_ptts& module_introduce_ptts_instance() {
                        static module_introduce_ptts inst;
                        return inst;
                }

                void module_introduce_ptts_set_funcs() {
                       
                }
        }
}
