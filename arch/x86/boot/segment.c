#include "segment.h"
#include "logger.h"
#include "panic.h"
#include "type.h"


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
	u64 *desc = (u64 *) base;
	for (; (u64) desc < base + limit; idx++, desc++) {
		DEBUG("SEGMENT DESCRIPTOR (0x%hx): %lx", idx, *desc);
	}
}

void segment_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started segment_init()");

	inspect_current_gdt();

	INFO("ended segment_init()");
	set_log_level(log_level);
	PANIC("FIN~");
}
