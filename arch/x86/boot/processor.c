#include "processor.h"
#include "assert.h"
#include "logger.h"
#include "panic.h"
#include "type.h"


#define RFLAGS_F_VM	(1ULL << 17)
#define RFLAGS_F_ID	(1ULL << 21)

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

static void set_rflags(u64 rflags)
{
	asm volatile (
		"push	%0\n\t"
		"popfq"
		:: "r" (rflags)
		: "memory"
	);
}

// プロセッサがCPUIDの機能を持っているのかどうかを確かめる。
static bool check_cpuid_feature()
{
	u64 rflags = get_rflags();
	DEBUG("check_cpuid_feature: rflags = %lx", rflags);
	set_rflags(rflags | RFLAGS_F_ID);
	rflags = get_rflags();
	DEBUG("check_cpuid_feature: rflags = %lx", rflags);
	return (rflags & RFLAGS_F_ID) == RFLAGS_F_ID;
}

#define CR0_F_PE	(1ULL << 0)

static u64 get_cr0()
{
	u64 cr0;
	asm volatile (
		"movq	%%cr0, %0"
		: "=r" (cr0)
	);
	return cr0;
}

void processor_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started processor_init()");

	u64 cr0 = get_cr0();
	CHECK(cr0 & CR0_F_PE); // not Real mode
	DEBUG("CR0: 0x%lx", cr0);
	
	u64 rflags = get_rflags();
	CHECK(!(rflags & RFLAGS_F_VM)); // not Virtual-8086 mode
	DEBUG("RFLAGS: 0x%lx", rflags);

	CHECK(check_cpuid_feature());

	while (1);

	INFO("ended processor_init()");
	set_log_level(log_level);

	PANIC("...");
}
