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

#define CPUID_FEAT_EDX_MSR	(1U << 5)

void verify_cpu_features()
{
	u32 eax, ebx, ecx, edx;

	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	
	DEBUG("CPU FEATURES: %%eax = 0x%x", eax);
	DEBUG("CPU FEATURES: %%ebx = 0x%x", ebx);
	DEBUG("CPU FEATURES: %%ecx = 0x%x", ecx);
	DEBUG("CPU FEATURES: %%edx = 0x%x", edx);
}

// MSR機能を持っているか？
bool check_cpu_feat_msr()
{
	u32 eax, ebx, ecx, edx;

	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	
	return (edx & CPUID_FEAT_EDX_MSR) != 0;
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

// MSR_ADDR_*
#define MSR_ADDR_IA32_EFER 0xC0000080

static u64 read_msr(u32 addr)
{
	u32 low, high;
	asm volatile (
		"rdmsr"
		: "=a" (low), "=d" (high)
		: "c" (addr)
	);
	return (((u64) high) << 32) | (u64) low;
}

// MSR_EFER_*
#define MSR_EFER_F_SCE	(1ULL <<  0) // (R/W) SYSCALL Enable
#define MSR_EFER_F_LME	(1ULL <<  8) // (R/W) IA-32e Mode Enable
#define MSR_EFER_F_LMA	(1ULL << 10) //  (R)  IA-32e Mode Active
#define MSR_EFER_F_NXE	(1ULL << 11) // (R/W) Execute Disable Bit Enable

void processor_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started processor_init()");

	// リアルモードでないことを確認
	u64 cr0 = get_cr0();
	CHECK(cr0 & CR0_F_PE); // not Real mode
	DEBUG("CR0: 0x%lx", cr0);
	
	// 仮想8086モードでないことを確認
	u64 rflags = get_rflags();
	CHECK(!(rflags & RFLAGS_F_VM)); // not Virtual-8086 mode
	DEBUG("RFLAGS: 0x%lx", rflags);

	// 各種CPUの機能が存在していることを確認
	CHECK(check_cpuid_feature()); // CPUがCPUID命令を実装しているか
	INFO("Processor supports CPUID instruction.");

	verify_cpu_vendor();
	verify_cpu_features();
	CHECK(check_cpu_feat_msr()); // CPUがMSRレジスタをもっているか
	INFO("Processor supports RDMSR and WRMSR instructions.");

	// IA-32eモードであることを確認
	u64 msr_efer = read_msr(MSR_ADDR_IA32_EFER);
	DEBUG("MSR[IA32_EFER] = %lx", msr_efer);
	CHECK(msr_efer & MSR_EFER_F_LMA);
	INFO("Processor is IA-32e mode.");
	// ＴＯＤＯ：

	INFO("ended processor_init()");
	set_log_level(log_level);
}
