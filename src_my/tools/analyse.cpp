#include "scene/player/role.hpp"
#include "config/utils.hpp"
#include "utils/service_thread.hpp"
#include "utils/log.hpp"
#include "utils/assert.hpp"
#include "db/connector.hpp"
#include "proto/data_rank.pb.h"
#include "utils/proto_utils.hpp"
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <list>
#include <set>
#include <ctime>

using namespace std;
using namespace nora;
namespace bpo = boost::program_options;
namespace pd = proto::data;

set<uint64_t> roles;
map<uint64_t, uint32_t> role2level;
map<uint32_t, set<uint64_t>> level2roles;
map<int, map<int, set<uint64_t>>> offline_day2roles;
map<int, map<int, set<uint64_t>>> first_login_day2roles;
shared_ptr<db::connector> gamedb;
shared_ptr<service_thread> main_st;
int page_size = 50;
int cur_page = 0;

void fetch_all_roles(const function<void()>& done_cb) {
        auto db_msg = make_shared<db::message>("get_roles",
                                               db::message::req_type::rt_select,
                                               [done_cb] (const shared_ptr<db::message>& msg) {
                                                       const auto& results = msg->results();
                                                       if (results.empty()) {
                                                               cout << "fetch all roles done\n";
                                                               done_cb();
                                                       } else {
                                                               for (const auto& i : results) {
                                                                       ASSERT(i.size() == 4);
                                                                       pd::role data;
                                                                       cout << "fetched username: " << boost::any_cast<string>(i[0]) << "\n";
                                                                       data.ParseFromString(boost::any_cast<string>(i[3]));
                                                                       scene::role r;
                                                                       r.parse_from(data);

                                                                       role2level[r.gid()] = r.level();
                                                                       level2roles[r.level()].insert(r.gid());
                                                                       time_t t = r.offline_time();
                                                                       if (t > 0) {
                                                                               auto *tm = localtime(&t);
                                                                               offline_day2roles[tm->tm_mon + 1][tm->tm_mday].insert(r.gid());
                                                                       }
                                                                       t = r.first_login_time();
                                                                       auto *tm = localtime(&t);
                                                                       first_login_day2roles[tm->tm_mon + 1][tm->tm_mday].insert(r.gid());
                                                               }
                                                               fetch_all_roles(done_cb);
                                                       }
                                               });
        db_msg->push_param(cur_page * page_size);
        db_msg->push_param((cur_page + 1) * page_size);
        gamedb->push_message(db_msg, main_st);
        cur_page += 1;
}

void fetch_role_list(const function<void()>& done_cb) {
        auto db_msg = make_shared<db::message>("load_gid_and_rolename",
                                               db::message::req_type::rt_select,
                                               [done_cb] (const auto& msg) {
                                                       auto& results = msg->results();
                                                       for (const auto& i : results) {
                                                               ASSERT(i.size() == 2);
                                                               auto gid = boost::any_cast<uint64_t>(i[0]);
                                                               roles.insert(gid);
                                                       }
                                                       cout << "fetch role list done, got " << roles.size() << " roles ..." << "\n";
                                                       done_cb();
                                               });
        gamedb->push_message(db_msg, main_st);
}

void analyse_roles() {
        cout << "==========level distribution:==========\n";
        for (const auto& i : level2roles) {
                cout << "level: " << i.first << " count: " << i.second.size() << "\n";
        }

        cout << "==========level distribution for those who left on:========\n";
        map<uint32_t, uint32_t> level2count;
        for (const auto& i : offline_day2roles) {
                for (const auto& j : i.second) {
                        level2count.clear();
                        cout << "==========month " << i.first << " day " << j.first << ":\n";
                        for (auto k : j.second) {
                                auto level = role2level[k];
                                level2count[level] += 1;
                        }
                        for (const auto& l : level2count) {
                                cout << "level: " << l.first << " count: " << l.second << "\n";
                        }
                }
        }

        cout << "==========level distribution for those who join and left on:==========\n";
        for (const auto& i : offline_day2roles) {
                for (const auto& j : i.second) {
                        level2count.clear();
                        cout << "==========month " << i.first << " day " << j.first << ":\n";
                        for (auto k : j.second) {
                                if (first_login_day2roles[i.first][j.first].count(k) > 0) {
                                        auto level = role2level[k];
                                        level2count[level] += 1;
                                }
                        }
                        for (const auto& l : level2count) {
                                cout << "level: " << l.first << " count: " << l.second << "\n";
                        }
                }
        }

        cout << "==========level to roles:==========\n";
        for (const auto& i : level2roles) {
                cout << "==========level " << i.first << ":\n";
                for (auto j : i.second) {
                        cout << j << "\n";
                }
        }

        cout << "\nall done\n";
        main_st->stop();
}

int main(int argc, char *argv[]) {
        bpo::options_description desc("options");
        desc.add_options()
                ("help", "produce help message")
                ("gamedb_ipport", bpo::value<string>(), "gamedb ipport, like 127.0.0.1:3306")
                ("gamedb_user", bpo::value<string>(), "gamedb username")
                ("gamedb_password", bpo::value<string>(), "gamedb password")
                ("gamedb_database", bpo::value<string>(), "gamedb database");

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
        if (!vm.count("gamedb_ipport") || !vm.count("gamedb_user") || !vm.count("gamedb_password") || !vm.count("gamedb_database")) {
                cout << "missing db info" << "\n";
                return 1;
        }

        config::load_config();

        auto gamedb_ipport = vm["gamedb_ipport"].as<string>();
        auto gamedb_user = vm["gamedb_user"].as<string>();
        auto gamedb_password = vm["gamedb_password"].as<string>();
        auto gamedb_database = vm["gamedb_database"].as<string>();

        gamedb = make_shared<db::connector>(gamedb_ipport,
                                            gamedb_user,
                                            gamedb_password,
                                            gamedb_database);
        gamedb->start();
        cout << "fetch role list ..." << "\n";
        main_st = make_shared<service_thread>("main");

        main_st->async_call(
                [] {
                        fetch_role_list(
                                [] {
                                        fetch_all_roles(
                                                [] {
                                                        analyse_roles();
                                                });
                                });
                });

        main_st->run();

        gamedb->stop();
        return 0;
}
