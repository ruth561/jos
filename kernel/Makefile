TARGET = kernel.elf
OBJS = main.o

CC = clang++
LD = ld.lld
CPPFLAGS = -O2 -Wall -g --target=x86_64-elf -std=c++17
CPPFLAGS += -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti
LDFLAGS  += --entry kernel_start -z norelro --image-base 0x100000 --static

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS)  -o $@ $^

%.o: %.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm $(TARGET) $(OBJS)
