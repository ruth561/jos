#include "segment.h"
#include "asm/cpu.h"
#include "logger.h"
#include "panic.h"
#include "type.h"
#include "assert.h"


struct desc_struct {
	u16	limit;
	u16	base_0_15;
	u8	base_16_23;
	u8	type: 4;
	u8	s: 1;
	u8	dpl: 2;
	u8	p: 1;
	u8	seg_limit: 4;
	u8	avl: 1;
	u8	l: 1;
	u8	d: 1;
	u8	g: 1;
	u8	base_24_31;
} __attribute__((packed));

struct selector_struct {
	u16	rpl: 2;
	u16	ti: 1;
	u16	index: 13;
} __attribute__((packed));

// GDTに設定されている値を取得する関数
void get_gdt(u16 *limit, u64 *base)
{
	u8 buf[10];
	u64 addr = (u64) buf;
	asm volatile (
		"sgdt	(%%rax)" 
		:
		: "a" (addr)
		: "memory"
	);
	*limit = *(u16 *) &buf[0];
	*base  = *(u64 *) &buf[2];
}

void inspect_current_gdt()
{
	u16 limit;
	u64 base;
	get_gdt(&limit, &base);
	DEBUG("LIMIT = 0x%hx", limit);
	DEBUG("BASE  = 0x%lx", base);

	u16 idx = 0;
	struct desc_struct *desc = (struct desc_struct *) base;
	CHECK(sizeof(struct desc_struct) == 8);
	CHECK(sizeof(struct selector_struct) == 2);
	DEBUG("========== GDT ==========");
	for (; (u64) desc < base + limit; idx++, desc++) {
		DEBUG("SEGMENT DESCRIPTOR (0x%hx): %lx\t[ %s%s%s%s%s ] [ DPL = %d ] [ TYPE = %d ]", 
			idx, *desc,
			desc->s ? "S" : "-",
			desc->p ? "P" : "-",
			desc->l ? "L" : "-",
			desc->d ? "D" : "-",
			desc->g ? "G" : "-",
			desc->dpl, desc->type);
	}

	struct selector_struct cs;
	u16 cs_val = get_cs();
	*(u16 *) &cs = cs_val;
	DEBUG("CS: %hx\t[ INDEX = %d ] [ TI = %d ] [ RPL = %d ]",
		cs_val, cs.index, cs.ti, cs.rpl);
}

void segment_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started segment_init()");

	inspect_current_gdt();

	INFO("ended segment_init()");
	set_log_level(log_level);
}
