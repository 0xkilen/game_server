#include "login.hpp"
#include "scene/player/player_mgr.hpp"

namespace ps = proto::ss;

namespace nora {
        namespace scene {

                using player_mgr_class = player_mgr<net::PLAYER_MGR_CONN>;

                string login::base_name() {
                        return "login";
                }

                login::login(const string& conn_name) : name_("login-" + conn_name) {
                }

                void login::static_init() {
                }

                void login::on_register_done() {
                        player_mgr_class::instance().send_username_to_logind();
                }

        }
}
