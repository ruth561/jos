#include "string.h"
#include "type.h"


// TODO: 高速化
void *memcpy(void *dst, const void *src, usize n)
{
        char *dstp = (char *) dst;
        const char *srcp = (const char *) src;

        for (usize i = 0; i < n; i++) {
                *dstp++ = *srcp++;
        }
        return dst;
}

// prefixがsのプレフィックスであればtrue(=1)を返す関数。
// 一致していなければfalse(=0)を返す。
static int match_prefix(const char *s, const char *prefix)
{
        while (*prefix) {
                if (*s++ != *prefix++) {
                        return false;
                }
        }
        return true;
}

// 符号なし32-bit整数を解析して文字列に変換する
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
static usize parse_x(char *buf, usize limit, u32 val)
{
        int i = 0;
        for (; i < 8 && i < limit; i++) {
                char digit = (val >> (4 * (7 - i))) & 0xF;
                if (digit < 10) {
                        *buf++ = '0' + digit;
                } else {
                        // 0xA <= digit <= 0xF
                        *buf++ = 'a' + digit - 10;
                }
        }
        return i;
}

// 符号なし64-bit整数を解析して文字列に変換する
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
static usize parse_lx(char *buf, usize limit, u64 val)
{
        int i = 0;
        for (; i < 16 && i < limit; i++) {
                char digit = (val >> (4 * (15 - i))) & 0xF;
                if (digit < 10) {
                        *buf++ = '0' + digit;
                } else {
                        // 0xA <= digit <= 0xF
                        *buf++ = 'a' + digit - 10;
                }
        }
        return i;
}

char *format_string(const char *fmt, ...)
{
        static char buf[FORMAT_STRING_BUF_SIZE];
        usize idx = 0;

        va_list args;
        va_start(args, fmt);
        while (*fmt && idx < FORMAT_STRING_BUF_SIZE - 1) {
                if (*fmt == '%') {
                        if (match_prefix(fmt, "%x")) {
                                fmt += 2;
                                u32 arg = va_arg(args, u32);
                                idx += parse_x(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
                        } else if (match_prefix(fmt, "%lx")) {
                                fmt += 3;
                                u64 arg = va_arg(args, u64);
                                idx += parse_lx(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
                        } else {
                                // TODO: error
                                // unknown format string
                                break;
                        }
                } else {
                        buf[idx++] = *fmt++;
                }
        }
        va_end(args);

        buf[idx] = '\0';
        return buf;
}