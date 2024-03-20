#include "processor.h"
#include "assert.h"
#include "logger.h"
#include "panic.h"


static u64 get_rflags()
{
	u64 rflags;
	asm volatile (
		"pushfq\n\t"
		"popq	%0"
		: "=r" (rflags)
		:
		: "memory"
	);
	return rflags;
}

void processor_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started processor_init()");

	u64 rflags = get_rflags();
	DEBUG("RFLAGS: 0x%x", rflags);
	while (1);

	INFO("ended processor_init()");
	set_log_level(log_level);

	PANIC("...");
}
