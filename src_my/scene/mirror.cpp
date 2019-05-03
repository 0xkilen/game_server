#include "mirror.hpp"
#include "scene/player/player_mgr.hpp"
#include "proto/ss_mirror.pb.h"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                string mirror::base_name() {
                        return "mirror";
                }

                mirror::mirror(const string& conn_name) : name_("mirror-" + conn_name) {
                }

                void mirror::static_init() {
                        mirror::register_proto_handler(ps::sm_role_event::descriptor(), &mirror::process_role_event);
                        mirror::register_proto_handler(ps::sm_server_roles_event::descriptor(), &mirror::process_server_roles_event);
                }

                void mirror::process_role_event(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::sm_role_event::sre);
                        player_mgr_class::instance().role_mirror_event(req.event());
                }

                void mirror::process_server_roles_event(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::sm_server_roles_event::ssre);
                        player_mgr_class::instance().server_roles_mirror_event(req.event());
                }

        }
}
