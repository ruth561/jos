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

typedef void (*comm_func_t)(const char *sub_comm);

struct comm_desc {
	const char *comm;
	usize comm_size;
	comm_func_t callback;
};

void screen_func(const char *sub_comm);

#define COMMS_ENTRY(name, func)	\
	{ .comm = name " ", .comm_size = sizeof(name), .callback = func }

static struct comm_desc comms[] = {
	COMMS_ENTRY("screen", screen_func),
};

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
	for (int i = 0; i < sizeof(comms) / sizeof(struct comm_desc); i++) {
		if (match_prefix(comm_buf, comms[i].comm)) {
			comms[i].callback(&comm_buf[comms[i].comm_size]);
			goto done;
		}
	}
	println_serial("Unknown command: %s", comm_buf);

done:
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


// 
// command functions
//

void screen_func(const char *sub_comm)
{
	if (match_prefix(sub_comm, "red")) {
		clear_screen(&Red);
	} else if (match_prefix(sub_comm, "blue")) {
		clear_screen(&Blue);
	} else if (match_prefix(sub_comm, "green")) {
		clear_screen(&Green);
	} else if (match_prefix(sub_comm, "black")) {
		clear_screen(&Black);
	} else if (match_prefix(sub_comm, "white")) {
		clear_screen(&White);
	} else {
		WARN("[ screen_func ] Unknown color: %s", sub_comm);
	}
}
