#include "asm/cpu.h"
#include "asm/io.h"


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
static void set_baud_rate(io_addr_t port, unsigned short divisor) {
        // DLAB-bitをセットする。
        IoOut8(port + SERIAL_REG_LINE_CONTROL, 0x80);
        // ボーレートの除数を書き込む。
        IoOut8(port + SERIAL_REG_DIVISOR_LOW, divisor & 0xFF);
        IoOut8(port + SERIAL_REG_DIVISOR_HIGH, (divisor >> 8) & 0xFF);
        // DLAB-bitをクリアする。
        IoOut8(port + SERIAL_REG_LINE_CONTROL, 0x80);
}

// ラインプロトコルをlctlで表された値に設定する関数。
// lctlに使えるフラグはLCTL_BITS_*マクロで定義されている。
static void set_line_protocol(io_addr_t port, unsigned char lctl) {
        IoOut8(port + SERIAL_REG_LINE_CONTROL, LCTL_BITS_8N1);
}

// portの初期化を行う関数
//      - port: ポートのI/Oアドレス空間のベースアドレス
int init_port(io_addr_t port) {
        // 割り込みをすべて禁止にする
        IoOut8(port + SERIAL_REG_INT_ENABLE, 0x00);
        set_baud_rate(port, 1);
        set_line_protocol(port, LCTL_BITS_8N1);
        // TODO: よく分かっていない
        IoOut8(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
        IoOut8(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
        IoOut8(port + 4, 0x1E);    // Set in loopback mode, test the serial chip
        IoOut8(port + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

        // Check if serial is faulty (i.e: not same byte as sent)
        if(IoIn8(port + 0) != 0xAE) {
                return 1;
        }

        // If serial is not faulty set it in normal operation mode
        // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
        IoOut8(port + 4, 0x0F);
        return 0;
}

static int is_transmit_empty(io_addr_t port) {
        return IoIn8(port + SERIAL_REG_LINE_STATUS) & (1u << LSTATUS_BIT_THRE);
}

// 1-byteのデータを送信する
void sendb(io_addr_t port, char data) {
        while (!is_transmit_empty(port));
        IoOut8(port + SERIAL_REG_DATA, data);
}

static int is_recv_ready(io_addr_t port) {
        return IoIn8(port + SERIAL_REG_LINE_STATUS) & (1u << LSTATUS_BIT_DR);
}

// 1-byteのデータを受信する
char recvb(io_addr_t port) {
        while (!is_recv_ready(port));
        return IoIn8(port + SERIAL_REG_DATA);
}

io_addr_t serial_init() {
        if (!init_port(COM1_PORT)) {
                global_serial_port = COM1_PORT;
        }
        if (!init_port(COM2_PORT)) {
                global_serial_port = COM2_PORT;
        }

        sendb(global_serial_port, 'S');
        sendb(global_serial_port, 'E');
        sendb(global_serial_port, 'E');
        sendb(global_serial_port, ' ');
        sendb(global_serial_port, 'Y');
        sendb(global_serial_port, 'O');
        sendb(global_serial_port, 'U');
        sendb(global_serial_port, '\n');

        sendb(COM1_PORT, 'H');
        sendb(COM1_PORT, 'E');
        sendb(COM1_PORT, 'L');
        sendb(COM1_PORT, 'L');
        sendb(COM1_PORT, 'O');
        sendb(COM1_PORT, '\n');

        return global_serial_port;
}
