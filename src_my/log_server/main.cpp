#include "log_serverd.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"

using namespace nora;

int main(int argc, char **argv) {
        srand(time(0));

        deamonize(argv);

        write_pid_file("log_serverd_pid");

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log_for_log_server("log_serverd", ptt.level());
        PTTS_MVP(options);

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        auto lsd = make_shared<log_serverd::log_serverd>(st);
        st->async_call(
                [lsd] {
                        lsd->start();
                });

        LOG_ILOG << "start";
        st->run();
        return 0;
}
