#include "utils/service_thread.hpp"
#include "db/connector.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/json2pb.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "proto/data_log.pb.h"
#include "proto/config_options.pb.h"
#include "log.hpp"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <unistd.h> //sleep

using namespace std;
using namespace nora;
namespace bpo = boost::program_options;

shared_ptr<service_thread> main_st;
shared_ptr<db::connector> gamedb;
shared_ptr<db::connector> logdb;
map<uint64_t, vector<pd::log_array>> gid2logs;
map<uint32_t, vector<pd::log_array>> type2logs;

int page_size = 50;
int cur_page = 0;

struct {
        vector<string> gids;
        vector<string> usernames;
        vector<string> rolenames;
} role_selector_params;

struct {
        vector<string> gids;
        vector<string> types;
        uint32_t start_time = 0;
        uint32_t end_time = system_clock::to_time_t(system_clock::now());
} log_selector_params;

struct {
        string ipport;
        string user;
        string password;
        string game_database;
        string log_database;
} db_params;

void select_logs_by_gid(uint64_t gid, bool stop);
void select_logs_by_type(uint32_t type, bool stop);
void select_logs_by_gid_and_type(uint64_t gid, uint32_t type, bool stop);

uint32_t standard_date_to_timestamp(string date) {
        tm tm_;
        char buf[128]= {0};
        strcpy(buf, date.c_str());
        // switch str to tm
        if (strptime(buf, "%Y-%m-%d %H:%M:%S", &tm_) == NULL) {
                cout << "参数错误: 查询日志的时间参数格式不正确，请修改！\n";
                cout << "date : " << date << "\n";
                return 1;
        }
        // switch tm to time_t
        return static_cast<uint32_t>(mktime(&tm_));
}

void handle_finished_db_msg(const shared_ptr<db::message>& db_msg, bool stop = true, uint64_t gid = 0) {
        if (db_msg->procedure() == "find_role_by_gid") {
                if (db_msg->results().empty()) {
                        cout << "role NOT_FOUND\n";
                } else {
                        const auto& result = db_msg->results().front();
                        pd::role role_data;
                        role_data.ParseFromString(boost::any_cast<string>(result[2]));
                        auto str_data = pb2json(role_data);
                        TOOL_TLOG << "role data:\n" << str_data;
                        cout << "role find by gid done\n";
                }
                if (stop) {
                        main_st->stop();
                        return;
                }

        } else if (db_msg->procedure() == "find_role") {
                if (db_msg->results().empty()) {
                        cout << "role NOT_FOUND" << "\n";
                } else {
                        const auto& result = db_msg->results().front();
                        pd::role role_data;
                        role_data.ParseFromString(boost::any_cast<string>(result[2]));
                        auto str_data = pb2json(role_data);
                        TOOL_TLOG << "role data:\n" << str_data;
                        cout << "role find by username done\n";
                }
                if (stop) {
                        main_st->stop();
                        return;
                }
        } else if (db_msg->procedure() == "get_roles") {
                if (db_msg->results().empty()) {
                        cout << "role NOT_FOUND" << "\n";
                } else {
                        const auto& result = db_msg->results().front();
                        pd::role role_data;
                        role_data.ParseFromString(boost::any_cast<string>(result[3]));
                        auto str_data = pb2json(role_data);
                        TOOL_TLOG << "role data:\n" << str_data;
                        cout << "role get roles done\n";
                }
                if (stop) {
                        main_st->stop();
                        return;
                }

        } else if (db_msg->procedure() == "find_role_by_name") {
                if (db_msg->results().empty()) {
                        cout << "role NOT_FOUND\n";
                }
                const auto& result = db_msg->results().front();
                pd::role role_data;
                role_data.ParseFromString(boost::any_cast<string>(result[2]));
                auto str_data = pb2json(role_data);
                TOOL_TLOG << "role data:\n" << str_data;
                cout << "role find by rolename done\n";
                if (stop) {
                        main_st->stop();
                        return;
                }

        } else if (db_msg->procedure() == "find_log" ||
                   db_msg->procedure() == "find_logs_by_gid_and_one_type" ||
                   db_msg->procedure() == "find_logs_by_type") {
                if (db_msg->results().empty()) {
                        if (stop) {
                                for (const auto& i : gid2logs) {
                                        TOOL_DLOG << "role :" << i.first;
                                        for (const auto& j : i.second) {
                                                for (const auto& k : j.logs()) {
                                                        auto time = clock::instance().time_str(static_cast<time_t>(k.time()));
                                                        auto str_data = pb2json(k);
                                                        TOOL_TLOG <<  time << " log data:\n" << str_data;
                                                }
                                        }
                                }
                                for (const auto& i : type2logs) {
                                        TOOL_DLOG << "type :" << i.first;
                                        for (const auto& j : i.second) {
                                                for (const auto& k : j.logs()) {
                                                        auto time = clock::instance().time_str(static_cast<time_t>(k.time()));
                                                        auto str_data = pb2json(k);
                                                        TOOL_TLOG <<  time << " log data:\n" << str_data;
                                                }
                                        }
                                }
                                cout << "log find done\n";
                                main_st->stop();
                                return;
                        }
                } else {
                        pd::log_array logs;
                        uint32_t type;
                        for (const auto& result : db_msg->results()) {
                                auto *log = logs.add_logs();
                                log->ParseFromString(boost::any_cast<string>(result[2]));
                                type = boost::any_cast<uint32_t>(result[1]);
                                log->set_type(static_cast<pd::log::log_type>(type));
                                auto time = boost::any_cast<uint32_t>(result[0]);
                                log->set_time(time);
                        }
                        if (gid != 0) {
                                if (gid2logs.count(gid) <= 0) {
                                        vector<pd::log_array> vec;
                                        vec.push_back(logs);
                                        gid2logs[gid] = vec;
                                } else {
                                        ASSERT(gid2logs.count(gid) > 0);
                                        gid2logs.at(gid).push_back(logs);
                                }
                        } else {
                                if (type2logs.count(type) <= 0) {
                                        vector<pd::log_array> vec;
                                        vec.push_back(logs);
                                        type2logs[type] = vec;
                                } else {
                                        ASSERT(type2logs.count(type) > 0);
                                        type2logs.at(type).push_back(logs);
                                }
                        }
                        if (db_msg->procedure() == "find_logs_by_gid_and_one_type") {
                                select_logs_by_gid_and_type(gid, boost::any_cast<uint32_t>(db_msg->results()[0][1]), stop);
                        } else if (db_msg->procedure() == "find_log") {
                                select_logs_by_gid(gid, stop);
                        } else if (db_msg->procedure() == "find_logs_by_type") {
                                select_logs_by_type(boost::any_cast<uint32_t>(db_msg->results()[0][1]), stop);
                        }
                }
        }
}

void select_logs_by_gid(uint64_t gid, bool stop) {
        page_size = 50;
        auto db_msg = make_shared<db::message>("find_log",
                                               db::message::req_type::rt_select,
                                               [gid, stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop, gid);
                                               });
        db_msg->push_param(gid);
        db_msg->push_param(log_selector_params.start_time);
        db_msg->push_param(log_selector_params.end_time);
        db_msg->push_param(cur_page * page_size);
        db_msg->push_param((cur_page + 1) * page_size);
        logdb->push_message(db_msg, main_st);
        cur_page += 1;
}

void select_logs_by_gid_and_type(uint64_t gid, uint32_t type, bool stop) {
        page_size = 50;
        auto db_msg = make_shared<db::message>("find_logs_by_gid_and_one_type",
                                               db::message::req_type::rt_select,
                                               [gid, stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop, gid);
                                               });
        db_msg->push_param(gid);
        db_msg->push_param(type);
        db_msg->push_param(log_selector_params.start_time);
        db_msg->push_param(log_selector_params.end_time);
        db_msg->push_param(cur_page * page_size);
        db_msg->push_param((cur_page + 1) * page_size);
        logdb->push_message(db_msg, main_st);
        cur_page += 1;
        
}

void select_logs_by_type(uint32_t type, bool stop) {
        page_size = 50;
        auto db_msg = make_shared<db::message>("find_logs_by_type",
                                               db::message::req_type::rt_select,
                                               [stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop);
                                               });
        db_msg->push_param(type);
        db_msg->push_param(log_selector_params.start_time);
        db_msg->push_param(log_selector_params.end_time);
        db_msg->push_param(cur_page * page_size);
        db_msg->push_param((cur_page + 1) * page_size);
        logdb->push_message(db_msg, main_st);
        cur_page += 1;
        
}

void select_role_by_gid(uint64_t gid, bool stop) {
        auto db_msg = make_shared<db::message>("find_role_by_gid",
                                               db::message::req_type::rt_select,
                                               [stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop);
                                               });
        db_msg->push_param(gid);
        gamedb->push_message(db_msg, main_st);
}

void select_all_role(bool stop) {
        auto db_msg = make_shared<db::message>("get_roles",
                                               db::message::req_type::rt_select,
                                               [stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop);
                                               });
        db_msg->push_param(0);
        db_msg->push_param(100);
        gamedb->push_message(db_msg, main_st);
}

void select_role_by_username(const string& username, bool stop) {
        auto db_msg = make_shared<db::message>("find_role",
                                               db::message::req_type::rt_select,
                                               [stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop);
                                               });
        db_msg->push_param(username);
        gamedb->push_message(db_msg, main_st);
}

void select_role_by_rolename(const string& rolename, bool stop) {
        auto db_msg = make_shared<db::message>("find_role_by_name",
                                               db::message::req_type::rt_select,
                                               [stop] (const auto& msg) {
                                                       handle_finished_db_msg(msg, stop);
                                               });
        db_msg->push_param(rolename);
        gamedb->push_message(db_msg, main_st);
}

int main(int argc, char *argv[]) {
        //sleep(30);
        srand(time(0));

        bpo::options_description desc("options");
        desc.add_options()
                ("help,h", "produce help message")
                ("data_type", bpo::value<string>(), "data type, log or game")
                ("role_gid", bpo::value<string>(), "role gids")
                ("username", bpo::value<string>(), "role usernames")
                ("rolename", bpo::value<string>(), "role rolenames")
                ("log_start_time", bpo::value<string>(), "log start time, as '2012-01-01 14:00:00'")
                ("log_end_time", bpo::value<string>(), "log end time, default(now), as '2012-01-01 16:00:00'")
                ("log_types", bpo::value<string>(), "log types, as 1,2,3 ...")
                ("ipport", bpo::value<string>(), "mysql ipport")
                ("user", bpo::value<string>(), "mysql user")
                ("password", bpo::value<string>(), "mysql password")
                ("game_db", bpo::value<string>(), "mysql game_db")
                ("log_db", bpo::value<string>(), "mysql log_db")
                ;

        bpo::variables_map vm;
        try {
                bpo::store(bpo::command_line_parser(argc, argv).options(desc).run(), vm);
                bpo::notify(vm);
        } catch (const exception& e) {
                cerr << e.what() << endl;
                return 1;
        }

        if (vm.count("help")) {
                cout << desc << "\n";
                return 0;
        }
        if (vm.count("data_type") <= 0) {
                cout << "查询器异常: data_type参数配置错误，请检查！\n";
                return 1;
        }

        main_st = make_shared<service_thread>("db_selector");
        clock::instance().start(main_st);

        init_log("selector", pc::options::TRACE);
        PTTS_LOAD(options);
        if (!PTTS_HAS(options, 1)) {
                cout << " 没有配置configure\n";
                return 1;
        }
        if (vm.count("game_db") > 0 || vm.count("log_db") > 0) {
                try {
                        if (vm.count("game_db") > 0) {
                                db_params.game_database = vm["game_db"].as<string>();
                        } else {
                                db_params.log_database = vm["log_db"].as<string>();
                        }
                        db_params.ipport = vm["ipport"].as<string>();
                        db_params.user = vm["user"].as<string>();
                        db_params.password = vm["password"].as<string>();
                } catch (const exception& e) {
                        cout << " 数据库参数配置错误，请检查!\n";
                        return 1;
                }
        } else {
                auto ptt = PTTS_GET_COPY(options, 1);
                db_params.ipport = ptt.game_db().ipport();
                db_params.user = ptt.game_db().user();
                db_params.password = ptt.game_db().password ();
                db_params.game_database = ptt.game_db().database();
                db_params.log_database = ptt.log_db().database();
        }

        if (vm["data_type"].as<string>() == "game") {
                if (!(vm.count("role_gid") || vm.count("username") || vm.count("rolename")) || vm.count("log_types")) {
                        cout << "查询器异常: 参数配置错误，请检查！\n";
                        return 1;
                }
                if (vm.count("role_gid") > 0) {
                        role_selector_params.gids = split_string(vm["role_gid"].as<string>(), ',');

                }
                if (vm.count("username") > 0) {
                        role_selector_params.usernames = split_string(vm["username"].as<string>(), ',');
                }
                if (vm.count("rolename") > 0) {
                        role_selector_params.rolenames = split_string(vm["rolename"].as<string>(), ',');
                }

                try {
                        gamedb = make_shared<db::connector>(db_params.ipport,
                                                            db_params.user,
                                                            db_params.password,
                                                            db_params.game_database);
                        gamedb->start();
                } catch (const exception& e) {
                        cout << "game数据库参数配置异常\n" << e.what();
                        return 1;
                }
        } else if (vm["data_type"].as<string>() == "log") {
                if (vm.count("role_gid") <= 0 && vm.count("log_types") <= 0) {
                        cout << "查询器异常: 玩家查询条件不全，请检查！\n";
                        return 1;
                }
                if (vm.count("role_gid") > 0) {
                        log_selector_params.gids = split_string(vm["role_gid"].as<string>(), ',');
                }
                if (vm.count("log_start_time") > 0) {
                        log_selector_params.start_time = standard_date_to_timestamp(vm["log_start_time"].as<string>());
                }
                if (vm.count("log_end_time") > 0) {
                        log_selector_params.end_time = standard_date_to_timestamp(vm["log_end_time"].as<string>());
                }
                if (vm.count("log_types") > 0) {
                        log_selector_params.types = split_string(vm["log_types"].as<string>(), ',');
                }
                if (log_selector_params.start_time > log_selector_params.end_time) {
                        cout << "参数错误: 查询日志的时间参数不合法，请修改！\n";
                        return 1;
                }

                try {
                        logdb = make_shared<db::connector>(db_params.ipport,
                                                           db_params.user,
                                                           db_params.password,
                                                           db_params.log_database);
                        logdb->start();
                } catch (const exception& e) {
                        cout << "log数据库参数配置异常\n" << e.what();
                        return 1;
                }
        } else {
                cout << "参数错误: data_type配置不合理，请检查！\n";
                return 1;
        }

        main_st->async_call(
                [vm] {
                        shared_ptr<db::message> db_msg;
                        if (vm["data_type"].as<string>() == "game") {
                                if (role_selector_params.gids.size() > 0) {
                                        auto stop = false;
                                        auto gids = role_selector_params.gids;
                                        for (const auto& i : gids) {
                                                if (role_selector_params.gids.size() - 1 == 0) {
                                                        stop = true;
                                                }
                                                if (stoul(i) != 0) {
                                                        select_role_by_gid(stoul(i), stop);
                                                } else {
                                                        select_all_role(stop);
                                                }
                                                role_selector_params.gids.erase(role_selector_params.gids.begin(), role_selector_params.gids.begin() + 1);
                                        }
                                } else if (role_selector_params.usernames.size() > 0) {
                                        auto stop = false;
                                        auto usernames = role_selector_params.usernames;
                                        for (const auto& i : usernames) {
                                                if (role_selector_params.usernames.size() - 1 == 0) {
                                                        stop = true;
                                                }
                                                select_role_by_username(i, stop);
                                                role_selector_params.usernames.erase(role_selector_params.usernames.begin(), role_selector_params.usernames.begin() + 1);
                                        }
                                } else if (role_selector_params.rolenames.size() > 0) {
                                        auto stop = false;
                                        auto rolenames = role_selector_params.rolenames;
                                        for (const auto& i : rolenames) {
                                                if (role_selector_params.rolenames.size() - 1 == 0) {
                                                        stop = true;
                                                }
                                                select_role_by_rolename(i, stop);
                                                role_selector_params.rolenames.erase(role_selector_params.rolenames.begin(), role_selector_params.rolenames.begin() + 1);
                                        }
                                }
                        }
                        if (vm["data_type"].as<string>() == "log") {
                                if (log_selector_params.gids.size() > 0) {
                                        if (vm.count("log_types") <= 0) {
                                                auto stop = false;
                                                auto gids = log_selector_params.gids;
                                                for (const auto& i : gids) {
                                                        if (log_selector_params.gids.size() - 1 == 0) {
                                                                stop = true;
                                                        }
                                                        cur_page = 0;
                                                        select_logs_by_gid(stoul(i), stop);
                                                        log_selector_params.gids.erase(log_selector_params.gids.begin(), log_selector_params.gids.begin() + 1);
                                                }
                                        } else {
                                                auto stop = false;
                                                auto gids = log_selector_params.gids;
                                                auto types = log_selector_params.types;
                                                for (const auto& i : gids) {
                                                        for (const auto& j : types) {
                                                                if (log_selector_params.gids.size() - 1 == 0) {
                                                                        stop = true;
                                                                }
                                                                cur_page = 0;
                                                                select_logs_by_gid_and_type(stoul(i), stoul(j), stop);
                                                        }
                                                        log_selector_params.gids.erase(log_selector_params.gids.begin(), log_selector_params.gids.begin() + 1);
                                                }
                                        }
                                } else {
                                        ASSERT(vm.count("log_types") > 0);
                                        auto stop = false;
                                        auto types = log_selector_params.types;
                                        for (const auto& i : types) {
                                                if (log_selector_params.types.size() - 1 == 0) {
                                                        stop = true;
                                                }
                                                cur_page = 0;
                                                select_logs_by_type(stoul(i), stop);
                                                log_selector_params.types.erase(log_selector_params.types.begin(), log_selector_params.types.begin() + 1);
                                        }
                                }
                        }
                });

        main_st->run();

        if (gamedb) {
                gamedb->stop();
        }
        if (logdb) {
                logdb->stop();
        }
        return 0;
}

