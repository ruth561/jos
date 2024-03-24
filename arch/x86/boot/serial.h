#pragma once

#include "asm/io.h"
#include "type.h"


extern io_addr_t global_serial_port;

io_addr_t serial_init();

// 割り込みの初期化が終わってから呼び出される初期化関数。
// 割り込みハンドラの設定を行う。
void serial_init_late();

// シリアル通信で文字を受け取ったときに呼び出されるコールバックの型
// ＴＯＤＯ：割り込みハンドラの内部で呼び出されるようになっているが、
// 割り込みハンドラの外部でも呼び出せるようにしたい。
typedef void (*serial_recv_callback_t)(char c);

void register_serial_recv_callback(serial_recv_callback_t callback);

// シリアル通信で1文字を送信する関数。
void serial_putc(char c);

// シリアルコンソールに文字列を送信する関数。
// シリアルポートを経由してログメッセージなどを送信するときに使われる。
void send_string_to_serial(const char *s);
