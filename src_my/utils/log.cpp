#include "log.hpp"
#include <chrono>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/thread.hpp>

using namespace std::chrono;

src::severity_logger<logging::trivial::severity_level> g_log;

namespace nora {

        void init_log(const string& prefix, pc::options::log_level level) {
                logging::add_file_log(
                        keywords::file_name = "logs/" + prefix + ".%Y-%m-%d_%H.%5N",
                        keywords::rotation_size = 100 * 1024 * 1024,
                        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::hours(1)),
                        keywords::format = "[%TimeStamp%] [%ThreadID%] %Message%",
                        keywords::min_free_space = 50 * 1024 * 1024,
                        keywords::filter = logging::trivial::severity != logging::trivial::fatal,
                        keywords::auto_flush = true
                        );

                change_log_level(level);

                logging::add_common_attributes();
        }

        void init_log_for_log_server(const string& prefix, pc::options::log_level level) {
                logging::add_file_log(
                        keywords::file_name = "logs/" + prefix + ".%Y-%m-%d_%H.%5N",
                        keywords::rotation_size = 100 * 1024 * 1024,
                        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::hours(1)),
                        keywords::format = "[%TimeStamp%] [%ThreadID%] %Message%",
                        keywords::min_free_space = 50 * 1024 * 1024,
                        keywords::filter = logging::trivial::severity != logging::trivial::fatal,
                        keywords::auto_flush = true
                        );

                logging::add_file_log(
                        keywords::file_name = "logs/%Y%m%d%H%M%S.log",
                        keywords::time_based_rotation = sinks::file::rotation_at_time_interval(boost::posix_time::minutes(5)),
                        keywords::format = "%Message%",
                        keywords::filter = logging::trivial::severity == logging::trivial::fatal,
                        keywords::min_free_space = 50 * 1024 * 1024
                        );

                change_log_level(level);

                logging::add_common_attributes();
        }

        void change_log_level(pc::options::log_level level) {
                switch (level) {
                case pc::options::TRACE:
                        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);
                        break;
                case pc::options::DEBUG:
                        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::debug);
                        break;
                case pc::options::INFO:
                        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
                        break;
                case pc::options::ERROR:
                        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error);
                        break;
                case pc::options::NONE:
                        logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::fatal);
                        break;
                }
        }

}
