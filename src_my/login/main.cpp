#include "logind.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/time_utils.hpp"
#include "utils/process_utils.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include <signal.h>

using namespace nora;

int main(int argc, char **argv) {
        srand(time(0));

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        if (ptt.login_info().logind_deamon()) {
                deamonize(argv);
        }

        write_pid_file("logind_pid");

        init_log("logind", ptt.level());
        PTTS_LOAD(channel_specify);
        PTTS_LOAD(serverlist);
        PTTS_LOAD(specific_server_list);
        PTTS_MVP(options);
        PTTS_MVP(channel_specify);
        PTTS_MVP(serverlist);
        PTTS_MVP(specific_server_list);

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        auto lg = make_shared<login::logind>(st);
        st->async_call(
                [lg] {
                        lg->start();
                });

        LOGIN_ILOG << "start";
        st->run();
        return 0;
}
