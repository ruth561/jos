#pragma once

#include "asm/io.h"
#include "type.h"


extern io_addr_t global_serial_port;

io_addr_t serial_init();
void sendb(io_addr_t port, u8 data);
u8 recvb(io_addr_t port);
