#include "log_server.hpp"
#include "scene/scened.hpp"
#include "net/stream.hpp"
#include "utils/service_thread.hpp"
#include "utils/exception.hpp"

#define SLOGSERVER_TLOG __TLOG << setw(20) << "[SLOGSERVER] "
#define SLOGSERVER_DLOG __DLOG << setw(20) << "[SLOGSERVER] "
#define SLOGSERVER_ILOG __ILOG << setw(20) << "[SLOGSERVER] "
#define SLOGSERVER_ELOG __ELOG << setw(20) << "[SLOGSERVER] "

namespace nora {
        namespace scene {

                log_server::log_server(const string& name) : name_(name) {
                }

                void log_server::static_init() {
                }

                string log_server::base_name() {
                        return "log_server";
                }
        }
}
