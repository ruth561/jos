#include "asm/cpu.h"
#include "boot.h"
#include "serial.h"

const char *text = "Hello from startup.c!";

void clear_screen(struct GopInfo *gop_info, unsigned int color)
{
        unsigned int *frame_buffer_base = gop_info->frame_buffer;
        for (int i = 0; i < gop_info->height; i++) {
                for (int j = 0; j < gop_info->width; j++) {
                        unsigned int *pixel = frame_buffer_base + i * gop_info->stride + j;
                        *pixel = color;
                }
        }
}

#define RED     0xFF0000
#define GREEN   0x00FF00
#define BLUE    0x0000FF

__attribute__((section(".stext")))
void startup(struct GopInfo *gop_info) {

        clear_screen(gop_info, RED);

        serial_init();

        clear_screen(gop_info, GREEN);

        // シリアルコンソールから受け取った文字に応じて画面の色を変化させる処理
        while (1) {
                char c = recvb(global_serial_port);
                switch (c) {
                        case 'r':
                                clear_screen(gop_info, RED);
                                break;
                        case 'b':
                                clear_screen(gop_info, BLUE);
                                break;
                        case 'g':
                                clear_screen(gop_info, GREEN);
                                break;
                        default:
                                sendb(global_serial_port, 'x');
                }
        }
        
        while (1) Halt();
}
