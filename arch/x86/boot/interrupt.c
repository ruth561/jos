#include "interrupt.h"
#include "asm/cpu.h"
#include "console.h"
#include "logger.h"
#include "panic.h"
#include "type.h"
#include "compiler.h"
#include "assert.h"
#include "utils.h"


#define NR_IDT_ENTRIES	256

struct gate_desc {
	u16	offset_15_0;
	u16	ss;
	struct {
		u8	ist : 3;
		u8	_0 : 5;
		u8	type : 4;
		u8	_1 : 1;
		u8	dpl : 2;
		u8	p : 1;
	} bits;
	u16	offset_16_31;
	u32	offset_32_63;
	u32	_2;
} __attribute__((packed));

ALIGN(8) struct gate_desc idt[NR_IDT_ENTRIES] = {0};

// 割り込みハンドラの関数名を作成するマクロ。
// DEFINE_INT_HANDLER(name)マクロによって関数を実装しておく必要がある。
#define INT_HANDLER(name) name##_int_handler

// 割り込みハンドラの本体を実装するためのマクロ。
// interrupt_handler.hで定義されている同名のマクロと合わせて使う。
// 詳細は、interrupt_handler.hのファイル上部のコメントを参照。
//
// 使い方としては、以下のようにマクロのあとに関数実装を続けて書く。
//
// DEFINE_INT_HANDLER(pf)
// {
//         ...
// }
// 
// 関数の実装内部では`regs`という引数が使える。この引数の型は
// struct regs_on_stack *であり、割り込みが起きたときのレジスタの
// 状態を示している。
#define DEFINE_INT_HANDLER(name)					\
	void name##_int_handler();					\
	void do_##name##_int_handler(struct regs_on_stack *regs)

// カーネルパニックを引き起こすハンドラの基本実装。
#define DEFINE_INT_HANDLER_BASIC(name)				\
	DEFINE_INT_HANDLER(name)				\
	{							\
		print_regs(regs);				\
		PANIC("[!-- EXCEPTION (#" #name ") --!]");	\
	}

static void print_regs(struct regs_on_stack *regs)
{
	FATAL("%%rax = 0x%lx, %%rbx = 0x%lx, %%rcx = 0x%lx, %%rcx = 0x%lx",
		regs->rax, regs->rbx, regs->rcx, regs->rdx);
	FATAL("%%rsi = 0x%lx, %%rdi = 0x%lx, %%rbp = 0x%lx, %%rsp = 0x%lx",
		regs->rsi, regs->rdi, regs->rbp, (u64) regs); // regsが指している場所こそrspの値
	FATAL("%%r8  = 0x%lx, %%r9  = 0x%lx, %%r10 = 0x%lx, %%r11 = 0x%lx",
		regs->r8 , regs->r9 , regs->r10, regs->r11);
	FATAL("%%r12 = 0x%lx, %%r13 = 0x%lx, %%r14 = 0x%lx, %%r15 = 0x%lx",
		regs->r12, regs->r13, regs->r14, regs->r15);
	FATAL("%%rip = 0x%lx, %%cs  = 0x%lx, %%ss  = 0x%lx, %%rsp = 0x%lx",
		regs->rip, regs->cs , regs->ss , regs->rsp);
	FATAL("error_code = 0x%lx, %%rflags = 0x%lx",
		regs->error_code, regs->rflags);
}

#pragma clang optimize off
int align_check(void *p, u64 align)
{
	u64 addr = (u64) p;
	if (addr % align) {
		return false;
	} else {
		return true;
	}
}
#pragma clang optimize on

DEFINE_INT_HANDLER(DE)	// 0
{
	// 適切にスタックの16-bytesアラインメントが行われていれば、
	// ここの検証は通過できるはず。
#pragma clang optimize off
	ALIGN(16) int data;
	CHECK(align_check(&data, 16));
#pragma clang optimize on

	print_regs(regs);
	PANIC("Divide Error");
}

DEFINE_INT_HANDLER_BASIC(DB)	// 1
DEFINE_INT_HANDLER_BASIC(NMI)	// 2
DEFINE_INT_HANDLER_BASIC(BP)	// 3
DEFINE_INT_HANDLER_BASIC(OF)	// 4
DEFINE_INT_HANDLER_BASIC(BR)	// 5
DEFINE_INT_HANDLER_BASIC(UD)	// 6
DEFINE_INT_HANDLER_BASIC(NM)	// 7
DEFINE_INT_HANDLER_BASIC(GP)	// 13

DEFINE_INT_HANDLER(PF)		// 14
{
	print_regs(regs);
	PANIC("Page Fault");
}

DEFINE_INT_HANDLER_BASIC(MF)	// 16
DEFINE_INT_HANDLER_BASIC(MC)	// 18
DEFINE_INT_HANDLER_BASIC(XM)	// 19
DEFINE_INT_HANDLER_BASIC(VE)	// 20

static void *int_handlers[] = {
	&INT_HANDLER(DE),
	&INT_HANDLER(DB),
	&INT_HANDLER(NMI),
	&INT_HANDLER(BP),
	&INT_HANDLER(OF),
	&INT_HANDLER(BR),
	&INT_HANDLER(UD),
	&INT_HANDLER(NM),
	(void *) 0,
	(void *) 0,
	(void *) 0,
	(void *) 0,
	(void *) 0,
	&INT_HANDLER(GP),
	&INT_HANDLER(PF),
	(void *) 0,
	&INT_HANDLER(MF),
	(void *) 0,
	&INT_HANDLER(MC),
	&INT_HANDLER(XM),
	&INT_HANDLER(VE),
};

void set_idt_entry(int vector, void *handler)
{
	CHECK(0 <= vector && vector < NR_IDT_ENTRIES);

	u64 address = (u64) handler;

	struct gate_desc *desc = &idt[vector];
	desc->offset_15_0 = address & 0xFFFF;
	desc->offset_16_31 = (address >> 16) & 0xFFFF;
	desc->offset_32_63 = (address >> 32) & 0xFFFFFFFF;
	desc->bits.type = 14;
	desc->bits.dpl = 0;
	desc->bits.p = 1;
	desc->ss = get_cs();
}

void interrupt_init()
{
	CHECK(sizeof(struct gate_desc) == 16);
	CHECK(is_aligned(&idt, 8));

	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	DEBUG("interrupt_init");

	// IDTの設定
	int vector = 0;
	for (; vector < (sizeof(int_handlers) / sizeof(void *)); vector++) {
		set_idt_entry(vector, int_handlers[vector]);
	}
	INFO("Registered %d interrupt handler", vector);
	
	// IDTRレジスタにIDTを登録
	u16 size = NR_IDT_ENTRIES * sizeof(struct gate_desc);
	u64 address = (u64) idt;
	DEBUG("lidt.size = 0x%hx", size);
	DEBUG("lidt.address = 0x%lx", address);
	load_idt(size, address);
	
	set_log_level(log_level);
	INFO("Interrupt initialization completed.");

	*(volatile int *) 0xfffffffffffffff0= 3; // PF?
	*(volatile int *) 0xdeadbeefcafebabe = 3; // GP
}
