#include "asm/cpu.h"
#include "asm/io.h"
#include "console.h"
#include "intel8259.h"
#include "interrupt.h"
#include "irq.h"
#include "panic.h"
#include "serial.h"


#define COM1_PORT 0x3F8
#define COM2_PORT 0x2F8

#define SERIAL_REG_DIVISOR_LOW          0
#define SERIAL_REG_DIVISOR_HIGH         1

#define SERIAL_REG_DATA                 0
#define SERIAL_REG_INT_ENABLE           1
#define SERIAL_REG_INT_IDENT            2
#define SERIAL_REG_LINE_CONTROL         3
#define SERIAL_REG_MODEM_CONTROL        4
#define SERIAL_REG_LINE_STATUS          5
#define SERIAL_REG_MODEM_STATUS         6
#define SERIAL_REG_SCRATCH              7

// Line Control Register Bits
#define LCTL_BITS_DATA_5        0b00000000
#define LCTL_BITS_DATA_6        0b00000001
#define LCTL_BITS_DATA_7        0b00000010
#define LCTL_BITS_DATA_8        0b00000011

#define LCTL_BITS_STOP_1        0b00000000
#define LCTL_BITS_STOP_2        0b00000100

#define LCTL_BITS_PARITY_NONE   0b00000000

#define LCTL_BITS_8N1 (LCTL_BITS_DATA_8 | LCTL_BITS_PARITY_NONE | LCTL_BITS_STOP_1)

// Modem Control Register Bits
#define MCTL_LOOPBACK   0b00010000

// Line Status Register Bits
#define LSTATUS_BIT_DR          0 // Data Ready
#define LSTATUS_BIT_THRE        5 // Transmitter Holding Register Empty

// グローバルに使われるシリアルポートのI/Oアドレス空間のベースアドレス。
// 複数のシリアルポートがある場合は、その中の1つがここにセットされる。
// ＴＯＤＯ：シリアルポートが見当たらなかったときの処理は実装していない。
io_addr_t global_serial_port;

// シリアルポートのボーレートを設定する関数。
//      - port: ポートのI/Oアドレス
//      - divisor: ボーレートを決める値（115200 / divisorがボーレートになる）
static void set_baud_rate(io_addr_t port, u16 divisor) {
        // DLAB-bitをセットする。
        outb(port + SERIAL_REG_LINE_CONTROL, 0x80);
        // ボーレートの除数を書き込む。
        outb(port + SERIAL_REG_DIVISOR_LOW, divisor & 0xFF);
        outb(port + SERIAL_REG_DIVISOR_HIGH, (divisor >> 8) & 0xFF);
}

// ラインプロトコルをlctlで表された値に設定する関数。
// lctlに使えるフラグはLCTL_BITS_*マクロで定義されている。
static void set_line_protocol(io_addr_t port, unsigned char lctl)
{
        outb(port + SERIAL_REG_LINE_CONTROL, LCTL_BITS_8N1);
}

#pragma clang optimize off
static void busy_loop()
{
        for (int i = 0; i < 10000000; i++);
}
#pragma clang optimize on

// モデムをループバックモードにして、送受信のテストを行う。
// テストをパスした場合は0を返し、パスしなかった場合は-1を返す。
static int loopback_test(io_addr_t port)
{
        u8 data;
        u8 save_mctl = inb(port + SERIAL_REG_MODEM_CONTROL);
        println_display("save_mctl: 0x%hhx", save_mctl);
        outb(port + SERIAL_REG_MODEM_CONTROL, MCTL_LOOPBACK);

        // 書き込んでからすぐに読み出しを行うと、テストに失敗する可能性があるので、
        // 途中にビジーループを挟むとよい。
        outb(port + SERIAL_REG_DATA, 0xAB);
        busy_loop();
        data = inb(port + SERIAL_REG_DATA);
        println_display("[ LOOPBACK TEST ] write: 0xAB, read: 0x%hhx", data);
        if (data != 0xAB) {
                outb(port + SERIAL_REG_MODEM_CONTROL, save_mctl);
                return -1;
        }

        outb(port + SERIAL_REG_DATA, 0x34);
        busy_loop();
        data = inb(port + SERIAL_REG_DATA);
        println_display("[ LOOPBACK TEST ] write: 0x34, read: 0x%hhx", data);
        if (data != 0x34) {
                outb(port + SERIAL_REG_MODEM_CONTROL, save_mctl);
                return -1;
        }

        // restore
        outb(port + SERIAL_REG_MODEM_CONTROL, save_mctl);
        return 0;
}

// portの初期化を行う関数
//      - port: ポートのI/Oアドレス空間のベースアドレス
int init_port(io_addr_t port) {
        println_display("[ init_port ] port: %hx", port);
        // 割り込みをすべて禁止にする
        outb(port + SERIAL_REG_INT_ENABLE, 0x00);
        set_baud_rate(port, 1);
        set_line_protocol(port, LCTL_BITS_8N1);
        // TODO: よく分かっていない
        outb(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
        outb(port + 1, 0x01);    // 割り込みを有効！

        int error = loopback_test(port);
        if (error) {
                println_display("Failed to loopback test (port = 0x%hx)", port);
                return error;
        }
        println_display("Successfully loopback test (port = 0x%hx)", port);
        return 0;
}

io_addr_t serial_init() {
        println_display("[ serial_init ]");
        if (!init_port(COM1_PORT)) {
                println_display("[ SUCCESS ] init COM1");
                global_serial_port = COM1_PORT;
        } else {
                println_display("[ ERROR ] failed to init COM1");
        }

        if (!init_port(COM2_PORT)) {
                println_display("[ SUCCESS ] init COM2");
                global_serial_port = COM2_PORT;
        } else {
                println_display("[ ERROR ] failed to init COM2");
        }
        println_display("[ INFO ] global_serial_port = %hx", global_serial_port);
        return global_serial_port;
}

// シリアルポートのIRQハンドラ
// 文字が送られてきたら発火し、処理を行う。
static void serial_irq_handler(struct regs_on_stack *regs)
{
        // シリアルコンソールから受け取った文字に応じて画面の色を変化させる処理
        int c;
	while ((c = try_recvb(global_serial_port)) >= 0) {
		switch (c) {
			case 'r':
				clear_screen(&Red);
				break;
			case 'b':
				clear_screen(&Blue);
				break;
			case 'g':
				clear_screen(&Green);
				break;
			default:
				sendb(global_serial_port, 'x');
		}
	}
        // 本当は特定の割り込みコントローラの実装を使いたくなかったけど、、（ＴＯＤＯ：）
        intel8259_end_of_interrupt(regs->vector - 32);
}

void serial_init_late()
{
        // COM1とCOM2の両方の割り込みハンドラの設定を行う。
        set_irq_handler(isa_irq_to_irq(ISA_IRQ_COM1), serial_irq_handler);
        set_irq_handler(isa_irq_to_irq(ISA_IRQ_COM2), serial_irq_handler);
}

int try_sendb(io_addr_t port, u8 data)
{
        u8 thre = inb(port + SERIAL_REG_LINE_STATUS) & (1u << LSTATUS_BIT_THRE);
        if (thre) {
                outb(port + SERIAL_REG_DATA, data);
                return 0;
        } else {
                return -1;
        }
}

void sendb(io_addr_t port, u8 data)
{
        while (try_sendb(port, data));
}

int try_recvb(io_addr_t port)
{
        u8 ready = inb(port + SERIAL_REG_LINE_STATUS) & (1u << LSTATUS_BIT_DR);
        if (ready) {
                int data = inb(port + SERIAL_REG_DATA);
                return data;
        } else {
                return -1;
        }
}

u8 recvb(io_addr_t port)
{
        int ret;
        while ((ret = try_recvb(port)) < 0);
        // CHECK(0 <= ret <= 255);
        return (u8) ret;
}
