#pragma once


enum PixelFormat {
        kRgb,
        kBgr,
};

struct GopInfo {
        void *frame_buffer;
        unsigned long long width;
        unsigned long long height;
        unsigned long long stride;
        enum PixelFormat pixel_format;
};
