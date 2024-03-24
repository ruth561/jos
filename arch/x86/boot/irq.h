#pragma once

// 割り込みを抽象化したレイヤー。以下のように、アーキテクチャごとの
// 割り込み処理を抽象化し、ドライバなどに提供する。
// interrupt.c ---> irq.c ---> other_source_file.c
// できればアーキテクチャ固有のところに配置したくない（ＴＯＤＯ：）。
//
// 抽象化された割り込みは、IRQという識別番号を持つ。
// 例えば、割り込みをIRQ=40に送信した場合、IRQ=40に登録された
// 割り込みハンドラが実行されることになっている。
//
// IRQの定義はアーキテクチャによって変わるが、x86の場合は
// IDTのベクタと一致する。IRQ #40とは割り込みベクタが40のものである。


#include "interrupt.h"
#include "type.h"


#define NR_IRQS		48	// とりあえず

typedef int irq_t;
typedef void (*irq_handler_t)(struct regs_on_stack *);

// ISA IRQのベース値
// ISA IRQは全部で16本あり、8本ごとにまとめてシステムのIRQにマップされる。
// マップされるベース値のことをここで定義している。
//
// 例えば、ISA IRQ #n（n = 0, 1, ..., 15）の割り込みは
// IRQ #(n + ISA_IRQ_BASE)としてシステムに通知される。
#define ISA_IRQ_BASE	0x20
#define ISA_IRQ_COM1	4
#define ISA_IRQ_COM2	3

// ISA IRQからIRQへの変換関数
static inline irq_t isa_irq_to_irq(irq_t isa_irq)
{
	return isa_irq + ISA_IRQ_BASE;
}

void set_irq_handler(irq_t irq, irq_handler_t handler);

irq_handler_t get_irq_handler(irq_t irq);

void irq_init();
