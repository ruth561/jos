BOOTLOADER = josloader/target/x86_64-unknown-uefi/debug/josloader.efi
KERNEL_IMAGE = kernel/kernel.elf
QEMU_ROOTFS = qemufs
QEMU_ARGS = -m 4G
QEMU_ARGS += -bios /usr/share/ovmf/OVMF.fd
QEMU_ARGS += -drive file=fat:rw:$(QEMU_ROOTFS),media=disk,format=raw
QEMU_ARGS += -s

.PHONY: qemu
qemu: $(KERNEL_IMAGE) $(BOOTLOADER)
	@mkdir -p $(QEMU_ROOTFS)/EFI/BOOT
	@cp $(BOOTLOADER) $(QEMU_ROOTFS)/EFI/BOOT/BOOTX64.EFI
	@cp $(KERNEL_IMAGE) $(QEMU_ROOTFS)/kernel.elf
	@echo "\tQEMU"
	@qemu-system-x86_64 $(QEMU_ARGS)

.PHONY: debug
debug:
	make qemu & gdb -q

.PHONY: clean
clean:
	@$(MAKE) $@ -C kernel

$(KERNEL_IMAGE):
	@$(MAKE) -C kernel

$(BOOTLOADER):
	@cd josloader; cargo build --target x86_64-unknown-uefi
