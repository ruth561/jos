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

// TODO: 高速化
void *memset(void *s, int c, usize n)
{
	u8 *p = s;
	while (n--) {
		*p++ = (u8) c;
	}
	return s;
}

int match_prefix(const char *s, const char *prefix)
{
	while (*prefix) {
		if (*s++ != *prefix++) {
			return false;
		}
	}
	return true;
}

// 符号なし8-bit整数を解析して文字列に変換する
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
static usize parse_hhx(char *buf, usize limit, u8 val)
{
	int i = 0;
	for (; i < 2 && i < limit; i++) {
		char digit = (val >> (4 * (1 - i))) & 0xF;
		if (digit < 10) {
			*buf++ = '0' + digit;
		} else {
			// 0xA <= digit <= 0xF
			*buf++ = 'a' + digit - 10;
		}
	}
	return i;
}

// 符号なし16-bit整数を解析して文字列に変換する
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
static usize parse_hx(char *buf, usize limit, u16 val)
{
	int i = 0;
	for (; i < 4 && i < limit; i++) {
		char digit = (val >> (4 * (3 - i))) & 0xF;
		if (digit < 10) {
			*buf++ = '0' + digit;
		} else {
			// 0xA <= digit <= 0xF
			*buf++ = 'a' + digit - 10;
		}
	}
	return i;
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

// 符号なし32-bit整数を解析して10進数の文字列に変換する関数
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
static usize parse_u(char *buf, usize limit, u32 val)
{
	int cnt = 0; // 書き込んだ数
	u32 base = 1000000000u;

	if (limit == 0) {
		return 0;
	}

	if (val == 0) {
		*buf = '0';
		return 1;
	}

	// val > 0
	int mid = 0;
	while (base && cnt < limit) {
		u32 digit = (val / base) % 10;
		base /= 10;
		if (mid == 0 && digit == 0) {
			continue;
		}

		*buf++ = '0' + digit;
		cnt++;
		if (!mid) {
			mid = 1;
		}
	}
	return cnt;
}

// 符号付き32-bit整数を解析して10進数の文字列に変換する関数
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - val: 解析する値
//      - 返り値: 書き込んだ数
//
// 負の数を扱うために以下のアプローチを取った。
// 負の場合は先に'-'を先頭につける。
// valの数値の部分だけを取り出し、それをu32の値として扱って出力する。
static usize parse_d(char *buf, usize limit, s32 val)
{
	int cnt = 0; // 書き込んだ数
	u32 uval = (u32) val;

	if (limit == 0) {
		return 0;
	}

	if (val == 0) {
		*buf = '0';
		return 1;
	}

	if (val < 0) {
		*buf++ = '-';
		cnt++;
		uval = (u32) -val;
		// 符号付き32-bit整数で表される値の範囲は[−2147483648, 2147483647]
		// となっており、負の数の方が1つだけ表現できる数が多い。そのため、
		// 0x80000000の場合に単に-valとすると挙動がおかしくなってしまうので、
		// ここで例外的に処理を行っている。
		if (val == 0x80000000) {
			uval = 0x80000000;
		}
	}

	return parse_u(buf, limit - cnt, uval) + cnt;
}

// 文字列を埋め込む関数
//      - buf: 書き込むバッファの先頭ポインタ
//      - limit: 書き込み文字数の上限
//      - s: 埋め込む文字列の先頭へのポインタ
//      - 返り値: 書き込んだ数
static usize parse_s(char *buf, usize limit, const char *s)
{
	int i = 0;
	for (; i < limit; i++) {
		if (*s == '\0') {
			break;
		}
		*buf++ = *s++;
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
			} else if (match_prefix(fmt, "%hx")) {
				fmt += 3;
				u16 arg = va_arg(args, u32); // u32じゃないとだめみたい
				idx += parse_hx(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
			} else if (match_prefix(fmt, "%hhx")) {
				fmt += 4;
				u8 arg = va_arg(args, u32); // u32じゃないとだめみたい
				idx += parse_hhx(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
			} else if (match_prefix(fmt, "%u")) { // 符号なし32-bit整数
				fmt += 2;
				u32 arg = va_arg(args, u32);
				idx += parse_u(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
			} else if (match_prefix(fmt, "%d")) { // 符号付き32-bit整数
				fmt += 2;
				s32 arg = va_arg(args, s32);
				idx += parse_d(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
			} else if (match_prefix(fmt, "%s")) {
				fmt += 2;
				const char *arg = va_arg(args, const char *);
				idx += parse_s(&buf[idx], FORMAT_STRING_BUF_SIZE - 1 - idx, arg);
			} else if (match_prefix(fmt, "%%")) {
				fmt += 2;
				buf[idx++] = '%';
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


static bool hex_str_to_u64(const char *s, u64 *out)
{
	*out = 0;
	while (*s) {
		*out *= 16;
		switch (*s) {
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*out += *s - '0';
				break;
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
				*out += *s - 'a' + 10;
				break;
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				*out += *s - 'A' + 10;
				break;
			default:
				return false;
		}
		s++;
	}
	return true;
}

bool str_to_u64(const char *s, u64 *out)
{
	if (match_prefix(s, "0x")) {
		return hex_str_to_u64(s += 2, out);
	} else {
		// ＴＯＤＯ：10進数とかにも対応したい
		return false;
	}
}
