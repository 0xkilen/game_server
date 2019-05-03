/*#include "bi.hpp"
#include "utils/service_thread.hpp"
#include "utils/yunying.hpp"
#include "utils/assert.hpp"
#include "utils/time_utils.hpp"
#include "utils/game_def.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "config/quest_ptts.hpp"
#include "config/item_ptts.hpp"
#include "config/adventure_ptts.hpp"
#include "config/recharge_ptts.hpp"
#include "proto/data_event.pb.h"
#include "log.hpp"
#include "utils/json2pb.hpp"
#include <curl/curl.h>
#include <map>
#include <sstream>

using namespace std;

namespace nora {
        namespace scene {

                namespace {

                        static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
                                auto *tree = (bpt::ptree *)userp;
                                size_t realsize = size * nmemb;
                                if (realsize > 0) {
                                        string str((char *)contents, realsize);
                                        SBI_TLOG << "recv msg:\n" << str;
                                        istringstream iss(str);
                                        read_json(iss, *tree);
                                }
                                return realsize;
                        }

                }

                bi::bi() {
                        null_ = fopen("/dev/null", "wb");
                }

                void bi::init() {
                        st_ = make_shared<service_thread>("yunying bi");
                        st_->start();
                        auto ptt = PTTS_GET_COPY(options, 1);
                        send_bi_ = ptt.scene_info().send_bi();
                }

                void bi::init(const shared_ptr<service_thread>& st) {
                        ASSERT(st);
                        st_ = st;
                }

                void bi::stop() {
                        if (st_) {
                                st_->stop();
                                st_.reset();
                        }
                        SBI_ILOG << "bi stop";
                }

                void bi::set_common_bi_msg(pd::yunying_bi_info* bi, const pd::yunying_client_info& yci, const string& username, const string& ip, uint64_t role, uint64_t league_id) {
                        auto origin_username = username;
                        if (split_string(username, '_').size() == 2) {
                                origin_username = split_string(username, '_')[0];
                        }
                        auto ptt = PTTS_GET_COPY(options, 1);
                        bi->set_open_id(origin_username);
                        bi->set_channel_id(to_string(yci.channel()));
                        bi->set_server_id(to_string(ptt.scene_info().server_id()));
                        bi->set_device_id(yci.device_id());
                        bi->set_ip(ip);
                        bi->set_version(yci.game_version());
                        bi->set_os_type(yci.platform());
                        bi->set_uid(origin_username);
                        bi->set_role_id(to_string(role));
                        bi->set_act_time(system_clock::to_time_t(system_clock::now()));
                        if (league_id != 0) {
                                bi->set_role_guild(to_string(league_id));
                        }
                }

                void bi::post(const string& addr, const map<string, string>& params, const function<void(pd::result, const bpt::ptree&)>& cb) {
                        string content;
                        for (const auto& i : params) {
                                if (!content.empty()) {
                                        content += '&';
                                }
                                content += i.first;
                                content += '=';
                                content += i.second;
                        }

                        CURLcode res;
                        ::CURL *curl = curl_easy_init();
                        ASSERT(curl);

                        struct curl_slist *headers = nullptr;
                        headers = curl_slist_append(headers, "Cache-Control: no-cache");
                        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
                        string length_str = string("Content-Length: ") + to_string(content.size());
                        headers = curl_slist_append(headers, length_str.c_str());
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                        curl_easy_setopt(curl, CURLOPT_URL, addr.c_str());
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, content.c_str());
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
                        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
                        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
                        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
                        curl_easy_setopt(curl, CURLOPT_STDERR, null_);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, null_);
                        bpt::ptree tree;
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&tree);
                        res = curl_easy_perform(curl);
                        if (res != CURLE_OK) {
                                SBI_ELOG << "curl_easy_perform() failed: ";
                                cb(pd::INTERNAL_ERROR, tree);
                        } else {
                                cb(pd::OK, tree);
                        }
                        curl_easy_cleanup(curl);
                        curl_slist_free_all(headers);
                }

                void bi::create_role(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, const string& role_name) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_name(role_name);
                        send_to_log_server_func_("role_create", bi);
                }

                void bi::login(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, const string& role_name, uint32_t level, uint32_t zhanli, uint32_t exp, uint32_t diamond, uint32_t gold, const string& gender, uint32_t vip_exp, uint32_t vip_level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_role_name(role_name);
                        bi.set_role_level(level);
                        bi.set_war_value(zhanli);
                        bi.set_role_exp(exp);
                        bi.set_money_gold(diamond);
                        bi.set_money_silver(gold);
                        bi.set_role_occupation(gender);
                        if (vip_exp == 0) {
                                bi.set_is_vip(0);
                        } else {
                                bi.set_is_vip(1);
                        }
                        bi.set_vip_level(vip_level);
                        send_to_log_server_func_("role_login", bi);
                }

                void bi::online_count(size_t count) {
                        if (!send_bi_) {
                                return;
                        }

                        auto ptt = PTTS_GET_COPY(options, 1);
                        pd::yunying_bi_info bi;
                        bi.set_time(clock::instance().now_time_str());
                        bi.set_online_cnt(to_string(count));
                        bi.set_version(ptt.login_info().client_version());
                        bi.set_server_id(to_string(ptt.scene_info().server_id()));
                        send_to_log_server_func_("realOnline", bi);
                }

                void bi::logout(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, time_t login_time, uint64_t league_id, pd::ce_origin origin, uint32_t level, uint32_t exp, uint32_t diamond, uint32_t gold, uint32_t zhanli, const string& role_name, const string& gender, uint32_t vip_exp, uint32_t vip_level) {
                        if (!send_bi_) {
                                return;
                        }

                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        auto now = system_clock::to_time_t(system_clock::now());
                        bi.set_online_time(to_string(now - login_time));
                        bi.set_role_level(level);
                        bi.set_role_exp(exp);
                        bi.set_money_gold(diamond);
                        bi.set_money_silver(gold);
                        bi.set_war_value(zhanli);
                        bi.set_role_name(role_name);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_occupation(gender);
                        if (vip_exp == 0) {
                                bi.set_is_vip(0);
                        } else {
                                bi.set_is_vip(1);
                        }
                        bi.set_vip_level(vip_level);
                        send_to_log_server_func_("role_logout", bi);
                }

                void bi::levelup(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t cur_level, uint32_t old_level, uint32_t last_levelup_time, uint32_t fl_time) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        auto now = system_clock::to_time_t(system_clock::now());
                        bi.set_before_level(old_level);
                        bi.set_after_level(cur_level);
                        bi.set_create_time(fl_time);
                        bi.set_cost_time(now - last_levelup_time);
                        send_to_log_server_func_("role_level_up", bi);
                }

                void bi::change_resource(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                         pd::resource_type type, int64_t old_count, int64_t cur_count, uint64_t league_id, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }

                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        if (type == pd::EXP) {
                                change_exp(username, yci, ip, role, type, old_count, cur_count, origin);
                        } else {
                                change_virtual_currency(username, yci, ip, role, type, old_count, cur_count, league_id, origin);
                        }
                }

                void bi::add_stuff(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint64_t league_id, pd::ce_origin origin, const string& item, int count,
                                   pd::resource_type cost_resource_type, int cost_resource_count) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_item_id(item);
                        bi.set_item_type(item);
                        bi.set_item_num(to_string(count));
                        bi.set_coin_type(to_string(static_cast<int>(cost_resource_type)));
                        bi.set_coin_num(to_string(cost_resource_count));
                        bi.set_mission_point("");
                        bi.set_task_point("");
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("itemGet", bi);
                }

                void bi::dec_stuff(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t league_id, uint64_t role, pd::ce_origin origin, const string& item, int count,
                                   uint32_t next_adventure, uint32_t cur_quest) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_item_id(item);
                        bi.set_item_type(item);
                        bi.set_item_num(to_string(count));
                        bi.set_mission_point(to_string(next_adventure));
                        bi.set_task_point(to_string(cur_quest));
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("itemUse", bi);
                }

                void bi::accept_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint64_t league_id, const string& quest_name) {
                        if (!send_bi_) {
                                return;
                        }
                        const auto& quest_ptt = PTTS_GET(quest, pttid);
                        auto type = pd::YQT_NORMAL;
                        if (quest_ptt._everyday_quest() > 0) {
                                type = pd::YQT_EVERYDAY;
                        } else if (quest_ptt._achievement_quest() > 0) {
                                type = pd::YQT_ACHIEVEMENT;
                        } else if (quest_ptt._activity_seven_days_quest()) {
                                type = pd::YQT_SEVEN_DAYS;
                        }

                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_task_id(to_string(pttid));
                        bi.set_task_name(quest_name);
                        bi.set_task_type(to_string(static_cast<int>(type)));
                        send_to_log_server_func_("task_begin", bi);
                }

                void bi::commit_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, const string& quest_name, uint32_t pttid, uint32_t duration) {
                        if (!send_bi_) {
                                return;
                        }
                        const auto& quest_ptt = PTTS_GET(quest, pttid);
                        auto type = pd::YQT_NORMAL;
                        if (quest_ptt._everyday_quest() > 0) {
                                type = pd::YQT_EVERYDAY;
                        } else if (quest_ptt._achievement_quest() > 0) {
                                type = pd::YQT_ACHIEVEMENT;
                        } else if (quest_ptt._activity_seven_days_quest() || quest_ptt._activity_seven_days_fuli_quest()) {
                                type = pd::YQT_SEVEN_DAYS;
                        }

                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_task_id(to_string(pttid));
                        bi.set_task_name(quest_name);
                        bi.set_task_type(to_string(static_cast<int>(type)));
                        bi.set_cost_time(duration);
                        send_to_log_server_func_("task_end", bi);
                }

                void bi::challenge_adventure(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                             uint64_t role, uint64_t league_id, uint32_t pttid, uint32_t duration, pd::result result, pd::ce_origin origin, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_mission_id(to_string(pttid));
                        const auto& adventure_ptt = PTTS_GET(adventure, pttid);
                        bi.set_mission_type(to_string(static_cast<int>(adventure_ptt._type() == pd::AT_ZHUXIAN ? pd::YMT_ZHUXIAN : pd::YMT_LIEZHUAN)));
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_duration(to_string(duration));
                        bi.set_is_succ(result == pd::OK ? "1" : "0");
                        bi.set_reason(result == pd::OK ? "" : pd::result_Name(result));
                        for (const auto& i : event_res.add_items()) {
                                bi.set_item_id(yunying_serialize_item(i.pttid()));
                                bi.set_before_num(i.old_count());
                                bi.set_after_num(i.cur_count());
                        }
                        for (const auto& i : event_res.change_resources()) {
                                if (i.type() == pd::GOLD || i.type() == pd::DIAMOND) {
                                        bi.set_money_type(to_string(yunying_serialize_resource(i.type())));
                                        bi.set_before_money(i.old_count());
                                        bi.set_after_money(i.cur_count());
                                }
                        }
                        send_to_log_server_func_("mission", bi);
                }

                void bi::activity_login_days(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t day) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_activity_id(to_string(static_cast<uint32_t>(pd::YAI_LOGIN_DAYS)));
                        bi.set_activity_type(to_string(day));
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("activityEnd", bi);
                }

                void bi::activity_online_time(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t day, int idx) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_activity_id(to_string(static_cast<uint32_t>(pd::YAI_LOGIN_DAYS)));
                        bi.set_activity_type(to_string(day));
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("activityEnd", bi);
                }

                void bi::activity_seven_days_fuli(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, uint32_t day, uint32_t idx, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_activity_id(to_string(static_cast<uint32_t>(pd::YAI_SEVEN_DAYS_FULI)));
                        bi.set_activity_type(to_string(begin_day + day));
                        bi.set_role_level(level);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(pd::CO_ACTIIVTY_SEVEN_DAY_FULI)));
                        for (const auto& i : event_res.add_items()) {
                                bi.set_item_id(yunying_serialize_item(i.pttid()));
                                bi.set_before_num(i.old_count());
                                bi.set_after_num(i.cur_count());
                        }
                        for (const auto& i : event_res.change_resources()) {
                                if (i.type() == pd::GOLD || i.type() == pd::DIAMOND) {
                                        bi.set_money_type(to_string(yunying_serialize_resource(i.type())));
                                        bi.set_before_money(i.old_count());
                                        bi.set_after_money(i.cur_count());
                                }
                        }
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("activityEnd", bi);
                }

                void bi::activity_seven_days_quest(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, uint32_t day, uint32_t idx, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_activity_id(to_string(static_cast<uint32_t>(pd::YAI_SEVEN_DAYS_QUEST)));
                        bi.set_activity_type(to_string(begin_day + day));
                        bi.set_role_level(level);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(pd::CO_ACTIIVTY_SEVEN_DAY_REWARD)));
                        for (const auto& i : event_res.add_items()) {
                                bi.set_item_id(yunying_serialize_item(i.pttid()));
                                bi.set_before_num(i.old_count());
                                bi.set_after_num(i.cur_count());
                        }
                        for (const auto& i : event_res.change_resources()) {
                                if (i.type() == pd::GOLD || i.type() == pd::DIAMOND) {
                                        bi.set_money_type(to_string(yunying_serialize_resource(i.type())));
                                        bi.set_before_money(i.old_count());
                                        bi.set_after_money(i.cur_count());
                                }
                        }
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("activityEnd", bi);
                }

                void bi::activity_seven_days_reward(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t begin_day, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_activity_id(to_string(static_cast<uint32_t>(pd::YAI_SEVEN_DAYS_REWARD)));
                        bi.set_activity_type(to_string(begin_day));
                        bi.set_role_level(level);
                        for (const auto& i : event_res.add_items()) {
                                bi.set_item_id(yunying_serialize_item(i.pttid()));
                                bi.set_before_num(i.old_count());
                                bi.set_after_num(i.cur_count());
                        }
                        for (const auto& i : event_res.change_resources()) {
                                if (i.type() == pd::GOLD || i.type() == pd::DIAMOND) {
                                        bi.set_money_type(to_string(yunying_serialize_resource(i.type())));
                                        bi.set_before_money(i.old_count());
                                        bi.set_after_money(i.cur_count());
                                }
                        }
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("activityEnd", bi);
                }

                void bi::cdkey(const string& username, const string& rolename, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const string& code, const function<void(pd::result, const pd::event_array&, const string&)>& cb) {
                        ASSERT(st_);
                        st_->async_call(
                                [this, username, rolename, yci, ip, role, level, code, cb] {
                                        auto ptt = PTTS_GET_COPY(options, 1);

                                        map<string, string> params;
                                        params["company"] = "yq";
                                        params["appId"] = ptt.yunying().app_id();
                                        params["roleId"] = to_string(role);
                                        params["channelId"] = to_string(yci.channel());
                                        params["roleLevel"] = to_string(level);
                                        params["serverId"] = to_string(ptt.scene_info().server_id());
                                        params["roleName"] = rolename;
                                        params["code"] = code;
                                        params["sign"] = yunying_calc_sign(params, ptt.yunying().app_secret());

                                        const auto& addr = ptt.yunying().gm_addr();
                                        post(addr + "/api/cdkey", params,
                                             [this, cb, exchange_code = code, yci, username, role, ip] (auto result, const auto& ptree) {
                                                     if (result != pd::OK) {
                                                             cb(result, pd::event_array(), "");
                                                     } else {
                                                             auto code = ptree.template get<string>("code");
                                                             if (code != "10000") {
                                                                     auto msg = ptree.template get<string>("msg");
                                                                     SBI_DLOG << "cdkey failed: " << code << " " << msg;
                                                                     cb(pd::INTERNAL_ERROR, pd::event_array(), msg);
                                                             } else {
                                                                     pd::event_array events;
                                                                     for (const auto& i : ptree.get_child("items")) {
                                                                             auto pttid = i.second.template get<string>("id");
                                                                             auto count = i.second.template get<string>("num");
                                                                             auto item_name = i.second.template get<string>("name");
                                                                             auto *event = events.add_events();
                                                                             if (yunying_item_to_add_event(pttid, *event)) {
                                                                                     event->add_arg(count);
                                                                             }
                                                                             this->gift_exchange(username, yci, ip, role, pttid, item_name, exchange_code);
                                                                     }
                                                                     cb(result, events, "");
                                                             }
                                                     }
                                             });
                                });
                }

                void bi::challenge_arena(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                             uint64_t role, uint64_t league_id, uint32_t duration, pd::result result) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_play_id(to_string(static_cast<uint32_t>(pd::YPI_ARENA)));
                        bi.set_play_type("1");
                        send_to_log_server_func_("playBegin", bi);

                        bi.set_duration(to_string(duration));
                        bi.set_is_succ(result == pd::OK ? "1" : "0");
                        bi.set_reason("");
                        send_to_log_server_func_("playEnd", bi);
                }

                void bi::league_issue_lucky_bag(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t lucky_bag_id) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_play_id(to_string(static_cast<uint32_t>(pd::YPI_LEAGUE_LUCKY_ISSUE_BAG)));
                        bi.set_play_type(to_string(lucky_bag_id));
                        send_to_log_server_func_("playBegin", bi);
                }

                void bi::league_open_lucky_bag(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t lucky_bag_id) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_play_id(to_string(static_cast<uint32_t>(pd::YPI_LEAGUE_LUCKY_OPEN_BAG)));
                        bi.set_play_type(to_string(lucky_bag_id));
                        bi.set_duration("0");
                        bi.set_is_succ("1");
                        bi.set_reason("");
                        send_to_log_server_func_("playEnd", bi);
                }

                void bi::challenge_league_campaign(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                                   uint64_t role, uint64_t league_id, uint32_t pttid, pd::result result) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_mission_id(to_string(pttid));
                        bi.set_mission_type(to_string(static_cast<int>((pd::YMT_LEAGUE_CAMPAGIN))));
                        bi.set_duration("0");
                        bi.set_is_succ(result == pd::OK ? "1" : "0");
                        bi.set_reason(result == pd::OK ? "" : pd::result_Name(result));
                        send_to_log_server_func_("mission", bi);
                }

                void bi::xinshou(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint64_t league_id, pd::ce_origin origin, const string& role_name, uint32_t level, uint32_t zhanli, uint32_t exp, uint32_t diamond) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_scene_id(to_string(pttid));
                        bi.set_role_name(role_name);
                        bi.set_role_level(level);
                        bi.set_war_value(zhanli);
                        bi.set_role_exp(exp);
                        bi.set_money_gold(diamond);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("new_user_guide", bi);
                }

                void bi::challenge_lieming(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, pd::ce_origin origin, uint32_t level, uint32_t zhanli, uint32_t pttid, uint32_t duration, pd::yunying_play_id type, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_play_id(to_string(static_cast<uint32_t>(type)));
                        bi.set_play_type("1");
                        send_to_log_server_func_("layBegin", bi);

                        bi.set_duration(to_string(duration));
                        bi.set_is_succ("1");
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        bi.set_war_value(zhanli);
                        for (const auto& i : event_res.add_items()) {
                                bi.set_item_id(yunying_serialize_item(i.pttid()));
                                bi.set_before_num(i.old_count());
                                bi.set_after_num(i.cur_count());
                        }
                        send_to_log_server_func_("layEnd", bi);
                }


                void bi::change_virtual_currency(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, pd::resource_type type, int old_count, int cur_count, uint64_t league_id, pd::ce_origin origin){
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_money_type(pd::resource_type_Name(type));
                        bi.set_before_money(old_count);
                        bi.set_after_money(cur_count);
                        bi.set_change_money(abs(cur_count - old_count));
                        auto change_type = (cur_count - old_count) >= 0 ? 1 : -1;
                        bi.set_change_type(change_type);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("money_stream", bi);
                }

                void bi::order_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                      uint32_t pttid, uint32_t price, uint64_t order) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_pay_amount(price);
                        bi.set_pay_type("");
                        bi.set_goods_type("");
                        bi.set_goods_id(to_string(pttid));
                        bi.set_cp_order_id(to_string(order));
                        send_to_log_server_func_("order_stream", bi);
                }

                void bi::recharge(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                  uint32_t pttid, uint32_t price, const string& yy_order,
                                  uint64_t order, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_pay_amount(price);
                        bi.set_pay_type("");
                        bi.set_goods_type("");
                        bi.set_goods_id(to_string(pttid));
                        bi.set_order_id(yy_order);
                        bi.set_cp_order_id(to_string(order));
                        uint32_t diamond = 0;
                        for (const auto& i : event_res.change_resources()) {
                                if (i.type() == pd::DIAMOND) {
                                        diamond += (i.cur_count() - i.old_count());
                                }
                        }
                        if (diamond > 0) {
                                bi.set_change_money(diamond);
                        }
                        send_to_log_server_func_("delivery_stream", bi);
                }

                void bi::stuff_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                      uint64_t league_id, pd::ce_origin origin, const string& pttid, uint32_t old_count, uint32_t cur_count) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_prop_type("");
                        bi.set_prop_id(pttid);
                        bi.set_before_prop(old_count);
                        bi.set_after_prop(cur_count);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_change_prop(cur_count > old_count ? cur_count - old_count : old_count - cur_count);
                        bi.set_change_type(cur_count > old_count ? 1 : -1);
                        bi.set_scene_id("");
                        send_to_log_server_func_("prop_stream", bi);
                }

                void bi::yunying_add_bi_info(pd::yunying_bi_info* bi, pc::good::good_type type, uint32_t good_id, uint32_t old_count, uint32_t cur_count) {
                        bi->set_goods_type(pc::good::good_type_Name(type));
                        bi->set_goods_id(to_string(good_id));
                        bi->set_before_num(old_count);
                        bi->set_after_num(cur_count);
                        bi->set_change_num(cur_count - old_count);
                }

                void bi::yunying_bi_parse_event_res(pd::yunying_bi_info* bi, pc::good::good_type type, uint32_t shop_id, uint32_t good_id, const pd::event_res& event_res) {
                        bi->set_shop_type(to_string(shop_id));
                        for (const auto& i : event_res.change_resources()) {
                                bi->set_money_type(pd::resource_type_Name(i.type()));
                                bi->set_change_money(i.old_count() - i.cur_count());
                                bi->set_before_money(i.old_count());
                                bi->set_after_money(i.cur_count());
                        }
                        for (const auto& i : event_res.add_items()) {
                                yunying_add_bi_info(bi, type, good_id, i.old_count(), i.cur_count());
                        }
                        for (auto i = 0; i < event_res.spine_collections_size(); ++i) {
                                yunying_add_bi_info(bi, type, good_id, 0, 1);
                        }
                        for (auto i = 0; i < event_res.huanzhuang_collections_size(); ++i) {
                                yunying_add_bi_info(bi, type, good_id, 0, 1);
                        }
                        for (auto i = 0; i < event_res.add_equipments_size(); ++i) {
                                yunying_add_bi_info(bi, type, good_id, 0, 1);
                        }
                        for (auto i = 0; i < event_res.mansion_add_hall_quest_size(); ++i) {
                                yunying_add_bi_info(bi, type, good_id, 0, 1);
                        }
                }

                void bi::shop_stream(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                     uint64_t league_id, pd::yunying_bi_info bi, pc::good::good_type type,
                                     uint32_t shop_id, uint32_t good_id, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        yunying_bi_parse_event_res(&bi, type, shop_id, good_id, event_res);
                        send_to_log_server_func_("shop_stream", bi);
                }

                void bi::gift_exchange(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                       const string& pttid, const string& item_name, const string& code){
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_gift_id(pttid);
                        bi.set_gift_name(item_name);
                        bi.set_exchange_code(code);
                        send_to_log_server_func_("gift_exchange", bi);
                }

                void bi::change_exp(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                    pd::resource_type type, int old_count, int cur_count, pd::ce_origin origin){
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_before_exp(old_count);
                        bi.set_after_exp(cur_count);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("role_exp", bi);
                }

                void bi::game_play_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role,
                                         const string& role_name, uint32_t level, uint32_t zhanli, const string& play_id,
                                         const pd::event_res& event_res, uint64_t league_id, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_role_name(role_name);
                        bi.set_role_level(level);
                        bi.set_war_value(zhanli);
                        bi.set_play_id(play_id);
                        bi.set_play_info(pb2json(event_res));
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        send_to_log_server_func_("game_play_start", bi);
                }

                void bi::game_play_end(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                       uint64_t role, const string& role_name, uint32_t level, uint32_t zhanli,
                                       const string& play_id, const pd::event_res& event_res, uint64_t league_id, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_role_name(role_name);
                        bi.set_role_level(level);
                        bi.set_war_value(zhanli);
                        bi.set_play_id(play_id);
                        bi.set_is_sucess(1);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_play_info(pb2json(event_res));
                        send_to_log_server_func_("game_play_end", bi);
                }

                void bi::add_actor(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                uint64_t role, uint32_t level, uint32_t actor, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        bi.set_partner_id(actor);
                        bi.set_role_id(to_string(role));
                        send_to_log_server_func_("partner_get", bi);
                }

                void bi::actor_add_star(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                uint64_t role, uint32_t level, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_id(to_string(role));
                        bi.set_role_level(level);
                        for (const auto& i : event_res.actor_add_star()) {
                                bi.set_partner_id(i.pttid());
                                bi.set_before_score(to_string(i.old_star()));
                                bi.set_after_score(to_string(i.cur_star()));
                        }
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_partner_intimacy", bi);
                }

                void bi::actor_levelup(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, pd::ce_origin origin, const pd::event_res_actor_levelup& actor_levelup) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        bi.set_partner_id(actor_levelup.pttid());
                        bi.set_before_level(actor_levelup.old_level());
                        bi.set_after_level(actor_levelup.cur_level());
                        send_to_log_server_func_("partner_level_up", bi);

                }

                void bi::actor_add_pin(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, pd::ce_origin origin,const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        for (const auto& i : event_res.actor_add_pin()) {
                                bi.set_partner_id(i.pttid());
                                bi.set_before_score(to_string(GUANPIN_HIGHEST_GPIN - i.old_pin()));
                                bi.set_after_score(to_string(GUANPIN_HIGHEST_GPIN - i.cur_pin()));
                        }
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_partner_gao_feng", bi);
                }

                void bi::actor_add_step(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        for (const auto& i : event_res.actor_add_step()) {
                                bi.set_partner_id(i.pttid());
                                bi.set_before_score(to_string(i.old_step()));
                                bi.set_after_score(to_string(i.cur_step()));
                        }
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_partner_xue_shi", bi);
                }

                void bi::equipment_merge(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, uint32_t pttid, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        for (const auto& i : event_res.equipment_levelup()) {
                                bi.set_soul_id(to_string(pttid));
                                bi.set_before_level(i.old_level());
                                bi.set_after_level(i.cur_level());
                        }
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_soul_strengthen", bi);
                }

                void bi::equipment_compose(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        for (const auto& i : event_res.add_equipments()) {
                                bi.set_before_soul_id("0");
                                bi.set_soul_id(to_string(i.pttid()));
                        }
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_soul_compose", bi);
                }

                void bi::equipment_xilian(const string& username, const pd::yunying_client_info& yci, const string& ip,
                                   uint64_t role, uint32_t level, uint32_t befor_score, uint32_t after_score, 
                                   uint64_t equipment, const pd::event_res& event_res) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_soul_id(to_string(equipment));
                        pd::dec_item_array dec_items;
                        for (const auto& i : event_res.dec_items()) {
                                auto *dec_item = dec_items.add_dec_items();
                                dec_item->set_pttid(stoul(yunying_serialize_item(i.pttid())));
                                dec_item->set_count(i.old_count() - i.cur_count());
                        }
                        bi.set_cost_prop(pb2json(dec_items));
                        send_to_log_server_func_("jxwy_soul_refined", bi);
                }

                void bi::activity_continue_recharge_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                void bi::activity_continue_recharge_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_limit_play_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                void bi::activity_limit_play_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_leiji_recharge_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                void bi::activity_leiji_recharge_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_leiji_consume_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                 void bi::activity_leiji_consume_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_festival_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                void bi::activity_festival_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_prize_wheel_spine(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint64_t league_id, uint32_t activity, pd::admin_activity_type type, const pd::event_res_array& event_array, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        bi.set_ext(pb2json(event_array));
                        send_to_log_server_func_("operation_activity", bi);
                }

                void bi::activity_tea_party(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, uint64_t league_id, uint32_t activity, pd::admin_activity_type type, const pd::event_res& event_res, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        bi.set_ext(pb2json(event_res));
                        send_to_log_server_func_("operation_activity", bi);
                }

                void bi::activity_prize_wheel_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_discount_goods_start(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_begin", bi);
                }

                void bi::activity_discount_goods_stop(const string& username, const pd::yunying_client_info& yci, const string& ip, uint32_t activity, pd::admin_activity_type type, uint32_t duration, uint64_t role, uint32_t level) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_role_level(level);
                        bi.set_activity_type(to_string(static_cast<uint32_t>(type)));
                        bi.set_activity_id(to_string(activity));
                        bi.set_is_succ("1");
                        bi.set_cost_time(duration * DAY * HOUR * MINUTE);
                        bi.set_act_time(system_clock::to_time_t(system_clock::now()));
                        send_to_log_server_func_("activity_end", bi);
                }

                void bi::activity_qianzhuang_buy(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        for (const auto& i : event_res.change_resources()) {
                                bi.set_money_type(pd::resource_type_Name(i.type()));
                                bi.set_change_money(i.old_count() - i.cur_count());
                                bi.set_before_money(i.old_count());
                                bi.set_after_money(i.cur_count());
                        }
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        send_to_log_server_func_("activity_end", bi);
                }
                
                void bi::activity_play(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint32_t pttid, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin, uint32_t play_id) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, 0);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        bi.set_play_type(to_string(play_id));
                        bi.set_start_info(pb2json(event_res));
                        send_to_log_server_func_("game_play_start", bi);
                }

                void bi::guanpin_promote(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, const pd::event_res& event_res, pd::ce_origin origin) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(origin)));
                        bi.set_role_level(level);
                        bi.set_play_type(to_string(static_cast<uint32_t>(pd::YPI_GUANPIN_PROMOTE)));
                        bi.set_start_info(pb2json(event_res));
                        send_to_log_server_func_("game_play_start", bi);
                }
                
                void bi::battle_set_formation(const string& username, const pd::yunying_client_info& yci, const string& ip, uint64_t role, uint64_t league_id, uint32_t level, uint32_t formation_idx) {
                        if (!send_bi_) {
                                return;
                        }
                        pd::yunying_bi_info bi;
                        set_common_bi_msg(&bi, yci, username, ip, role, league_id);
                        bi.set_reason_id(to_string(static_cast<uint32_t>(pd::CO_BATTLE_SET_FORMATION)));
                        bi.set_role_level(level);
                        bi.set_play_type(to_string(static_cast<uint32_t>(pd::YPI_GUANPIN_PROMOTE)));
                        bi.set_formation_id(formation_idx);
                        send_to_log_server_func_("game_play_start", bi);
                }
        }
}
*/