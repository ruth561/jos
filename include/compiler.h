#pragma once

// コンパイラに組み込まれたトークンなどを使いやすいマクロとして
// 定義したヘッダファイル。


#define ALIGN(x) __attribute__((aligned(x)))

#define LIKELY(condition) __builtin_expect(!!condition, 1)
#define UNLIKELY(condition) __builtin_expect(!!condition, 0)

#define UNREACHABLE() __builtin_unreachable()
