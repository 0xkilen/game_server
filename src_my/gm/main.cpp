#include "gmd.hpp"
#include "log.hpp"
#include "utils/service_thread.hpp"
#include "utils/process_utils.hpp"
#include "utils/time_utils.hpp"
#include "net/conn.hpp"
#include "config/options_ptts.hpp"

using namespace nora;

string to_string(const auto& ptt) {
        stringstream ss;

        ss << "gm-ipport: " << ptt.gm_info().scened_ipport().ip()<< " " << ptt.ptt.gm_info().scened_ipport().port() << endl;
        ss << "yunying-ipport: " << ptt.gmd_yunying_ipport().ip()<< " " << ptt.gmd_yunying_ipport().port() << endl;
        ss << "out: gmd.out";

        return ss.str();
}

int main(int argc, char **argv) {
        srand(time(0));

        deamonize(argv);
        write_pid_file("gmd_pid");

        PTTS_LOAD(options);
        auto ptt = PTTS_GET_COPY(options, 1u);
        init_log("gmd", ptt.level());
        PTTS_MVP(options);

        auto st = make_shared<service_thread>("main");
        auto gm = make_shared<gm::gmd>(st);
        clock::instance().start(st);

        st->async_call(
                [gm] {
                        gm->start();
                });

        GM_ILOG << "start";
        st->run();
        return 0;
}
