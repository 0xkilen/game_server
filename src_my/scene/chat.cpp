#include "chat.hpp"
#include "scene/player/player_mgr.hpp"
#include "proto/ss_chat.pb.h"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                string chat::base_name() {
                        return "chat";
                }

                chat::chat(const string& conn_name) : name_("chat-" + conn_name) {
                }

                void chat::static_init() {
                        chat::register_proto_handler(ps::sch_cross_world_chat::descriptor(), &chat::process_cross_world_chat);
                        chat::register_proto_handler(ps::ch2s_system_chat_all_server_rsp::descriptor(), &chat::process_system_chat_all_server);
                }

                void chat::process_cross_world_chat(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::sch_cross_world_chat::scwc);
                        player_mgr_class::instance().cross_world_chat(req.from(), req.time(), req.from_server(), req.content(), req.voice());
                }

                void chat::process_system_chat_all_server(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::ch2s_system_chat_all_server_rsp::cscasr);
                        player_mgr_class::instance().send_system_chat(req.system_chat(), req.dynamic_data());
                }

        }
}
