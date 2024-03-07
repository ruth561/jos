#include "asm/cpu.h"

__attribute__((section(".stext")))
void startup(void) {
        while (1) Halt();
}
