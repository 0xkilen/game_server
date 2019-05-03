#include "dbcached.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "net/conn.hpp"
#include "config/options_ptts.hpp"
#include <condition_variable>
#include <mutex>

using namespace nora;

int main(int argc, char **argv) {
        srand(time(0));

        deamonize(argv);
        write_pid_file("dbcached_pid");

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log("dbcached", ptt.level());
        PTTS_MVP(options);

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        auto dbcached = make_shared<dbcache::dbcached>(st);
        st->async_call(
                [dbcached] {
                        dbcached->start();
                });

        DBCACHE_ILOG << "start";
        st->run();
        return 0;
}
