#pragma once

#include "boot.h"
#include "display.h"


// ディスプレイコンソールの初期化を行う関数
// ディスプレイコンソールとは、ディスプレイ上に出力されるコンソールのこと。
void display_console_init(struct GopInfo *gop_info, struct PixelColor *fg,
                          struct PixelColor *bg);

// ディスプレイコンソールに文字列を書き込む関数。コンソールの右端に達した場合は
// 置き換えしたりせず、書き込みを終了する。
//      - s: 書き込む文字列へのポインタ
//      - 返り値: 書き込んだ文字数
usize put_string_to_display(const char *s);

#define printd(s, ...) put_string_to_display(format_string(s, __VA_ARGS__))
