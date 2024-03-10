#pragma once

#include "type.h"


inline void *memcpy(void *dst, const void *src, usize n)
{
        char *dstp = (char *) dst;
        const char *srcp = (const char *) src;

        for (usize i = 0; i < n; i++) {
                *dstp++ = *srcp++;
        }
        return dst;
}
