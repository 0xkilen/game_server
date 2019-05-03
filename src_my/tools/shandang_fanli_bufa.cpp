#include "utils/service_thread.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "utils/game_def.hpp"
#include "db/connector.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "utils/string_utils.hpp"
#include "utils/json2pb.hpp"
#include "config/utils.hpp"
#include "config/recharge_ptts.hpp"
#include "config/options_ptts.hpp"
#include "config/mail_ptts.hpp"
#include "scene/yunying/bi.hpp"
#include "scene/event.hpp"
#include "scene/mail/mail_logic.hpp"
#include "scene/mail/mail.hpp"
#include "proto/data_log.pb.h"
#include "proto/data_event.pb.h"
#include "proto/config_options.pb.h"
#include "proto/config_mail.pb.h"
#include "log.hpp"
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/any.hpp>
#include <unistd.h> //sleep

using namespace std;
using namespace nora;
namespace bpo = boost::program_options;

struct fanli {
        uint32_t count = 0;
        string server_id;
};
shared_ptr<service_thread> main_st;
map<uint64_t, map<uint32_t, fanli>> role2recharge_count;
string file_name;
set<uint64_t> role_not_exit;
uint32_t role_count;
uint32_t sent_count = 0;

shared_ptr<db::connector> gamedb;
struct {
        string ipport;
        string user;
        string password;
        string game_database;
} db_params;

void add_fanli_mail();

void set_not_exit_role() {
        role_count = role2recharge_count.size();
        for (const auto& i : role2recharge_count) {
                auto db_msg = make_shared<db::message>("find_role_by_gid", db::message::req_type::rt_select,
                                                        [role = i.first] (const auto& msg) {
                                                                if (msg->results().empty()) {
                                                                        role_not_exit.insert(role);
                                                                }
                                                                --role_count;
                                                                if (role_count == 0) {
                                                                        add_fanli_mail();
                                                                }
                                                        });
                db_msg->push_param(i.first);
                gamedb->push_message(db_msg, main_st);
        }
}

void write_day_recharge_file() {
        ofstream day_out("day_recharge_" + db_params.game_database + ".txt");
        if (day_out.is_open()) {
                day_out << "role_id,recharge_id,recharge_times,server_id" << endl;
        }
        for (const auto& i : role2recharge_count) {
                for (const auto& j : i.second) {
                        const auto& ptt = PTTS_GET(recharge, j.first);
                        if (ptt.has_days()) {
                                if (day_out.is_open()) {
                                        string data = to_string(i.first) + "," + to_string(j.first) + "," + to_string(j.second.count) + "," + j.second.server_id;
                                        day_out << data << endl;
                                }
                        }
                }
        }
        day_out.close();
}

void load_data() {
        ifstream in(file_name);
        if (in.is_open()) {
                string data;
                auto count = 0;
                const auto& ptts = PTTS_GET_ALL(recharge);
                while(getline(in, data)) {
                        ++count;
                        if (count == 1) {
                                continue;
                        }
                        auto vec_str = split_string(data, ',');
                        if (vec_str.size() != 5) {
                                continue;
                        }
                        auto role = stoul(vec_str[2]);
                        for (const auto& i : ptts) {
                                if (i.second.yunying_id() == vec_str[1]) {
                                        if (role2recharge_count.count(role) <= 0) {
                                                role2recharge_count[role][i.second.id()].count = 1;
                                                role2recharge_count[role][i.second.id()].server_id = vec_str[4];
                                        } else {
                                                if (role2recharge_count[role].count(i.second.id()) <= 0) {
                                                        role2recharge_count.at(role)[i.second.id()].count = 1;
                                                        role2recharge_count[role][i.second.id()].server_id = vec_str[4];
                                                } else {
                                                        role2recharge_count.at(role).at(i.second.id()).count += 1;
                                                }
                                        }
                                        break;
                                }
                        }
                }
        }
        write_day_recharge_file();
        in.close();
}

void add_fanli_mail() {
        ofstream sent_out("sent_role_" + db_params.game_database + ".txt");
        for (const auto& i : role2recharge_count) {
                if (role_not_exit.count(i.first) > 0) {
                        continue;
                }
                pd::event_array events;
                auto rebate_percent = 1000;
                for (const auto& j : i.second) {
                        const auto& ptt = PTTS_GET(recharge, j.first);
                        rebate_percent = ptt.rebate_percent();
                        for (auto k = 1u; k <= j.second.count; ++k) {
                                nora::scene::event_merge(events, ptt.rebate_events());
                        }
                }
                pd::ce_env ce;
                ce.set_add_percent(rebate_percent);
                nora::scene::event_adopt_env(events, ce);

                auto m = nora::scene::mail_new_mail(i.first, REBATE_MAIL_ID, pd::dynamic_data(), events);
                auto db_msg = make_shared<db::message>("add_mail", db::message::req_type::rt_insert,
                                                        [] (const auto& msg) {
                                                                --sent_count;
                                                                if (sent_count <= 0) {
                                                                        main_st->stop();
                                                                }
                                                        });
                db_msg->push_param(m.gid());
                db_msg->push_param(i.first);
                string mail_str;
                m.SerializeToString(&mail_str);
                db_msg->push_param(mail_str);
                gamedb->push_message(db_msg, main_st);
                ++sent_count;
                sent_out << to_string(i.first) << endl;
        }

        if (sent_count <= 0) {
                main_st->stop();
        }
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
                ("file_name", bpo::value<string>(), "file name")
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
        if (vm.count("file_name") <= 0 || vm.count("ipport") <= 0 || vm.count("user") <= 0 || vm.count("password") <= 0 || vm.count("game_db") <= 0) {
                cout << " params error" << endl;
                return 1;
        }

        file_name = vm["file_name"].as<string>();
        db_params.game_database = vm["game_db"].as<string>();
        db_params.ipport = vm["ipport"].as<string>();
        db_params.user = vm["user"].as<string>();
        db_params.password = vm["password"].as<string>();
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


        main_st = make_shared<service_thread>("db_shandang_fanli_bufa");
        clock::instance().start(main_st);

        init_log("shandang_fanli_bua", pc::options::TRACE);
        PTTS_LOAD(recharge);
        PTTS_LOAD(mail);

        main_st->async_call(
                [] {
                        load_data();
                        if (role2recharge_count.size() <= 0) {
                                main_st->stop();
                                if (gamedb) {
                                        gamedb->stop();
                                }
                                cout << "找不到文件" << endl;
                                return;
                        }
                        set_not_exit_role();
                });
        main_st->run();

        if (gamedb) {
                gamedb->stop();
        }

        return 0;
}

