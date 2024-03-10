#pragma once

#include "type.h"


#define FONT_HEIGHT 8
#define FONT_WIDTH 8
#define PRINTABLE(c) (0x20 <= (u8) c && (u8) c < 0x80)

extern u8 font8x8[0x80][8];
