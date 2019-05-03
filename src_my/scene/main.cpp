#include "scened.hpp"
#include "log.hpp"
#include "config/options_ptts.hpp"
#include "config/utils.hpp"
#include "utils/log.hpp"
#include "utils/process_utils.hpp"
#include <curl/curl.h>

using namespace std;
using namespace nora;

int main(int argc, char **argv) {
        srand(time(0));

        auto st = make_shared<service_thread>("main");
        clock::instance().start(st);

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        if (ptt.scene_info().scened_deamon()) {
                deamonize(argv);
        }
        write_pid_file("scened_pid");

        init_log("scened", ptt.level());

        PTTS_MVP(options);

        st->async_call(
                [st] {
                        //curl_global_init(CURL_GLOBAL_ALL);

                        if (!PTTS_HAS(options, 1)) {
                                SCENE_ELOG << "scened need options";
                                exit(1);
                        }

                        scene::scened::instance().start(st);
                });

        st->run();
        //curl_global_cleanup();

        return 0;
}
