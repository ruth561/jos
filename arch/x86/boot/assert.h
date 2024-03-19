#pragma once

#include "error.h"
#include "logger.h"
#include "panic.h"
#include "compiler.h"


#define CHECK(condition)		\
	if (UNLIKELY(!condition)) {	\
		PANIC("CHECK failed.");	\
	}
