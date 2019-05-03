#include "scene.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "config/options_ptts.hpp"
#include "proto/data_yunying.pb.h"
#include "utils/assert.hpp"
#include "utils/time_utils.hpp"

namespace nora {
        namespace log_serverd {

                scene::scene(const string& name) : name_(name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::s2l_bi_info_sync::descriptor(), &scene::process_bi_info_sync);
                }

                void scene::process_bi_info_sync(const ps::base *msg) {
                        const auto& sync = msg->GetExtension(ps::s2l_bi_info_sync::sbis);
                        auto ptt = PTTS_GET_COPY(options, 1);
                        auto app_id = ptt.yunying().app_id();
                        auto app_secret = ptt.yunying().app_secret();
                        const auto& type = sync.addr();
                        auto time = to_string(system_clock::to_time_t(system_clock::now()));
                        string sync_str = pb2json(sync);
                        BILOG << type + "|" + app_id + "|" + app_secret + "|" + time + " \t " + sync_str;
                }

                ostream& operator<<(ostream& os, const scene& s) {
                        return os << s.name_;
                }

        }
}
