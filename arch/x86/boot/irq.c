#include "irq.h"
#include "assert.h"
#include "intel8259.h"
#include "logger.h"
#include "panic.h"


void unregistered_irq_handler(struct regs_on_stack *regs)
{
	PANIC("Unregistered IRQ #%d handler.", regs->vector);
}

irq_handler_t irq_handlers[NR_IRQS];

void irq_eoi(irq_t irq)
{
	if (ISA_IRQ_BASE <= irq && irq < ISA_IRQ_BASE + NR_ISA_IRQS) {
		intel8259_end_of_interrupt(irq_to_isa_irq(irq));
	} else {
		PANIC("Cannot send EOI to unregistered IRQ #%d", irq);
	}
}

void set_irq_handler(irq_t irq, irq_handler_t handler)
{
	CHECK(0 <= irq && irq < NR_IRQS);
	irq_handlers[irq] = handler;
}

irq_handler_t get_irq_handler(irq_t irq)
{
	CHECK(0 <= irq && irq < NR_IRQS);
	return irq_handlers[irq];
}

void irq_init()
{
	INFO("irq_init()");
	for (irq_t irq = 0; irq < NR_IRQS; irq++) {
		irq_handlers[irq] = unregistered_irq_handler;
	}
}

