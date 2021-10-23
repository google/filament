/*

SDL_rotate.c: rotates 32bit or 8bit surfaces

Shamelessly stolen from SDL_gfx by Andreas Schiffler. Original copyright follows:

Copyright (C) 2001-2011  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
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

   3. This notice may not be removed or altered from any source
   distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net

*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_RENDER_SW && !SDL_RENDER_DISABLED

#if defined(__WIN32__)
#include "../../core/windows/SDL_windows.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "SDL.h"
#include "SDL_rotate.h"

/* ---- Internally used structures */

/* !
\brief A 32 bit RGBA pixel.
*/
typedef struct tColorRGBA {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} tColorRGBA;

/* !
\brief A 8bit Y/palette pixel.
*/
typedef struct tColorY {
    Uint8 y;
} tColorY;

/* !
\brief Returns maximum of two numbers a and b.
*/
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))

/* !
\brief Number of guard rows added to destination surfaces.

This is a simple but effective workaround for observed issues.
These rows allocate extra memory and are then hidden from the surface.
Rows are added to the end of destination surfaces when they are allocated.
This catches any potential overflows which seem to happen with
just the right src image dimensions and scale/rotation and can lead
to a situation where the program can segfault.
*/
#define GUARD_ROWS (2)

/* !
\brief Returns colorkey info for a surface
*/
static Uint32
_colorkey(SDL_Surface *src)
{
    Uint32 key = 0;
    if (SDL_HasColorKey(src)) {
        SDL_GetColorKey(src, &key);
    }
    return key;
}


/* !
\brief Internal target surface sizing function for rotations with trig result return.

\param width The source surface width.
\param height The source surface height.
\param angle The angle to rotate in degrees.
\param dstwidth The calculated width of the destination surface.
\param dstheight The calculated height of the destination surface.
\param cangle The sine of the angle
\param sangle The cosine of the angle

*/
void
SDLgfx_rotozoomSurfaceSizeTrig(int width, int height, double angle,
                               int *dstwidth, int *dstheight,
                               double *cangle, double *sangle)
{
    /* The trig code below gets the wrong size (due to FP inaccuracy?) when angle is a multiple of 90 degrees */
    int angle90 = (int)(angle/90);
    if(angle90 == angle/90) { /* if the angle is a multiple of 90 degrees */
        angle90 %= 4;
        if(angle90 < 0) angle90 += 4; /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
        if(angle90 & 1) {
            *dstwidth  = height;
            *dstheight = width;
            *cangle = 0;
            *sangle = angle90 == 1 ? -1 : 1; /* reversed because our rotations are clockwise */
        } else {
            *dstwidth  = width;
            *dstheight = height;
            *cangle = angle90 == 0 ? 1 : -1;
            *sangle = 0;
        }
    } else {
        double x, y, cx, cy, sx, sy;
        double radangle;
        int dstwidthhalf, dstheighthalf;
        /*
        * Determine destination width and height by rotating a centered source box
        */
        radangle = angle * (M_PI / -180.0); /* reverse the angle because our rotations are clockwise */
        *sangle = SDL_sin(radangle);
        *cangle = SDL_cos(radangle);
        x = (double)(width / 2);
        y = (double)(height / 2);
        cx = *cangle * x;
        cy = *cangle * y;
        sx = *sangle * x;
        sy = *sangle * y;

        dstwidthhalf = MAX((int)
            SDL_ceil(MAX(MAX(MAX(SDL_fabs(cx + sy), SDL_fabs(cx - sy)), SDL_fabs(-cx + sy)), SDL_fabs(-cx - sy))), 1);
        dstheighthalf = MAX((int)
            SDL_ceil(MAX(MAX(MAX(SDL_fabs(sx + cy), SDL_fabs(sx - cy)), SDL_fabs(-sx + cy)), SDL_fabs(-sx - cy))), 1);
        *dstwidth = 2 * dstwidthhalf;
        *dstheight = 2 * dstheighthalf;
    }
}

/* Computes source pointer X/Y increments for a rotation that's a multiple of 90 degrees. */
static void
computeSourceIncrements90(SDL_Surface * src, int bpp, int angle, int flipx, int flipy,
                          int *sincx, int *sincy, int *signx, int *signy)
{
    int pitch = flipy ? -src->pitch : src->pitch;
    if (flipx) {
        bpp = -bpp;
    }
    switch (angle) { /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
    case 0: *sincx = bpp; *sincy = pitch - src->w * *sincx; *signx = *signy = 1; break;
    case 1: *sincx = -pitch; *sincy = bpp - *sincx * src->h; *signx = 1; *signy = -1; break;
    case 2: *sincx = -bpp; *sincy = -src->w * *sincx - pitch; *signx = *signy = -1; break;
    case 3: default: *sincx = pitch; *sincy = -*sincx * src->h - bpp; *signx = -1; *signy = 1; break;
    }
    if (flipx) {
        *signx = -*signx;
    }
    if (flipy) {
        *signy = -*signy;
    }
}

/* Performs a relatively fast rotation/flip when the angle is a multiple of 90 degrees. */
#define TRANSFORM_SURFACE_90(pixelType) \
    int dy, dincy = dst->pitch - dst->w*sizeof(pixelType), sincx, sincy, signx, signy;                      \
    Uint8 *sp = (Uint8*)src->pixels, *dp = (Uint8*)dst->pixels, *de;                                        \
                                                                                                            \
    computeSourceIncrements90(src, sizeof(pixelType), angle, flipx, flipy, &sincx, &sincy, &signx, &signy); \
    if (signx < 0) sp += (src->w-1)*sizeof(pixelType);                                                      \
    if (signy < 0) sp += (src->h-1)*src->pitch;                                                             \
                                                                                                            \
    for (dy = 0; dy < dst->h; sp += sincy, dp += dincy, dy++) {                                             \
        if (sincx == sizeof(pixelType)) { /* if advancing src and dest equally, use memcpy */               \
            SDL_memcpy(dp, sp, dst->w*sizeof(pixelType));                                                   \
            sp += dst->w*sizeof(pixelType);                                                                 \
            dp += dst->w*sizeof(pixelType);                                                                 \
        } else {                                                                                            \
            for (de = dp + dst->w*sizeof(pixelType); dp != de; sp += sincx, dp += sizeof(pixelType)) {      \
                *(pixelType*)dp = *(pixelType*)sp;                                                          \
            }                                                                                               \
        }                                                                                                   \
    }

static void
transformSurfaceRGBA90(SDL_Surface * src, SDL_Surface * dst, int angle, int flipx, int flipy)
{
    TRANSFORM_SURFACE_90(tColorRGBA);
}

static void
transformSurfaceY90(SDL_Surface * src, SDL_Surface * dst, int angle, int flipx, int flipy)
{
    TRANSFORM_SURFACE_90(tColorY);
}

#undef TRANSFORM_SURFACE_90

/* !
\brief Internal 32 bit rotozoomer with optional anti-aliasing.

Rotates and zooms 32 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
parameters by scanning the destination surface and applying optionally anti-aliasing
by bilinear interpolation.
Assumes src and dst surfaces are of 32 bit depth.
Assumes dst surface was allocated with the correct dimensions.

\param src Source surface.
\param dst Destination surface.
\param cx Horizontal center coordinate.
\param cy Vertical center coordinate.
\param isin Integer version of sine of angle.
\param icos Integer version of cosine of angle.
\param flipx Flag indicating horizontal mirroring should be applied.
\param flipy Flag indicating vertical mirroring should be applied.
\param smooth Flag indicating anti-aliasing should be used.
*/
static void
_transformSurfaceRGBA(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos, int flipx, int flipy, int smooth)
{
    int x, y, t1, t2, dx, dy, xd, yd, sdx, sdy, ax, ay, ex, ey, sw, sh;
    tColorRGBA c00, c01, c10, c11, cswap;
    tColorRGBA *pc, *sp;
    int gap;

    /*
    * Variable setup
    */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    sw = src->w - 1;
    sh = src->h - 1;
    pc = (tColorRGBA*) dst->pixels;
    gap = dst->pitch - dst->w * 4;

    /*
    * Switch between interpolating and non-interpolating code
    */
    if (smooth) {
        for (y = 0; y < dst->h; y++) {
            dy = cy - y;
            sdx = (ax + (isin * dy)) + xd;
            sdy = (ay - (icos * dy)) + yd;
            for (x = 0; x < dst->w; x++) {
                dx = (sdx >> 16);
                dy = (sdy >> 16);
                if (flipx) dx = sw - dx;
                if (flipy) dy = sh - dy;
                if ((dx > -1) && (dy > -1) && (dx < (src->w-1)) && (dy < (src->h-1))) {
                    sp = (tColorRGBA *) ((Uint8 *) src->pixels + src->pitch * dy) + dx;
                    c00 = *sp;
                    sp += 1;
                    c01 = *sp;
                    sp += (src->pitch/4);
                    c11 = *sp;
                    sp -= 1;
                    c10 = *sp;
                    if (flipx) {
                        cswap = c00; c00=c01; c01=cswap;
                        cswap = c10; c10=c11; c11=cswap;
                    }
                    if (flipy) {
                        cswap = c00; c00=c10; c10=cswap;
                        cswap = c01; c01=c11; c11=cswap;
                    }
                    /*
                    * Interpolate colors
                    */
                    ex = (sdx & 0xffff);
                    ey = (sdy & 0xffff);
                    t1 = ((((c01.r - c00.r) * ex) >> 16) + c00.r) & 0xff;
                    t2 = ((((c11.r - c10.r) * ex) >> 16) + c10.r) & 0xff;
                    pc->r = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.g - c00.g) * ex) >> 16) + c00.g) & 0xff;
                    t2 = ((((c11.g - c10.g) * ex) >> 16) + c10.g) & 0xff;
                    pc->g = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.b - c00.b) * ex) >> 16) + c00.b) & 0xff;
                    t2 = ((((c11.b - c10.b) * ex) >> 16) + c10.b) & 0xff;
                    pc->b = (((t2 - t1) * ey) >> 16) + t1;
                    t1 = ((((c01.a - c00.a) * ex) >> 16) + c00.a) & 0xff;
                    t2 = ((((c11.a - c10.a) * ex) >> 16) + c10.a) & 0xff;
                    pc->a = (((t2 - t1) * ey) >> 16) + t1;
                }
                sdx += icos;
                sdy += isin;
                pc++;
            }
            pc = (tColorRGBA *) ((Uint8 *) pc + gap);
        }
    } else {
        for (y = 0; y < dst->h; y++) {
            dy = cy - y;
            sdx = (ax + (isin * dy)) + xd;
            sdy = (ay - (icos * dy)) + yd;
            for (x = 0; x < dst->w; x++) {
                dx = (sdx >> 16);
                dy = (sdy >> 16);
                if ((unsigned)dx < (unsigned)src->w && (unsigned)dy < (unsigned)src->h) {
                    if(flipx) dx = sw - dx;
                    if(flipy) dy = sh - dy;
                    *pc = *((tColorRGBA *)((Uint8 *)src->pixels + src->pitch * dy) + dx);
                }
                sdx += icos;
                sdy += isin;
                pc++;
            }
            pc = (tColorRGBA *) ((Uint8 *) pc + gap);
        }
    }
}

/* !

\brief Rotates and zooms 8 bit palette/Y 'src' surface to 'dst' surface without smoothing.

Rotates and zooms 8 bit RGBA/ABGR 'src' surface to 'dst' surface based on the control
parameters by scanning the destination surface.
Assumes src and dst surfaces are of 8 bit depth.
Assumes dst surface was allocated with the correct dimensions.

\param src Source surface.
\param dst Destination surface.
\param cx Horizontal center coordinate.
\param cy Vertical center coordinate.
\param isin Integer version of sine of angle.
\param icos Integer version of cosine of angle.
\param flipx Flag indicating horizontal mirroring should be applied.
\param flipy Flag indicating vertical mirroring should be applied.
*/
static void
transformSurfaceY(SDL_Surface * src, SDL_Surface * dst, int cx, int cy, int isin, int icos, int flipx, int flipy)
{
    int x, y, dx, dy, xd, yd, sdx, sdy, ax, ay;
    tColorY *pc;
    int gap;

    /*
    * Variable setup
    */
    xd = ((src->w - dst->w) << 15);
    yd = ((src->h - dst->h) << 15);
    ax = (cx << 16) - (icos * cx);
    ay = (cy << 16) - (isin * cx);
    pc = (tColorY*) dst->pixels;
    gap = dst->pitch - dst->w;
    /*
    * Clear surface to colorkey
    */
    SDL_memset(pc, (int)(_colorkey(src) & 0xff), dst->pitch * dst->h);
    /*
    * Iterate through destination surface
    */
    for (y = 0; y < dst->h; y++) {
        dy = cy - y;
        sdx = (ax + (isin * dy)) + xd;
        sdy = (ay - (icos * dy)) + yd;
        for (x = 0; x < dst->w; x++) {
            dx = (sdx >> 16);
            dy = (sdy >> 16);
            if ((unsigned)dx < (unsigned)src->w && (unsigned)dy < (unsigned)src->h) {
                if (flipx) dx = (src->w-1)-dx;
                if (flipy) dy = (src->h-1)-dy;
                *pc = *((tColorY *)src->pixels + src->pitch * dy + dx);
            }
            sdx += icos;
            sdy += isin;
            pc++;
        }
        pc += gap;
    }
}


/* !
\brief Rotates and zooms a surface with different horizontal and vertival scaling factors and optional anti-aliasing.

Rotates a 32-bit or 8-bit 'src' surface to newly created 'dst' surface.
'angle' is the rotation in degrees, 'centerx' and 'centery' the rotation center. If 'smooth' is set
then the destination 32-bit surface is anti-aliased. 8-bit surfaces must have a colorkey. 32-bit
surfaces must have a 8888 layout with red, green, blue and alpha masks (any ordering goes).
The blend mode of the 'src' surface has some effects on generation of the 'dst' surface: The NONE
mode will set the BLEND mode on the 'dst' surface. The MOD mode either generates a white 'dst'
surface and sets the colorkey or fills the it with the colorkey before copying the pixels.
When using the NONE and MOD modes, color and alpha modulation must be applied before using this function.

\param src The surface to rotozoom.
\param angle The angle to rotate in degrees.
\param centerx The horizontal coordinate of the center of rotation
\param zoomy The vertical coordinate of the center of rotation
\param smooth Antialiasing flag; set to SMOOTHING_ON to enable.
\param flipx Set to 1 to flip the image horizontally
\param flipy Set to 1 to flip the image vertically
\param dstwidth The destination surface width
\param dstheight The destination surface height
\param cangle The angle cosine
\param sangle The angle sine
\return The new rotated surface.

*/

SDL_Surface *
SDLgfx_rotateSurface(SDL_Surface * src, double angle, int centerx, int centery, int smooth, int flipx, int flipy, int dstwidth, int dstheight, double cangle, double sangle)
{
    SDL_Surface *rz_dst;
    int is8bit, angle90;
    int i;
    SDL_BlendMode blendmode;
    Uint32 colorkey = 0;
    int colorKeyAvailable = SDL_FALSE;
    double sangleinv, cangleinv;

    /* Sanity check */
    if (src == NULL)
        return NULL;

    if (SDL_HasColorKey(src)) {
        if (SDL_GetColorKey(src, &colorkey) == 0) {
            colorKeyAvailable = SDL_TRUE;
        }
    }

    /* This function requires a 32-bit surface or 8-bit surface with a colorkey */
    is8bit = src->format->BitsPerPixel == 8 && colorKeyAvailable;
    if (!(is8bit || (src->format->BitsPerPixel == 32 && src->format->Amask)))
        return NULL;

    /* Calculate target factors from sin/cos and zoom */
    sangleinv = sangle*65536.0;
    cangleinv = cangle*65536.0;

    /* Alloc space to completely contain the rotated surface */
    rz_dst = NULL;
    if (is8bit) {
        /* Target surface is 8 bit */
        rz_dst = SDL_CreateRGBSurface(0, dstwidth, dstheight + GUARD_ROWS, 8, 0, 0, 0, 0);
        if (rz_dst != NULL) {
            for (i = 0; i < src->format->palette->ncolors; i++) {
                rz_dst->format->palette->colors[i] = src->format->palette->colors[i];
            }
            rz_dst->format->palette->ncolors = src->format->palette->ncolors;
        }
    } else {
        /* Target surface is 32 bit with source RGBA ordering */
        rz_dst = SDL_CreateRGBSurface(0, dstwidth, dstheight + GUARD_ROWS, 32,
                                      src->format->Rmask, src->format->Gmask,
                                      src->format->Bmask, src->format->Amask);
    }

    /* Check target */
    if (rz_dst == NULL)
        return NULL;

    /* Adjust for guard rows */
    rz_dst->h = dstheight;

    SDL_GetSurfaceBlendMode(src, &blendmode);

    if (colorKeyAvailable == SDL_TRUE) {
        /* If available, the colorkey will be used to discard the pixels that are outside of the rotated area. */
        SDL_SetColorKey(rz_dst, SDL_TRUE, colorkey);
        SDL_FillRect(rz_dst, NULL, colorkey);
    } else if (blendmode == SDL_BLENDMODE_NONE) {
        blendmode = SDL_BLENDMODE_BLEND;
    } else if (blendmode == SDL_BLENDMODE_MOD || blendmode == SDL_BLENDMODE_MUL) {
        /* Without a colorkey, the target texture has to be white for the MOD and MUL blend mode so
         * that the pixels outside the rotated area don't affect the destination surface.
         */
        colorkey = SDL_MapRGBA(rz_dst->format, 255, 255, 255, 0);
        SDL_FillRect(rz_dst, NULL, colorkey);
        /* Setting a white colorkey for the destination surface makes the final blit discard
         * all pixels outside of the rotated area. This doesn't interfere with anything because
         * white pixels are already a no-op and the MOD blend mode does not interact with alpha.
         */
        SDL_SetColorKey(rz_dst, SDL_TRUE, colorkey);
    }

    SDL_SetSurfaceBlendMode(rz_dst, blendmode);

    /* Lock source surface */
    if (SDL_MUSTLOCK(src)) {
        SDL_LockSurface(src);
    }

    /* check if the rotation is a multiple of 90 degrees so we can take a fast path and also somewhat reduce
     * the off-by-one problem in _transformSurfaceRGBA that expresses itself when the rotation is near
     * multiples of 90 degrees.
     */
    angle90 = (int)(angle/90);
    if (angle90 == angle/90) {
        angle90 %= 4;
        if (angle90 < 0) angle90 += 4; /* 0:0 deg, 1:90 deg, 2:180 deg, 3:270 deg */
    } else {
        angle90 = -1;
    }

    if (is8bit) {
        /* Call the 8-bit transformation routine to do the rotation */
        if(angle90 >= 0) {
            transformSurfaceY90(src, rz_dst, angle90, flipx, flipy);
        } else {
            transformSurfaceY(src, rz_dst, centerx, centery, (int)sangleinv, (int)cangleinv,
                              flipx, flipy);
        }
    } else {
        /* Call the 32-bit transformation routine to do the rotation */
        if (angle90 >= 0) {
            transformSurfaceRGBA90(src, rz_dst, angle90, flipx, flipy);
        } else {
            _transformSurfaceRGBA(src, rz_dst, centerx, centery, (int)sangleinv, (int)cangleinv,
                                  flipx, flipy, smooth);
        }
    }

    /* Unlock source surface */
    if (SDL_MUSTLOCK(src)) {
        SDL_UnlockSurface(src);
    }

    /* Return rotated surface */
    return rz_dst;
}

#endif /* SDL_VIDEO_RENDER_SW && !SDL_RENDER_DISABLED */
