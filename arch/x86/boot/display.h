#pragma once

#include "boot.h"
#include "type.h"


struct PixelColor {
        u8 r;
        u8 g;
        u8 b;
};

void display_init(struct GopInfo *gop_info_);
void clear_screen(struct PixelColor *color);
