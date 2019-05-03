#include "utils.hpp"
#include "proto/config_options.pb.h"
#include "options_ptts.hpp"

namespace nora {
        namespace config {

                options_ptts& options_ptts_instance() {
                        static options_ptts inst;
                        return inst;
                }

                void options_ptts_set_funcs() {
                        options_ptts_instance().check_func_ = [] (const auto& ptt) {
                                if (ptt.scene_info().player_thread_count() > 128 || ptt.scene_info().player_thread_count() <= 0) {
                                        CONFIG_ELOG << "argument for player_thread_count out of range: " << ptt.scene_info().player_thread_count();
                                        exit(1);
                                }
                                if (ptt.scene_info().server_id() < 0 || ptt.scene_info().server_id() > 2047) {
                                        CONFIG_ELOG << "argument for server_id out of range: " << ptt.scene_info().server_id();
                                        exit(1);
                                }
                                if (ptt.scened_client_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.scened_client_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for scened_client_ipport port out of range: " << ptt.scened_client_ipport().port();
                                        exit(1);
                                }
                                if (ptt.chat_info().scened_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.chat_info().scened_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for chat_info scened_ipport port out of range: " << ptt.chat_info().scened_ipport().port();
                                        exit(1);
                                }
                                if (ptt.login_info().gmd_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.login_info().gmd_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for login_info gmd_ipport port out of range: " << ptt.login_info().gmd_ipport().port();
                                        exit(1);
                                }
                                if (ptt.scened_dbcached_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.scened_dbcached_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for scened_dbcached_ipport port out of range: " << ptt.scened_dbcached_ipport().port();
                                        exit(1);
                                }
                                if (ptt.mirror_info().scened_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.mirror_info().scened_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for mirror_info scened_ipport port out of range: " << ptt.mirror_info().scened_ipport().port();
                                        exit(1);
                                }
                                if (ptt.gm_info().scened_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.gm_info().scened_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for gm_info scened_ipport port out of range: " << ptt.gm_info().scened_ipport().port();
                                        exit(1);
                                }
                                for (const auto& i : ptt.route_info().ipport()) {
                                        if (i.port() > numeric_limits<unsigned short>::max() ||
                                            i.port() < numeric_limits<unsigned short>::min()) {
                                                CONFIG_ELOG << "argument for route_info_ipport port out of range: " << i.port();
                                                exit(1);
                                        }
                                }
                                if (ptt.gmd_yunying_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.gmd_yunying_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for gmd_yunying_ipport port out of range: " << ptt.gmd_yunying_ipport().port();
                                        exit(1);
                                }
                                if (ptt.scene_info().max_gladiators() > ptt.dbcache_info().max_gladiators()) {
                                        CONFIG_ELOG << "scene gladiators " << ptt.scene_info().max_gladiators() << " greater than dbcache gladiators " << ptt.dbcache_info().max_gladiators();
                                        exit(1);
                                }
                                if (ptt.login_info().scened_ipport().port() > numeric_limits<unsigned short>::max() ||
                                    ptt.login_info().scened_ipport().port() < numeric_limits<unsigned short>::min()) {
                                        CONFIG_ELOG << "argument for login_info scened_ipport port out of range: " << ptt.login_info().scened_ipport().port();
                                        exit(1);
                                }
                        };
                }
        }
}
