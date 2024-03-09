#include "display.h"
#include "boot.h"


static struct GopInfo gop_info;

void display_init(struct GopInfo *gop_info_)
{
        gop_info.frame_buffer = gop_info_->frame_buffer;
        gop_info.height = gop_info_->height;
        gop_info.width = gop_info_->width;
        gop_info.stride = gop_info_->stride;
        gop_info.pixel_format = gop_info_->pixel_format;
}

// color to int
// PixecColorの値をもとに、実際にフレームバッファに書き込む値に変換する関数。
static u32 c2i(struct PixelColor *color)
{
        u32 ret = 0xFFFFFF;
        switch (gop_info.pixel_format) {
                case kRgb:
                        ret = color->r | (color->g << 8) | (color->b << 16);
                        break;
                case kBgr:
                        ret = color->b | (color->g << 8) | (color->r << 16);
                        break;
                default:
                        // TODO: error
                        break;
        }
        return ret;
}

// 書き込む位置がフレームバッファからはみ出ていないかをチェックせずに書き込む関数。
//      - x: 上からの位置
//      - y: 左からの位置
void inline write_pixel_uncheck(u32 x, u32 y, u32 pixel)
{
        *(u32 *) ((u32 *) gop_info.frame_buffer + x * gop_info.stride + y) = pixel;
}

void write_pixel(u32 x, u32 y, struct PixelColor *color)
{
        u32 pixel = c2i(color);

        if (x < gop_info.height && y < gop_info.width) {
                write_pixel_uncheck(x, y, pixel);
        } else {
                // TODO: error
        }
}

void write_rectangle(u32 x, u32 y, u32 h, u32 w, struct PixelColor *color)
{
        u32 pixel = c2i(color);

        if (x + h < gop_info.height && y + w < gop_info.width) {
                for (u32 i = x; i < x + h; i++) {
                        for (u32 j = y; j < y + w; j++) {
                                write_pixel_uncheck(i, j, pixel);
                        }
                }
        } else {
                // TODO: error
        }
}

void write_square(u32 x, u32 y, u32 size, struct PixelColor *color)
{
        write_rectangle(x, y, size, size, color);
}

void clear_screen(struct PixelColor *color)
{
        u32 pixel = c2i(color);

        for (u32 i = 0; i < gop_info.height; i++) {
                for (u32 j = 0; j < gop_info.width; j++) {
                        write_pixel_uncheck(i, j, pixel);
                }
        }
}
