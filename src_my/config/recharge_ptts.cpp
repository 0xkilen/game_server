#include "utils.hpp"
#include "recharge_ptts.hpp"
#include "mail_ptts.hpp"

using namespace std;

namespace pc = proto::config;

namespace nora {
        namespace config {

                recharge_ptts& recharge_ptts_instance() {
                        static recharge_ptts inst;
                        return inst;
                }

                void recharge_ptts_set_funcs() {
                        recharge_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " check events failed";
                                }
                                if (ptt.days() > 0) {
                                        if (!check_events(ptt.day_events())) {
                                                CONFIG_ELOG << ptt.id() << " check days events failed";
                                        }
                                }
                        };
                        recharge_ptts_instance().modify_func_ = [] (auto& ptt) {
                                if (ptt.days() > 0) {
                                        if (ptt.id() == 1) {
                                                ptt.set__type(pc::RT_GOLD_MONTH_CARD);
                                        }
                                        if (ptt.id() == 2) {
                                                ptt.set__type(pc::RT_GENERAL_MONTH_CARD);
                                        }
                                }
                        };
                        recharge_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.events())) {
                                        CONFIG_ELOG << ptt.id() << " verify events failed";
                                }
                                if (ptt.days() > 0) {
                                        if (!verify_events(ptt.day_events())) {
                                                CONFIG_ELOG << ptt.id() << " verify days events failed";
                                        }
                                }
                                if (ptt.has_day_events_mail()) {
                                        if (!PTTS_HAS(mail, ptt.day_events_mail())) {
                                                CONFIG_ELOG << "day events mail not exit " << ptt.day_events_mail();
                                        }
                                }
                        };
                }

                vip_ptts& vip_ptts_instance() {
                        static vip_ptts inst;
                        return inst;
                }

                void vip_ptts_set_funcs() {
                        vip_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.has_box()) {
                                        if (!check_conditions(ptt.box().conditions())) {
                                                CONFIG_ELOG << ptt.level() << " check box conditions failed";
                                        }
                                        if (!check_events(ptt.box().events())) {
                                                CONFIG_ELOG << ptt.level() << " check box events failed";
                                        }
                                }
                        };
                        vip_ptts_instance().modify_func_ = [] (auto& ptt) {
                                auto *conditions = ptt.mutable_box()->mutable_conditions();

                                auto *c = conditions->add_conditions();
                                c->set_type(pd::condition::VIP_LEVEL_GE);
                                c->add_arg(to_string(ptt.level()));

                                if (ptt.has_buy_box()) {
                                        modify_events_by_conditions(ptt.buy_box().conditions(), *ptt.mutable_buy_box()->mutable_events());
                                }
                        };
                        vip_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_box()) {
                                        if (!verify_conditions(ptt.box().conditions())) {
                                                CONFIG_ELOG << ptt.level() << " verify box conditions failed";
                                        }
                                        if (!verify_events(ptt.box().events())) {
                                                CONFIG_ELOG << ptt.level() << " verify box events failed";
                                        }
                                }
                        };
                }

        }
}
