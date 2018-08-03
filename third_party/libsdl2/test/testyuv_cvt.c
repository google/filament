/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include "SDL.h"

#include "testyuv_cvt.h"


static float clip3(float x, float y, float z)
{
    return ((z < x) ? x : ((z > y) ? y : z));
}

static void RGBtoYUV(Uint8 * rgb, int *yuv, SDL_YUV_CONVERSION_MODE mode, int monochrome, int luminance)
{
    if (mode == SDL_YUV_CONVERSION_JPEG) {
        /* Full range YUV */
        yuv[0] = (int)(0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]);
        yuv[1] = (int)((rgb[2] - yuv[0]) * 0.565 + 128);
        yuv[2] = (int)((rgb[0] - yuv[0]) * 0.713 + 128);
    } else {
        // This formula is from Microsoft's documentation:
        // https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx
        // L = Kr * R + Kb * B + (1 - Kr - Kb) * G
        // Y =                   floor(2^(M-8) * (219*(L-Z)/S + 16) + 0.5);
        // U = clip3(0, (2^M)-1, floor(2^(M-8) * (112*(B-L) / ((1-Kb)*S) + 128) + 0.5));
        // V = clip3(0, (2^M)-1, floor(2^(M-8) * (112*(R-L) / ((1-Kr)*S) + 128) + 0.5));
        float S, Z, R, G, B, L, Kr, Kb, Y, U, V;

        if (mode == SDL_YUV_CONVERSION_BT709) {
            /* BT.709 */
            Kr = 0.2126f;
            Kb = 0.0722f;
        } else {
            /* BT.601 */
            Kr = 0.299f;
            Kb = 0.114f;
        }

        S = 255.0f;
        Z = 0.0f;
        R = rgb[0];
        G = rgb[1];
        B = rgb[2];
        L = Kr * R + Kb * B + (1 - Kr - Kb) * G;
        Y = (Uint8)SDL_floorf((219*(L-Z)/S + 16) + 0.5f);
        U = (Uint8)clip3(0, 255, SDL_floorf((112.0f*(B-L) / ((1.0f-Kb)*S) + 128) + 0.5f));
        V = (Uint8)clip3(0, 255, SDL_floorf((112.0f*(R-L) / ((1.0f-Kr)*S) + 128) + 0.5f));

        yuv[0] = (Uint8)Y;
        yuv[1] = (Uint8)U;
        yuv[2] = (Uint8)V;
    }

    if (monochrome) {
        yuv[1] = 128;
        yuv[2] = 128;
    }

    if (luminance != 100) {
        yuv[0] = yuv[0] * luminance / 100;
        if (yuv[0] > 255)
            yuv[0] = 255;
    }
}

static void ConvertRGBtoPlanar2x2(Uint32 format, Uint8 *src, int pitch, Uint8 *out, int w, int h, SDL_YUV_CONVERSION_MODE mode, int monochrome, int luminance)
{
    int x, y;
    int yuv[4][3];
    Uint8 *Y1, *Y2, *U, *V;
    Uint8 *rgb1, *rgb2;
    int rgb_row_advance = (pitch - w*3) + pitch;
    int UV_advance;

    rgb1 = src;
    rgb2 = src + pitch;

    Y1 = out;
    Y2 = Y1 + w;
    switch (format) {
    case SDL_PIXELFORMAT_YV12:
        V = (Y1 + h * w);
        U = V + ((h + 1)/2)*((w + 1)/2);
        UV_advance = 1;
        break;
    case SDL_PIXELFORMAT_IYUV:
        U = (Y1 + h * w);
        V = U + ((h + 1)/2)*((w + 1)/2);
        UV_advance = 1;
        break;
    case SDL_PIXELFORMAT_NV12:
        U = (Y1 + h * w);
        V = U + 1;
        UV_advance = 2;
        break;
    case SDL_PIXELFORMAT_NV21:
        V = (Y1 + h * w);
        U = V + 1;
        UV_advance = 2;
        break;
    default:
        SDL_assert(!"Unsupported planar YUV format");
        return;
    }

    for (y = 0; y < (h - 1); y += 2) {
        for (x = 0; x < (w - 1); x += 2) {
            RGBtoYUV(rgb1, yuv[0], mode, monochrome, luminance);
            rgb1 += 3;
            *Y1++ = (Uint8)yuv[0][0];

            RGBtoYUV(rgb1, yuv[1], mode, monochrome, luminance);
            rgb1 += 3;
            *Y1++ = (Uint8)yuv[1][0];

            RGBtoYUV(rgb2, yuv[2], mode, monochrome, luminance);
            rgb2 += 3;
            *Y2++ = (Uint8)yuv[2][0];

            RGBtoYUV(rgb2, yuv[3], mode, monochrome, luminance);
            rgb2 += 3;
            *Y2++ = (Uint8)yuv[3][0];

            *U = (Uint8)SDL_floorf((yuv[0][1] + yuv[1][1] + yuv[2][1] + yuv[3][1])/4.0f + 0.5f);
            U += UV_advance;

            *V = (Uint8)SDL_floorf((yuv[0][2] + yuv[1][2] + yuv[2][2] + yuv[3][2])/4.0f + 0.5f);
            V += UV_advance;
        }
        /* Last column */
        if (x == (w - 1)) {
            RGBtoYUV(rgb1, yuv[0], mode, monochrome, luminance);
            rgb1 += 3;
            *Y1++ = (Uint8)yuv[0][0];

            RGBtoYUV(rgb2, yuv[2], mode, monochrome, luminance);
            rgb2 += 3;
            *Y2++ = (Uint8)yuv[2][0];

            *U = (Uint8)SDL_floorf((yuv[0][1] + yuv[2][1])/2.0f + 0.5f);
            U += UV_advance;

            *V = (Uint8)SDL_floorf((yuv[0][2] + yuv[2][2])/2.0f + 0.5f);
            V += UV_advance;
        }
        Y1 += w;
        Y2 += w;
        rgb1 += rgb_row_advance;
        rgb2 += rgb_row_advance;
    }
    /* Last row */
    if (y == (h - 1)) {
        for (x = 0; x < (w - 1); x += 2) {
            RGBtoYUV(rgb1, yuv[0], mode, monochrome, luminance);
            rgb1 += 3;
            *Y1++ = (Uint8)yuv[0][0];

            RGBtoYUV(rgb1, yuv[1], mode, monochrome, luminance);
            rgb1 += 3;
            *Y1++ = (Uint8)yuv[1][0];

            *U = (Uint8)SDL_floorf((yuv[0][1] + yuv[1][1])/2.0f + 0.5f);
            U += UV_advance;

            *V = (Uint8)SDL_floorf((yuv[0][2] + yuv[1][2])/2.0f + 0.5f);
            V += UV_advance;
        }
        /* Last column */
        if (x == (w - 1)) {
            RGBtoYUV(rgb1, yuv[0], mode, monochrome, luminance);
            *Y1++ = (Uint8)yuv[0][0];

            *U = (Uint8)yuv[0][1];
            U += UV_advance;

            *V = (Uint8)yuv[0][2];
            V += UV_advance;
        }
    }
}

static void ConvertRGBtoPacked4(Uint32 format, Uint8 *src, int pitch, Uint8 *out, int w, int h, SDL_YUV_CONVERSION_MODE mode, int monochrome, int luminance)
{
    int x, y;
    int yuv[2][3];
    Uint8 *Y1, *Y2, *U, *V;
    Uint8 *rgb;
    int rgb_row_advance = (pitch - w*3);

    rgb = src;

    switch (format) {
    case SDL_PIXELFORMAT_YUY2:
        Y1 = out;
        U = out+1;
        Y2 = out+2;
        V = out+3;
        break;
    case SDL_PIXELFORMAT_UYVY:
        U = out;
        Y1 = out+1;
        V = out+2;
        Y2 = out+3;
        break;
    case SDL_PIXELFORMAT_YVYU:
        Y1 = out;
        V = out+1;
        Y2 = out+2;
        U = out+3;
        break;
    default:
        SDL_assert(!"Unsupported packed YUV format");
        return;
    }

    for (y = 0; y < h; ++y) {
        for (x = 0; x < (w - 1); x += 2) {
            RGBtoYUV(rgb, yuv[0], mode, monochrome, luminance);
            rgb += 3;
            *Y1 = (Uint8)yuv[0][0];
            Y1 += 4;

            RGBtoYUV(rgb, yuv[1], mode, monochrome, luminance);
            rgb += 3;
            *Y2 = (Uint8)yuv[1][0];
            Y2 += 4;

            *U = (Uint8)SDL_floorf((yuv[0][1] + yuv[1][1])/2.0f + 0.5f);
            U += 4;

            *V = (Uint8)SDL_floorf((yuv[0][2] + yuv[1][2])/2.0f + 0.5f);
            V += 4;
        }
        /* Last column */
        if (x == (w - 1)) {
            RGBtoYUV(rgb, yuv[0], mode, monochrome, luminance);
            rgb += 3;
            *Y2 = *Y1 = (Uint8)yuv[0][0];
            Y1 += 4;
            Y2 += 4;

            *U = (Uint8)yuv[0][1];
            U += 4;

            *V = (Uint8)yuv[0][2];
            V += 4;
        }
        rgb += rgb_row_advance;
    }
}

SDL_bool ConvertRGBtoYUV(Uint32 format, Uint8 *src, int pitch, Uint8 *out, int w, int h, SDL_YUV_CONVERSION_MODE mode, int monochrome, int luminance)
{
    switch (format)
    {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        ConvertRGBtoPlanar2x2(format, src, pitch, out, w, h, mode, monochrome, luminance);
        return SDL_TRUE;
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        ConvertRGBtoPacked4(format, src, pitch, out, w, h, mode, monochrome, luminance);
        return SDL_TRUE;
    default:
        return SDL_FALSE;
    }
}

int CalculateYUVPitch(Uint32 format, int width)
{
    switch (format)
    {
    case SDL_PIXELFORMAT_YV12:
    case SDL_PIXELFORMAT_IYUV:
    case SDL_PIXELFORMAT_NV12:
    case SDL_PIXELFORMAT_NV21:
        return width;
    case SDL_PIXELFORMAT_YUY2:
    case SDL_PIXELFORMAT_UYVY:
    case SDL_PIXELFORMAT_YVYU:
        return 4*((width + 1)/2);
    default:
        return 0;
    }
}

/* vi: set ts=4 sw=4 expandtab: */
