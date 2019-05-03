#include "feige_ptts.hpp"
#include "plot_ptts.hpp"
#include "utils.hpp"
#include "proto/data_event.pb.h"
#include "proto/data_feige.pb.h"
#include "utils/assert.hpp"

namespace pd = proto::data;

namespace nora {
        namespace config {

                feige_message_ptts& feige_message_ptts_instance() {
                        static feige_message_ptts inst;
                        return inst;
                }

                void feige_message_ptts_set_funcs() {
                        feige_message_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (const auto& i : ptt.responses()) {
                                        for (auto j = 0; j < i.next_messages_size(); ++j) {
                                                auto nm = i.next_messages(j);
                                                if (!PTTS_HAS(feige_message, nm)) {
                                                        CONFIG_ELOG << ptt.id() << " next message not exist " << nm;
                                                }
                                                if (j < i.next_messages_size() - 1) {
                                                        const auto& nm_ptt = PTTS_GET(feige_message, nm);
                                                        if (nm_ptt.responses_size() > 0) {
                                                                CONFIG_ELOG << ptt.id() << " responses between messages" << nm;
                                                        }
                                                }
                                        }

                                }
                        };
                }

                feige_ptts& feige_ptts_instance() {
                        static feige_ptts inst;
                        return inst;
                }

                void feige_ptts_set_funcs() {
                        feige_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.start_responses_size() == 0 && ptt.start_messages_size() == 0) {
                                        CONFIG_ELOG << ptt.id() << " no start";
                                }
                                if (ptt.start_responses_size() != 0 && ptt.start_messages_size() != 0) {
                                        CONFIG_ELOG << ptt.id() << " start_responses and start_messages can not all exist";
                                }
                                if (!check_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " check end events failed";
                                }
                        };
                        feige_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.start_messages_size() > 0) {
                                        for (auto i = 0; i < ptt.start_messages_size() - 2; ++i) {
                                                auto sm = ptt.start_messages(i);
                                                if (!PTTS_HAS(feige_message, sm)) {
                                                        continue;
                                                }
                                                const auto& sm_ptt = PTTS_GET(feige_message, sm);
                                                if (sm_ptt.responses_size() > 0) {
                                                        CONFIG_ELOG << ptt.id() << " start middle message has response";
                                                }
                                        }
                                }
                                for (auto i : ptt.start_messages()) {
                                        if (!PTTS_HAS(feige_message, i)) {
                                                CONFIG_ELOG << ptt.id() << " start message not exist " << i;
                                        }
                                }
                                if (ptt.has_end_plot() && !PTTS_HAS(plot, ptt.end_plot())) {
                                        CONFIG_ELOG << ptt.id() << " end plot not exist " << ptt.end_plot();
                                }
                                if (!verify_events(ptt.finish_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify end events failed";
                                }
                        };
                }

                feige_chuanwen_ptts& feige_chuanwen_ptts_instance() {
                        static feige_chuanwen_ptts inst;
                        return inst;
                }

                void feige_chuanwen_ptts_set_funcs() {
                        feige_chuanwen_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (!check_events(ptt.read_events())) {
                                        CONFIG_ELOG << ptt.id() << " check read events failed";
                                }
                        };
                        feige_chuanwen_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (!verify_events(ptt.read_events())) {
                                        CONFIG_ELOG << ptt.id() << " verify read events failed";
                                }
                        };
                }

                feige_chuanwen_group_ptts& feige_chuanwen_group_ptts_instance() {
                        static feige_chuanwen_group_ptts inst;
                        return inst;
                }

                void feige_chuanwen_group_ptts_set_funcs() {
                        feige_chuanwen_group_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                for (auto i : ptt.chuanwen()) {
                                        if (!PTTS_HAS(feige_chuanwen, i)) {
                                                CONFIG_ELOG << ptt.id() << " chuanwen not exist " << i;
                                        }
                                }
                        };
                }

                feige_cg_ptts& feige_cg_ptts_instance() {
                        static feige_cg_ptts inst;
                        return inst;
                }

                void feige_cg_ptts_set_funcs() {
                }

                major_city_bubble_ptts& major_city_bubble_ptts_instance() {
                        static major_city_bubble_ptts inst;
                        return inst;
                }
                void major_city_bubble_ptts_set_funcs() {
                        major_city_bubble_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                if (ptt.has_plot()) {
                                        if (!PTTS_HAS(plot, ptt.plot())) {
                                                CONFIG_ELOG << ptt.id() << " city bubble plot not exist " << ptt.plot();
                                        }
                                }
                        };
                }

        }
}
