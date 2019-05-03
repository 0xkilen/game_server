#include "mail_logic.hpp"
#include "config/utils.hpp"
#include "config/mail_ptts.hpp"
#include "scene/event.hpp"
#include "utils/gid.hpp"

namespace nora {
        namespace scene {

                pd::mail mail_new_mail(uint64_t role, uint32_t pttid, const pd::dynamic_data& dynamic_data, const pd::event_array& extra_events) {
                        const auto& ptt = PTTS_GET(mail, pttid);
                        auto events = ptt.events();
                        event_merge(events, extra_events);

                        pd::mail m;
                        m.set_gid(gid::instance().new_gid(gid_type::MAIL));
                        if (dynamic_data.has_time()) {
                                m.set_time(dynamic_data.time());
                        } else {
                                m.set_time(system_clock::to_time_t(system_clock::now()));
                        }
                        m.set_pttid(pttid);
                        *m.mutable_dynamic_data() = dynamic_data;
                        *m.mutable_events() = events;
                        return m;
                }

                pd::mail mail_new_mail(uint64_t role, const string& title, const string& content, const pd::dynamic_data& dynamic_data, const pd::event_array& events) {
                        pd::mail m;
                        m.set_gid(gid::instance().new_gid(gid_type::MAIL));
                        if (dynamic_data.has_time()) {
                                m.set_time(dynamic_data.has_time());
                        } else {
                                m.set_time(system_clock::to_time_t(system_clock::now()));
                        }
                        m.set_title(title);
                        m.set_content(content);
                        *m.mutable_dynamic_data() = dynamic_data;
                        *m.mutable_events() = events;
                        return m;
                }

        }
}
