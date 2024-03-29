#pragma once

#include "logger.h"
#include "asm/cpu.h"
#include "compiler.h"


// カーネルパニックを引き起こすマクロ。
// このマクロ内は無限ループが実装されており、マクロから抜けてくる
// ことはない。
// ＴＯＤＯ：関数のバックトレースなども実装したい。
#define PANIC(...)						\
	do {							\
		FATAL(__VA_ARGS__);				\
		FATAL("++++++ KERNEL PANIC ++++++");		\
		while (1) Halt();				\
		UNREACHABLE();					\
	} while (0)
