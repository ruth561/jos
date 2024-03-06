QEMU_ARGS = -m 4G \
	-bios /usr/share/ovmf/OVMF.fd \
	-drive file=fat:rw:boot/fs,media=disk,format=raw

.PHONY: qemu
qemu:
	@$(MAKE) -C boot
	@mkdir -p boot/fs/EFI/BOOT
	@cp boot/boot.efi boot/fs/EFI/BOOT/BOOTX64.EFI
	qemu-system-x86_64 $(QEMU_ARGS)

.PHONY: clean
clean:
	@$(MAKE) $@ -C boot
