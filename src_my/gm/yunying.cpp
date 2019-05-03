#include "yunying.hpp"
#include "log.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "utils/json2pb.hpp"
#include "utils/crypto.hpp"
#include "utils/yunying.hpp"
#include "config/options_ptts.hpp"
#include "config/activity_ptts.hpp"
#include "utils/assert.hpp"
#include "utils/game_def.hpp"
#include "utils/time_utils.hpp"
#include "proto/data_admin.pb.h"

#define APP_KEY "d8fadf5187162983176bfa48d5c06a7e"
#define MAX_ITME_COUNT 100000000

namespace nora {
        namespace gm {

                yunying::yunying(const shared_ptr<service_thread>& st, const shared_ptr<net::server<net::HTTP_CONN>>& nsv, const shared_ptr<net::HTTP_CONN>& c, uint32_t sid) :
                        name_("gm-yunying" + c->name()), st_(st), nsv_(nsv), conn_(c), sid_(sid) {
                        register_proto_handler(py::request::descriptor(), &yunying::process_request);
                }

                void yunying::process_msg(const shared_ptr<net::recvstream>& msg) {
                        ASSERT(st_->check_in_thread());
                        py::base ys_msg;
                        ASSERT(ys_msg.ParseFromIstream(msg.get()));
                        GM_TLOG << *this << " process msg\n" << ys_msg.DebugString();
                        process(this, &ys_msg);
                }

                bool yunying::check_params(const map<string, string>& params, const set<string>& params_keys) {
                        for (const auto& i : params_keys) {
                                if (params.count(i) <= 0) {
                                        GM_ELOG << "missing param " << i;
                                        return false;
                                }
                        }
                        return true;
                }

                void yunying::process_request(const py::base *msg) {
                        const auto& req = msg->GetExtension(py::request::req);
                        GM_TLOG << *this << " process msg\n" << req.DebugString();

                        map<string, string> params;
                        for (const auto& i : req.params()) {
                                params[i.name()] = i.value();
                        }

#ifndef _DEBUG
                        if (params.count("sign") == 0) {
                                send_error_to_yunying(py::INVALID_SIGNITURE, "no sign");
                                return;
                        }

                        auto ptt = PTTS_GET_COPY(options, 1);
                        auto sign = yunying_calc_sign(params, ptt.yunying().app_secret());
                        if (sign != params.at("sign")) {
                                send_error_to_yunying(py::INVALID_SIGNITURE, "your sign: " + params.at("sign") + " our sign: " + sign);
                                return;
                        }
#endif

                        string msg_param;
                        try {
                                if (req.uri() == "/punish_player") {
                                        if (check_params(params, {"serverId", "roleId", "type", "reason"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoull(params.at("roleId"));
                                                msg_param = "type";
                                                auto type = stoi(params.at("type"));
                                                msg_param = "time";
                                                int int_time = 0;
                                                if (params.count("time") > 0) {
                                                        int_time = stoi(params.at("time"));
                                                }
                                                msg_param = "reason";
                                                auto reason = params.at("reason");
                                                uint32_t time;
                                                if (int_time < 0) {
                                                        time = numeric_limits<uint32_t>::max();
                                                } else {
                                                        time = static_cast<uint32_t>(int_time);
                                                        uint32_t now_time = system_clock::to_time_t(system_clock::now());
                                                        time *= 60;
                                                        time += now_time;
                                                }
                                                switch (type) {
                                                case 1:
                                                        ASSERT(gag_role_by_gid_func_);
                                                        gag_role_by_gid_func_(sid_, server_id, role, time, reason);
                                                        break;
                                                case 2:
                                                        ASSERT(ban_role_by_gid_func_);
                                                        ban_role_by_gid_func_(sid_, server_id, role, time, reason);
                                                        break;
                                                case 3:
                                                        ASSERT(kick_role_by_gid_func_);
                                                        kick_role_by_gid_func_(sid_, server_id, role, reason);
                                                        break;
                                                default:
                                                        throw invalid_argument("");
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/unpunish_player") {
                                        if (check_params(params, {"serverId", "roleId", "type"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoull(params.at("roleId"));
                                                msg_param = "type";
                                                auto type = stoi(params.at("type"));
                                                switch (type) {
                                                case 1:
                                                        ASSERT(remove_gag_role_by_gid_func_);
                                                        remove_gag_role_by_gid_func_(sid_, server_id, role);
                                                        break;
                                                case 2:
                                                        ASSERT(remove_ban_role_by_gid_func_);
                                                        remove_ban_role_by_gid_func_(sid_, server_id, role);
                                                        break;
                                                default:
                                                        throw invalid_argument("");
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/get_role_info") {
                                        if (check_params(params, {"serverId"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                if (params.count("roleId") > 0 && params.count("roleName") <= 0) {
                                                        msg_param = "roleId";
                                                        auto role = stoull(params.at("roleId"));
                                                        ASSERT(find_role_by_gid_func_);
                                                        find_role_by_gid_func_(sid_, server_id, role);
                                                } else if (params.count("roleName") > 0 && params.count("roleId") <= 0) {
                                                        msg_param = "roleName";
                                                        auto rolename = params.at("roleName");
                                                        ASSERT(find_role_by_rolename_func_);
                                                        find_role_by_rolename_func_(sid_, server_id, rolename);
                                                } else if (params.count("roleName") > 0 && params.count("roleId") > 0) {
                                                        msg_param = "roleId";
                                                        auto role = stoull(params.at("roleId"));
                                                        ASSERT(find_role_by_gid_func_);
                                                        find_role_by_gid_func_(sid_, server_id, role);
                                                }else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }

                                } else if (req.uri() == "/fetch_world_chat") {
                                        if (check_params(params, {"serverId"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                ASSERT(fetch_world_chat_func_);
                                                fetch_world_chat_func_(sid_, server_id);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/fetch_rank_type_list") {
                                        fetch_rank_type_list(sid_);
                                } else if (req.uri() == "/fetch_rank_list") {
                                        if (check_params(params, {"serverId", "rankType", "pageSize"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "rankType";
                                                auto rank_type = params.at("rankType");
                                                msg_param = "pageSize";
                                                auto page_size = stoul(params.at("pageSize"));
                                                if (page_size <= 0) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid uri param pageSize");
                                                        return;
                                                }
                                                ASSERT(fetch_rank_list_func_);
                                                fetch_rank_list_func_(sid_, server_id, rank_type, page_size);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/set_log_level") {
                                        if (check_params(params, {"logLevel"})) {
                                                msg_param = "logLevel";
                                                auto log_level = params.at("logLevel");
                                                pc::options::log_level level;
                                                if (log_level == "1") {
                                                        level = pc::options::TRACE;
                                                } else if (log_level == "2") {
                                                        level = pc::options::INFO;
                                                } else if (log_level == "3") {
                                                        level = pc::options::ERROR;
                                                } else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid uri param");
                                                        return;
                                                }
                                                set_log_level(level);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/send_email") {
                                        if (check_params(params, {"serverId", "mailId", "title", "content", "channelId"})) {
                                                pd::gid_array gids;
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "mailId";
                                                auto mail_id = stoul(params.at("mailId"));
                                                if (params.count("roleIds") > 0) {
                                                        msg_param = "roleIds";
                                                        auto roles = params.at("roleIds");
                                                        auto str_gids = split_string(roles, ';');
                                                        for (auto i : str_gids) {
                                                                gids.add_gids(stoull(i));
                                                        }
                                                } else {
                                                        gids = pd::gid_array();
                                                }

                                                pd::event_array events = pd::event_array();
                                                msg_param = "title";
                                                auto title = params.at("title");
                                                if (string_length(title) > 40) {
                                                        send_error_to_yunying(py::INVALID_ARG, "title size > 40");
                                                        return;
                                                }
                                                msg_param = "content";
                                                auto content = params.at("content");
                                                msg_param = "items";
                                                if (params.count("items") > 0) {
                                                        auto attachment_array = split_string(params.at("items"), ',');
                                                        for (auto i : attachment_array) {
                                                                auto *event = events.add_events();
                                                                auto str_attachments = split_string(i, ':');
                                                                if (str_attachments.size() != 3) {
                                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param items");
                                                                        return;
                                                                }
                                                                if (stoul(str_attachments[2]) > MAX_ITME_COUNT) {
                                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param items");
                                                                        return;
                                                                }

                                                                if (!yunying_item_to_add_event(str_attachments[0], *event)) {
                                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param item type");
                                                                        return;
                                                                }

                                                                event->add_arg(str_attachments[2]);
                                                        }
                                                }
                                                msg_param = "channelId";
                                                auto channel_id = stoul(params.at("channelId"));
                                                send_mail_func_(sid_, server_id, channel_id, gids, mail_id, title, content, events);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if(req.uri() == "/manage_system_announcement" || req.uri() == "/manage_flow_announcement") {
                                        if (check_params(params, {"serverId", "startTime", "endTime", "content", "noticeId", "isPublish"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "startTime";
                                                auto start_time = stoul(params.at("startTime"));
                                                msg_param = "endTime";
                                                auto stop_time = stoul(params.at("endTime"));
                                                msg_param = "content";
                                                auto content = params.at("content");
                                                msg_param = "noticeId";
                                                auto notice_id = stoul(params.at("noticeId"));
                                                msg_param = "isPublish";
                                                bool publish = true;
                                                if (stoi(params.at("isPublish")) != 1) {
                                                        publish = false;
                                                }
                                                uint32_t times = 0u;
                                                msg_param = "showTimes";
                                                if (params.count("showTimes") > 0) {
                                                        times = stoul(params.at("showTimes"));
                                                }
                                                if (params.count("rollTimes") > 0) {
                                                        times = stoul(params.at("rollTimes"));
                                                }
                                                uint32_t interval_time = 0u;
                                                msg_param = "showInterval";
                                                if (params.count("showInterval") > 0) {
                                                        interval_time = stoul(params.at("showInterval"));
                                                }
                                                msg_param = "rollInterval";
                                                if (params.count("rollInterval") > 0) {
                                                        interval_time = stoul(params.at("rollInterval"));
                                                }
                                                pd::announcement announcement;
                                                if ((server_id < 0 ||
                                                     interval_time < 0u ||
                                                     times < 0u ||
                                                     start_time < 0u ||
                                                     stop_time < 0u) ||
                                                    notice_id < 0u ||
                                                    start_time > stop_time) {
                                                        send_error_to_yunying(py::INVALID_URI, "invalid param");
                                                        return;
                                                } else {
                                                        announcement.set_server_id(server_id);
                                                        announcement.set_times(times);
                                                        auto now_time = static_cast<uint32_t>(system_clock::to_time_t(system_clock::now()));
                                                        if (publish) {
                                                                if (start_time <= now_time) {
                                                                        start_time = now_time;
                                                                        if (start_time > stop_time) {
                                                                                send_error_to_yunying(py::INVALID_URI, "invalid param, start_time > stop_time");
                                                                                return;
                                                                        }
                                                                }
                                                        } else {
                                                                start_time = now_time;
                                                                stop_time = now_time;
                                                        }
                                                        if (req.uri() == "/manage_system_announcement") {
                                                                announcement.set_type(pd::SYSTEM_ANNOUNCEMENT);
                                                        } else {
                                                                msg_param = "channelId";
                                                                if (params.count("channelId") > 0) {
                                                                        auto channel_id = stoul(params.at("channelId"));
                                                                        announcement.set_channel_id(channel_id);
                                                                        announcement.set_type(pd::FLOW_LIGHT_ANNOUNCEMENT);
                                                                } else {
                                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                                                }
                                                        }
                                                        announcement.set_start_time(start_time);
                                                        announcement.set_stop_time(stop_time);
                                                        announcement.set_content(content);
                                                        announcement.set_system_id(notice_id);
                                                        announcement.set_interval_time(interval_time * 60);
                                                        announcement.set_publish(publish);
                                                        ASSERT(manage_announcement_func_);
                                                        manage_announcement_func_(sid_, announcement);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/server_images_announcement") {
                                        if (check_params(params, {"imgUrl", "serverId"})) {
                                                msg_param = "imgUrl";
                                                map<string, uint32_t> anns;
                                                for (const auto& i : split_string(params.at("imgUrl"), ',')) {
                                                        auto ims = split_string(i, '|');
                                                        ASSERT(ims.size() == 2);
                                                        anns[ims[0]] = stoul(ims[1]);
                                                }
                                                msg_param = "serverId";
                                                auto server_id = stoul(params.at("serverId"));
                                                server_images_announcement_func_(sid_, server_id, anns);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if(req.uri() == "/fetch_money_type") {
                                        fetch_money_type(sid_);
                                } else if (req.uri() == "/internal_recharge") {
                                        if (check_params(params, {"serverId", "roleId", "rechargeId"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoull(params.at("roleId"));
                                                msg_param = "rechargeId";
                                                auto recharge_id = stoul(params.at("rechargeId"));
                                                ASSERT(internal_recharge_func_);
                                                internal_recharge_func_(sid_, server_id, role, recharge_id);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/recharge") {
                                        if (check_params(params, {"cp_orderno", "orderno", "currency", "price", "paytime", "product_name", "ext_info"})) {
                                                msg_param = "cp_orderno";
                                                auto order = stoul(params.at("cp_orderno"));
                                                msg_param = "orderno";
                                                auto yy_orderno = params.at("orderno");
                                                msg_param = "currency";
                                                auto currency = params.at("currency");
                                                msg_param = "price";
                                                auto price = stoul(params.at("price"));
                                                msg_param = "paytime";
                                                auto paytime = stoul(params.at("paytime"));
                                                msg_param = "product_name";
                                                auto product_name = params.at("product_name");
                                                msg_param = "ext_info";
                                                auto ext_info = params.at("ext_info");
                                                if (price < 0 || paytime <= 0 || ext_info == "") {
                                                        send_error_to_yunying(py::INVALID_URI, "invalid param");
                                                        return;
                                                }
                                                ASSERT(recharge_func_);
                                                recharge_func_(sid_, order, yy_orderno, currency, price, paytime, product_name, ext_info);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/reissue_recharge") {
                                        if (check_params(params, {"serverId", "roleId", "orderNo"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoull(params.at("roleId"));
                                                msg_param = "orderNo";
                                                auto order = stoul(params.at("orderNo"));
                                                if (order < 0) {
                                                        send_error_to_yunying(py::INVALID_URI, "invalid param " + params.at("order"));
                                                        return;
                                                }
                                                ASSERT(reissue_recharge_func_);
                                                reissue_recharge_func_(sid_, server_id, role, order);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() == "/item_change_log") {
                                        if (check_params(params, {"serverId", "roleId", "pageIdx", "pageSize"})) {
                                                uint32_t start_time;
                                                uint32_t end_time;
                                                if (params.count("startTime") > 0) {
                                                        msg_param = "startTime";
                                                        start_time = stoul(params.at("startTime"));
                                                } else {
                                                        start_time  = 0;
                                                }
                                                if (params.count("endTime") > 0) {
                                                        msg_param = "endTime";
                                                        end_time = stoul(params.at("endTime"));
                                                } else {
                                                        end_time = system_clock::to_time_t(system_clock::now());
                                                }
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoll(params.at("roleId"));
                                                msg_param = "pageIdx";
                                                auto page_idx = stoi(params.at("pageIdx"));
                                                msg_param = "pageSize";
                                                auto page_size = stoi(params.at("pageSize"));
                                                uint32_t item_id = 0;
                                                if (params.count("itemId") > 0) {
                                                        msg_param = "itemId";
                                                        item_id = stoul(params.at("itemId"));
                                                }
                                                ASSERT(fetch_log_func_);
                                                fetch_log_func_(sid_, server_id, role, item_id, start_time, end_time, page_idx, page_size);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/add_to_white_list") {
                                        if (check_params(params, {"serverId", "ip"})) {
                                                set<string> ips;
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "ip";
                                                auto original_ips = params.at("ip");
                                                if (params.count("ip") <= 0) {
                                                        send_error_to_yunying(py::INVALID_URI, "invalid uri " + req.uri());
                                                } else {
                                                        auto str_ips = split_string(original_ips, ':');
                                                        for (auto i : str_ips) {
                                                                ips.insert(i);
                                                        }
                                                        add_to_white_list_func_(sid_, server_id, ips);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/remove_from_white_list") {
                                        if (check_params(params, {"serverId", "ip"})) {
                                                set<string> ips;
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "ip";
                                                auto original_ips = params.at("ip");
                                                if (params.count("ip") <= 0) {
                                                        send_error_to_yunying(py::INVALID_URI, "invalid uri " + req.uri());
                                                } else {
                                                        auto str_ips = split_string(original_ips, ':');
                                                        for (auto i : str_ips) {
                                                                ips.insert(i);
                                                        }
                                                        remove_from_white_list_func_(sid_, server_id, ips);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }

                                } else if (req.uri() =="/fetch_white_list") {
                                        auto server_id = 0;
                                        if (params.count("serverId") > 0) {
                                                msg_param = "serverId";
                                                server_id = stoi(params.at("serverId"));
                                        }
                                        fetch_white_list_func_(sid_, server_id);
                                } else if (req.uri() =="/fetch_punished" || req.uri() =="/fetch_sent_mail") {
                                        if (check_params(params, {"serverId", "pageIdx", "pageSize"})) {
                                                uint32_t start_time = 0;
                                                uint32_t end_time = system_clock::to_time_t(system_clock::now());;
                                                uint64_t role = 0;
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                if (params.count("roleId") > 0) {
                                                        msg_param = "roleId";
                                                        role = stoll(params.at("roleId"));
                                                }
                                                if (params.count("startTime") > 0) {
                                                        msg_param = "startTime";
                                                        start_time = stoul(params.at("startTime"));
                                                }
                                                if (params.count("endTime") > 0) {
                                                        msg_param = "endTime";
                                                        end_time = stoul(params.at("endTime"));
                                                }
                                                msg_param = "pageIdx";
                                                auto page_idx = stoi(params.at("pageIdx"));
                                                msg_param = "pageSize";
                                                auto page_size = stoi(params.at("pageSize"));
                                                if (start_time > end_time) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param, start_time > end_time");
                                                        return;
                                                }
                                                if (req.uri() =="/fetch_punished") {
                                                        ASSERT(fetch_punished_func_);
                                                        fetch_punished_func_(sid_, server_id, role, start_time, end_time, page_idx, page_size);
                                                } else {
                                                        ASSERT(fetch_sent_mail_func_);
                                                        fetch_sent_mail_func_(sid_, server_id, role, start_time, end_time, page_idx, page_size);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/fetch_login_record") {
                                        if (check_params(params, {"serverId", "roleId", "pageIdx", "pageSize"})) {
                                                set<string> ips;
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "roleId";
                                                auto role = stoll(params.at("roleId"));
                                                msg_param = "pageIdx";
                                                auto page_idx = stoi(params.at("pageIdx"));
                                                msg_param = "pageSize";
                                                auto page_size = stoi(params.at("pageSize"));
                                                ASSERT(fetch_login_record_func_);
                                                fetch_login_record_func_(sid_, server_id, role, page_idx, page_size);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/fetch_recharge_record") {
                                        if (check_params(params, {"serverId", "channelId", "roleId", "pageIdx", "pageSize"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "channelId";
                                                auto channel_id = stoi(params.at("channelId"));
                                                msg_param = "roleId";
                                                auto role = stoll(params.at("roleId"));
                                                msg_param = "pageIdx";
                                                auto page_idx = stoi(params.at("pageIdx"));
                                                msg_param = "pageSize";
                                                auto page_size = stoi(params.at("pageSize"));
                                                uint32_t order = 0;
                                                if (params.count("orderNo") > 0) {
                                                        msg_param = "orderNo";
                                                        order = stoll(params.at("orderNo"));
                                                }
                                                uint32_t start_time = 0;
                                                uint32_t end_time = system_clock::to_time_t(system_clock::now());;
                                                if (params.count("startTime") > 0) {
                                                        msg_param = "startTime";
                                                        start_time = stoul(params.at("startTime"));
                                                }
                                                if (params.count("endTime") > 0) {
                                                        msg_param = "endTime";
                                                        end_time = stoul(params.at("endTime"));
                                                }
                                                ASSERT(fetch_recharge_record_func_);
                                                fetch_recharge_record_func_(sid_, server_id, channel_id, order, role, start_time, end_time, page_idx, page_size);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/fetch_currency_record") {
                                        if (check_params(params, {"serverId", "moneyType", "roleId", "pageIdx", "pageSize"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoi(params.at("serverId"));
                                                msg_param = "moneyType";
                                                string type;
                                                if (stoul(params.at("moneyType")) == 0) {
                                                        type = "CURRENCY_GOLD,CURRENCY_DIAMOND";
                                                } else {
                                                        pd::resource_type resource_type;
                                                        int_to_enum(resource_type, stoul(params.at("moneyType")) % 10000000);
                                                        type = "CURRENCY_" + pd::resource_type_Name(resource_type);
                                                }
                                                msg_param = "roleId";
                                                auto role = stoll(params.at("roleId"));
                                                msg_param = "pageIdx";
                                                auto page_idx = stoi(params.at("pageIdx"));
                                                msg_param = "pageSize";
                                                auto page_size = stoi(params.at("pageSize"));
                                                uint32_t start_time = 0;
                                                uint32_t end_time = system_clock::to_time_t(system_clock::now());;
                                                if (params.count("startTime") > 0) {
                                                        msg_param = "startTime";
                                                        start_time = stoul(params.at("startTime"));
                                                }
                                                if (params.count("endTime") > 0) {
                                                        msg_param = "endTime";
                                                        end_time = stoul(params.at("endTime"));
                                                }
                                                ASSERT(fetch_currency_record_func_);
                                                fetch_currency_record_func_(sid_, server_id, type, role, start_time, end_time, page_idx, page_size);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/before_login_manage_announcement" || req.uri() == "/server_manage_announcement") {
                                        if (check_params(params, {"noticeId", "isPublish", "title", "content", "channelId"})) {
                                                msg_param = "noticeId";
                                                auto notice_id = stoul(params.at("noticeId"));
                                                msg_param = "title";
                                                auto title = params.at("title");
                                                msg_param = "content";
                                                auto content = params.at("content");
                                                msg_param = "channelId";
                                                auto channel_id = stoul(params.at("channelId"));
                                                pd::gonggao gonggao;
                                                if (req.uri() == "/before_login_manage_announcement") {
                                                        gonggao.set_type(pd::BEFORE_LOGIN_ANNOUNCEMENT);
                                                } else if (req.uri() == "/server_manage_announcement") {
                                                        gonggao.set_type(pd::SERVER_ANNOUNCEMENT);
                                                }
                                                msg_param = "isPublish";
                                                if (stoi(params.at("isPublish")) == 1) {
                                                        gonggao.set_publish(true);
                                                } else {
                                                        gonggao.set_publish(false);
                                                }
                                                gonggao.set_system_id(notice_id);
                                                gonggao.set_title(title);
                                                gonggao.set_content(content);
                                                gonggao.set_channel_id(channel_id);
                                                ASSERT(login_gonggao_func_);
                                                login_gonggao_func_(sid_, gonggao);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/fetch_role_list") {
                                        if (check_params(params, {"uId"})) {
                                                msg_param = "uId";
                                                auto username = params.at("uId");
                                                ASSERT(fetch_role_list_func_);
                                                fetch_role_list_func_(sid_, username);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/activity") {
                                        if (check_params(params, {"startTime", "endTime", "type", "serverIds"})) {
                                                msg_param = "activityId";
                                                auto activity = 0u;
                                                if (params.count("activityId") > 0) {
                                                        activity = stoul(params.at("activityId"));
                                                }
                                                msg_param = "startTime";
                                                auto start_time = refresh_day_from_time_t(stoul(params.at("startTime")));
                                                msg_param = "endTime";
                                                auto end_time = refresh_day_from_time_t(stoul(params.at("endTime")));
                                                msg_param = "type";
                                                auto type = stoul(params.at("type"));
                                                if (type == 0 || type >= pd::AAT_COUNT) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid type");
                                                        return;
                                                }
                                                auto open_activity = false;
                                                msg_param = "status";
                                                if (params.at("status") == "1" ) {
                                                        open_activity = true;
                                                }
                                                msg_param = "serverIds";
                                                auto server_ids = split_string(params.at("serverIds"), ',');
                                                if (end_time < start_time) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                                        return;
                                                }
                                                ASSERT(operates_activity_func_);
                                                operates_activity_func_(sid_, activity, start_time, end_time - start_time + 1, static_cast<pd::admin_activity_type>(type), server_ids, open_activity);
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/activity_config") {
                                        fetch_activity_config_done();
                                } else if (req.uri() =="/add_stuff") {
                                        if (check_params(params, {"serverId", "itemId", "itemNum"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoul(params.at("serverId"));
                                                uint64_t role = 0;
                                                string rolename = "";
                                                if (params.count("roleId") > 0) {
                                                        role = stoul(params.at("roleId"));
                                                } else if (params.count("roleName") > 0) {
                                                        rolename = params.at("roleName");
                                                } else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param roleId or roleName");
                                                        return;
                                                }
                                                msg_param = "itemId";
                                                auto item = params.at("itemId");
                                                msg_param = "itemNum";
                                                auto item_count = params.at("itemNum");
                                                if (stoi(params.at("itemNum")) < 0) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param itemNum");
                                                        return;
                                                }
                                                pd::event_array events;
                                                auto *event = events.add_events();
                                                if (yunying_item_to_add_event(item, *event)) {
                                                        event->add_arg(item_count);
                                                } else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param itemId");
                                                        return;
                                                }
                                                if (role != 0) {
                                                        ASSERT(add_stuff_by_gid_func_);
                                                        add_stuff_by_gid_func_(sid_, server_id, role, events);
                                                } else {
                                                        ASSERT(rolename != "");
                                                        ASSERT(add_stuff_by_rolename_func_);
                                                        add_stuff_by_rolename_func_(sid_, server_id, rolename, events);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                } else if (req.uri() =="/dec_stuff") {
                                        if (check_params(params, {"serverId", "itemId", "itemNum"})) {
                                                msg_param = "serverId";
                                                auto server_id = stoul(params.at("serverId"));
                                                uint64_t role = 0;
                                                string rolename = "";
                                                if (params.count("roleId") > 0) {
                                                        role = stoul(params.at("roleId"));
                                                } else if (params.count("roleName") > 0) {
                                                        rolename = params.at("roleName");
                                                } else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param roleId or roleName");
                                                        return;
                                                }
                                                msg_param = "itemId";
                                                auto item = params.at("itemId");
                                                msg_param = "itemNum";
                                                auto item_count = params.at("itemNum");
                                                if (stoi(params.at("itemNum")) < 0) {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param itemNum");
                                                        return;
                                                }
                                                pd::event_array events;
                                                auto *event = events.add_events();
                                                if (yunying_item_to_dec_event(item, *event)) {
                                                        event->add_arg(item_count);
                                                } else {
                                                        send_error_to_yunying(py::INVALID_ARG, "invalid param itemId");
                                                        return;
                                                }
                                                if (role != 0) {
                                                        ASSERT(dec_stuff_by_gid_func_);
                                                        dec_stuff_by_gid_func_(sid_, server_id, role, events);
                                                } else {
                                                        ASSERT(rolename != "");
                                                        ASSERT(dec_stuff_by_rolename_func_);
                                                        dec_stuff_by_rolename_func_(sid_, server_id, rolename, events);
                                                }
                                        } else {
                                                send_error_to_yunying(py::INVALID_ARG, "invalid param");
                                        }
                                }
                        } catch (const std::out_of_range& e) {
                                send_error_to_yunying(py::INVALID_ARG, "missing param " + msg_param);
                        } catch (const std::invalid_argument& e) {
                                send_error_to_yunying(py::INVALID_ARG, "bad param " + msg_param);
                        } catch (...) {
                                send_error_to_yunying(py::INVALID_ARG, "unknown " + msg_param);
                        }
                }

                void yunying::send_error_to_yunying(py::error_code ec, const string& msg) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result result;
                        result.set_code(ec);
                        result.set_msg(msg);
                        reply->set_content(pb2json(result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::send_to_yunying(const py::base& msg) {
                        GM_TLOG << *this << " send " << msg.DebugString();
                        auto ss = make_shared<net::sendstream>();
                        msg.SerializeToOstream(ss.get());
                        nsv_->send(conn_, ss);
                        stop();
                }

                void yunying::stop() {
                        ADD_TIMER(
                                st_,
                                ([this, self = shared_from_this()] (auto canceled, const auto& timer) {
                                        if (stop_cb_ && conn_) {
                                                stop_cb_(conn_);
                                        }
                                        if (nsv_ && conn_) {
                                                nsv_->disconnect(conn_);
                                                conn_.reset();
                                        }
                                        stop_cb_ = nullptr;
                                        GM_DLOG << *this << " stop";
                                }), 1s);
                }

                ostream& operator<<(ostream& os, const yunying& s) {
                        return os << s.name_;
                }

                void yunying::kick_role_by_gid_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::PUNISH_FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::gag_role_by_gid_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::PUNISH_FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::ban_role_by_gid_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::PUNISH_FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::remove_punish_by_gid_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::CANCEL_PUNISH_FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::find_role_done(const pd::yunying_role_info& role, pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data;
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                                data = pb2json(role);
                        } else {
                                py::result py_result;
                                py_result.set_code(py::FIND_ROLE_FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_world_chat_done(const pd::world_chat_array& world_chat, pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data = "";
                        if (result == pd::OK) {
                                for (auto i : world_chat.world_chat()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (world_chat.world_chat_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                replace_string(data, "rolename", "roleName");
                                replace_string(data, "gid", "roleId");
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                        } else {
                                py::result py_result;
                                py_result.set_code(py::FETCH_WORLD_CHAT_FAILED);
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_rank_type_list(uint32_t sid) {
                        pd::rank_info_list_array yrila;
                        map<string, string> rank_types = {{"RT_LEVEL", "等级"}, {"RT_ZHANLI", "战力"}, {"RT_RECEIVE_GIFT", "魅力值"}, {"RT_GIVE_GIFT", "慷慨值"}, {"RT_MANSION_FANCY", "府邸豪华度"}, {"ARENA", "竞技场"}};
                        for(auto i : rank_types) {
                                auto *rank_info_list = yrila.add_rank_info_list();
                                rank_info_list->set_ranktype(i.first);
                                rank_info_list->set_rankname(i.second);
                        }

                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data = "";
                        for (auto i : yrila.rank_info_list()) {
                                data += pb2json(i);
                                data += ",";
                        }
                        if (yrila.rank_info_list_size() > 0) {
                                data.erase(data.length() - 1, 1);
                        }
                        data.insert(data.begin(), '[');
                        data.insert(data.end(), ']');
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::set_money_list(auto& mrla, uint32_t id, const auto& ptts) {
                        for (const auto& i : ptts) {
                                auto *money_list = mrla.add_money_list();
                                money_list->set_id(id + i.second.id());
                                money_list->set_name("");
                        }
                }

                void yunying::fetch_money_type(uint32_t sid) {
                        pd::money_list_array mrla = yunying_money_list();

                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data = "";
                        for (auto i : mrla.money_list()) {
                                data += pb2json(i);
                                data += ",";
                        }
                        if (mrla.money_list_size() > 0) {
                                data.erase(data.length() - 1, 1);
                        }
                        data.insert(data.begin(), '[');
                        data.insert(data.end(), ']');
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_rank_list_done(pd::result result, const pd::rank_list_array& rank_list_array) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data = "";
                        if (result == pd::OK) {
                                string original_data;
                                string title = "\"title\":";
                                data = "\"list\":[ [";
                                if (rank_list_array.rank_list_size() > 0) {
                                        string str = pb2json(rank_list_array.rank_list(0));
                                        for ( auto i : split_string(str, ',')) {
                                                title = title + split_string(i, ':')[0] + ",";
                                        }
                                        title.erase(title.length() - 1, 1);
                                        title.insert(title.end(), ']');
                                        replace(title.begin() + 6, title.end(), '{', '[');
                                        title.insert(title.end(), ',');
                                }
                                for (auto i : rank_list_array.rank_list()) {
                                        original_data += pb2json(i);
                                        original_data += ",";
                                }
                                auto datas = split_string(original_data, ',');
                                for (auto i : datas) {
                                        data += split_string(i, ':')[1];
                                        if (strchr(split_string(i, ':')[1].c_str(), '}')) {
                                                data += ", [";
                                        } else {
                                                data += ",";
                                        }
                                }
                                if (rank_list_array.rank_list_size() > 0) {
                                        data.erase(data.length() - 3, 3);
                                }
                                data.insert(data.end(), ']');
                                replace(data.begin(), data.end(), '}', ']');
                                data = title + data;
                                data.insert(data.begin(), '{');
                                data.insert(data.end(), '}');
                        } else {
                                py::result py_result;
                                py_result.set_code(py::FETCH_ARENA_LIST_FAILED);
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::set_log_level(pc::options::log_level level) {
                        change_log_level(level);
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        reply->set_content(pc::options_log_level_Name(level));
                        send_to_yunying(rsp_msg);
                }

                void yunying::send_mail_done(pd::result result, const pd::gid_array& roles) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        string data = "";
                        py_result.set_msg(pd::result_Name(result));
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                                data += pb2json(py_result);
                        } else {
                                py_result.set_code(py::SEND_MAIL_FAILED);
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::recharge_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else if (result == pd::RECHARGE_ORDER_RECHARGED) {
                                py_result.set_code(py::RECHARGE_ORDER_RECHARGED);
                        } else {
                                py_result.set_code(py::RECHARGE_FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_log_done(pd::result result, const pd::yunying_item_change_array& item_changes) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : item_changes.item_change()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (item_changes.item_change_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::manage_announcement_done(pd::result result, const vector<int>& keys) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data = "";
                        py::result py_result;
                        py_result.set_msg(pd::result_Name(result));
                        if (result == pd::OK) {
                                if (!keys.empty()) {
                                        pd::server_ids server_ids;
                                        for (auto i : keys) {
                                                server_ids.add_ids(i);
                                        }
                                        data = pb2json(server_ids);
                                }
                                py_result.set_code(py::NO_ERROR);
                                data += pb2json(py_result);
                        } else {
                                py_result.set_code(py::BROADCAST_FAILED);
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::server_images_announcements_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::add_to_white_list_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::remove_from_white_list_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_white_list_done(pd::result result, const pd::white_list_array& white_lists) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        string data;
                        if (result == pd::OK) {
                                data = pb2json(white_lists);
                                if (white_lists.white_lists_size() > 0) {
                                        data.erase(0, 16);
                                        data.erase(data.length() - 1, 1);
                                } else {
                                        replace(data.begin(), data.end(), '}', ']');
                                        replace(data.begin(), data.end(), '{', '[');
                                }
                        } else {
                                py::result py_result;
                                if (result == pd::NOT_FOUND) {
                                        py_result.set_code(py::NO_ERROR);
                                } else {
                                        py_result.set_code(py::FAILED);
                                }
                                py_result.set_msg(pd::result_Name(result));
                                data = pb2json(py_result);
                        }
                        reply->set_content(data);
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_punished_done(pd::result result, const pd::yunying_fetch_punished_info_array& yspia) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : yspia.fetch_punished_info()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (yspia.fetch_punished_info_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_sent_mail_done(pd::result result, const pd::yunying_fetch_sent_mail_info_array& yssmia) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : yssmia.fetch_sent_mail_info()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (yssmia.fetch_sent_mail_info_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_login_record_done(pd::result result, const pd::yunying_fetch_login_record_array& yflra) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : yflra.fetch_login_record()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (yflra.fetch_login_record_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_recharge_record_done(pd::result result, const pd::yunying_fetch_recharge_record_array& yfrra) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : yfrra.fetch_recharge_record()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (yfrra.fetch_recharge_record_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_currency_record_done(pd::result result, const pd::yunying_fetch_currency_record_array& yfcra) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                string data = "";
                                for (auto i : yfcra.fetch_currency_record()) {
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (yfcra.fetch_currency_record_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::login_gonggao_done() {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        py_result.set_code(py::NO_ERROR);
                        py_result.set_msg(pd::result_Name(pd::OK));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_role_list_done(const pd::yunying_role_simple_info_array& role_infos, pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                                string data;
                                for (auto i : role_infos.role_simple_infos()) {
                                        if (pb2json(i) == "{}") {
                                                continue;
                                        }
                                        data += pb2json(i);
                                        data += ",";
                                }
                                if (!data.empty() && role_infos.role_simple_infos_size() > 0) {
                                        data.erase(data.length() - 1, 1);
                                }
                                data.insert(data.begin(), '[');
                                data.insert(data.end(), ']');
                                reply->set_content(data);
                        } else {
                                py_result.set_code(py::FAILED);
                                py_result.set_msg(pd::result_Name(result));
                                reply->set_content(pb2json(py_result));
                        }
                        send_to_yunying(rsp_msg);
                }

                void yunying::add_stuff_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::dec_stuff_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::operates_activity_done(pd::result result) {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        py::result py_result;
                        if (result == pd::OK) {
                                py_result.set_code(py::NO_ERROR);
                        } else {
                                py_result.set_code(py::FAILED);
                        }
                        py_result.set_msg(pd::result_Name(result));
                        reply->set_content(pb2json(py_result));
                        send_to_yunying(rsp_msg);
                }

                void yunying::fetch_activity_config_done() {
                        py::base rsp_msg;
                        auto *reply = rsp_msg.MutableExtension(py::reply::rep);
                        reply->set_status(py::OK);
                        pd:: activity_config_array activity_configs;
                        activity_configs.set_code(to_string(static_cast<uint32_t>(py::NO_ERROR)));
                        activity_configs.set_msg(pd::result_Name(pd::OK));
                        auto *wheel_config = activity_configs.add_data();
                        auto wheel_ptts = PTTS_GET_ALL(activity_prize_wheel);
                        wheel_config->set_typeid_(1);
                        wheel_config->set_typename_("转盘");
                        for (const auto& i : wheel_ptts) {
                                auto * wheel_list = wheel_config->add_activitylist();
                                wheel_list->set_activityid(i.second.id());
                                wheel_list->set_activityname(i.second.name());
                        }
                        auto *limit_play_config = activity_configs.add_data();
                        auto limit_play_ptts = PTTS_GET_ALL(activity_limit_play);
                        limit_play_config->set_typeid_(2);
                        limit_play_config->set_typename_("限时玩法");
                        for (const auto& i : limit_play_ptts) {
                                auto * limit_play_list = limit_play_config->add_activitylist();
                                limit_play_list->set_activityid(i.second.id());
                                limit_play_list->set_activityname(i.second.name());
                        }

                        auto *continue_recharge_config = activity_configs.add_data();
                        auto continue_recharge_ptts = PTTS_GET_ALL(activity_continue_recharge);
                        continue_recharge_config->set_typeid_(3);
                        continue_recharge_config->set_typename_("连续充值");
                        for (const auto& i : continue_recharge_ptts) {
                               auto * continue_recharge_list = continue_recharge_config->add_activitylist();
                               continue_recharge_list->set_activityid(i.second.id());
                               continue_recharge_list->set_activityname(i.second.name());
                        }

                        auto *discount_good_config = activity_configs.add_data();
                        auto discount_good_ptts = PTTS_GET_ALL(activity_discount_goods);
                        discount_good_config->set_typeid_(4);
                        discount_good_config->set_typename_("折扣贩售");
                        for (const auto& i : discount_good_ptts) {
                               auto * discount_good_list = discount_good_config->add_activitylist();
                               discount_good_list->set_activityid(i.second.id());
                               discount_good_list->set_activityname(i.second.name());
                        }

                        auto *recharge_rank_config = activity_configs.add_data();
                        recharge_rank_config->set_typeid_(5);
                        recharge_rank_config->set_typename_("充值排行");

                        auto *leiji_recharge_config = activity_configs.add_data();
                        auto leiji_recharge_ptts = PTTS_GET_ALL(activity_leiji_recharge);
                        leiji_recharge_config->set_typeid_(6);
                        leiji_recharge_config->set_typename_("累计充值");
                        for (const auto& i : leiji_recharge_ptts) {
                               auto * leiji_recharge_list = leiji_recharge_config->add_activitylist();
                               leiji_recharge_list->set_activityid(i.second.id());
                               leiji_recharge_list->set_activityname(i.second.name());
                        }

                        auto *leiji_consume_config = activity_configs.add_data();
                        auto leiji_consume_ptts = PTTS_GET_ALL(activity_leiji_consume);
                        leiji_consume_config->set_typeid_(7);
                        leiji_consume_config->set_typename_("累计消费");
                        for (const auto& i : leiji_consume_ptts) {
                               auto * leiji_consume_list = leiji_consume_config->add_activitylist();
                               leiji_consume_list->set_activityid(i.second.id());
                               leiji_consume_list->set_activityname(i.second.name());
                        }

                        auto *tea_party_config = activity_configs.add_data();
                        auto tea_party_ptts = PTTS_GET_ALL(activity_tea_party);
                        tea_party_config->set_typeid_(8);
                        tea_party_config->set_typename_("茶话会");
                        for (const auto& i : tea_party_ptts) {
                               auto * tea_party_list = tea_party_config->add_activitylist();
                               tea_party_list->set_activityid(i.second.id());
                               tea_party_list->set_activityname(i.second.name());
                        }

                        auto *festival_config = activity_configs.add_data();
                        auto festival_ptts = PTTS_GET_ALL(activity_festival);
                        festival_config->set_typeid_(9);
                        festival_config->set_typename_("节日活动");
                        for (const auto& i : festival_ptts) {
                               auto * festival_list = festival_config->add_activitylist();
                               festival_list->set_activityid(i.second.id());
                               ASSERT(i.second.titles_size() > 0);
                               festival_list->set_activityname(i.second.titles(0));
                        }
                        reply->set_content(pb2json(activity_configs));
                        send_to_yunying(rsp_msg);
                }
        }
}
