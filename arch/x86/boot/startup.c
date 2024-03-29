#include "asm/cpu.h"
#include "boot.h"
#include "console.h"
#include "interrupt.h"
#include "jdb.h"
#include "panic.h"
#include "processor.h"
#include "serial.h"
#include "display.h"
#include "type.h"
#include "string.h"
#include "logger.h"
#include "segment.h"


void hoge(int n) {
	if (n == 0) {
		X86_INT3();
		return;
	}
	hoge(n - 1);
}

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

	processor_init();
	segment_init();

	logger_init(LOG_LEVEL_DEBUG);

	interrupt_init();

	serial_init_late();

	jdb_init();

	for (int i = 0; i < 10; i++) {
		hoge(i + 3);
	}
	// INFO("Initialization completed!");

	while (1) Halt();
}
