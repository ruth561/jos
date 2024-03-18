#include "asm/cpu.h"
#include "boot.h"
#include "console.h"
#include "serial.h"
#include "display.h"
#include "type.h"
#include "string.h"
#include "logger.h"


struct PixelColor Red = {
        .r = 0xFF,
        .g = 0,
        .b = 0,
};

struct PixelColor Green = {
        .r = 0,
        .g = 0xFF,
        .b = 0,
};

struct PixelColor Blue = {
        .r = 0,
        .g = 0,
        .b = 0xFF,
};

struct PixelColor White = {
        .r = 0xFF,
        .g = 0xFF,
        .b = 0xFF,
};

struct PixelColor Black = {
        .r = 0,
        .g = 0,
        .b = 0,
};

__attribute__((section(".stext")))
void startup(struct GopInfo *gop_info) {

        display_init(gop_info);
        display_console_init(gop_info, &Black, &White);

        put_string_to_display("Hello, World!\n");
        put_string_to_display("I'm TAKUMI.\n");
        put_string_to_display("Nice to meet you~\n");

        printd("gop_info.frame_buffer: 0x%lx\n", (u64) gop_info->frame_buffer);
        printd("gop_info.height: 0x%x\n", gop_info->height);
        printd("gop_info.width: 0x%x\n", gop_info->width);
        printd("gop_info.stride: 0x%x\n", gop_info->stride);

        serial_init();
        logger_init(10);

        DEBUG("DEBUG!");
        INFO("INFO!");
        WARN("WARN!");
        ERROR("ERROR!");
        FATAL("FATAL!");
        INFO("This is %s message %x", "INFO", 0xdeadbeef);

        DEBUG("format test: 0u -> %u !!!", 0u);
        DEBUG("format test: 1u -> %u !!!", 1u);
        DEBUG("format test: 7u -> %u !!!", 7u);
        DEBUG("format test: 1729u -> %u !!!", 1729u);
        DEBUG("format test: 4294967295u -> %u !!!", 4294967295u);

        DEBUG("format test: 0 -> %d !!!", 0);
        DEBUG("format test: -1729 -> %d !!!", -1729);
        DEBUG("format test: 1105 -> %d !!!", 1105);
        DEBUG("format test: 2147483647 -> %d !!!", 0x7FFFFFFF);
        DEBUG("format test: -2147483648  -> %d !!!", 0x80000000);

        printd("[ INFO ] finished to initialization!\n");

        // シリアルコンソールから受け取った文字に応じて画面の色を変化させる処理
        while (1) {
                char c = recvb(global_serial_port);
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

        while (1) Halt();
}
