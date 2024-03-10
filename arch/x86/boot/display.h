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

// 長方形を色colorで描画する関数。
//      - (x, y): 長方形の左上のピクセルの位置
//              - x: フレームバッファの上からの位置
//              - y: フレームバッファの左からの位置
//      - h: 長方形の縦の辺の長さ
//      - w: 長方形の横の辺の長さ
void write_rectangle(u32 x, u32 y, u32 h, u32 w, struct PixelColor *color);

// ASCII文字を描画する関数。
//      - (x, y): ASCII文字の左上のピクセルの位置
//              - x: フレームバッファの上からの位置
//              - y: フレームバッファの左からの位置
//      - c: 描画する文字
//      - fg: 文字の色
//      - bg: 背景色
//      - 返り値: 描画に成功したら0を返し、失敗したら-1を返す
int write_char(u32 x, u32 y, char c, struct PixelColor *fg, struct PixelColor *bg);

// ASCII文字列を描画する関数。指定した場所から右に続けて描画していく。
// フレームの外にはみ出た場合は折り返したりはしない。
//      - (x, y): ASCII文字の左上のピクセルの位置
//              - x: フレームバッファの上からの位置
//              - y: フレームバッファの左からの位置
//      - s: 描画する文字列へのポインタ
//      - len: 描画する文字列の長さ
//      - fg: 文字の色
//      - bg: 背景色
//      - 返り値: すべての描画に成功したら0を返し、失敗したら-1を返す
int write_string(u32 x, u32 y, const char *s, usize len,
                 struct PixelColor *fg, struct PixelColor *bg);

// 正方形を色colorで描画する関数。
//      - (x, y): 正方形の左上のピクセルの位置
//              - x: フレームバッファの上からの位置
//              - y: フレームバッファの左からの位置
//      - size: 正方形の一辺の長さ
void write_square(u32 x, u32 y, u32 size, struct PixelColor *color);
