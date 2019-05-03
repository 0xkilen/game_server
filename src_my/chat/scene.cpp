#include "scene.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "proto/ss_chat.pb.h"
#include "utils/client_process_mgr.hpp"

namespace nora {
        namespace chat {

                using scene_mgr = client_process_mgr<scene>;

                string scene::base_name() {
                        return "scene";
                }

                scene::scene(const string& conn_name) : name_("chat-scene-" + conn_name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::sch_cross_world_chat::descriptor(), &scene::process_cross_world_chat);
                        register_proto_handler(ps::s2ch_system_chat_all_server_req::descriptor(), &scene::process_system_chat_all_server);
                }

                void scene::process_cross_world_chat(const ps::base *msg) {
                        scene_mgr::instance().broadcast_msg(*msg);
                }

                void scene::process_system_chat_all_server(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::s2ch_system_chat_all_server_req::sscasr);
                        ps::base rsp_msg;
                        auto *rsp = rsp_msg.MutableExtension(ps::ch2s_system_chat_all_server_rsp::cscasr);
                        rsp->set_system_chat(req.system_chat());
                        *rsp->mutable_dynamic_data() = req.dynamic_data();
                        scene_mgr::instance().broadcast_msg(rsp_msg);
                }

        }
}
