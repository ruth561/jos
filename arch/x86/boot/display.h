#pragma once

#include "boot.h"


struct PixelColor {
        u8 r;
        u8 g;
        u8 b;
};

void display_init(struct GopInfo *gop_info_);
void clear_screen(struct PixelColor *color);

// 1つのピクセルをcolorで塗る関数。
//      - x: 上からの位置
//      - y: 左からの位置
void write_pixel(u32 x, u32 y, struct PixelColor *color);
