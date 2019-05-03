#pragma once

#include "utils/log.hpp"

#define LOGIN_TLOG __TLOG << setw(20) << "[LOGIN] "
#define LOGIN_DLOG __DLOG << setw(20) << "[LOGIN] "
#define LOGIN_ILOG __ILOG << setw(20) << "[LOGIN] "
#define LOGIN_ELOG __ELOG << setw(20) << "[LOGIN] "

#define LGM_TLOG __TLOG << setw(20) << "[LOGINGM] "
#define LGM_DLOG __DLOG << setw(20) << "[LOGINGM] "
#define LGM_ILOG __ILOG << setw(20) << "[LOGINGM] "
#define LGM_ELOG __ELOG << setw(20) << "[LOGINGM] "

#define CM_TLOG __TLOG << setw(20) << "[CLIENT_MGR] "
#define CM_DLOG __DLOG << setw(20) << "[CLIENT_MGR] "
#define CM_ILOG __ILOG << setw(20) << "[CLIENT_MGR] "
#define CM_ELOG __ELOG << setw(20) << "[CLIENT_MGR] "

#define CLIENT_TLOG __TLOG << setw(20) << "[CLIENT] "
#define CLIENT_DLOG __DLOG << setw(20) << "[CLIENT] "
#define CLIENT_ILOG __ILOG << setw(20) << "[CLIENT] "
#define CLIENT_ELOG __ELOG << setw(20) << "[CLIENT] "
