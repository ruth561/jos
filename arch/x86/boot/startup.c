#include "asm/cpu.h"
#include "boot.h"
#include "console.h"
#include "serial.h"
#include "display.h"
#include "type.h"
#include "string.h"


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

        printd("%lx, %lx, %lx\n", 0, 1, 2);
        printd("%x %x\n", 0xdeadbeef, 0xcafebabe);
        
        while (1);

        serial_init();

        clear_screen(&Green);
        for (int i = 0; i < 200; i++) {
                write_pixel(i, i, &Red);
                write_pixel(i, 2 * i, &Red);
        }

        write_rectangle(100, 100, 200, 300, &Blue);
        write_rectangle(600, 600, 1000, 1000, &Blue); // これは描画されない
        write_square(200, 300, 400, &Red);

        write_char(100, 100, 'B', &Red, &Green);
        write_string(200, 50, "Hello, World!", 14, &Red, &Green);
        write_string(0, 0,
                " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~",
                96, &Black, &White);

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
