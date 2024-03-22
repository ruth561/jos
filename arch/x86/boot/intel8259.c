#include "intel8259.h"
#include "asm/cpu.h"
#include "assert.h"
#include "logger.h"


// ポート番号（PIC0がプライマリ、PIC1がセカンダリ）
#define PIC0_COMM	0x20
#define PIC0_DATA	0x21
#define PIC1_COMM	0xA0
#define PIC1_DATA	0xA1

// Initialization Control Word
#define ICW1_INIT	0x10
#define ICW1_ICW4	0x01

#define ICW4_8086	0x01


void remap(u8 primary_vector_offset, u8 secondary_vector_offset)
{
	u8 pic0_mask = inb(PIC0_DATA);
	u8 pic1_mask = inb(PIC1_DATA);
	DEBUG("pic0_mask: 0x%hhx", pic0_mask);
	DEBUG("pic1_mask: 0x%hhx", pic1_mask);

	// プライマリの初期化
	outb(PIC0_COMM, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC0_DATA, primary_vector_offset);
	io_wait();
	outb(PIC0_DATA, 0b00000100); // 第2ピンにセカンダリが接続されている
	io_wait();
	outb(PIC0_DATA, ICW4_8086);
	io_wait();

	// セカンダリの初期化
	outb(PIC1_COMM, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, secondary_vector_offset);
	io_wait();
	outb(PIC1_DATA, 2); // 第2ピンにセカンダリが接続されている
	io_wait();
	outb(PIC1_DATA, ICW4_8086);
	io_wait();

	outb(PIC0_DATA, pic0_mask);
	outb(PIC1_DATA, pic1_mask);
}

void clear_mask(int irq)
{
	DEBUG("clear_mask: clear IRQ #%d", irq);
	if (irq < 8) { // primary
		u8 mask = inb(PIC0_DATA);
		io_wait();
		DEBUG("clear_mask: primary mask 0x%hhx -> 0x%hhx", mask, mask & ~(1 << irq));
		outb(PIC0_DATA, mask & ~(1 << irq));
	} else { // secondary
		irq -= 8;
		CHECK(irq < 8);
		u8 mask = inb(PIC1_DATA);
		io_wait();
		DEBUG("clear_mask: secondary mask 0x%hhx -> 0x%hhx", mask, mask & ~(1 << irq));
		outb(PIC1_DATA, mask & ~(1 << irq));
	}
	INFO("Clear IRQ #%d mask.", irq);
}

#define PIC_EOI	0x20

void intel8259_end_of_interrupt(int irq)
{
	if (irq >= 8) {
		outb(PIC1_COMM, PIC_EOI);
	}
	
	outb(PIC0_COMM, PIC_EOI);
}

void intel8259_init()
{
	int log_level = set_log_level(LOG_LEVEL_DEBUG);
	INFO("started intel8259_init()");

	remap(0x20, 0x28);
	clear_mask(3);
	clear_mask(4);
	
	INFO("ended intel8259_init()");
	set_log_level(log_level);
}
