TARGET	= kernel.elf
OBJS	= main.o

CC	= clang++
LD	= ld.lld

CCFLAGS	= -O2 -Wall -g --target=x86_64-elf -std=c++17
CCFLAGS	+= -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti
LDFLAGS	+= --entry kernel_start -z norelro --image-base 0x100000 --static

$(TARGET): $(OBJS)
	@echo "LD\t$@"
	@$(LD) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	@echo "CC\t$@"
	@$(CC) $(CCFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(TARGET) $(OBJS)
