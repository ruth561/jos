#include "jdb.h"
#include "assert.h"
#include "console.h"
#include "interrupt.h"
#include "irq.h"
#include "logger.h"
#include "panic.h"
#include "string.h"
#include "type.h"


// JDBの状態を表すマクロ
#define JDB_STATE_STOP		0
#define JDB_STATE_RUNNING	1

// JDBの現在の状態
static int jdb_state = JDB_STATE_RUNNING;
static struct regs_on_stack *jdb_regs;

static void new_cmd();

// その値が有効なアドレス値かどうかを検証する関数
static bool is_valid_address_naive(u64 addr)
{
	return 0x100000 <= addr && addr < 0x200000;
}

// ブレークポイントに達したときに発火するIRQハンドラ。
static void on_break_point(struct regs_on_stack *regs)
{
	CHECK(jdb_state == JDB_STATE_RUNNING);
	jdb_state = JDB_STATE_STOP;
	jdb_regs = regs;

	println_serial("break at 0x%lx", regs->rip);
	new_cmd();

	// シリアルから入力を受け付け、プログラムを再度実行するまで待機する。
	while (jdb_state == JDB_STATE_STOP) {
		Halt();
	}

	CHECK(jdb_state != JDB_STATE_STOP);
}


//
// jdb shellの実装
//

#define CMD_BUF_SIZE	80
static int idx;
static char cmd_buf[CMD_BUF_SIZE];
static const char *prompt = "(jdb) ";

typedef void (*cmd_func_t)(const char *sub_cmd);

// 1つのコマンドに関する情報をまとめた構造体
// コマンド名とそのコマンドを処理する
struct cmd_desc {
	const char *cmd;
	usize cmd_size;
	cmd_func_t cmd_func;
};

static void do_info(const char *sub_cmd);
static void do_continue(const char *sub_cmd);
static void do_backtrace(const char *sub_cmd);

#define CMDS_ENTRY(name, func)	\
	{ .cmd = name, .cmd_size = sizeof(name), .cmd_func = func }

static struct cmd_desc cmds[] = {
	CMDS_ENTRY("info", do_info),
	CMDS_ENTRY("continue", do_continue),
	CMDS_ENTRY("backtrace", do_backtrace),
};

// 新しいコマンドを受け付ける準備をする関数
static void new_cmd()
{
	idx = 0;
	print_serial(prompt);
}

// コマンドを処理するメイン部分
static void do_cmd()
{
	CHECK(0 <= idx);
	CHECK(idx < CMD_BUF_SIZE);

	cmd_buf[idx] = '\0';
	for (int i = 0; i < sizeof(cmds) / sizeof(struct cmd_desc); i++) {
		if (match_prefix(cmd_buf, cmds[i].cmd)) {
			cmds[i].cmd_func(&cmd_buf[cmds[i].cmd_size]);
			goto done;
		}
	}
	println_serial("Unknown command: %s", cmd_buf);

done:
	if (jdb_state == JDB_STATE_STOP) {
		new_cmd();
	}
}

// シリアル通信で文字を受け取ったときに呼ばれるコールバック
// 状態がJDB_STATE_STOPのときにのみ入力を受け付ける。
static void serial_recv_callback(char c)
{
	if (jdb_state == JDB_STATE_RUNNING) {
		return;
	}

	switch (c) {
		// ＴＯＤＯ：シリアル通信における改行文字の使われ方がよくわかっていない、、
		case '\r':
		case '\n':
			println_serial("");
			do_cmd();
			break;
		default:
			putc_serial(c);
			if (idx < CMD_BUF_SIZE - 1) {
				cmd_buf[idx++] = c;
			} else {
				ERROR("cmd_buf is full!");
			}
	}
}

void jdb_init()
{
	jdb_state = JDB_STATE_RUNNING;
	register_serial_recv_callback(serial_recv_callback);
	set_irq_handler(IRQ_BP, on_break_point);
}


//
// JDBコマンドの実装
//

static void do_info(const char *sub_cmd)
{
	DEBUG("do_info");
	if (match_prefix(sub_cmd, "regs")) {
		println_serial("%%rax = 0x%lx, %%rbx = 0x%lx, %%rcx = 0x%lx, %%rcx = 0x%lx",
			jdb_regs->rax, jdb_regs->rbx, jdb_regs->rcx, jdb_regs->rdx);
		println_serial("%%rsi = 0x%lx, %%rdi = 0x%lx, %%rbp = 0x%lx, %%rsp = 0x%lx",
			jdb_regs->rsi, jdb_regs->rdi, jdb_regs->rbp, (u64) jdb_regs); // jdb_regsが指している場所こそrspの値
		println_serial("%%r8  = 0x%lx, %%r9  = 0x%lx, %%r10 = 0x%lx, %%r11 = 0x%lx",
			jdb_regs->r8 , jdb_regs->r9 , jdb_regs->r10, jdb_regs->r11);
		println_serial("%%r12 = 0x%lx, %%r13 = 0x%lx, %%r14 = 0x%lx, %%r15 = 0x%lx",
			jdb_regs->r12, jdb_regs->r13, jdb_regs->r14, jdb_regs->r15);
		println_serial("%%rip = 0x%lx, %%cs  = 0x%lx, %%ss  = 0x%lx, %%rsp = 0x%lx",
			jdb_regs->rip, jdb_regs->cs , jdb_regs->ss , jdb_regs->rsp);
		println_serial("error_code = 0x%lx, %%rflags = 0x%lx",
			jdb_regs->error_code, jdb_regs->rflags);
	}
}

static void do_continue(const char *sub_cmd)
{
	DEBUG("do_continue");
	jdb_state = JDB_STATE_RUNNING;
}

static void do_backtrace(const char *sub_cmd)
{
	DEBUG("do_backtrace");
	// スタックは以下のような状態になっている。
	// |-------------|
	// |   ret addr  | 
	// |-------------|
	// |  saved rbp  | <--- rbp
	// |-------------|

	u32 depth = 0;
	println_serial("%hhx: # 0x%lx <-- rip", depth++, jdb_regs->rip);

	u64 rbp = jdb_regs->rbp;
	u64 ret_addr = *(u64 *) (rbp + 8);
	while (is_valid_address_naive(ret_addr)) {
		println_serial("%hhx: # 0x%lx", depth++, ret_addr);
		rbp = *(u64 *) (rbp);
		ret_addr = *(u64 *) (rbp + 8);
	}
}
