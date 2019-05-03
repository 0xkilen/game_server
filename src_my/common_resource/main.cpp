#include "common_resourced.hpp"
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

        write_pid_file("common_resourced_pid");

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log("common_resourced", ptt.level());
        PTTS_MVP(options);

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        st->async_call(
                [st] {
                        common_resource::common_resourced::instance().start();
                });

        COMMON_RESOURCE_ILOG << "start";
        st->run();
        return 0;
}
