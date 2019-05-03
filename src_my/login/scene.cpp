#include "scene.hpp"
#include "config/utils.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"

namespace nora {
        namespace login {

                map<uint32_t, set<string>> scene::server_id2username_;

                string scene::base_name() {
                        return "scene";
                }

                scene::scene(const string& conn_name) : name_("scene-login-" + conn_name) {
                }

                void scene::static_init(const shared_ptr<service_thread>& st) {
                        register_proto_handler(ps::s2l_username_sync::descriptor(), &scene::process_username_sync);
                }

                void scene::process_username_sync(const ps::base *msg) {
                        const auto& sync = msg->GetExtension(ps::s2l_username_sync::sus);
                        set<string> usernames;
                        for (const auto& i : sync.username()) {
                                usernames.insert(split_string(i, '_')[0]);
                        }
                        server_id2username_[sync.server_id()] = usernames;
                }

                set<uint32_t> scene::fetch_server_ids(const string& username) {
                        set<uint32_t> server_ids;
                        for (const auto& i : server_id2username_) {
                                if (i.second.count(username) > 0) {
                                        server_ids.insert(i.first);
                                }
                        }
                        return server_ids;
                }

        }
}
