#pragma once

#include "asm/io.h"

inline void Halt() { asm("hlt"); }

inline void IoOut8(io_addr_t addr, unsigned char value) {
        asm volatile (
                "outb   %0, %w1"
                :
                : "a" (value), "Nd" (addr)
        );
}

inline unsigned char IoIn8(io_addr_t addr) {
        unsigned char ret;
        asm volatile (
                "inb    %w1, %0"
                : "=a" (ret)
                : "Nd" (addr)
        );
        return ret;
}

inline void IoOut32(io_addr_t addr, unsigned int value) {
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

inline unsigned int IoIn32(io_addr_t addr) {
        unsigned int ret;
        asm volatile (
                "inl    %w1, %0"
                : "=a" (ret)
                : "Nd" (addr)
        );
        return ret;
}
