TARGET = boot.efi
OBJS = main.o

CC = x86_64-w64-mingw32-gcc
CFLAGS = -Wall -Wextra
CFLAGS += -e efi_main -nostdinc -nostdlib -fno-builtin 
CFLAGS += -Wl,--subsystem,10

$(TARGET): $(OBJS)
	@echo "\tLINK\t$@"
	@$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	@echo "\tCC\t$@"
	@$(CC) $(CFLAGS) -c -o $@ $<
