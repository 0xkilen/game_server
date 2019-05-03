#include "utils.hpp"
#include "serverlist_ptts.hpp"
#include "channel_specify_ptts.hpp"

namespace nora {
        namespace config {
                serverlist_ptts& serverlist_ptts_instance() {
                        static serverlist_ptts inst;
                        return inst;
                }

                void serverlist_ptts_set_funcs() {
                        serverlist_ptts_instance().verify_func_ = [] (const auto& ptt) {
                                bool in_channel_specify = false;
                                const auto& channel_specify_ptt = PTTS_GET_ALL(channel_specify);
                                for (auto i : ptt.channel_ids()) {
                                        for (const auto& j : channel_specify_ptt) {
                                                for (auto k : j.second.channel_id()) {
                                                        if (i == k) {
                                                                in_channel_specify = true;
                                                                break;
                                                        }
                                                }
                                                if (in_channel_specify) {
                                                        in_channel_specify = false;
                                                        break;
                                                } else {
                                                        CONFIG_ELOG << i << " not in channel_specify";
                                                }
                                        }
                                }
                        };
                }

                logind_server_ptts& logind_server_ptts_instance() {
                        static logind_server_ptts inst;
                        return inst;
                }

                void logind_server_ptts_set_funcs() {
                }

                specific_server_list_ptts& specific_server_list_ptts_instance() {
                        static specific_server_list_ptts inst;
                        return inst;
                }

                void specific_server_list_ptts_set_funcs() {
                }
        }
}
