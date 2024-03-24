#include "interrupt.h"
#include "asm/cpu.h"
#include "console.h"
#include "display.h"
#include "intel8259.h"
#include "logger.h"
#include "panic.h"
#include "type.h"
#include "compiler.h"
#include "assert.h"
#include "utils.h"
#include "irq.h"


// 割り込みベクタから例外の名前への変換用配列。
static const char *exception_str[NR_EXCEPTIONS] = {
	"DE",
	"DB",
	"NMI",
	"BP",
	"OF",
	"BR",
	"UD",
	"NM",
	"DF",
	"CSO",
	"TS",
	"NP",
	"SS",
	"GP",
	"PF",
	"RSVD",
	"MF",
	"AC",
	"MC",
	"XM",
	"VE",
};

// IDTエントリのデータ構造
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

// 割り込みハンドラは割り込みベクタごとに別名で実装されており、その
// 関数名は"int<vector>_handler"である。このマクロは、その関数名を
// 作るためのマクロである。
#define INT_HANDLER_NAME(vector) int##vector##_handler

void INT_HANDLER_NAME(0)();	// EXCEPTION
void INT_HANDLER_NAME(1)();
void INT_HANDLER_NAME(2)();
void INT_HANDLER_NAME(3)();
void INT_HANDLER_NAME(4)();
void INT_HANDLER_NAME(5)();
void INT_HANDLER_NAME(6)();
void INT_HANDLER_NAME(7)();
void INT_HANDLER_NAME(8)();
void INT_HANDLER_NAME(9)();
void INT_HANDLER_NAME(10)();
void INT_HANDLER_NAME(11)();
void INT_HANDLER_NAME(12)();
void INT_HANDLER_NAME(13)();
void INT_HANDLER_NAME(14)();
void INT_HANDLER_NAME(15)();
void INT_HANDLER_NAME(16)();
void INT_HANDLER_NAME(17)();
void INT_HANDLER_NAME(18)();
void INT_HANDLER_NAME(19)();
void INT_HANDLER_NAME(20)();
void INT_HANDLER_NAME(32)();	// EXTERNAL INTERRUPT
void INT_HANDLER_NAME(33)();
void INT_HANDLER_NAME(34)();
void INT_HANDLER_NAME(35)();
void INT_HANDLER_NAME(36)();
void INT_HANDLER_NAME(37)();
void INT_HANDLER_NAME(38)();
void INT_HANDLER_NAME(39)();
void INT_HANDLER_NAME(40)();
void INT_HANDLER_NAME(41)();
void INT_HANDLER_NAME(42)();
void INT_HANDLER_NAME(43)();
void INT_HANDLER_NAME(44)();
void INT_HANDLER_NAME(45)();
void INT_HANDLER_NAME(46)();
void INT_HANDLER_NAME(47)();

static void *int_handlers[NR_IRQS] = {
	&INT_HANDLER_NAME(0),
	&INT_HANDLER_NAME(1),
	&INT_HANDLER_NAME(2),
	&INT_HANDLER_NAME(3),
	&INT_HANDLER_NAME(4),
	&INT_HANDLER_NAME(5),
	&INT_HANDLER_NAME(6),
	&INT_HANDLER_NAME(7),
	&INT_HANDLER_NAME(8),
	&INT_HANDLER_NAME(9),
	&INT_HANDLER_NAME(10),
	&INT_HANDLER_NAME(11),
	&INT_HANDLER_NAME(12),
	&INT_HANDLER_NAME(13),
	&INT_HANDLER_NAME(14),
	&INT_HANDLER_NAME(15),
	&INT_HANDLER_NAME(16),
	&INT_HANDLER_NAME(17),
	&INT_HANDLER_NAME(18),
	&INT_HANDLER_NAME(19),
	&INT_HANDLER_NAME(20),
	(void *) 21,
	(void *) 22,
	(void *) 23,
	(void *) 24,
	(void *) 25,
	(void *) 26,
	(void *) 27,
	(void *) 28,
	(void *) 29,
	(void *) 30,
	(void *) 31,
	&INT_HANDLER_NAME(32),
	&INT_HANDLER_NAME(33),
	&INT_HANDLER_NAME(34),
	&INT_HANDLER_NAME(35),
	&INT_HANDLER_NAME(36),
	&INT_HANDLER_NAME(37),
	&INT_HANDLER_NAME(38),
	&INT_HANDLER_NAME(39),
	&INT_HANDLER_NAME(40),
	&INT_HANDLER_NAME(41),
	&INT_HANDLER_NAME(42),
	&INT_HANDLER_NAME(43),
	&INT_HANDLER_NAME(44),
	&INT_HANDLER_NAME(45),
	&INT_HANDLER_NAME(46),
	&INT_HANDLER_NAME(47),
};

static void print_regs(struct regs_on_stack *regs)
{
	println_serial("%%rax = 0x%lx, %%rbx = 0x%lx, %%rcx = 0x%lx, %%rcx = 0x%lx",
		regs->rax, regs->rbx, regs->rcx, regs->rdx);
	println_serial("%%rsi = 0x%lx, %%rdi = 0x%lx, %%rbp = 0x%lx, %%rsp = 0x%lx",
		regs->rsi, regs->rdi, regs->rbp, (u64) regs); // regsが指している場所こそrspの値
	println_serial("%%r8  = 0x%lx, %%r9  = 0x%lx, %%r10 = 0x%lx, %%r11 = 0x%lx",
		regs->r8 , regs->r9 , regs->r10, regs->r11);
	println_serial("%%r12 = 0x%lx, %%r13 = 0x%lx, %%r14 = 0x%lx, %%r15 = 0x%lx",
		regs->r12, regs->r13, regs->r14, regs->r15);
	println_serial("%%rip = 0x%lx, %%cs  = 0x%lx, %%ss  = 0x%lx, %%rsp = 0x%lx",
		regs->rip, regs->cs , regs->ss , regs->rsp);
	println_serial("error_code = 0x%lx, %%rflags = 0x%lx",
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

// 共通の例外ハンドラとして実行される関数。
void common_exception_handler(struct regs_on_stack *regs)
{
	println_serial("[!-- EXCEPTION (#%s) --!]", exception_str[regs->vector]);
	print_regs(regs);
	PANIC("...");
}

// #PF例外ハンドラ
struct error_code_pf {
	u8	p: 1;
	u8	w_r: 1;
	u8	u_s: 1;
	u8	rsvd: 1;
	u8	i_d: 1;
	u8	pk: 1;
	u8	ss: 1;
	u8	hlat: 1;
	u8	_0: 7;
	u8	sgx: 1;
	u16	_1;
} __attribute__((packed));

void page_fault_handler(struct regs_on_stack *regs)
{
	// 適切にスタックの16-bytesアラインメントが行われていれば、
	// ここの検証は通過できるはず。
#pragma clang optimize off
	ALIGN(16) int data;
	CHECK(align_check(&data, 16));
#pragma clang optimize on

	print_regs(regs);

	struct error_code_pf *ec = (struct error_code_pf *) &regs->error_code;

	println_serial("[!-- EXCEPTION (#PF) --!]");
	println_serial("Instruction Address: 0x%lx", regs->rip);
	println_serial("Page-Fault Address: 0x%lx", get_cr2());
	println_serial("Error Cause:")
	println_serial("\tCaused by %s.", ec->p ? "PAGE-LEVEL PROTECTION" : "NON-PRESENT PAGE");
	println_serial("\tCaused by %s access.", ec->w_r ? "WRITE" : "READ");
	println_serial("\tCaused by %s-mode access.", ec->u_s ? "USER" : "SUPERVISOR");
	println_serial("\t%s by RESERVED bit violation.", ec->rsvd ? "Caused" : "Not caused");
	println_serial("\t%s by INSTRUCTION FETCH.", ec->i_d ? "Caused" : "Not caused");
	println_serial("\t%s by PROTECTION-KEY violation.", ec->pk ? "Caused" : "Not caused");
	println_serial("\t%s by SHADOW-STACK access.", ec->ss ? "Caused" : "Not caused");
	println_serial("\t%s by HLAT paging.", ec->hlat ? "Caused" : "Not caused");
	println_serial("\t%s by SGX-specific violation.", ec->sgx ? "Caused" : "Not caused");
	PANIC("...");
}

// すべての割り込みハンドラはこの関数を呼び出すことになっている。
// regs->vectorをもとに、その割り込みがどこから送られてきたのかを判断し、
// それぞれの処理に分配する。
void do_common_int_handler(struct regs_on_stack *regs)
{
	// スタックのアラインメントチェック
	// この関数を呼び出す関数はアセンブリで記述しているので、
	// スタックのアラインメント制約を守れているか、を確認する。
	ALIGN(16) int data;
	CHECK(align_check(&data, 16));
	get_irq_handler(regs->vector)(regs);
}

void set_idt_entry(int vector, void *handler)
{
	CHECK(0 <= vector && vector < NR_IDT_ENTRIES);

	u64 address = (u64) handler;

	struct gate_desc *desc = &idt[vector];
	memset(desc, 0, sizeof(struct gate_desc));
	desc->offset_15_0 = address & 0xFFFF;
	desc->offset_16_31 = (address >> 16) & 0xFFFF;
	desc->offset_32_63 = (address >> 32) & 0xFFFFFFFF;
	desc->bits.type = 14;
	desc->bits.dpl = 0;
	desc->bits.p = 1;
	desc->ss = get_cs();
	DEBUG("gate (%hx): %lx %lx", vector, *(u64 *) &idt[vector], *((u64 *) &idt[vector] + 1));
}

void interrupt_init()
{
	CHECK(sizeof(struct gate_desc) == 16);
	CHECK(is_aligned(&idt, 8));
	CHECK(sizeof(struct error_code_pf) == 4);

	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	DEBUG("interrupt_init");

	// IDTの設定
	// ※ irqとvectorの番号が対応関係にある。
	int vector = 0;
	for (; vector < NR_IRQS; vector++) {
		set_idt_entry(vector, int_handlers[vector]);
	}

	// IRQの初期化をここで行う。
	irq_init();

	// デフォルトのIRQハンドラを設定する。
	irq_t irq = 0;
	for (; irq < NR_EXCEPTIONS; irq++) {
		set_irq_handler(irq, common_exception_handler);
	}

	// 特別なIRQハンドラを設定する。
	set_irq_handler(IRQ_PF, page_fault_handler); // 14はハードコーディング（ＴＯＤＯ：）
	
	// IDTRレジスタにIDTを登録
	u16 size = NR_IDT_ENTRIES * sizeof(struct gate_desc);
	u64 address = (u64) idt;
	DEBUG("lidt.size = 0x%hx", size);
	DEBUG("lidt.address = 0x%lx", address);
	load_idt(size, address);

	// 割り込みコントローラの初期化を行う
	intel8259_init();

	// 割り込みを有効にし、
	// IFフラグがセットされていることを確認する。
	X86_STI();
	u64 rflags = get_rflags();
	DEBUG("RFLAGS: %lx", rflags);

	set_log_level(log_level);
	INFO("Interrupt initialization completed.");
}
