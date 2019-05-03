#pragma once

#define BOOST_LOG_DYN_LINK 1

#include "proto/config_options.pb.h"
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <string>
#include <iomanip>

namespace pc = proto::config;
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
using namespace std;

extern src::severity_logger<logging::trivial::severity_level> g_log;

#define __TLOG BOOST_LOG_SEV(g_log, logging::trivial::trace) << "[TRACE] "
#define __DLOG BOOST_LOG_SEV(g_log, logging::trivial::debug) << "[DEBUG] "
#define __ILOG BOOST_LOG_SEV(g_log, logging::trivial::info) << " [INFO] "
#define __ELOG BOOST_LOG_SEV(g_log, logging::trivial::error) << "[ERROR] "
#define __BILOG BOOST_LOG_SEV(g_log, logging::trivial::fatal)

#define TLOG __TLOG << setw(20) << " [NONE] "
#define DLOG __DLOG << setw(20) << " [NONE] "
#define ILOG __ILOG << setw(20) << " [NONE] "
#define ELOG __ELOG << setw(20) << " [NONE] "

namespace nora {

        void init_log(const string& prefix, pc::options::log_level level);
        void init_log_for_log_server(const string& prefix, pc::options::log_level level);
        void change_log_level(pc::options::log_level level);

}
