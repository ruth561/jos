#pragma once

#include "boot.h"
#include "display.h"
#include "string.h"
#include "serial.h"

// ディスプレイコンソールの初期化を行う関数
// ディスプレイコンソールとは、ディスプレイ上に出力されるコンソールのこと。
void display_console_init(struct GopInfo *gop_info, struct PixelColor *fg,
			  struct PixelColor *bg);

// ディスプレイコンソールに文字列を書き込む関数。コンソールの右端に達した場合は
// 置き換えしたりせず、書き込みを終了する。
//      - s: 書き込む文字列へのポインタ
//      - 返り値: 書き込んだ文字数
usize put_string_to_display(const char *s);

#define print_display(format, ...) put_string_to_display(format_string(format, ##__VA_ARGS__))

#define println_display(format, ...)			\
	do {						\
		print_display(format, ##__VA_ARGS__);	\
		print_display("\n");			\
	} while (0);

// シリアルコンソールの初期化を行う関数
void serial_console_init();

// シリアルコンソールに文字列を送信する関数。
// シリアルポートを経由してログメッセージなどを送信するときに使われる。
void send_string_to_serial(const char *s);

#define print_serial(format, ...) send_string_to_serial(format_string(format, ##__VA_ARGS__))

#define println_serial(format, ...)			\
	do {						\
		print_serial(format, ##__VA_ARGS__);	\
		print_serial("\r\n");			\
	} while (0);
