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

// cpuid命令
// どの引数も出力に使われる。eaxのみ、入力にも使われる。
void cpuid(u32 *eax, u32 *ebx, u32 *ecx, u32 *edx)
{
	asm volatile (
		"movl	%0, %%eax\n\t"
		"cpuid\n\t"
		"movl	%%eax, %0\n\t"
		"movl	%%ebx, %1\n\t"
		"movl	%%ecx, %2\n\t"
		"movl	%%edx, %3\n\t" 
		: "+r" (*eax), "+r" (*ebx), "+r" (*ecx), "+r" (*edx)
		:
		: "eax", "ebx", "ecx", "edx"
	);
}

// CPUベンダを確認する
void verify_cpu_vendor()
{
	u32 eax, ebx, ecx, edx;
	char vendor_id_buf[13];
	eax = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	
	u32 *p = (u32 *) vendor_id_buf;
	*p++ = ebx;
	*p++ = edx;
	*p++ = ecx;
	vendor_id_buf[12] = '\0';

	DEBUG("CPU VENDOR ID: \"%s\"", vendor_id_buf);
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

	u32 eax, ebx, ecx, edx;
	eax = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	DEBUG("CPUID.%hhxH: eax = 0x%x, ebx = 0x%x, ecx = 0x%x, edx = 0x%x",
		0, eax, ebx, ecx, edx);
	verify_cpu_vendor();

	while (1);

	INFO("ended processor_init()");
	set_log_level(log_level);

	PANIC("...");
}
