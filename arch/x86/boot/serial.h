#pragma once

#include "asm/io.h"

extern io_addr_t global_serial_port;

io_addr_t serial_init();
void sendb(io_addr_t port, char data);
char recvb(io_addr_t port);
