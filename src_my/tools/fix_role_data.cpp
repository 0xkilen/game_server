#include "utils/service_thread.hpp"
#include "db/connector.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "utils/string_utils.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "proto/data_player.pb.h"
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

uint64_t role_gid = 0;
int add_exp = 0;
bool replace_name = false;

void stop() {
        if (gamedb) {
                gamedb->stop();
        }
        if (main_st) {
                main_st->stop();
        }
}

void fix_role_data(const pd::role& data) {
        auto db_msg = make_shared<db::message>("update_role",
                                       db::message::req_type::rt_update,
                                       [data] (const auto& msg) {
                                                cout << "fix role data done\n";
                                                TOOL_ELOG << "fix role data done, role data " << data.DebugString();
                                                stop();
                                                return;
                                       });
        db_msg->push_param(role_gid);
        db_msg->push_param(data.data().name());
        string data_str;
        data.SerializeToString(&data_str);
        db_msg->push_param(data_str);
        gamedb->push_message(db_msg, main_st);
}

void find_role_by_gid() {
        auto db_msg = make_shared<db::message>("find_role_by_gid",
                                               db::message::req_type::rt_select,
                                               [] (const auto& msg) {
                                                        if (msg->results().empty()) {
                                                                cout << "role NOT_FOUND\n";
                                                                stop();
                                                                return;
                                                        }
                                                        const auto& result = msg->results().front();
                                                        pd::role data;
                                                        data.ParseFromString(boost::any_cast<string>(result[2]));
                                                        TOOL_ELOG << "original role data " << data.DebugString();
                                                        if (add_exp != 0) {
                                                                if (data.has_resources()) {
                                                                        auto cur_exp = add_exp;
                                                                        uint32_t idx = 0;
                                                                        bool has_exp = false;
                                                                        for (const auto& i : data.resources().values()) {
                                                                                ++idx;
                                                                                if (i.type() == pd::EXP) {
                                                                                        cur_exp += i.value();
                                                                                        has_exp = true;
                                                                                        break;
                                                                                }
                                                                        }
                                                                        if (has_exp) {
                                                                                auto *exp = data.mutable_resources()->mutable_values(idx - 1);
                                                                                exp->set_value(cur_exp);
                                                                        } else {
                                                                                auto *exp = data.mutable_resources()->add_values();
                                                                                exp->set_type(pd::EXP);
                                                                                exp->set_value(cur_exp);
                                                                        }
                                                                        fix_role_data(data);
                                                                }
                                                        }
                                                        if (replace_name) {
                                                                auto *role_data = data.mutable_data();
                                                                role_data->set_name(boost::any_cast<string>(result[1]));
                                                                fix_role_data(data);
                                                        }
                                               });
        db_msg->push_param(role_gid);
        gamedb->push_message(db_msg, main_st);
}

int main(int argc, char *argv[]) {
        srand(time(0));

        bpo::options_description desc("options");
        desc.add_options()
                ("help,h", "produce help message")
                ("role_gid", bpo::value<string>(), "role gid")
                ("add_exp", bpo::value<string>(), "add exp")
                ("replace_name", bpo::value<bool>(), "replace name (true or false)");


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

        main_st = make_shared<service_thread>("fix_role_data");
        clock::instance().start(main_st);

        init_log("fix_role_data", pc::options::TRACE);

        if (!vm.count("role_gid")) {
                cout << "查询器异常: 参数配置错误，请检查！\n";
                return 1;
        }
        if (!vm.count("role_gid") && !vm.count("replace_name")) {
                cout << "查询器异常: 参数配置错误，请检查！\n";
                return 1;
        }
        if (vm.count("add_exp")) {
                add_exp = stoi(vm["add_exp"].as<string>());
        }
        if (vm.count("replace_name")) {
                replace_name = vm["replace_name"].as<bool>();
        }
        role_gid = stoul(vm["role_gid"].as<string>());

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1);
        auto database = ptt.game_db().database();
        auto ipport = ptt.game_db().ipport();
        auto password = ptt.game_db().password();
        auto user = ptt.game_db().user();
        try {
                gamedb = make_shared<db::connector>(ipport,
                                                    user,
                                                    password,
                                                    database);
                gamedb->start();
        } catch (const exception& e) {
                cout << "game查询异常\n" << e.what();
                return 1;
        }

        main_st->async_call(
                [vm] {
                        shared_ptr<db::message> db_msg;
                        find_role_by_gid();
                });

        main_st->run();
        return 0;
}

