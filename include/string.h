#pragma once

#include "type.h"

typedef __builtin_va_list va_list;
#define va_start(v, l)	__builtin_va_start(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v, T)	__builtin_va_arg(v, T)
#define va_copy(d, s)	__builtin_va_copy(d, s)


// srcからdstへn-bytes分をコピーする関数
void *memcpy(void *dst, const void *src, usize n);

// sへ文字cをn個書き込んでいく処理
void *memset(void *s, int c, usize n);

// prefixがsのプレフィックスであればtrue(=1)を返す関数。
// 一致していなければfalse(=0)を返す。
int match_prefix(const char *s, const char *prefix);

#define FORMAT_STRING_BUF_SIZE 0x100

// フォーマット文字列と可変長の引数を受け取り、フォーマット文字列に
// 値を挿入した新しい文字列を作成して返す関数。作成する文字列の長さ
// には上限が決められており、FORMAT_STRING_BUF_SIZEで定義される値を
// 超える文字列は整形できない。
//
// 現状、実装されているフォーマット文字は以下の通り。
//      - %x: 符号なし32-bit整数を受け取り、それを8桁の16進数文字列に
//            して挿入する。先頭は0埋めされる。
//      - %lx: 符号なし64-bit整数を受け取り、それを16桁の16進数文字列に
//             して挿入する。先頭は0埋めされる。
char *format_string(const char *format, ...);

// 文字列を数字と解釈してu64の値に変換する関数。
// 変換に成功すれば*outに結果を書き込みtrueを返す。
// 変換に失敗した場合はfalseを返す。
bool str_to_u64(const char *s, u64 *out);
