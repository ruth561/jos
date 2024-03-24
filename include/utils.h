#pragma once

#include "string.h"
#include "type.h"


// ポインタpのアドレスがalignでアラインメントされていれば1を返す。
// アラインメントされていなければ0を返す。
static inline int is_aligned(void *p, u64 align)
{
	return !(((u64) p) % align);
}
