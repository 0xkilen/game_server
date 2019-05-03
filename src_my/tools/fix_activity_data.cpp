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
#include "proto/data_activity.pb.h"
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

int fund_value = 0;
pd::activity_mgr activity_mgr;

struct {
        string ipport;
        string user;
        string password;
        string game_database;
} db_params;

void fix_activity_fund() {
        auto* fund = activity_mgr.mutable_fund();
        fund->set_value(fund_value);
        auto db_msg = make_shared<db::message>("save_activity_mgr",
                                               db::message::req_type::rt_update,
                                               [] (const auto& msg) {
                                                       main_st->stop();
                                                       gamedb->stop();
                                               });
        string str;
        activity_mgr.SerializeToString(&str);
        db_msg->push_param(str);
        gamedb->push_message(db_msg, main_st);
        cout << "fix data done" << endl;
}

void start() {
        auto db_msg = make_shared<db::message>("get_activity_mgr",
                                               db::message::req_type::rt_select,
                                               [] (const shared_ptr<db::message>& msg) {
                                                       auto& results = msg->results();
                                                       if (!results.empty()) {
                                                               ASSERT(results.size() == 1);
                                                               const auto& result = results.front();
                                                               ASSERT(result.size() == 1);
                                                               activity_mgr.ParseFromString(boost::any_cast<string>(result[0]));\
                                                               fix_activity_fund();
                                                       }
                                               });
        gamedb->push_message(db_msg, main_st);
}

int main(int argc, char *argv[]) {
        //sleep(30);
        srand(time(0));

        bpo::options_description desc("options");
        desc.add_options()
                ("help,h", "produce help message")
                ("ipport", bpo::value<string>(), "mysql ipport")
                ("user", bpo::value<string>(), "mysql user")
                ("password", bpo::value<string>(), "mysql password")
                ("game_db", bpo::value<string>(), "mysql game_db")
                ("value", bpo::value<int>(), "fund value")
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

        main_st = make_shared<service_thread>("fix_activity_data");
        clock::instance().start(main_st);

        init_log("fix_activity_data", pc::options::TRACE);
        if (vm.count("game_db") <= 0 || vm.count("ipport") <= 0 || vm.count("user") <= 0 || vm.count("password") <= 0 || (vm.count("value") <= 0)) {
                cout << " 参数错误" << endl;
                main_st->stop();
                return 1;
        }
        db_params.game_database = vm["game_db"].as<string>();
        db_params.ipport = vm["ipport"].as<string>();
        db_params.user = vm["user"].as<string>();
        db_params.password = vm["password"].as<string>();
        fund_value = vm["value"].as<int>();
        if (fund_value <= 0) {
                cout << "value error " << endl;
                main_st->stop();
                return 1;
        }

        try {
                gamedb = make_shared<db::connector>(db_params.ipport,
                                                    db_params.user,
                                                    db_params.password,
                                                    db_params.game_database);
                gamedb->start();
        } catch (const exception& e) {
                cout << " 参数错误" << endl;
                main_st->stop();
                return 1;
        }

        main_st->async_call(
                [] {
                        start();
                });

        main_st->run();

        if (gamedb) {
                gamedb->stop();
        }
        return 0;
}

