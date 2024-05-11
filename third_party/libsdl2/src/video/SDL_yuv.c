/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../SDL_internal.h"

#include "SDL_endian.h"
#include "SDL_video.h"
#include "SDL_pixels_c.h"

#include "yuv2rgb/yuv_rgb.h"

#define SDL_YUV_SD_THRESHOLD    576


static SDL_YUV_CONVERSION_MODE SDL_YUV_ConversionMode = SDL_YUV_CONVERSION_BT601;


void SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_MODE mode)
{
    SDL_YUV_ConversionMode = mode;
}

SDL_YUV_CONVERSION_MODE SDL_GetYUVConversionMode()
{
    return SDL_YUV_ConversionMode;
}

SDL_YUV_CONVERSION_MODE SDL_GetYUVConversionModeForResolution(int width, int height)
{
    SDL_YUV_CONVERSION_MODE mode = SDL_GetYUVConversionMode();
    if (mode == SDL_YUV_CONVERSION_AUTOMATIC) {
        if (height <= SDL_YUV_SD_THRESHOLD) {
            mode = SDL_YUV_CONVERSION_BT601;
        } else {
            mode = SDL_YUV_CONVERSION_BT709;
        }
    }
    return mode;
}

static int GetYUVConversionType(int width, int height, YCbCrType *yuv_type)
{
    switch (SDL_GetYUVConversionModeForResolution(width, height)) {
    case SDL_YUV_CONVERSION_JPEG:
        *yuv_type = YCBCR_JPEG;
        break;
    case SDL_YUV_CONVERSION_BT601:
        *yuv_type = YCBCR_601;
        break;
    case SDL_YUV_CONVERSION_BT709:
        *yuv_type = YCBCR_709;
        break;
    default:
        return SDL_SetError("Unexpected YUV conversion mode");
    }
    return 0;
}

static SDL_bool IsPlanar2x2Format(Uint32 format)
{
    return (format == SDL_PIXELFORMAT_YV12 ||
            format == SDL_PIXELFORMAT_IYUV ||
            format == SDL_PIXELFORMAT_NV12 ||
            format == SDL_PIXELFORMAT_NV21);
}

static SDL_bool IsPacked4Format(Uint32 format)
{
    return (format == SDL_PIXELFORMAT_YUY2 ||
            format == SDL_PIXELFORMAT_UYVY ||
            format == SDL_PIXELFORMAT_YVYU);
}

static int GetYUVPlanes(int width, int height, Uint32 format, const void *yuv, int yuv_pitch,
	                    const Uint8 **y, const Uint8 **u, const Uint8 **v, Uint32 *y_stride, Uint32 *uv_stride)
{
	const Uint8 *planes[3] = { NULL, NULL, NULL };
	int pitches[3] = { 0, 0, 0 };

    switch (format) {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
        pitches[0] = yuv_pitch;
        pitches[1] = (pitches[0] + 1) / 2;
        pitches[2] = (pitches[0] + 1) / 2;
        planes[0] = (const Uint8 *)yuv;
        planes[1] = planes[0] + pitches[0] * height;
        planes[2] = planes[1] + pitches[1] * ((height + 1) / 2);
        break;
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        pitches[0] = yuv_pitch;
        planes[0] = (const Uint8 *)yuv;
        break;
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        pitches[0] = yuv_pitch;
        pitches[1] = 2 * ((pitches[0] + 1) / 2);
        planes[0] = (const Uint8 *)yuv;
        planes[1] = planes[0] + pitches[0] * height;
        break;
    default:
        return SDL_SetError("GetYUVPlanes(): Unsupported YUV format: %s", SDL_GetPixelFormatName(format));
    }

    switch (format) {
    case SDL_PIXELFORMAT_YV12:
        *y = planes[0];
        *y_stride = pitches[0];
        *v = planes[1];
        *u = planes[2];
        *uv_stride = pitches[1];
        break;
    case SDL_PIXELFORMAT_IYUV:
        *y = planes[0];
        *y_stride = pitches[0];
        *v = planes[2];
        *u = planes[1];
        *uv_stride = pitches[1];
        break;
    case SDL_PIXELFORMAT_YUY2:
        *y = planes[0];
        *y_stride = pitches[0];
        *v = *y + 3;
        *u = *y + 1;
        *uv_stride = pitches[0];
        break;
    case SDL_PIXELFORMAT_UYVY:
        *y = planes[0] + 1;
        *y_stride = pitches[0];
        *v = *y + 1;
        *u = *y - 1;
        *uv_stride = pitches[0];
        break;
    case SDL_PIXELFORMAT_YVYU:
        *y = planes[0];
        *y_stride = pitches[0];
        *v = *y + 1;
        *u = *y + 3;
        *uv_stride = pitches[0];
        break;
    case SDL_PIXELFORMAT_NV12:
        *y = planes[0];
        *y_stride = pitches[0];
        *u = planes[1];
        *v = *u + 1;
        *uv_stride = pitches[1];
        break;
    case SDL_PIXELFORMAT_NV21:
        *y = planes[0];
        *y_stride = pitches[0];
        *v = planes[1];
        *u = *v + 1;
        *uv_stride = pitches[1];
        break;
    default:
        /* Should have caught this above */
        return SDL_SetError("GetYUVPlanes[2]: Unsupported YUV format: %s", SDL_GetPixelFormatName(format));
    }
    return 0;
}

static SDL_bool yuv_rgb_sse(
    Uint32 src_format, Uint32 dst_format,
	Uint32 width, Uint32 height, 
	const Uint8 *y, const Uint8 *u, const Uint8 *v, Uint32 y_stride, Uint32 uv_stride, 
	Uint8 *rgb, Uint32 rgb_stride, 
	YCbCrType yuv_type)
{
#ifdef __SSE2__
    if (!SDL_HasSSE2()) {
        return SDL_FALSE;
    }

    if (src_format == SDL_PIXELFORMAT_YV12 ||
        src_format == SDL_PIXELFORMAT_IYUV) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuv420_rgb565_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuv420_rgb24_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuv420_rgba_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuv420_bgra_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuv420_argb_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuv420_abgr_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }

    if (src_format == SDL_PIXELFORMAT_YUY2 ||
        src_format == SDL_PIXELFORMAT_UYVY ||
        src_format == SDL_PIXELFORMAT_YVYU) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuv422_rgb565_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuv422_rgb24_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuv422_rgba_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuv422_bgra_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuv422_argb_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuv422_abgr_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }

    if (src_format == SDL_PIXELFORMAT_NV12 ||
        src_format == SDL_PIXELFORMAT_NV21) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuvnv12_rgb565_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuvnv12_rgb24_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuvnv12_rgba_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuvnv12_bgra_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuvnv12_argb_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuvnv12_abgr_sseu(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }
#endif
    return SDL_FALSE;
}

static SDL_bool yuv_rgb_std(
    Uint32 src_format, Uint32 dst_format,
	Uint32 width, Uint32 height, 
	const Uint8 *y, const Uint8 *u, const Uint8 *v, Uint32 y_stride, Uint32 uv_stride, 
	Uint8 *rgb, Uint32 rgb_stride, 
	YCbCrType yuv_type)
{
    if (src_format == SDL_PIXELFORMAT_YV12 ||
        src_format == SDL_PIXELFORMAT_IYUV) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuv420_rgb565_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuv420_rgb24_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuv420_rgba_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuv420_bgra_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuv420_argb_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuv420_abgr_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }

    if (src_format == SDL_PIXELFORMAT_YUY2 ||
        src_format == SDL_PIXELFORMAT_UYVY ||
        src_format == SDL_PIXELFORMAT_YVYU) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuv422_rgb565_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuv422_rgb24_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuv422_rgba_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuv422_bgra_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuv422_argb_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuv422_abgr_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }

    if (src_format == SDL_PIXELFORMAT_NV12 ||
        src_format == SDL_PIXELFORMAT_NV21) {

        switch (dst_format) {
        case SDL_PIXELFORMAT_RGB565:
            yuvnv12_rgb565_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB24:
            yuvnv12_rgb24_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGBX8888:
        case SDL_PIXELFORMAT_RGBA8888:
            yuvnv12_rgba_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGRX8888:
        case SDL_PIXELFORMAT_BGRA8888:
            yuvnv12_bgra_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_RGB888:
        case SDL_PIXELFORMAT_ARGB8888:
            yuvnv12_argb_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        case SDL_PIXELFORMAT_BGR888:
        case SDL_PIXELFORMAT_ABGR8888:
            yuvnv12_abgr_std(width, height, y, u, v, y_stride, uv_stride, rgb, rgb_stride, yuv_type);
            return SDL_TRUE;
        default:
            break;
        }
    }
    return SDL_FALSE;
}

int
SDL_ConvertPixels_YUV_to_RGB(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
	const Uint8 *y = NULL;
    const Uint8 *u = NULL;
    const Uint8 *v = NULL;
    Uint32 y_stride = 0;
    Uint32 uv_stride = 0;
    YCbCrType yuv_type = YCBCR_601;

    if (GetYUVPlanes(width, height, src_format, src, src_pitch, &y, &u, &v, &y_stride, &uv_stride) < 0) {
        return -1;
    }

    if (GetYUVConversionType(width, height, &yuv_type) < 0) {
        return -1;
    }

    if (yuv_rgb_sse(src_format, dst_format, width, height, y, u, v, y_stride, uv_stride, (Uint8*)dst, dst_pitch, yuv_type)) {
        return 0;
    }

    if (yuv_rgb_std(src_format, dst_format, width, height, y, u, v, y_stride, uv_stride, (Uint8*)dst, dst_pitch, yuv_type)) {
        return 0;
    }

    /* No fast path for the RGB format, instead convert using an intermediate buffer */
    if (dst_format != SDL_PIXELFORMAT_ARGB8888) {
        int ret;
        void *tmp;
        int tmp_pitch = (width * sizeof(Uint32));

        tmp = SDL_malloc(tmp_pitch * height);
        if (tmp == NULL) {
            return SDL_OutOfMemory();
        }

        /* convert src/src_format to tmp/ARGB8888 */
        ret = SDL_ConvertPixels_YUV_to_RGB(width, height, src_format, src, src_pitch, SDL_PIXELFORMAT_ARGB8888, tmp, tmp_pitch);
        if (ret < 0) {
            SDL_free(tmp);
            return ret;
        }

        /* convert tmp/ARGB8888 to dst/RGB */
        ret = SDL_ConvertPixels(width, height, SDL_PIXELFORMAT_ARGB8888, tmp, tmp_pitch, dst_format, dst, dst_pitch);
        SDL_free(tmp);
        return ret;
    }

    return SDL_SetError("Unsupported YUV conversion");
}

struct RGB2YUVFactors
{
    int y_offset;
    float y[3]; /* Rfactor, Gfactor, Bfactor */
    float u[3]; /* Rfactor, Gfactor, Bfactor */
    float v[3]; /* Rfactor, Gfactor, Bfactor */
};

static int
SDL_ConvertPixels_ARGB8888_to_YUV(int width, int height, const void *src, int src_pitch, Uint32 dst_format, void *dst, int dst_pitch)
{
    const int src_pitch_x_2    = src_pitch * 2;
    const int height_half      = height / 2;
    const int height_remainder = (height & 0x1);
    const int width_half       = width / 2;
    const int width_remainder  = (width & 0x1);
    int i, j;
 
    static struct RGB2YUVFactors RGB2YUVFactorTables[SDL_YUV_CONVERSION_BT709 + 1] =
    {
        /* ITU-T T.871 (JPEG) */
        {
            0,
            {  0.2990f,  0.5870f,  0.1140f },
            { -0.1687f, -0.3313f,  0.5000f },
            {  0.5000f, -0.4187f, -0.0813f },
        },
        /* ITU-R BT.601-7 */
        {
            16,
            {  0.2568f,  0.5041f,  0.0979f },
            { -0.1482f, -0.2910f,  0.4392f },
            {  0.4392f, -0.3678f, -0.0714f },
        },
        /* ITU-R BT.709-6 */
        {
            16,
            { 0.1826f,  0.6142f,  0.0620f },
            {-0.1006f, -0.3386f,  0.4392f },
            { 0.4392f, -0.3989f, -0.0403f },
        },
    };
    const struct RGB2YUVFactors *cvt = &RGB2YUVFactorTables[SDL_GetYUVConversionModeForResolution(width, height)];

#define MAKE_Y(r, g, b) (Uint8)((int)(cvt->y[0] * (r) + cvt->y[1] * (g) + cvt->y[2] * (b) + 0.5f) + cvt->y_offset)
#define MAKE_U(r, g, b) (Uint8)((int)(cvt->u[0] * (r) + cvt->u[1] * (g) + cvt->u[2] * (b) + 0.5f) + 128)
#define MAKE_V(r, g, b) (Uint8)((int)(cvt->v[0] * (r) + cvt->v[1] * (g) + cvt->v[2] * (b) + 0.5f) + 128)

#define READ_2x2_PIXELS                                                                                         \
        const Uint32 p1 = ((const Uint32 *)curr_row)[2 * i];                                                    \
        const Uint32 p2 = ((const Uint32 *)curr_row)[2 * i + 1];                                                \
        const Uint32 p3 = ((const Uint32 *)next_row)[2 * i];                                                    \
        const Uint32 p4 = ((const Uint32 *)next_row)[2 * i + 1];                                                \
        const Uint32 r = ((p1 & 0x00ff0000) + (p2 & 0x00ff0000) + (p3 & 0x00ff0000) + (p4 & 0x00ff0000)) >> 18; \
        const Uint32 g = ((p1 & 0x0000ff00) + (p2 & 0x0000ff00) + (p3 & 0x0000ff00) + (p4 & 0x0000ff00)) >> 10; \
        const Uint32 b = ((p1 & 0x000000ff) + (p2 & 0x000000ff) + (p3 & 0x000000ff) + (p4 & 0x000000ff)) >> 2;  \

#define READ_2x1_PIXELS                                                                                         \
        const Uint32 p1 = ((const Uint32 *)curr_row)[2 * i];                                                    \
        const Uint32 p2 = ((const Uint32 *)next_row)[2 * i];                                                    \
        const Uint32 r = ((p1 & 0x00ff0000) + (p2 & 0x00ff0000)) >> 17;                                         \
        const Uint32 g = ((p1 & 0x0000ff00) + (p2 & 0x0000ff00)) >> 9;                                          \
        const Uint32 b = ((p1 & 0x000000ff) + (p2 & 0x000000ff)) >> 1;                                          \

#define READ_1x2_PIXELS                                                                                         \
        const Uint32 p1 = ((const Uint32 *)curr_row)[2 * i];                                                    \
        const Uint32 p2 = ((const Uint32 *)curr_row)[2 * i + 1];                                                \
        const Uint32 r = ((p1 & 0x00ff0000) + (p2 & 0x00ff0000)) >> 17;                                         \
        const Uint32 g = ((p1 & 0x0000ff00) + (p2 & 0x0000ff00)) >> 9;                                          \
        const Uint32 b = ((p1 & 0x000000ff) + (p2 & 0x000000ff)) >> 1;                                          \

#define READ_1x1_PIXEL                                                                                          \
        const Uint32 p = ((const Uint32 *)curr_row)[2 * i];                                                     \
        const Uint32 r = (p & 0x00ff0000) >> 16;                                                                \
        const Uint32 g = (p & 0x0000ff00) >> 8;                                                                 \
        const Uint32 b = (p & 0x000000ff);                                                                      \

#define READ_TWO_RGB_PIXELS                                                                                     \
        const Uint32 p = ((const Uint32 *)curr_row)[2 * i];                                                     \
        const Uint32 r = (p & 0x00ff0000) >> 16;                                                                \
        const Uint32 g = (p & 0x0000ff00) >> 8;                                                                 \
        const Uint32 b = (p & 0x000000ff);                                                                      \
        const Uint32 p1 = ((const Uint32 *)curr_row)[2 * i + 1];                                                \
        const Uint32 r1 = (p1 & 0x00ff0000) >> 16;                                                              \
        const Uint32 g1 = (p1 & 0x0000ff00) >> 8;                                                               \
        const Uint32 b1 = (p1 & 0x000000ff);                                                                    \
        const Uint32 R = (r + r1)/2;                                                                            \
        const Uint32 G = (g + g1)/2;                                                                            \
        const Uint32 B = (b + b1)/2;                                                                            \

#define READ_ONE_RGB_PIXEL  READ_1x1_PIXEL

    switch (dst_format) 
    {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        {
            const Uint8 *curr_row, *next_row;
            
            Uint8 *plane_y;
            Uint8 *plane_u;
            Uint8 *plane_v;
            Uint8 *plane_interleaved_uv;
            Uint32 y_stride, uv_stride, y_skip, uv_skip;

            GetYUVPlanes(width, height, dst_format, dst, dst_pitch,
	                     (const Uint8 **)&plane_y, (const Uint8 **)&plane_u, (const Uint8 **)&plane_v,
                         &y_stride, &uv_stride);
            plane_interleaved_uv = (plane_y + height * y_stride);
            y_skip = (y_stride - width);

            curr_row = (const Uint8*)src;

            /* Write Y plane */
            for (j = 0; j < height; j++) {
                for (i = 0; i < width; i++) {
                    const Uint32 p1 = ((const Uint32 *)curr_row)[i];
                    const Uint32 r = (p1 & 0x00ff0000) >> 16;
                    const Uint32 g = (p1 & 0x0000ff00) >> 8;
                    const Uint32 b = (p1 & 0x000000ff);
                    *plane_y++ = MAKE_Y(r, g, b);
                }
                plane_y += y_skip;
                curr_row += src_pitch;
            }

            curr_row = (const Uint8*)src;
            next_row = (const Uint8*)src;
            next_row += src_pitch;

            if (dst_format == SDL_PIXELFORMAT_YV12 || dst_format == SDL_PIXELFORMAT_IYUV)
            {
                /* Write UV planes, not interleaved */
                uv_skip = (uv_stride - (width + 1)/2);
                for (j = 0; j < height_half; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_2x2_PIXELS;
                        *plane_u++ = MAKE_U(r, g, b);
                        *plane_v++ = MAKE_V(r, g, b);
                    }
                    if (width_remainder) {
                        READ_2x1_PIXELS;
                        *plane_u++ = MAKE_U(r, g, b);
                        *plane_v++ = MAKE_V(r, g, b);
                    }
                    plane_u += uv_skip;
                    plane_v += uv_skip;
                    curr_row += src_pitch_x_2;
                    next_row += src_pitch_x_2;
                }
                if (height_remainder) {
                    for (i = 0; i < width_half; i++) {
                        READ_1x2_PIXELS;
                        *plane_u++ = MAKE_U(r, g, b);
                        *plane_v++ = MAKE_V(r, g, b);
                    }
                    if (width_remainder) {
                        READ_1x1_PIXEL;
                        *plane_u++ = MAKE_U(r, g, b);
                        *plane_v++ = MAKE_V(r, g, b);
                    }
                    plane_u += uv_skip;
                    plane_v += uv_skip;
                }
            }
            else if (dst_format == SDL_PIXELFORMAT_NV12)
            {
                uv_skip = (uv_stride - ((width + 1)/2)*2);
                for (j = 0; j < height_half; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_2x2_PIXELS;
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                    }
                    if (width_remainder) {
                        READ_2x1_PIXELS;
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                    }
                    plane_interleaved_uv += uv_skip;
                    curr_row += src_pitch_x_2;
                    next_row += src_pitch_x_2;
                }
                if (height_remainder) {
                    for (i = 0; i < width_half; i++) {
                        READ_1x2_PIXELS;
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                    }
                    if (width_remainder) {
                        READ_1x1_PIXEL;
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                    }
                }
            } 
            else /* dst_format == SDL_PIXELFORMAT_NV21 */
            {
                uv_skip = (uv_stride - ((width + 1)/2)*2);
                for (j = 0; j < height_half; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_2x2_PIXELS;
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                    }
                    if (width_remainder) {
                        READ_2x1_PIXELS;
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                    }
                    plane_interleaved_uv += uv_skip;
                    curr_row += src_pitch_x_2;
                    next_row += src_pitch_x_2;
                }
                if (height_remainder) {
                    for (i = 0; i < width_half; i++) {
                        READ_1x2_PIXELS;
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                    }
                    if (width_remainder) {
                        READ_1x1_PIXEL;
                        *plane_interleaved_uv++ = MAKE_V(r, g, b);
                        *plane_interleaved_uv++ = MAKE_U(r, g, b);
                    }
                }
            }
        }
        break;

    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        {
            const Uint8 *curr_row = (const Uint8*) src;
            Uint8 *plane           = (Uint8*) dst;
            const int row_size = (4 * ((width + 1) / 2));
            int plane_skip;

            if (dst_pitch < row_size) {
                return SDL_SetError("Destination pitch is too small, expected at least %d\n", row_size);
            }
            plane_skip = (dst_pitch - row_size);

            /* Write YUV plane, packed */
            if (dst_format == SDL_PIXELFORMAT_YUY2) 
            {
                for (j = 0; j < height; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_TWO_RGB_PIXELS;
                        /* Y U Y1 V */
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_U(R, G, B);
                        *plane++ = MAKE_Y(r1, g1, b1);
                        *plane++ = MAKE_V(R, G, B);
                    }
                    if (width_remainder) {
                        READ_ONE_RGB_PIXEL;
                        /* Y U Y V */
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_U(r, g, b);
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_V(r, g, b);
                    }
                    plane += plane_skip;
                    curr_row += src_pitch;
                }
            } 
            else if (dst_format == SDL_PIXELFORMAT_UYVY)
            {
                for (j = 0; j < height; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_TWO_RGB_PIXELS;
                        /* U Y V Y1 */
                        *plane++ = MAKE_U(R, G, B);
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_V(R, G, B);
                        *plane++ = MAKE_Y(r1, g1, b1);
                    }
                    if (width_remainder) {
                        READ_ONE_RGB_PIXEL;
                        /* U Y V Y */
                        *plane++ = MAKE_U(r, g, b);
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_V(r, g, b);
                        *plane++ = MAKE_Y(r, g, b);
                    }
                    plane += plane_skip;
                    curr_row += src_pitch;
                }
            }
            else if (dst_format == SDL_PIXELFORMAT_YVYU)
            {
                for (j = 0; j < height; j++) {
                    for (i = 0; i < width_half; i++) {
                        READ_TWO_RGB_PIXELS;
                        /* Y V Y1 U */
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_V(R, G, B);
                        *plane++ = MAKE_Y(r1, g1, b1);
                        *plane++ = MAKE_U(R, G, B);
                    }
                    if (width_remainder) {
                        READ_ONE_RGB_PIXEL;
                        /* Y V Y U */
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_V(r, g, b);
                        *plane++ = MAKE_Y(r, g, b);
                        *plane++ = MAKE_U(r, g, b);
                    }
                    plane += plane_skip;
                    curr_row += src_pitch;
                }
            }
        }
        break;

    default:
        return SDL_SetError("Unsupported YUV destination format: %s", SDL_GetPixelFormatName(dst_format));
    }
#undef MAKE_Y
#undef MAKE_U
#undef MAKE_V
#undef READ_2x2_PIXELS
#undef READ_2x1_PIXELS
#undef READ_1x2_PIXELS
#undef READ_1x1_PIXEL
#undef READ_TWO_RGB_PIXELS
#undef READ_ONE_RGB_PIXEL
    return 0;
}

int
SDL_ConvertPixels_RGB_to_YUV(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
#if 0 /* Doesn't handle odd widths */
    /* RGB24 to FOURCC */
    if (src_format == SDL_PIXELFORMAT_RGB24) {
        Uint8 *y;
        Uint8 *u;
        Uint8 *v;
        Uint32 y_stride;
        Uint32 uv_stride;
        YCbCrType yuv_type;

        if (GetYUVPlanes(width, height, dst_format, dst, dst_pitch, (const Uint8 **)&y, (const Uint8 **)&u, (const Uint8 **)&v, &y_stride, &uv_stride) < 0) {
            return -1;
        }

        if (GetYUVConversionType(width, height, &yuv_type) < 0) {
            return -1;
        }

        rgb24_yuv420_std(width, height, src, src_pitch, y, u, v, y_stride, uv_stride, yuv_type);
        return 0;
    }
#endif

    /* ARGB8888 to FOURCC */
    if (src_format == SDL_PIXELFORMAT_ARGB8888) {
        return SDL_ConvertPixels_ARGB8888_to_YUV(width, height, src, src_pitch, dst_format, dst, dst_pitch);
    }

    /* not ARGB8888 to FOURCC : need an intermediate conversion */
    {
        int ret;
        void *tmp;
        int tmp_pitch = (width * sizeof(Uint32));

        tmp = SDL_malloc(tmp_pitch * height);
        if (tmp == NULL) {
            return SDL_OutOfMemory();
        }

        /* convert src/src_format to tmp/ARGB8888 */
        ret = SDL_ConvertPixels(width, height, src_format, src, src_pitch, SDL_PIXELFORMAT_ARGB8888, tmp, tmp_pitch);
        if (ret == -1) {
            SDL_free(tmp);
            return ret;
        }

        /* convert tmp/ARGB8888 to dst/FOURCC */
        ret = SDL_ConvertPixels_ARGB8888_to_YUV(width, height, tmp, tmp_pitch, dst_format, dst, dst_pitch);
        SDL_free(tmp);
        return ret;
    }
}

static int
SDL_ConvertPixels_YUV_to_YUV_Copy(int width, int height, Uint32 format,
        const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int i;

    if (IsPlanar2x2Format(format)) {
        /* Y plane */
        for (i = height; i--;) {
            SDL_memcpy(dst, src, width);
            src = (const Uint8*)src + src_pitch;
            dst = (Uint8*)dst + dst_pitch;
        }

        if (format == SDL_PIXELFORMAT_YV12 || format == SDL_PIXELFORMAT_IYUV) {
            /* U and V planes are a quarter the size of the Y plane, rounded up */
            width = (width + 1) / 2;
            height = (height + 1) / 2;
            src_pitch = (src_pitch + 1) / 2;
            dst_pitch = (dst_pitch + 1) / 2;
            for (i = height * 2; i--;) {
                SDL_memcpy(dst, src, width);
                src = (const Uint8*)src + src_pitch;
                dst = (Uint8*)dst + dst_pitch;
            }
        } else if (format == SDL_PIXELFORMAT_NV12 || format == SDL_PIXELFORMAT_NV21) {
            /* U/V plane is half the height of the Y plane, rounded up */
            height = (height + 1) / 2;
            width = ((width + 1) / 2)*2;
            src_pitch = ((src_pitch + 1) / 2)*2;
            dst_pitch = ((dst_pitch + 1) / 2)*2;
            for (i = height; i--;) {
                SDL_memcpy(dst, src, width);
                src = (const Uint8*)src + src_pitch;
                dst = (Uint8*)dst + dst_pitch;
            }
        }
        return 0;
    }

    if (IsPacked4Format(format)) {
        /* Packed planes */
        width = 4 * ((width + 1) / 2);
        for (i = height; i--;) {
            SDL_memcpy(dst, src, width);
            src = (const Uint8*)src + src_pitch;
            dst = (Uint8*)dst + dst_pitch;
        }
        return 0;
    }

    return SDL_SetError("SDL_ConvertPixels_YUV_to_YUV_Copy: Unsupported YUV format: %s", SDL_GetPixelFormatName(format));
}

static int
SDL_ConvertPixels_SwapUVPlanes(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int y;
    const int UVwidth = (width + 1)/2;
    const int UVheight = (height + 1)/2;

    /* Skip the Y plane */
    src = (const Uint8 *)src + height * src_pitch;
    dst = (Uint8 *)dst + height * dst_pitch;

    if (src == dst) {
        int UVpitch = (dst_pitch + 1)/2;
        Uint8 *tmp;
        Uint8 *row1 = dst;
        Uint8 *row2 = (Uint8 *)dst + UVheight * UVpitch;

        /* Allocate a temporary row for the swap */
        tmp = (Uint8 *)SDL_malloc(UVwidth);
        if (!tmp) {
            return SDL_OutOfMemory();
        }
        for (y = 0; y < UVheight; ++y) {
            SDL_memcpy(tmp, row1, UVwidth);
            SDL_memcpy(row1, row2, UVwidth);
            SDL_memcpy(row2, tmp, UVwidth);
            row1 += UVpitch;
            row2 += UVpitch;
        }
        SDL_free(tmp);
    } else {
        const Uint8 *srcUV;
        Uint8 *dstUV;
        int srcUVPitch = ((src_pitch + 1)/2);
        int dstUVPitch = ((dst_pitch + 1)/2);

        /* Copy the first plane */
        srcUV = (const Uint8 *)src;
        dstUV = (Uint8 *)dst + UVheight * dstUVPitch;
        for (y = 0; y < UVheight; ++y) {
            SDL_memcpy(dstUV, srcUV, UVwidth);
            srcUV += srcUVPitch;
            dstUV += dstUVPitch;
        }

        /* Copy the second plane */
        dstUV = (Uint8 *)dst;
        for (y = 0; y < UVheight; ++y) {
            SDL_memcpy(dstUV, srcUV, UVwidth);
            srcUV += srcUVPitch;
            dstUV += dstUVPitch;
        }
    }
    return 0;
}

static int
SDL_ConvertPixels_PackUVPlanes_to_NV(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch, SDL_bool reverseUV)
{
    int x, y;
    const int UVwidth = (width + 1)/2;
    const int UVheight = (height + 1)/2;
    const int srcUVPitch = ((src_pitch + 1)/2);
    const int srcUVPitchLeft = srcUVPitch - UVwidth;
    const int dstUVPitch = ((dst_pitch + 1)/2)*2;
    const int dstUVPitchLeft = dstUVPitch - UVwidth*2;
    const Uint8 *src1, *src2;
    Uint8 *dstUV;
    Uint8 *tmp = NULL;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    /* Skip the Y plane */
    src = (const Uint8 *)src + height * src_pitch;
    dst = (Uint8 *)dst + height * dst_pitch;

    if (src == dst) {
        /* Need to make a copy of the buffer so we don't clobber it while converting */
        tmp = (Uint8 *)SDL_malloc(2*UVheight*srcUVPitch);
        if (!tmp) {
            return SDL_OutOfMemory();
        }
        SDL_memcpy(tmp, src, 2*UVheight*srcUVPitch);
        src = tmp;
    }

    if (reverseUV) {
        src2 = (const Uint8 *)src;
        src1 = src2 + UVheight * srcUVPitch;
    } else {
        src1 = (const Uint8 *)src;
        src2 = src1 + UVheight * srcUVPitch;
    }
    dstUV = (Uint8 *)dst;

    y = UVheight;
    while (y--) {
        x = UVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            while (x >= 16) {
                __m128i u = _mm_loadu_si128((__m128i *)src1);
                __m128i v = _mm_loadu_si128((__m128i *)src2);
                __m128i uv1 = _mm_unpacklo_epi8(u, v);
                __m128i uv2 = _mm_unpackhi_epi8(u, v);
                _mm_storeu_si128((__m128i*)dstUV, uv1);
                _mm_storeu_si128((__m128i*)(dstUV + 16), uv2);
                src1 += 16;
                src2 += 16;
                dstUV += 32;
                x -= 16;
            }
        }
#endif
        while (x--) {
            *dstUV++ = *src1++;
            *dstUV++ = *src2++;
        }
        src1 += srcUVPitchLeft;
        src2 += srcUVPitchLeft;
        dstUV += dstUVPitchLeft;
    }

    if (tmp) {
        SDL_free(tmp);
    }
    return 0;
}

static int
SDL_ConvertPixels_SplitNV_to_UVPlanes(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch, SDL_bool reverseUV)
{
    int x, y;
    const int UVwidth = (width + 1)/2;
    const int UVheight = (height + 1)/2;
    const int srcUVPitch = ((src_pitch + 1)/2)*2;
    const int srcUVPitchLeft = srcUVPitch - UVwidth*2;
    const int dstUVPitch = ((dst_pitch + 1)/2);
    const int dstUVPitchLeft = dstUVPitch - UVwidth;
    const Uint8 *srcUV;
    Uint8 *dst1, *dst2;
    Uint8 *tmp = NULL;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    /* Skip the Y plane */
    src = (const Uint8 *)src + height * src_pitch;
    dst = (Uint8 *)dst + height * dst_pitch;

    if (src == dst) {
        /* Need to make a copy of the buffer so we don't clobber it while converting */
        tmp = (Uint8 *)SDL_malloc(UVheight*srcUVPitch);
        if (!tmp) {
            return SDL_OutOfMemory();
        }
        SDL_memcpy(tmp, src, UVheight*srcUVPitch);
        src = tmp;
    }

    if (reverseUV) {
        dst2 = (Uint8 *)dst;
        dst1 = dst2 + UVheight * dstUVPitch;
    } else {
        dst1 = (Uint8 *)dst;
        dst2 = dst1 + UVheight * dstUVPitch;
    }
    srcUV = (const Uint8 *)src;

    y = UVheight;
    while (y--) {
        x = UVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            __m128i mask = _mm_set1_epi16(0x00FF);
            while (x >= 16) {
                __m128i uv1 = _mm_loadu_si128((__m128i*)srcUV);
                __m128i uv2 = _mm_loadu_si128((__m128i*)(srcUV+16));
                __m128i u1 = _mm_and_si128(uv1, mask);
                __m128i u2 = _mm_and_si128(uv2, mask);
                __m128i u = _mm_packus_epi16(u1, u2);
                __m128i v1 = _mm_srli_epi16(uv1, 8);
                __m128i v2 = _mm_srli_epi16(uv2, 8);
                __m128i v = _mm_packus_epi16(v1, v2);
                _mm_storeu_si128((__m128i*)dst1, u);
                _mm_storeu_si128((__m128i*)dst2, v);
                srcUV += 32;
                dst1 += 16;
                dst2 += 16;
                x -= 16;
            }
        }
#endif
        while (x--) {
            *dst1++ = *srcUV++;
            *dst2++ = *srcUV++;
        }
        srcUV += srcUVPitchLeft;
        dst1 += dstUVPitchLeft;
        dst2 += dstUVPitchLeft;
    }

    if (tmp) {
        SDL_free(tmp);
    }
    return 0;
}

static int
SDL_ConvertPixels_SwapNV(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int UVwidth = (width + 1)/2;
    const int UVheight = (height + 1)/2;
    const int srcUVPitch = ((src_pitch + 1)/2)*2;
    const int srcUVPitchLeft = (srcUVPitch - UVwidth*2)/sizeof(Uint16);
    const int dstUVPitch = ((dst_pitch + 1)/2)*2;
    const int dstUVPitchLeft = (dstUVPitch - UVwidth*2)/sizeof(Uint16);
    const Uint16 *srcUV;
    Uint16 *dstUV;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    /* Skip the Y plane */
    src = (const Uint8 *)src + height * src_pitch;
    dst = (Uint8 *)dst + height * dst_pitch;

    srcUV = (const Uint16 *)src;
    dstUV = (Uint16 *)dst;
    y = UVheight;
    while (y--) {
        x = UVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            while (x >= 8) {
                __m128i uv = _mm_loadu_si128((__m128i*)srcUV);
                __m128i v = _mm_slli_epi16(uv, 8);
                __m128i u = _mm_srli_epi16(uv, 8);
                __m128i vu = _mm_or_si128(v, u);
                _mm_storeu_si128((__m128i*)dstUV, vu);
                srcUV += 8;
                dstUV += 8;
                x -= 8;
            }
        }
#endif
        while (x--) {
            *dstUV++ = SDL_Swap16(*srcUV++);
        }
        srcUV += srcUVPitchLeft;
        dstUV += dstUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_Planar2x2_to_Planar2x2(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
    if (src != dst) {
        /* Copy Y plane */
        int i;
        const Uint8 *srcY = (const Uint8 *)src;
        Uint8 *dstY = (Uint8 *)dst;
        for (i = height; i--; ) {
            SDL_memcpy(dstY, srcY, width);
            srcY += src_pitch;
            dstY += dst_pitch;
        }
    }

    switch (src_format) {
    case SDL_PIXELFORMAT_YV12:
        switch (dst_format) {
        case SDL_PIXELFORMAT_IYUV:
            return SDL_ConvertPixels_SwapUVPlanes(width, height, src, src_pitch, dst, dst_pitch);
        case SDL_PIXELFORMAT_NV12:
            return SDL_ConvertPixels_PackUVPlanes_to_NV(width, height, src, src_pitch, dst, dst_pitch, SDL_TRUE);
        case SDL_PIXELFORMAT_NV21:
            return SDL_ConvertPixels_PackUVPlanes_to_NV(width, height, src, src_pitch, dst, dst_pitch, SDL_FALSE);
        default:
            break;
        }
        break;
    case SDL_PIXELFORMAT_IYUV:
        switch (dst_format) {
        case SDL_PIXELFORMAT_YV12:
            return SDL_ConvertPixels_SwapUVPlanes(width, height, src, src_pitch, dst, dst_pitch);
        case SDL_PIXELFORMAT_NV12:
            return SDL_ConvertPixels_PackUVPlanes_to_NV(width, height, src, src_pitch, dst, dst_pitch, SDL_FALSE);
        case SDL_PIXELFORMAT_NV21:
            return SDL_ConvertPixels_PackUVPlanes_to_NV(width, height, src, src_pitch, dst, dst_pitch, SDL_TRUE);
        default:
            break;
        }
        break;
    case SDL_PIXELFORMAT_NV12:
        switch (dst_format) {
        case SDL_PIXELFORMAT_YV12:
            return SDL_ConvertPixels_SplitNV_to_UVPlanes(width, height, src, src_pitch, dst, dst_pitch, SDL_TRUE);
        case SDL_PIXELFORMAT_IYUV:
            return SDL_ConvertPixels_SplitNV_to_UVPlanes(width, height, src, src_pitch, dst, dst_pitch, SDL_FALSE);
        case SDL_PIXELFORMAT_NV21:
            return SDL_ConvertPixels_SwapNV(width, height, src, src_pitch, dst, dst_pitch);
        default:
            break;
        }
        break;
    case SDL_PIXELFORMAT_NV21:
        switch (dst_format) {
        case SDL_PIXELFORMAT_YV12:
            return SDL_ConvertPixels_SplitNV_to_UVPlanes(width, height, src, src_pitch, dst, dst_pitch, SDL_FALSE);
        case SDL_PIXELFORMAT_IYUV:
            return SDL_ConvertPixels_SplitNV_to_UVPlanes(width, height, src, src_pitch, dst, dst_pitch, SDL_TRUE);
        case SDL_PIXELFORMAT_NV12:
            return SDL_ConvertPixels_SwapNV(width, height, src, src_pitch, dst, dst_pitch);
        default:
            break;
        }
        break;
    default:
        break;
    }
    return SDL_SetError("SDL_ConvertPixels_Planar2x2_to_Planar2x2: Unsupported YUV conversion: %s -> %s", SDL_GetPixelFormatName(src_format), SDL_GetPixelFormatName(dst_format));
}

#define PACKED4_TO_PACKED4_ROW_SSE2(shuffle)                                                        \
    while (x >= 4) {                                                                                \
        __m128i yuv = _mm_loadu_si128((__m128i*)srcYUV);                                            \
        __m128i lo = _mm_unpacklo_epi8(yuv, _mm_setzero_si128());                                   \
        __m128i hi = _mm_unpackhi_epi8(yuv, _mm_setzero_si128());                                   \
        lo = _mm_shufflelo_epi16(lo, shuffle);                                                      \
        lo = _mm_shufflehi_epi16(lo, shuffle);                                                      \
        hi = _mm_shufflelo_epi16(hi, shuffle);                                                      \
        hi = _mm_shufflehi_epi16(hi, shuffle);                                                      \
        yuv = _mm_packus_epi16(lo, hi);                                                             \
        _mm_storeu_si128((__m128i*)dstYUV, yuv);                                                    \
        srcYUV += 16;                                                                               \
        dstYUV += 16;                                                                               \
        x -= 4;                                                                                     \
    }                                                                                               \

static int
SDL_ConvertPixels_YUY2_to_UYVY(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(2, 3, 0, 1));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            Y1 = srcYUV[0];
            U = srcYUV[1];
            Y2 = srcYUV[2];
            V = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = U;
            dstYUV[1] = Y1;
            dstYUV[2] = V;
            dstYUV[3] = Y2;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_YUY2_to_YVYU(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(1, 2, 3, 0));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            Y1 = srcYUV[0];
            U = srcYUV[1];
            Y2 = srcYUV[2];
            V = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = Y1;
            dstYUV[1] = V;
            dstYUV[2] = Y2;
            dstYUV[3] = U;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_UYVY_to_YUY2(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(2, 3, 0, 1));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            U = srcYUV[0];
            Y1 = srcYUV[1];
            V = srcYUV[2];
            Y2 = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = Y1;
            dstYUV[1] = U;
            dstYUV[2] = Y2;
            dstYUV[3] = V;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_UYVY_to_YVYU(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(0, 3, 2, 1));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            U = srcYUV[0];
            Y1 = srcYUV[1];
            V = srcYUV[2];
            Y2 = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = Y1;
            dstYUV[1] = V;
            dstYUV[2] = Y2;
            dstYUV[3] = U;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_YVYU_to_YUY2(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(1, 2, 3, 0));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            Y1 = srcYUV[0];
            V = srcYUV[1];
            Y2 = srcYUV[2];
            U = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = Y1;
            dstYUV[1] = U;
            dstYUV[2] = Y2;
            dstYUV[3] = V;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_YVYU_to_UYVY(int width, int height, const void *src, int src_pitch, void *dst, int dst_pitch)
{
    int x, y;
    const int YUVwidth = (width + 1)/2;
    const int srcYUVPitchLeft = (src_pitch - YUVwidth*4);
    const int dstYUVPitchLeft = (dst_pitch - YUVwidth*4);
    const Uint8 *srcYUV = (const Uint8 *)src;
    Uint8 *dstYUV = (Uint8 *)dst;
#ifdef __SSE2__
    const SDL_bool use_SSE2 = SDL_HasSSE2();
#endif

    y = height;
    while (y--) {
        x = YUVwidth;
#ifdef __SSE2__
        if (use_SSE2) {
            PACKED4_TO_PACKED4_ROW_SSE2(_MM_SHUFFLE(2, 1, 0, 3));
        }
#endif
        while (x--) {
            Uint8 Y1, U, Y2, V;

            Y1 = srcYUV[0];
            V = srcYUV[1];
            Y2 = srcYUV[2];
            U = srcYUV[3];
            srcYUV += 4;

            dstYUV[0] = U;
            dstYUV[1] = Y1;
            dstYUV[2] = V;
            dstYUV[3] = Y2;
            dstYUV += 4;
        }
        srcYUV += srcYUVPitchLeft;
        dstYUV += dstYUVPitchLeft;
    }
    return 0;
}

static int
SDL_ConvertPixels_Packed4_to_Packed4(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
    switch (src_format) {
    case SDL_PIXELFORMAT_YUY2:
        switch (dst_format) {
        case SDL_PIXELFORMAT_UYVY:
            return SDL_ConvertPixels_YUY2_to_UYVY(width, height, src, src_pitch, dst, dst_pitch);
        case SDL_PIXELFORMAT_YVYU:
            return SDL_ConvertPixels_YUY2_to_YVYU(width, height, src, src_pitch, dst, dst_pitch);
        default:
            break;
        }
        break;
    case SDL_PIXELFORMAT_UYVY:
        switch (dst_format) {
        case SDL_PIXELFORMAT_YUY2:
            return SDL_ConvertPixels_UYVY_to_YUY2(width, height, src, src_pitch, dst, dst_pitch);
        case SDL_PIXELFORMAT_YVYU:
            return SDL_ConvertPixels_UYVY_to_YVYU(width, height, src, src_pitch, dst, dst_pitch);
        default:
            break;
        }
        break;
    case SDL_PIXELFORMAT_YVYU:
        switch (dst_format) {
        case SDL_PIXELFORMAT_YUY2:
            return SDL_ConvertPixels_YVYU_to_YUY2(width, height, src, src_pitch, dst, dst_pitch);
        case SDL_PIXELFORMAT_UYVY:
            return SDL_ConvertPixels_YVYU_to_UYVY(width, height, src, src_pitch, dst, dst_pitch);
        default:
            break;
        }
        break;
    default:
        break;
    }
    return SDL_SetError("SDL_ConvertPixels_Packed4_to_Packed4: Unsupported YUV conversion: %s -> %s", SDL_GetPixelFormatName(src_format), SDL_GetPixelFormatName(dst_format));
}

static int
SDL_ConvertPixels_Planar2x2_to_Packed4(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
    int x, y;
    const Uint8 *srcY1, *srcY2, *srcU, *srcV;
    Uint32 srcY_pitch, srcUV_pitch;
    Uint32 srcY_pitch_left, srcUV_pitch_left, srcUV_pixel_stride;
    Uint8 *dstY1, *dstY2, *dstU1, *dstU2, *dstV1, *dstV2;
    Uint32 dstY_pitch, dstUV_pitch;
    Uint32 dst_pitch_left;

    if (src == dst) {
        return SDL_SetError("Can't change YUV plane types in-place");
    }

    if (GetYUVPlanes(width, height, src_format, src, src_pitch,
                     &srcY1, &srcU, &srcV, &srcY_pitch, &srcUV_pitch) < 0) {
        return -1;
    }
    srcY2 = srcY1 + srcY_pitch;
    srcY_pitch_left = (srcY_pitch - width);

    if (src_format == SDL_PIXELFORMAT_NV12 || src_format == SDL_PIXELFORMAT_NV21) {
        srcUV_pixel_stride = 2;
        srcUV_pitch_left = (srcUV_pitch - 2*((width + 1)/2));
    } else {
        srcUV_pixel_stride = 1;
        srcUV_pitch_left = (srcUV_pitch - ((width + 1)/2));
    }

    if (GetYUVPlanes(width, height, dst_format, dst, dst_pitch,
                     (const Uint8 **)&dstY1, (const Uint8 **)&dstU1, (const Uint8 **)&dstV1,
                     &dstY_pitch, &dstUV_pitch) < 0) {
        return -1;
    }
    dstY2 = dstY1 + dstY_pitch;
    dstU2 = dstU1 + dstUV_pitch;
    dstV2 = dstV1 + dstUV_pitch;
    dst_pitch_left = (dstY_pitch - 4*((width + 1)/2));

    /* Copy 2x2 blocks of pixels at a time */
    for (y = 0; y < (height - 1); y += 2) {
        for (x = 0; x < (width - 1); x += 2) {
            /* Row 1 */
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstU1 = *srcU;
            *dstV1 = *srcV;

            /* Row 2 */
            *dstY2 = *srcY2++;
            dstY2 += 2;
            *dstY2 = *srcY2++;
            dstY2 += 2;
            *dstU2 = *srcU;
            *dstV2 = *srcV;

            srcU += srcUV_pixel_stride;
            srcV += srcUV_pixel_stride;
            dstU1 += 4;
            dstU2 += 4;
            dstV1 += 4;
            dstV2 += 4;
        }

        /* Last column */
        if (x == (width - 1)) {
            /* Row 1 */
            *dstY1 = *srcY1;
            dstY1 += 2;
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstU1 = *srcU;
            *dstV1 = *srcV;

            /* Row 2 */
            *dstY2 = *srcY2;
            dstY2 += 2;
            *dstY2 = *srcY2++;
            dstY2 += 2;
            *dstU2 = *srcU;
            *dstV2 = *srcV;

            srcU += srcUV_pixel_stride;
            srcV += srcUV_pixel_stride;
            dstU1 += 4;
            dstU2 += 4;
            dstV1 += 4;
            dstV2 += 4;
        }

        srcY1 += srcY_pitch_left + srcY_pitch;
        srcY2 += srcY_pitch_left + srcY_pitch;
        srcU += srcUV_pitch_left;
        srcV += srcUV_pitch_left;
        dstY1 += dst_pitch_left + dstY_pitch;
        dstY2 += dst_pitch_left + dstY_pitch;
        dstU1 += dst_pitch_left + dstUV_pitch;
        dstU2 += dst_pitch_left + dstUV_pitch;
        dstV1 += dst_pitch_left + dstUV_pitch;
        dstV2 += dst_pitch_left + dstUV_pitch;
    }

    /* Last row */
    if (y == (height - 1)) {
        for (x = 0; x < (width - 1); x += 2) {
            /* Row 1 */
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstU1 = *srcU;
            *dstV1 = *srcV;

            srcU += srcUV_pixel_stride;
            srcV += srcUV_pixel_stride;
            dstU1 += 4;
            dstV1 += 4;
        }

        /* Last column */
        if (x == (width - 1)) {
            /* Row 1 */
            *dstY1 = *srcY1;
            dstY1 += 2;
            *dstY1 = *srcY1++;
            dstY1 += 2;
            *dstU1 = *srcU;
            *dstV1 = *srcV;

            srcU += srcUV_pixel_stride;
            srcV += srcUV_pixel_stride;
            dstU1 += 4;
            dstV1 += 4;
        }
    }
    return 0;
}

static int
SDL_ConvertPixels_Packed4_to_Planar2x2(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
    int x, y;
    const Uint8 *srcY1, *srcY2, *srcU1, *srcU2, *srcV1, *srcV2;
    Uint32 srcY_pitch, srcUV_pitch;
    Uint32 src_pitch_left;
    Uint8 *dstY1, *dstY2, *dstU, *dstV;
    Uint32 dstY_pitch, dstUV_pitch;
    Uint32 dstY_pitch_left, dstUV_pitch_left, dstUV_pixel_stride;

    if (src == dst) {
        return SDL_SetError("Can't change YUV plane types in-place");
    }

    if (GetYUVPlanes(width, height, src_format, src, src_pitch,
                     &srcY1, &srcU1, &srcV1, &srcY_pitch, &srcUV_pitch) < 0) {
        return -1;
    }
    srcY2 = srcY1 + srcY_pitch;
    srcU2 = srcU1 + srcUV_pitch;
    srcV2 = srcV1 + srcUV_pitch;
    src_pitch_left = (srcY_pitch - 4*((width + 1)/2));

    if (GetYUVPlanes(width, height, dst_format, dst, dst_pitch,
                     (const Uint8 **)&dstY1, (const Uint8 **)&dstU, (const Uint8 **)&dstV,
                     &dstY_pitch, &dstUV_pitch) < 0) {
        return -1;
    }
    dstY2 = dstY1 + dstY_pitch;
    dstY_pitch_left = (dstY_pitch - width);

    if (dst_format == SDL_PIXELFORMAT_NV12 || dst_format == SDL_PIXELFORMAT_NV21) {
        dstUV_pixel_stride = 2;
        dstUV_pitch_left = (dstUV_pitch - 2*((width + 1)/2));
    } else {
        dstUV_pixel_stride = 1;
        dstUV_pitch_left = (dstUV_pitch - ((width + 1)/2));
    }

    /* Copy 2x2 blocks of pixels at a time */
    for (y = 0; y < (height - 1); y += 2) {
        for (x = 0; x < (width - 1); x += 2) {
            /* Row 1 */
            *dstY1++ = *srcY1;
            srcY1 += 2;
            *dstY1++ = *srcY1;
            srcY1 += 2;

            /* Row 2 */
            *dstY2++ = *srcY2;
            srcY2 += 2;
            *dstY2++ = *srcY2;
            srcY2 += 2;

            *dstU = (Uint8)(((Uint32)*srcU1 + *srcU2)/2);
            *dstV = (Uint8)(((Uint32)*srcV1 + *srcV2)/2);

            srcU1 += 4;
            srcU2 += 4;
            srcV1 += 4;
            srcV2 += 4;
            dstU += dstUV_pixel_stride;
            dstV += dstUV_pixel_stride;
        }

        /* Last column */
        if (x == (width - 1)) {
            /* Row 1 */
            *dstY1 = *srcY1;
            srcY1 += 2;
            *dstY1++ = *srcY1;
            srcY1 += 2;

            /* Row 2 */
            *dstY2 = *srcY2;
            srcY2 += 2;
            *dstY2++ = *srcY2;
            srcY2 += 2;

            *dstU = (Uint8)(((Uint32)*srcU1 + *srcU2)/2);
            *dstV = (Uint8)(((Uint32)*srcV1 + *srcV2)/2);

            srcU1 += 4;
            srcU2 += 4;
            srcV1 += 4;
            srcV2 += 4;
            dstU += dstUV_pixel_stride;
            dstV += dstUV_pixel_stride;
        }

        srcY1 += src_pitch_left + srcY_pitch;
        srcY2 += src_pitch_left + srcY_pitch;
        srcU1 += src_pitch_left + srcUV_pitch;
        srcU2 += src_pitch_left + srcUV_pitch;
        srcV1 += src_pitch_left + srcUV_pitch;
        srcV2 += src_pitch_left + srcUV_pitch;
        dstY1 += dstY_pitch_left + dstY_pitch;
        dstY2 += dstY_pitch_left + dstY_pitch;
        dstU += dstUV_pitch_left;
        dstV += dstUV_pitch_left;
    }

    /* Last row */
    if (y == (height - 1)) {
        for (x = 0; x < (width - 1); x += 2) {
            *dstY1++ = *srcY1;
            srcY1 += 2;
            *dstY1++ = *srcY1;
            srcY1 += 2;

            *dstU = *srcU1;
            *dstV = *srcV1;

            srcU1 += 4;
            srcV1 += 4;
            dstU += dstUV_pixel_stride;
            dstV += dstUV_pixel_stride;
        }

        /* Last column */
        if (x == (width - 1)) {
            *dstY1 = *srcY1;
            *dstU = *srcU1;
            *dstV = *srcV1;
        }
    }
    return 0;
}

int
SDL_ConvertPixels_YUV_to_YUV(int width, int height,
         Uint32 src_format, const void *src, int src_pitch,
         Uint32 dst_format, void *dst, int dst_pitch)
{
    if (src_format == dst_format) {
        if (src == dst) {
            /* Nothing to do */
            return 0;
        }
        return SDL_ConvertPixels_YUV_to_YUV_Copy(width, height, src_format, src, src_pitch, dst, dst_pitch);
    }

    if (IsPlanar2x2Format(src_format) && IsPlanar2x2Format(dst_format)) {
        return SDL_ConvertPixels_Planar2x2_to_Planar2x2(width, height, src_format, src, src_pitch, dst_format, dst, dst_pitch);
    } else if (IsPacked4Format(src_format) && IsPacked4Format(dst_format)) {
        return SDL_ConvertPixels_Packed4_to_Packed4(width, height, src_format, src, src_pitch, dst_format, dst, dst_pitch);
    } else if (IsPlanar2x2Format(src_format) && IsPacked4Format(dst_format)) {
        return SDL_ConvertPixels_Planar2x2_to_Packed4(width, height, src_format, src, src_pitch, dst_format, dst, dst_pitch);
    } else if (IsPacked4Format(src_format) && IsPlanar2x2Format(dst_format)) {
        return SDL_ConvertPixels_Packed4_to_Planar2x2(width, height, src_format, src, src_pitch, dst_format, dst, dst_pitch);
    } else {
        return SDL_SetError("SDL_ConvertPixels_YUV_to_YUV: Unsupported YUV conversion: %s -> %s", SDL_GetPixelFormatName(src_format), SDL_GetPixelFormatName(dst_format));
    }
}

/* vi: set ts=4 sw=4 expandtab: */
