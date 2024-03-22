#pragma once

#include "asm/io.h"
#include "type.h"


extern io_addr_t global_serial_port;

io_addr_t serial_init();

// 割り込みの初期化が終わってから呼び出される初期化関数。
// 割り込みハンドラの設定を行う。
void serial_init_late();

// ポート宛てに1-byteのデータを送信しようと試みる関数。
// 送信に成功すれば0を、失敗すれば-1を返す。
// UARTの送信バッファは16-bytesしかないので、バッファが
// いっぱいになっていると送信できない可能性がある。
int try_sendb(io_addr_t port, u8 data);

// portにdataを送信する。送信バッファがいっぱいのときは
// 処理が停止するので注意！
void sendb(io_addr_t port, u8 data);

// ポートから1-byteのデータを受信しようと試みる関数。
// 受信するデータがあれば、そのデータを返す。
// データはu8型であり、[0, 255]の間におさまる。
// 受信するデータがなかった場合は-1を返す。
int try_recvb(io_addr_t port);

// portから1-byteのデータを受信する関数。
// 受信バッファにデータがない場合はブロックするので注意。
u8 recvb(io_addr_t port);
