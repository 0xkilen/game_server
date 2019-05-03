#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include "config/player_ptts.hpp"
#include "player/cplayer_mgr.hpp"
#include "log.hpp"
#include <iostream>
#include <boost/program_options.hpp>
#include <string>

using namespace std;
using namespace nora;
namespace bpo = boost::program_options;
namespace pc = proto::config;

int main(int argc, char *argv[]) {

        // main thread
        string server;
        string host;
        unsigned short port;
        int login_count = 0;
        int login_frequency = 100;
        int behavior_tree_root = 10000;
        try {
                bpo::options_description desc("Allowed options");
                desc.add_options()
                        ("help,h", "produce help message")
                        ("server,s", bpo::value<string>(&server)->default_value("10.1.1.90:10217"), "set login server ip, e.g. '10.1.1.90:10217'")
                        ("count,c", bpo::value<int>(), "set player login count")
                        ("frequency,f", bpo::value<int>(), "set per second player login frequency")
                        ("behavior_tree_root,r", bpo::value<int>(), "set player behavior tree root")
                        ;

                bpo::variables_map vm;
                bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
                bpo::notify(vm);


                if(vm.count("help")) {
                        cout << desc << endl;
                        return 1;
                }
                if(vm.count("server")) {
                        server = vm["server"].as<string>();
                }
                if (vm.count("count")) {
                        login_count = vm["count"].as<int>();
                }
                if (vm.count("frequency")) {
                        login_frequency = vm["frequency"].as<int>();
                }
                if (vm.count("behavior_tree_root")) {
                        behavior_tree_root = vm["behavior_tree_root"].as<int>();
                }

                host = split_string(server, ':')[0];
                stringstream stream;
                stream << split_string(server, ':')[1];
                stream >> port;
        } catch (const exception& e) {
                cerr << "error: " << e.what() << endl;
                return 1;
        }

        srand(time(0));
        deamonize(argv);
        write_pid_file("robot_pid");

        // open robot thread
        auto st = make_shared<service_thread>("robot");
        clock::instance().start(st);

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log("robot", ptt.level());

        config::load_config();
        config::load_dirty_word();
        config::check_config();

        PTTS_SET_FUNCS(robot);
        PTTS_LOAD(robot);
        PTTS_MVP(robot);
        PTTS_MVP(options);

        auto cpm = make_shared<scene::cplayer_mgr>(st, login_count, login_frequency, behavior_tree_root);
        st->async_call(
                [host, port, cpm] {
                        cpm->start(host, port);
                });

        st->run();
        return 0;
}
