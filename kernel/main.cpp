extern "C" void kernel_start(void) {
        while (1) {
                asm volatile ("hlt");
        }
}