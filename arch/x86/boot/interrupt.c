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

// #DE
DEFINE_INT_HANDLER(de)
{
	print_regs(regs);
	PANIC("Divide Error");
}

void set_idt_entry(int vector, void *handler)
{
	CHECK(0 <= vector && vector < NR_IDT_ENTRIES);
	CHECK(is_aligned(&idt, 8));
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

	set_idt_entry(0, &INT_HANDLER(de));
	
	u16 size = NR_IDT_ENTRIES * sizeof(struct gate_desc);
	u64 address = (u64) idt;
	DEBUG("lidt.size = 0x%hx", size);
	DEBUG("lidt.address = 0x%lx", address);
	load_idt(size, address);
	
	set_log_level(log_level);

	INFO("Interrupt initialization completed.");

	// ゼロ除算を行う
	asm volatile (
		"movq $-1, %rdx\n\t"
		"movl $0, %eax\n\t"
    		"movl $1, %ecx\n\t"
		"divl %ecx"
	);
}
