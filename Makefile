ARCH = x86

MAKE_ARGS = -s

# ブートローダーはjosloaderによって作成される
BOOTLOADER = josloader/target/x86_64-unknown-uefi/debug/josloader.efi
# ブートローダーから実際にメモリにロードされるイメージファイル
KERNEL_BOOT_IMAGE = arch/$(ARCH)/boot/kernel
# もろもろの初期化処理を行なったあとに実行されるカーネルのメイン部分
# ＴＯＤＯ：
KERNEL_IMAGE = kernel/kernel.elf

QEMU_ROOTFS = qemufs
QEMU_ARGS = -m 4G
QEMU_ARGS += -bios /usr/share/ovmf/OVMF.fd
QEMU_ARGS += -drive file=fat:rw:$(QEMU_ROOTFS),media=disk,format=raw
# QEMU_ARGS += -monitor stdio
QEMU_ARGS += -serial stdio
QEMU_ARGS += -monitor telnet::6666,server,nowait
QEMU_ARGS += -s

PWD = $(shell pwd)
INCLUDE_PATH = $(PWD)/include $(PWD)/arch/$(ARCH)/include
export INCLUDE_PATH

CFLAGS_EXPORT = -g -gdwarf-4
export CFLAGS_EXPORT

.PHONY: target
target: $(KERNEL_IMAGE) $(KERNEL_BOOT_IMAGE)

.PHONY: qemu
qemu: $(KERNEL_IMAGE) $(KERNEL_BOOT_IMAGE) $(BOOTLOADER)
	@mkdir -p $(QEMU_ROOTFS)/EFI/BOOT
	@cp $(BOOTLOADER) $(QEMU_ROOTFS)/EFI/BOOT/BOOTX64.EFI
	@cp $(KERNEL_BOOT_IMAGE) $(QEMU_ROOTFS)/kernel.elf
	@echo "\tQEMU"
	@qemu-system-x86_64 $(QEMU_ARGS)

# QEMUを起動後、GDBとQEMUモニターを新しい端末で開く
.PHONY: debug
debug:
	make qemu &
	gnome-terminal -- gdb -q
	gnome-terminal -- telnet localhost 6666

.PHONY: clean
clean:
	@$(MAKE) $(MAKE_ARGS) clean -C kernel
	@$(MAKE) $(MAKE_ARGS) clean -C arch/$(ARCH)/boot

.PHONY: $(KERNEL_IMAGE)
$(KERNEL_IMAGE):
	@$(MAKE) $(MAKE_ARGS) -C kernel

.PHONY: $(KERNEL_BOOT_IMAGE)
$(KERNEL_BOOT_IMAGE):
	@$(MAKE) $(MAKE_ARGS) -C arch/$(ARCH)/boot

.PHONY: $(BOOTLOADER)
$(BOOTLOADER):
	@cd josloader; cargo build --target x86_64-unknown-uefi

.PHONY: test
test:
	@echo $(PWD)
	@echo $(INCLUDE_PATH)

# USBストレージに書き込む
.PHONY: write_usb
write_usb:
	rm /media/ruth/UEFI\ TEST/kernel.elf
	cp qemufs/kernel.elf /media/ruth/UEFI\ TEST/
