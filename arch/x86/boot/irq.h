#pragma once

// 割り込みを抽象化したレイヤー。以下のように、アーキテクチャごとの
// 割り込み処理を抽象化し、ドライバなどに提供する。
// interrupt.c ---> irq.c ---> other_source_file.c
// できればアーキテクチャ固有のところに配置したくない（ＴＯＤＯ：）。
//
// 抽象化の方法としては、ＴＯＤＯ：

#include "interrupt.h"
#include "type.h"


#define NR_IRQS		48	// とりあえず

typedef int irq_t;
typedef void (*irq_handler_t)(struct regs_on_stack *);

void set_irq_handler(irq_t irq, irq_handler_t handler);

irq_handler_t get_irq_handler(irq_t irq);

void irq_init();
