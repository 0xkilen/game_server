#pragma once

#include "proto/data_mail.pb.h"

using namespace std;
namespace pd = proto::data;

namespace nora {
        namespace scene {

                pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data, const pd::event_array& extra_events);
                pd::mail mail_new_mail(uint64_t role, const string& title, const string& content, const pd::dynamic_data& dynamic_data, const pd::event_array& events);

        }
}
