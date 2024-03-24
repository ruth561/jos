#include "jdb.h"
#include "assert.h"
#include "console.h"
#include "interrupt.h"
#include "irq.h"
#include "logger.h"
#include "panic.h"
#include "type.h"


// JDBの状態を表すマクロ
#define JDB_STATE_STOP		0
#define JDB_STATE_RUNNING	1

// JDBの現在の状態
int jdb_state = JDB_STATE_RUNNING;

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
	CHECK(jdb_state == JDB_STATE_RUNNING);
	jdb_state = JDB_STATE_STOP;

	println_serial("break at 0x%lx", regs->rip);
	back_trace(regs);

	// シリアルから入力を受け付け、プログラムを再度実行するまで待機する。
	while (jdb_state == JDB_STATE_STOP) {
		Halt();
	}

	CHECK(jdb_state != JDB_STATE_STOP);
}

// jdb shellの実装

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

static void do_info();
static void do_continue();

#define CMDS_ENTRY(name, func)	\
	{ .cmd = name, .cmd_size = sizeof(name), .cmd_func = func }

static struct cmd_desc cmds[] = {
	CMDS_ENTRY("info", do_info),
	CMDS_ENTRY("continue", do_continue),
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
	new_cmd();
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
	new_cmd();
	register_serial_recv_callback(serial_recv_callback);
	set_irq_handler(IRQ_BP, on_break_point);
}


//
// JDBコマンドの実装
//

static void do_info()
{
	println_serial("do_info");
}

static void do_continue()
{
	println_serial("do_continue");
}
