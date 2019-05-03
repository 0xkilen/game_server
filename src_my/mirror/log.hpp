#pragma once

#include "utils/log.hpp"

#define MIRROR_TLOG __TLOG << setw(20) << "[ROLE] "
#define MIRROR_DLOG __DLOG << setw(20) << "[ROLE] "
#define MIRROR_ILOG __ILOG << setw(20) << "[ROLE] "
#define MIRROR_ELOG __ELOG << setw(20) << "[ROLE] "
