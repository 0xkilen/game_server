#pragma once

#include "utils/ptts.hpp"
#include "proto/config_serverlist.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using serverlist_ptts = ptts<pc::serverlist>;
                serverlist_ptts& serverlist_ptts_instance();
                void serverlist_ptts_set_funcs();

                using logind_server_ptts = ptts<pc::logind_server>;
                logind_server_ptts& logind_server_ptts_instance();
                void logind_server_ptts_set_funcs();

                using specific_server_list_ptts = ptts<pc::specific_server_list>;
                specific_server_list_ptts& specific_server_list_ptts_instance();
                void specific_server_list_ptts_set_funcs();
        }
}
