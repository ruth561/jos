#include "jdb.h"
#include "console.h"
#include "interrupt.h"
#include "irq.h"
#include "logger.h"
#include "panic.h"
#include "type.h"


// その値が有効なアドレス値かどうかを検証する関数
bool is_valid_address_naive(u64 addr)
{
	return 0x100000 <= addr && addr < 0x200000;
}

// とりあえず非ゼロであればよいものとする。
// ＴＯＤＯ：
bool is_valid_rbp(u64 rbp)
{
	return 0 < rbp;
}

u64 get_saved_rbp(u64 rbp)
{
	// スタックは以下のような状態になっている。
	// |-------------|
	// |  saved rbp  | <--- rbp
	// |-------------|
	return *(u64 *) (rbp);
}

// rbpから戻りアドレスを取得する関数
u64 get_ret_addr(u64 rbp)
{
	// スタックは以下のような状態になっている。
	// |-------------|
	// |   ret addr  | 
	// |-------------|
	// |  saved rbp  | <--- rbp
	// |-------------|
	return *(u64 *) (rbp + 8);
}

static void back_trace(struct regs_on_stack *regs)
{
	println_serial("========== back trace ==========");
	u32 depth = 0;
	u64 rbp = regs->rbp;
	u64 ret_addr = get_ret_addr(rbp);
	while (is_valid_address_naive(ret_addr)) {
		println_serial("%hhx: # 0x%lx", depth, ret_addr);
		rbp = get_saved_rbp(rbp);
		ret_addr = get_ret_addr(rbp);
		depth++;
	}

	while (1) Halt();
}

// ブレークポイントに達したときに発火するIRQハンドラ。
static void on_break_point(struct regs_on_stack *regs)
{
	println_serial("break at 0x%lx", regs->rip);
	back_trace(regs);
	// ＴＯＤＯ：未実装
}

void jdb_init()
{
	set_irq_handler(IRQ_BP, on_break_point);
}
