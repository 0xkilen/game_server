#pragma once

#include "net/client.hpp"
#include "net/conn.hpp"
#include "db/connector.hpp"
#include "proto/processor.hpp"
#include "proto/ss_gm.pb.h"
#include "proto/cs_arena.pb.h"
#include "proto/data_mail.pb.h"
#include "proto/data_rank.pb.h"
#include "proto/data_yunying.pb.h"
#include "config/options_ptts.hpp"
#include "config/spine_ptts.hpp"
#include "config/equipment_ptts.hpp"
#include "config/serverlist_ptts.hpp"
#include "utils/string_utils.hpp"
#include "utils/yunying.hpp"
#include "utils/server_process_base.hpp"
#include <string>
#include <memory>

using namespace std;
namespace ps = proto::ss;
namespace pd = proto::data;
namespace pcs = proto::cs;

namespace nora {
        class service_thread;
        namespace scene {

                class gm : public proto::processor<gm, ps::base>,
                           public server_process_base,
                           public enable_shared_from_this<gm> {
                public:
                        gm(const string& name);
                        static void static_init();
                        static string base_name();
                        void start();
                        friend ostream& operator<<(ostream& os, const gm& g) {
                                return os << g.name_;
                        }

                private:
                        void process_gag_role_by_gid_req(const ps::base *msg);
                        void process_ban_role_by_gid_req(const ps::base *msg);
                        void process_remove_punish_by_gid_req(const ps::base *msg);
                        void process_kick_role_by_gid_req(const ps::base *msg);
                        void process_fetch_world_chat_req(const ps::base *msg);
                        void process_find_role_req(const ps::base *msg);
                        void process_internal_recharge_req(const ps::base *msg);
                        void process_fetch_rank_list_req(const ps::base *msg);
                        void process_send_mail_req(const ps::base *msg);
                        void process_manage_announcement_req(const ps::base *msg);
                        void process_images_announcement(const ps::base *msg);
                        void process_reissue_recharge_req(const ps::base *msg);
                        void process_recharge_req(const ps::base *msg);
                        void process_fetch_log_req(const ps::base *msg);
                        void process_remove_from_white_list_req(const ps::base *msg);
                        void process_fetch_white_list_req(const ps::base *msg);
                        void process_fetch_punished_req(const ps::base *msg);
                        void process_fetch_mail_log_req(const ps::base *msg);
                        void process_fetch_login_record_req(const ps::base *msg);
                        void process_fetch_recharge_record_req(const ps::base *msg);
                        void process_fetch_currency_record_req(const ps::base *msg);
                        void process_fetch_role_list_req(const ps::base *msg);
                        void process_add_stuff_req(const ps::base *msg);
                        void process_dec_stuff_req(const ps::base *msg);
                        void process_operates_activity_req(const ps::base *msg);
                        void set_punished(const pd::punish_role& pr, pd::yunying_fetch_punished_info_array& yspia, int server_id);
                        void set_recharge_record(pd::yunying_fetch_recharge_record *recharge_record, const pd::log_recharge& log, uint32_t time, const string& server_name);
                        void set_currency_record(pd::yunying_fetch_currency_record *currency_record, const pd::log_currency_change& log, uint32_t time);
                        void set_item_change(const pd::log& log, pd::yunying_item_change_array& item_changes);

                        const string name_;
                        uint32_t id_ = 0;
                };

                inline string gm::base_name() {
                        return "gm";
                }
        }
}
