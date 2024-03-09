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

void clear_screen(struct PixelColor *color)
{
        u32 *frame_buffer_base = gop_info.frame_buffer;
        u32 pixel;
        
        switch (gop_info.pixel_format) {
                case kRgb:
                        pixel = color->r | (color->g << 8) | (color->b << 16);
                        break;
                case kBgr:
                        pixel = color->b | (color->g << 8) | (color->r << 16);
                        break;
                default:
                        // TODO: error
                        break;
        }

        for (int i = 0; i < gop_info.height; i++) {
                for (int j = 0; j < gop_info.width; j++) {
                        *(u32 *) (frame_buffer_base + i * gop_info.stride + j) = pixel;
                }
        }
}
