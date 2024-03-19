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

__attribute__((interrupt()))
void basic_int_handler(void *frame)
{
	while (1);
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
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	DEBUG("interrupt_init");

	set_idt_entry(0, &basic_int_handler);
	
	u16 size = NR_IDT_ENTRIES * sizeof(struct gate_desc);
	u64 address = (u64) idt;
	DEBUG("lidt.size = 0x%hx", size);
	DEBUG("lidt.address = 0x%lx", address);
	load_idt(size, address);
	
	set_log_level(log_level);

	INFO("Interrupt initialization completed.");

	// ゼロ除算を行う
	asm volatile (
		"movl $0, %eax\n\t"
    		"movl $1, %ecx\n\t"
		"divl %ecx"
	);
}
