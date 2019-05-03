#pragma once

#include "exception.hpp"

#ifdef _DEBUG
#define ASSERT(expression) assert(expression)
#define ASSERT_MSG(expression, msg) assert(expression)
#else
#define ASSERT(expression) if (!(expression)) { \
                THROW(nora::assert_failed);     \
        }
#define ASSERT_MSG(expression, msg) if (!(expression)) {    \
                THROW_MSG(nora::assert_failed, msg);        \
        }
#endif
