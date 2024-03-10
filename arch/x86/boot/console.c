// TODO: C++で抽象化したい
//       Consoleという抽象基底クラスを実装し、そのクラスを継承する派生クラス
//       として、DisplayConsoleやSerialConsoleなどを実装したい。       

#include "console.h"
#include "display.h"
#include "font.h"


struct Cursor {
        u32 x;
        u32 y;
};

struct Console {
        u32 height;
        u32 width;
        struct Cursor cursor;
        struct PixelColor fg;
        struct PixelColor bg;
};

// ディスプレイ用のコンソールインスタンス
static struct Console display_console;

void display_console_init(struct GopInfo *gop_info, struct PixelColor *fg,
                          struct PixelColor *bg)
{
        display_console.height = gop_info->height / FONT_HEIGHT;
        display_console.width = gop_info->width / FONT_WIDTH;
        display_console.cursor.x = 0;
        display_console.cursor.y = 0;
        display_console.fg = *fg;
        display_console.bg = *bg;

        clear_screen(&display_console.bg);
}

static void display_console_scroll()
{
        scroll_screen(FONT_HEIGHT, &display_console.bg);
}

static int display_console_putchar(char c)
{
        if (c == '\n') {
                display_console.cursor.y = 0;
                display_console.cursor.x++;
                if (display_console.cursor.x == display_console.height) {
                        display_console.cursor.x--;
                        display_console_scroll();
                }
                return 0;
        }

        int error = write_char(
                display_console.cursor.x * FONT_HEIGHT,
                display_console.cursor.y * FONT_WIDTH,
                c, &display_console.fg, &display_console.bg);
        if (error) {
                // TODO: error
                return error;
        }
        display_console.cursor.y++;
        return 0;
}

usize put_string_to_display(const char *s)
{
        usize cnt = 0;

        while (*s) {
                int error = display_console_putchar(*s++);
                if (error) {
                        // TODO: error
                        return cnt;
                }
                cnt++;
        }
        return cnt;
}
