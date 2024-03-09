#pragma once

#include "type.h"


enum PixelFormat {
        kRgb,
        kBgr,
};

struct GopInfo {
        void *frame_buffer;
        u64 width;
        u64 height;
        u64 stride;
        enum PixelFormat pixel_format;
};
