#include "shell.h"
#include "assert.h"
#include "console.h"
#include "display.h"
#include "logger.h"
#include "serial.h"
#include "string.h"


#define COMM_BUF_SIZE	80
static int idx; 
static char comm_buf[COMM_BUF_SIZE];
static const char *prompt = "(jdb) ";

static void new_comm()
{
	idx = 0;
	print_serial(prompt);
}

static void do_comm()
{
	CHECK(0 <= idx);
	CHECK(idx < COMM_BUF_SIZE);

	comm_buf[idx] = '\0';
	if (match_prefix(comm_buf, "a")) {
		println_serial("a");
	} else if (match_prefix(comm_buf, "r")) {
		clear_screen(&Red);
	} else {
		println_serial("Unknown command: %s", comm_buf);
	}

	new_comm();
}

static void input(char c)
{
	switch (c) {
		case '\r':
		case '\n':
			println_serial("");
			do_comm();
			break;
		default:
			serial_putc(c);
			if (idx < COMM_BUF_SIZE - 1) {
				comm_buf[idx++] = c;
			} else {
				ERROR("comm_buf is full!");
			}
	}
}

void shell_init()
{
	INFO("shell_init()");

	new_comm();
	register_serial_recv_callback(input);
}
