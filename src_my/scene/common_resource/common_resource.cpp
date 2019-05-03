#include "common_resource.hpp"
#include "scene/player/player_mgr.hpp"
#include "proto/ss_common_resource.pb.h"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;
                using common_resource_mgr = server_process_mgr<common_resource>;

                string common_resource::base_name() {
                        return "common_resource";
                }

                common_resource::common_resource(const string& conn_name) : name_("common_resource-" + conn_name) {
                }

                void common_resource::static_init() {
                        common_resource::register_proto_handler(ps::cr2s_lock_fanli_rsp::descriptor(), &common_resource::process_lock_fanli_rsp);
                        common_resource::register_proto_handler(ps::cr2s_lock_come_back_rsp::descriptor(), &common_resource::process_lock_come_back_rsp);
                }

                void common_resource::process_lock_fanli_rsp(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::cr2s_lock_fanli_rsp::clfr);
                        if (req.result() == pd::OK) {
                                player_mgr_class::instance().lock_fanli_done(req.username(), req.fanlis());
                                proto::ss::base sync_msg;
                                auto *sync = sync_msg.MutableExtension(ps::s2cr_remove_fanli::srf);
                                sync->set_username(req.username());
                                common_resource_mgr::instance().send_msg(id(), sync_msg);
                        }
                }

                void common_resource::process_lock_come_back_rsp(const ps::base *msg) {
                        const auto& req = msg->GetExtension(ps::cr2s_lock_come_back_rsp::clcbr);
                        if (req.result() == pd::OK) {
                                player_mgr_class::instance().lock_come_back_done(req.username());
                                proto::ss::base sync_msg;
                                auto *sync = sync_msg.MutableExtension(ps::s2cr_remove_come_back::srcb);
                                sync->set_username(req.username());
                                common_resource_mgr::instance().send_msg(id(), sync_msg);
                        }
                }

        }
}
