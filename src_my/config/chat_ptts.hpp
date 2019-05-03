#pragma once

#include "utils/ptts.hpp"
#include "proto/config_chat.pb.h"

using namespace std;

namespace nora {
        namespace config {

                using chat_ptts = ptts<proto::config::chat>;
                chat_ptts& chat_ptts_instance();
                void chat_ptts_set_funcs();

                using system_chat_ptts = ptts<proto::config::system_chat>;
                system_chat_ptts& system_chat_ptts_instance();
                void system_chat_ptts_set_funcs();
        }
}
