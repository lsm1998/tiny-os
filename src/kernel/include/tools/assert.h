#ifndef __ASSERT_H__
#define __ASSERT_H__

#include "tools/log.h"

#ifdef RELEASE
#define assert(condition) ((void)0)
#else
#define assert(condition)                                                         \
    do {                                                                          \
        if (!(condition))                                                         \
        {                                                                         \
            panic(__FILE__, __LINE__, __func__, "Assertion failed: " #condition); \
        }                                                                         \
    } while (0)
#endif

#endif // __ASSERT_H__