#include "prepare.hpp"
#include "utils/log.hpp"
#include "config/options_ptts.hpp"
#include "role_merger.hpp"
#include "notice_merger.hpp"
#include "mail_merger.hpp"
#include "arena_merger.hpp"
#include "marriage_merger.hpp"
#include "child_merger.hpp"
#include "rank_merger.hpp"
#include "huanzhuang_pvp_merger.hpp"
#include "fief_merger.hpp"
#include "mansion_merger.hpp"
#include "mansion_mgr_merger.hpp"
#include "activity_mgr_merge.hpp"
#include "used_rolename_merger.hpp"
#include "guanpin_merger.hpp"
#include "league_war_merger.hpp"
#include "league_merger.hpp"
#include "server_merger.hpp"
#include "tower_merger.hpp"
#include <boost/program_options.hpp>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace nora;
namespace bpo = boost::program_options;

int main(int argc, char *argv[]) {
        srand(time(0));

        // deamonize(argv);

        bpo::options_description desc("options");
        desc.add_options()
                ("help", "produce help message")
                ("from_ipport", bpo::value<string>(), "from db ipport, like 127.0.0.1:3306")
                ("from_user", bpo::value<string>(), "from db username")
                ("from_password", bpo::value<string>(), "from db password")
                ("from_game_database", bpo::value<string>(), "from db game database")
                ("to_ipport", bpo::value<string>(), "to db ipport, like 127.0.0.1:3306")
                ("to_user", bpo::value<string>(), "to db username")
                ("to_password", bpo::value<string>(), "to db password")
                ("to_game_database", bpo::value<string>(), "to db database");
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
        if (!vm.count("from_ipport") || !vm.count("from_user") || !vm.count("from_password") || !vm.count("from_game_database")) {
                cout << "missing from db info" << "\n";
                return 1;
        }
        if (!vm.count("to_ipport") || !vm.count("to_user") || !vm.count("to_password") || !vm.count("to_game_database")) {
                cout << "missing to db info" << "\n";
                return 1;
        }
        auto from_ipport = vm["from_ipport"].as<string>();
        auto from_user = vm["from_user"].as<string>();
        auto from_password = vm["from_password"].as<string>();
        auto from_game_database = vm["from_game_database"].as<string>();
        auto to_ipport = vm["to_ipport"].as<string>();
        auto to_user = vm["to_user"].as<string>();
        auto to_password = vm["to_password"].as<string>();
        auto to_game_database = vm["to_game_database"].as<string>();
        if (from_ipport == to_ipport && from_game_database == to_game_database) {
                cout << "from gamedb and to gamedb are the same" << "\n";
                return 1;
        }
        init_log("merger", pc::options::TRACE);

        prepare::instance().init(from_ipport,
                                 from_user,
                                 from_password,
                                 from_game_database,
                                 to_ipport,
                                 to_user,
                                 to_password,
                                 to_game_database);
        role_merger::instance().init(from_ipport,
                                     from_user,
                                     from_password,
                                     from_game_database,
                                     to_ipport,
                                     to_user,
                                     to_password,
                                     to_game_database);
        notice_merger::instance().init(from_ipport,
                                       from_user,
                                       from_password,
                                       from_game_database,
                                       to_ipport,
                                       to_user,
                                       to_password,
                                       to_game_database);
        mail_merger::instance().init(from_ipport,
                                     from_user,
                                     from_password,
                                     from_game_database,
                                     to_ipport,
                                     to_user,
                                     to_password,
                                     to_game_database);
        marriage_merger::instance().init(from_ipport,
                                         from_user,
                                         from_password,
                                         from_game_database,
                                         to_ipport,
                                         to_user,
                                         to_password,
                                         to_game_database);
        child_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);
        rank_merger::instance().init(from_ipport,
                                     from_user,
                                     from_password,
                                     from_game_database,
                                     to_ipport,
                                     to_user,
                                     to_password,
                                     to_game_database);
        huanzhuang_pvp_merger::instance().init(from_ipport,
                                               from_user,
                                               from_password,
                                               from_game_database,
                                               to_ipport,
                                               to_user,
                                               to_password,
                                               to_game_database);
        fief_merger::instance().init(from_ipport,
                                     from_user,
                                     from_password,
                                     from_game_database,
                                     to_ipport,
                                     to_user,
                                     to_password,
                                     to_game_database);
        mansion_merger::instance().init(from_ipport,
                                        from_user,
                                        from_password,
                                        from_game_database,
                                        to_ipport,
                                        to_user,
                                        to_password,
                                        to_game_database);
        mansion_mgr_merger::instance().init(from_ipport,
                                            from_user,
                                            from_password,
                                            from_game_database,
                                            to_ipport,
                                            to_user,
                                            to_password,
                                            to_game_database);
        used_rolename_merger::instance().init(from_ipport,
                                              from_user,
                                              from_password,
                                              from_game_database,
                                              to_ipport,
                                              to_user,
                                              to_password,
                                              to_game_database);
        arena_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);
        guanpin_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);
        league_war_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);
        league_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);
        activity_mgr_merger::instance().init(from_ipport,
                                     from_user,
                                     from_password,
                                     from_game_database,
                                     to_ipport,
                                     to_user,
                                     to_password,
                                     to_game_database);
        server_merger::instance().init(from_ipport,
                                       from_user,
                                       from_password,
                                       from_game_database,
                                       to_ipport,
                                       to_user,
                                       to_password,
                                       to_game_database);
        tower_merger::instance().init(from_ipport,
                                      from_user,
                                      from_password,
                                      from_game_database,
                                      to_ipport,
                                      to_user,
                                      to_password,
                                      to_game_database);

        mutex m;
        condition_variable cv;
        auto waiting_count = 0;
        auto done_cb = [&waiting_count, &m, &cv] {
                {
                        lock_guard<mutex> lk(m);
                        waiting_count -= 1;
                }
                cv.notify_one();
        };

        waiting_count = 2;
        prepare::instance().done_cb_ = done_cb;
        prepare::instance().start();
        mail_merger::instance().done_cb_ = done_cb;
        mail_merger::instance().start();
        {
                unique_lock<mutex> lk(m);
                cv.wait(lk, [&waiting_count] { return waiting_count == 0; });
        }

        waiting_count += 1;
        role_merger::instance().done_cb_ = done_cb;
        role_merger::instance().start();
        waiting_count += 1;
        notice_merger::instance().done_cb_ = done_cb;
        notice_merger::instance().start();
        waiting_count += 1;
        marriage_merger::instance().done_cb_ = done_cb;
        marriage_merger::instance().start();
        waiting_count += 1;
        child_merger::instance().done_cb_ = done_cb;
        child_merger::instance().start();
        waiting_count += 1;
        rank_merger::instance().done_cb_ = done_cb;
        rank_merger::instance().start();
        waiting_count += 1;
        huanzhuang_pvp_merger::instance().done_cb_ = done_cb;
        huanzhuang_pvp_merger::instance().start();
        waiting_count += 1;
        fief_merger::instance().done_cb_ = done_cb;
        fief_merger::instance().start();
        waiting_count += 1;
        mansion_merger::instance().done_cb_ = done_cb;
        mansion_merger::instance().start();
        waiting_count += 1;
        mansion_mgr_merger::instance().done_cb_ = done_cb;
        mansion_mgr_merger::instance().start();
        waiting_count += 1;
        used_rolename_merger::instance().done_cb_ = done_cb;
        used_rolename_merger::instance().start();
        waiting_count += 1;
        arena_merger::instance().done_cb_ = done_cb;
        arena_merger::instance().start();
        waiting_count += 1;
        guanpin_merger::instance().done_cb_ = done_cb;
        guanpin_merger::instance().start();
        waiting_count += 1;
        league_war_merger::instance().done_cb_ = done_cb;
        league_war_merger::instance().start();
        waiting_count += 1;
        activity_mgr_merger::instance().done_cb_ = done_cb;
        activity_mgr_merger::instance().start();
        waiting_count += 1;
        tower_merger::instance().done_cb_ = done_cb;
        tower_merger::instance().start();
        waiting_count += 1;
        server_merger::instance().done_cb_ = done_cb;
        server_merger::instance().start();        
        waiting_count += 1;
        league_merger::instance().done_cb_ = done_cb;
        league_merger::instance().start();
        {
                unique_lock<mutex> lk(m);
                cv.wait(lk, [&waiting_count] { return waiting_count == 0; });
        }

        return 0;
}
