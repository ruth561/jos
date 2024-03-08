#include "asm/cpu.h"
#include "boot.h"

const char *text = "Hello from startup.c!";

__attribute__((section(".stext")))
void startup(struct GopInfo *gop_info) {

        unsigned int *frame_buffer_base = gop_info->frame_buffer;
        for (int i = 0; i < gop_info->height; i++) {
                for (int j = 0; j < gop_info->width; j++) {
                        unsigned int *pixel = frame_buffer_base + i * gop_info->stride + j;
                        *pixel = 0xcccccccc;
                }
        }
        
        while (1) Halt();
}
