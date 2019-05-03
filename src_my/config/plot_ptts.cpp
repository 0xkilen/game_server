#include "utils.hpp"
#include "plot_ptts.hpp"

namespace pc = proto::config;

namespace nora {
        namespace config {

                plot_ptts& plot_ptts_instance() {
                        static plot_ptts inst;
                        return inst;
                }

                void plot_ptts_set_funcs() {
                        plot_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.actions()) {
                                        if (i.automatic()) {
                                                continue;
                                        }
                                        auto missing_subtitle = true;
                                        for (const auto& j : i.actions()) {
                                                switch (j.type()) {
                                                case pc::plot_action::SHOW_ACTOR_DESC:
                                                case pc::plot_action::DIALOG_BUBBLE:
                                                case pc::plot_action::ACTOR_SPINE_ANIMATION:
                                                case pc::plot_action::ACTOR_ANIMATION:
                                                case pc::plot_action::SUBTITLE:
                                                case pc::plot_action::END:
                                                case pc::plot_action::OPTIONS:
                                                case pc::plot_action::HIGHLIGHT:
                                                        missing_subtitle = false;
                                                        break;
                                                default:
                                                        break;
                                                }
                                                if (!missing_subtitle) {
                                                        break;
                                                }
                                        } if (missing_subtitle) {
                                                CONFIG_ELOG << ptt.id() << " actions " << " missing SUBTITLE";
                                        }
                                }
                        };
                        plot_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.actions()) {
                                        for (const auto& j : i.actions()) {
                                                if (j.type() == pc::plot_action::OPTIONS) {
                                                        auto plot_options_pttid = stoul(j.arg(0));
                                                        if (!PTTS_HAS(plot_options, plot_options_pttid)) {
                                                                CONFIG_ELOG << ptt.id() << " options " << plot_options_pttid << " not exist";
                                                                continue;
                                                        }
                                                        const auto& options_ptt = PTTS_GET(plot_options, plot_options_pttid);
                                                        for (const auto& k : options_ptt.options()) {
                                                                if (k.has_plot_pttid()) {
                                                                        if (!PTTS_HAS(plot, k.plot_pttid())) {
                                                                                CONFIG_ELOG << ptt.id() << " option plot " << k.plot_pttid() << " not exist";
                                                                        }
                                                                        if (k.has_battle()) {
                                                                                const auto& ob = k.battle();
                                                                                if (ob.has_win_plot() && !PTTS_HAS(plot, ob.win_plot())) {
                                                                                        CONFIG_ELOG << ptt.id() << " battle win plot " << ob.win_plot() << " not exist";
                                                                                }
                                                                                if (ob.has_lose_plot() && !PTTS_HAS(plot, ob.lose_plot())) {
                                                                                        CONFIG_ELOG << ptt.id() << " battle lose plot " << ob.lose_plot() << " not exist";
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }

                                if (ptt.has_next_plot()) {
                                        if (!PTTS_HAS(plot, ptt.next_plot())) {
                                                CONFIG_ELOG << ptt.id() << " next plot " << ptt.next_plot() << " not exist";
                                        }
                                }
                                if (ptt.has_female_plot()) {
                                        if (!PTTS_HAS(plot, ptt.female_plot())) {
                                                CONFIG_ELOG << ptt.id() << " female plot " << ptt.female_plot() << " not exist";
                                        }
                                }
                                if (ptt.has_male_plot()) {
                                        if (!PTTS_HAS(plot, ptt.male_plot())) {
                                                CONFIG_ELOG << ptt.id() << " male plot " << ptt.male_plot() << " not exist";
                                        }
                                }
                        };
                        plot_ptts_instance().enrich_func_ = [] (auto& ptts) {
                                set<uint32_t> prevs;
                                map<uint32_t, uint32_t> pttid2prev;
                                for (const auto& i : ptts) {
                                        if (!i.second.lock_option()) {
                                                continue;
                                        }
                                        for (const auto& j : i.second.actions()) {
                                                for (const auto& k : j.actions()) {
                                                        if (k.type() == pc::plot_action::OPTIONS) {
                                                                auto options_pttid = stoul(k.arg(0));
                                                                if (!PTTS_HAS(plot_options, options_pttid)) {
                                                                        continue;
                                                                }
                                                                const auto& options_ptt = PTTS_GET(plot_options, options_pttid);
                                                                for (const auto& l : options_ptt.options()) {
                                                                        if (l.has_plot_pttid()) {
                                                                                if (pttid2prev.count(l.plot_pttid()) > 0) {
                                                                                        CONFIG_ELOG << l.plot_pttid() << " has two prev plot " << pttid2prev.at(l.plot_pttid()) << " and " << i.first;
                                                                                }
                                                                                prevs.insert(i.first);
                                                                                pttid2prev[l.plot_pttid()] = i.first;
                                                                        }
                                                                        if (l.has_battle()) {
                                                                                const auto& ob = l.battle();
                                                                                if (ob.has_win_plot()) {
                                                                                        if (pttid2prev.count(ob.win_plot()) > 0) {
                                                                                                CONFIG_ELOG << ob.win_plot() << " has two prev plot " << pttid2prev.at(ob.win_plot()) << " and " << i.first;
                                                                                        }
                                                                                        prevs.insert(i.first);
                                                                                        pttid2prev[ob.win_plot()] = i.first;
                                                                                }
                                                                                if (ob.has_lose_plot()) {
                                                                                        if (pttid2prev.count(ob.lose_plot()) > 0) {
                                                                                                CONFIG_ELOG << ob.lose_plot() << " has two prev plot " << pttid2prev.at(ob.lose_plot()) << " and " << i.first;
                                                                                        }
                                                                                        prevs.insert(i.first);
                                                                                        pttid2prev[ob.lose_plot()] = i.first;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                        if (i.second.has_female_plot()) {
                                                if (pttid2prev.count(i.second.female_plot()) > 0) {
                                                        CONFIG_ELOG << i.second.female_plot() << " has two prev plot " << pttid2prev.at(i.second.female_plot()) << " and " << i.first;
                                                }
                                                prevs.insert(i.first);
                                                pttid2prev[i.second.female_plot()] = i.first;
                                        }
                                        if (i.second.has_male_plot()) {
                                                if (pttid2prev.count(i.second.male_plot()) > 0) {
                                                        CONFIG_ELOG << i.second.male_plot() << " has two prev plot " << pttid2prev.at(i.second.male_plot()) << " and " << i.first;
                                                }
                                                prevs.insert(i.first);
                                                pttid2prev[i.second.male_plot()] = i.first;
                                        }
                                        if (i.second.has_next_plot()) {
                                                if (pttid2prev.count(i.second.next_plot()) > 0) {
                                                        CONFIG_ELOG << i.second.next_plot() << " has two prev plot " << pttid2prev.at(i.second.next_plot()) << " and " << i.first;
                                                }
                                                prevs.insert(i.first);
                                                pttid2prev[i.second.next_plot()] = i.first;
                                        }
                                }

                                for (auto& i : ptts) {
                                        if (!i.second.lock_option()) {
                                                continue;
                                        }
                                        if (pttid2prev.count(i.first) > 0) {
                                                i.second.set__prev_plot(pttid2prev.at(i.first));
                                        }
                                        if (prevs.count(i.first) == 0) {
                                                i.second.set__the_end(true);
                                        }
                                }

                                for (const auto& i : pttid2prev) {
                                        list<uint32_t> pttids;
                                        auto pttid = i.first;
                                        auto prev_pttid = i.second;
                                        pttids.push_back(pttid);
                                        while (pttid2prev.count(prev_pttid) > 0) {
                                                pttids.push_back(prev_pttid);
                                                pttid = prev_pttid;
                                                prev_pttid = pttid2prev[prev_pttid];
                                        }

                                        for (auto i : pttids) {
                                                if (ptts.count(i) > 0) {
                                                        ptts.at(i).set__begin_plot(prev_pttid);
                                                }
                                        }
                                        ptts.at(prev_pttid).set__begin_plot(prev_pttid);
                                }
                        };
                }

                plot_options_ptts& plot_options_ptts_instance() {
                        static plot_options_ptts inst;
                        return inst;
                }

                void plot_options_ptts_set_funcs() {
                        plot_options_ptts_instance().check_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.options()) {
                                        if (!check_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check conditions failed";
                                        }
                                        if (!check_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " check events failed";
                                        }
                                        if (!check_conditions(i.unlock_conditions())) {
                                                CONFIG_ELOG << ptt.id() << " check unlock conditions failed";
                                        }
                                }
                        };
                        plot_options_ptts_instance().modify_func_ = [] (auto& ptt) {
                                for (auto& i : *ptt.mutable_options()) {
                                        modify_events_by_conditions(i.unlock_conditions(), *i.mutable__unlock_events());
                                }
                        };
                        plot_options_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.options()) {
                                        if (!verify_conditions(i.conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify conditions failed";
                                        }
                                        if (!verify_events(i.events())) {
                                                CONFIG_ELOG << ptt.id() << " verify events failed";
                                        }
                                        if (!verify_conditions(i.unlock_conditions())) {
                                                CONFIG_ELOG << ptt.id() << " verify unlock conditions failed";
                                        }
                                        if (!verify_events(i._unlock_events())) {
                                                CONFIG_ELOG << ptt.id() << " verify unlock events failed";
                                        }
                                }
                        };
                }

                plot_options_group_ptts& plot_options_group_ptts_instance() {
                        static plot_options_group_ptts inst;
                        return inst;
                }

                void plot_options_group_ptts_set_funcs() {
                        plot_options_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.options_array()) {
                                        if (!PTTS_HAS(plot_options, i)) {
                                                CONFIG_ELOG << ptt.id() << " plot options not exist " << i;
                                                continue;
                                        }
                                        auto& plot_options_ptt = PTTS_GET(plot_options, i);
                                        plot_options_ptt.set__group(ptt.id());
                                }
                        };
                }

        }
}
