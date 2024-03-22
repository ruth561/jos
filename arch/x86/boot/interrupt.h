#pragma once

#include "type.h"


// 割り込みハンドラでスタックに積まれるレジスタ。
// 割り込みハンドラのメイン処理部分の引数で渡される。
struct regs_on_stack {
	u64 rax;
	u64 rbx;
	u64 rcx;
	u64 rdx;
	u64 rdi;
	u64 rsi;
	u64 rbp;
	u64 r8;
	u64 r9;
	u64 r10;
	u64 r11;
	u64 r12;
	u64 r13;
	u64 r14;
	u64 r15;

	u64 vector;
	u64 error_code;
	u64 rip;
	u64 cs;
	u64 rflags;
	u64 rsp; // 割り込みを発生させたときのRSPの値
	u64 ss;
};

typedef void (*int_handler_t)(struct regs_on_stack *);

// ＴＯＤＯ：
void add_int_handler(u8 vector, int_handler_t handler);

// 割り込みの初期化。
// 各種例外ハンドラの設定などを行う。
void interrupt_init();
