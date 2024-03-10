
# startup関連の処理を先頭に配置したELF形式のファイル
KERNEL_ELF_IMAGE = kernel.elf
# KERNEL_ELF_IMAGEからobjcopyによってraw binaryにしたもの
# このファイルが実際にブートローダーからメモリ0x100000に
# ロードされる。
KERNEL_IMAGE = kernel
# KERNEL_ELF_IMAGEを作るのに必要なオブジェクトファイルたち
OBJS = startup.o serial.o display.o font.o console.o string.o
# KERNEL_ELF_IMAGEをリンクするときに使うリンカスクリプト
LINKER_SCRIPT = kernel.ld

CC = clang
LD = ld.lld
CFLAGS = -Wall -g -O1 --target=x86_64-elf
CFLAGS += -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti
CFLAGS += $(addprefix -I,$(INCLUDE_PATH))
LDFLAGS  = -Tkernel.ld -Map=kernel.map


all: $(KERNEL_IMAGE)

$(KERNEL_IMAGE): $(KERNEL_ELF_IMAGE)
	@echo "\tOBJCOPY\t$@"
	@objcopy -O binary -R .note -R .comment -S $< $@

$(KERNEL_ELF_IMAGE): $(OBJS) $(LINKER_SCRIPT)
	@echo "\tLD\t$@"
	@$(LD) -g $(LDFLAGS) $(OBJS) -T$(LINKER_SCRIPT) -o $@

%.o: %.c
	@echo "\tCC\t$@"
	@$(CC) $(CFLAGS) -c $<

.PHONY: clean
clean:
	rm $(KERNEL_ELF_IMAGE) $(KERNEL_IMAGE) $(OBJS)
