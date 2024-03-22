#pragma once

#include "type.h"
#include "asm/io.h"

inline void Halt() { asm("hlt"); }

inline void IoOut8(io_addr_t addr, u8 value) {
        asm volatile (
                "outb   %0, %w1"
                :
                : "a" (value), "Nd" (addr)
        );
}

inline u8 IoIn8(io_addr_t addr) {
        u8 ret;
        asm volatile (
                "inb    %w1, %0"
                : "=a" (ret)
                : "Nd" (addr)
        );
        return ret;
}

inline void IoOut32(io_addr_t addr, u32 value) {
        asm volatile (
                "outl   %0, %w1"
                :
                // out命令では使用するレジスタが決まっていて、
                // eax（データ）とdx（アドレス）である。そのため、
                // インラインアセンブラでも使用するレジスタを
                // 指定してあげる必要がある。
                // なお、アドレスの指定には8-bit即値を用いることも
                // できるため、Nという制約も追加している。
                : "a" (value), "Nd" (addr)
        );
}

inline u32 IoIn32(io_addr_t addr) {
        u32 ret;
        asm volatile (
                "inl    %w1, %0"
                : "=a" (ret)
                : "Nd" (addr)
        );
        return ret;
}

// lidt命令を行う関数。lidt命令には、IDTのベースアドレスと
// そのサイズを指定する必要がある。
inline void load_idt(u16 size, u64 address)
{
        char lidt_val[10];
        *(u16 *) &lidt_val[0] = size;
        *(u64 *) &lidt_val[2] = address;
        asm volatile ("lidt %0" :: "m" (lidt_val));
}

// 現在のCSレジスタの値を取得する関数。
inline u16 get_cs()
{
        u16 ret;
        asm volatile ("movw %%cs, %0" : "=a" (ret));
        return ret;
}

#define X86_INT3() asm volatile ("int3");

inline u64 get_cr2()
{
        u64 ret;
	asm volatile (
		"movq	%%cr2, %0"
		: "=r" (ret)
	);
	return ret;
}
