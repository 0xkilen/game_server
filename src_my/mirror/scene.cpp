#include "scene.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "config/mirror_ptts.hpp"
#include "proto/ss_mirror.pb.h"
#include "utils/client_process_mgr.hpp"

namespace nora {
        namespace mirror {

                using scene_mgr = client_process_mgr<scene>;

                string scene::base_name() {
                        return "scene";
                }

                scene::scene(const string& conn_name) : name_("mirror-scene-" + conn_name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::sm_role_event::descriptor(), &scene::send_to_mirror);
                        // @cage, 功能ok, 但是代码不明确, 可以把process_role_event改名为转发给目标scene这样的名字
                        register_proto_handler(ps::sm_server_roles_event::descriptor(), &scene::send_to_mirror);
                }

                void scene::send_to_mirror(const ps::base *msg) {
                        if (!PTTS_HAS(mirror, id())) {
                                return;
                        }

                        const auto& ptt = PTTS_GET(mirror, id());
                        for (auto i : ptt.to_servers()) {
                                scene_mgr::instance().send_msg(i, *msg);
                        }
                }

        }
}
