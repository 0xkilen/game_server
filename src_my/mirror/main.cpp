#include "mirrord.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "config/utils.hpp"
#include "config/options_ptts.hpp"
#include "config/mirror_ptts.hpp"

using namespace nora;

int main(int argc, char **argv) {
        srand(time(0));

        deamonize(argv);

        write_pid_file("mirrord_pid");

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log("mirrord", ptt.level());
        PTTS_MVP(options);
        PTTS_LOAD(mirror);
        PTTS_MVP(mirror);

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        auto mirrord = make_shared<mirror::mirrord>(st);
        st->async_call(
                [mirrord] {
                        mirrord->start();
                });

        MIRROR_ILOG << "start";
        st->run();
        return 0;
}
