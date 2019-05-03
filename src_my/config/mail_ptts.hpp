#pragma once

#include "utils/ptts.hpp"
#include "proto/config_mail.pb.h"

using namespace std;
namespace pc = proto::config;

namespace nora {
        namespace config {

                using mail_ptts = ptts<pc::mail>;
                mail_ptts& mail_ptts_instance();
                void mail_ptts_set_funcs();
        }
}
