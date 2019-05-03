#pragma once

#include "scene/log.hpp"

#define SGM_TLOG __TLOG << setw(20) << "[SCENEGM] "
#define SGM_DLOG __DLOG << setw(20) << "[SCENEGM] "
#define SGM_ILOG __ILOG << setw(20) << "[SCENEGM] "
#define SGM_ELOG __ELOG << setw(20) << "[SCENEGM] "

#define SGMMGR_TLOG __TLOG << setw(20) << "[SCENEGMMGR] "
#define SGMMGR_DLOG __DLOG << setw(20) << "[SCENEGMMGR] "
#define SGMMGR_ILOG __ILOG << setw(20) << "[SCENEGMMGR] "
#define SGMMGR_ELOG __ELOG << setw(20) << "[SCENEGMMGR] "
