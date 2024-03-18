#include "asm/cpu.h"
#include "boot.h"
#include "console.h"
#include "panic.h"
#include "serial.h"
#include "display.h"
#include "type.h"
#include "string.h"
#include "logger.h"


struct PixelColor Red = {
	.r = 0xFF,
	.g = 0,
	.b = 0,
};

struct PixelColor Green = {
	.r = 0,
	.g = 0xFF,
	.b = 0,
};

struct PixelColor Blue = {
	.r = 0,
	.g = 0,
	.b = 0xFF,
};

struct PixelColor White = {
	.r = 0xFF,
	.g = 0xFF,
	.b = 0xFF,
};

struct PixelColor Black = {
	.r = 0,
	.g = 0,
	.b = 0,
};

__attribute__((section(".stext")))
void startup(struct GopInfo *gop_info) {

	// ＴＯＤＯ：初期化の順番が、
	//	1. ディスプレイを初期化
	//	2. ディスプレイのコンソールを初期化
	//	3. シリアルポートを初期化
	//	4. シリアル通信を利用してロガーを初期化
	// といった感じになっているが、ここらへんの順序を明確に決めたい。
	display_init(gop_info);
	display_console_init(gop_info, &Black, &White);

	serial_init();
	serial_console_init();

	logger_init(LOG_LEVEL_INFO);


	INFO("Initialization completed!");

	// シリアルコンソールから受け取った文字に応じて画面の色を変化させる処理
	while (1) {
		char c = recvb(global_serial_port);
		switch (c) {
			case 'r':
				clear_screen(&Red);
				break;
			case 'b':
				clear_screen(&Blue);
				break;
			case 'g':
				clear_screen(&Green);
				break;
			default:
				sendb(global_serial_port, 'x');
		}
	}

	while (1) Halt();
}
