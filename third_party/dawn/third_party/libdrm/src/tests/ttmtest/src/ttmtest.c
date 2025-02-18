/**************************************************************************
 * 
 * Copyright 2007 Tungsten Graphics, Inc., Cedar Park, TX., USA
 * All Rights Reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * 
 **************************************************************************/
/*
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <drm/drm.h>
#include "xf86dri.h"
#include "xf86drm.h"
#include "stdio.h"
#include "sys/types.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "sys/mman.h"

typedef struct
{
    enum
    {
	haveNothing,
	haveDisplay,
	haveConnection,
	haveDriverName,
	haveDeviceInfo,
	haveDRM,
	haveContext
    }
    state;

    Display *display;
    int screen;
    drm_handle_t sAreaOffset;
    char *curBusID;
    char *driverName;
    int drmFD;
    XVisualInfo visualInfo;
    XID id;
    drm_context_t hwContext;
    void *driPriv;
    int driPrivSize;
    int fbSize;
    int fbOrigin;
    int fbStride;
    drm_handle_t fbHandle;
    int ddxDriverMajor;
    int ddxDriverMinor;
    int ddxDriverPatch;
} TinyDRIContext;

#ifndef __x86_64__
static unsigned
fastrdtsc(void)
{
    unsigned eax;
    __asm__ volatile ("\t"
	"pushl  %%ebx\n\t"
	"cpuid\n\t" ".byte 0x0f, 0x31\n\t" "popl %%ebx\n":"=a" (eax)
	:"0"(0)
	:"ecx", "edx", "cc");

    return eax;
}
#else
static unsigned
fastrdtsc(void)
{
    unsigned eax;
    __asm__ volatile ("\t" "cpuid\n\t" ".byte 0x0f, 0x31\n\t":"=a" (eax)
	:"0"(0)
	:"ecx", "edx", "ebx", "cc");

    return eax;
}
#endif

void
bmError(int val, const char *file, const char *function, int line)
{
    fprintf(stderr, "Fatal video memory manager error \"%s\".\n"
	"Check kernel logs or set the LIBGL_DEBUG\n"
	"environment variable to \"verbose\" for more info.\n"
	"Detected in file %s, line %d, function %s.\n",
	strerror(-val), file, line, function);
    abort();
}

#define BM_CKFATAL(val)					       \
  do{							       \
    int tstVal = (val);					       \
    if (tstVal) 					       \
      bmError(tstVal, __FILE__, __FUNCTION__, __LINE__);       \
  } while(0);

static unsigned
time_diff(unsigned t, unsigned t2)
{
    return ((t < t2) ? t2 - t : 0xFFFFFFFFU - (t - t2 - 1));
}

static int
releaseContext(TinyDRIContext * ctx)
{
    switch (ctx->state) {
    case haveContext:
	uniDRIDestroyContext(ctx->display, ctx->screen, ctx->id);
    case haveDRM:
	drmClose(ctx->drmFD);
    case haveDeviceInfo:
	XFree(ctx->driPriv);
    case haveDriverName:
	XFree(ctx->driverName);
    case haveConnection:
	XFree(ctx->curBusID);
	uniDRICloseConnection(ctx->display, ctx->screen);
    case haveDisplay:
	XCloseDisplay(ctx->display);
    default:
	break;
    }
    return -1;
}

static void
readBuf(void *buf, unsigned long size)
{
    volatile unsigned *buf32 = (unsigned *)buf;
    unsigned *end = (unsigned *)buf32 + size / sizeof(*buf32);

    while (buf32 < end) {
	(void)*buf32++;
    }
}

static int
benchmarkBuffer(TinyDRIContext * ctx, unsigned long size,
    unsigned long *ticks)
{
    unsigned long curTime, oldTime;
    int ret;
    drmBO buf;
    void *virtual;

    /*
     * Test system memory objects.
     */
    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOCreate(ctx->drmFD, size, 0, NULL,
			   DRM_BO_FLAG_READ |
			   DRM_BO_FLAG_WRITE |
			   DRM_BO_FLAG_MEM_LOCAL, 0, &buf));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOMap(ctx->drmFD, &buf,
	    DRM_BO_FLAG_READ | DRM_BO_FLAG_WRITE, 0, &virtual));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0xF0, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0x0F, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    readBuf(virtual, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOUnmap(ctx->drmFD, &buf));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    /*
     * Test TT bound buffer objects.
     */

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOSetStatus(ctx->drmFD, &buf,
			     DRM_BO_FLAG_MEM_TT, 
			     DRM_BO_MASK_MEM, 
			      0,0,0));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOMap(ctx->drmFD, &buf,
	    DRM_BO_FLAG_READ | DRM_BO_FLAG_WRITE, 0, &virtual));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0xF0, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0x0F, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    readBuf(virtual, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    BM_CKFATAL(drmBOUnmap(ctx->drmFD, &buf));

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOSetStatus(ctx->drmFD, &buf,
			     DRM_BO_FLAG_MEM_LOCAL, DRM_BO_MASK_MEM, 0, 0,0));
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    /*
     * Test cached buffers objects.
     */

    oldTime = fastrdtsc();
    ret = drmBOSetStatus(ctx->drmFD, &buf,
			 DRM_BO_FLAG_MEM_TT | 
			 DRM_BO_FLAG_CACHED | 
			 DRM_BO_FLAG_FORCE_CACHING,
			 DRM_BO_MASK_MEMTYPE | 
			 DRM_BO_FLAG_FORCE_CACHING,
			 0, 0, 0);
    curTime = fastrdtsc();

    if (ret) {
	printf("Couldn't bind cached. Probably no support\n");
	BM_CKFATAL(drmBOUnreference(ctx->drmFD, &buf));
	return 1;
    }
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    BM_CKFATAL(drmBOMap(ctx->drmFD, &buf,
	    DRM_BO_FLAG_READ | DRM_BO_FLAG_WRITE, 0, &virtual));

    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0xF0, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    memset(virtual, 0x0F, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    oldTime = fastrdtsc();
    readBuf(virtual, buf.size);
    curTime = fastrdtsc();
    *ticks++ = time_diff(oldTime, curTime);

    BM_CKFATAL(drmBOUnmap(ctx->drmFD, &buf));
    BM_CKFATAL(drmBOUnreference(ctx->drmFD, &buf));

    return 0;
}

static void
testAGP(TinyDRIContext * ctx)
{
    unsigned long ticks[128], *pTicks;
    unsigned long size = 8 * 1024;
    int ret;

    ret = benchmarkBuffer(ctx, size, ticks);
    if (ret < 0) {
	fprintf(stderr, "Buffer error %s\n", strerror(-ret));
	return;
    }
    pTicks = ticks;

    printf("Buffer size %d bytes\n", size);
    printf("System memory timings ********************************\n");
    printf("Creation took            %12lu ticks\n", *pTicks++);
    printf("Mapping took             %12lu ticks\n", *pTicks++);
    printf("Writing took             %12lu ticks\n", *pTicks++);
    printf("Writing Again took       %12lu ticks\n", *pTicks++);
    printf("Reading took             %12lu ticks\n", *pTicks++);
    printf("Unmapping took           %12lu ticks\n", *pTicks++);

    printf("\nTT Memory timings ************************************\n");
    printf("Moving to TT took        %12lu ticks\n", *pTicks++);
    printf("Mapping in TT took       %12lu ticks\n", *pTicks++);
    printf("Writing to TT took       %12lu ticks\n", *pTicks++);
    printf("Writing again to TT took %12lu ticks\n", *pTicks++);
    printf("Reading from TT took     %12lu ticks\n", *pTicks++);
    printf("Moving to system took    %12lu ticks\n", *pTicks++);

    if (ret == 1)
	return;

    printf("\nCached TT Memory timings *****************************\n");
    printf("Moving to CTT took       %12lu ticks\n", *pTicks++);
    printf("Mapping in CTT took      %12lu ticks\n", *pTicks++);
    printf("Writing to CTT took      %12lu ticks\n", *pTicks++);
    printf("Re-writing to CTT took   %12lu ticks\n", *pTicks++);
    printf("Reading from CTT took    %12lu ticks\n", *pTicks++);
    printf("\n\n");
}

int
main()
{
    int ret, screen, isCapable;
    char *displayName = ":0";
    TinyDRIContext ctx;
    unsigned magic;

    ctx.screen = 0;
    ctx.state = haveNothing;
    ctx.display = XOpenDisplay(displayName);
    if (!ctx.display) {
	fprintf(stderr, "Could not open display\n");
	return releaseContext(&ctx);
    }
    ctx.state = haveDisplay;

    ret =
	uniDRIQueryDirectRenderingCapable(ctx.display, ctx.screen,
	&isCapable);
    if (!ret || !isCapable) {
	fprintf(stderr, "No DRI on this display:sceen\n");
	return releaseContext(&ctx);
    }

    if (!uniDRIOpenConnection(ctx.display, ctx.screen, &ctx.sAreaOffset,
	    &ctx.curBusID)) {
	fprintf(stderr, "Could not open DRI connection.\n");
	return releaseContext(&ctx);
    }
    ctx.state = haveConnection;

    if (!uniDRIGetClientDriverName(ctx.display, ctx.screen,
	    &ctx.ddxDriverMajor, &ctx.ddxDriverMinor,
	    &ctx.ddxDriverPatch, &ctx.driverName)) {
	fprintf(stderr, "Could not get DRI driver name.\n");
	return releaseContext(&ctx);
    }
    ctx.state = haveDriverName;

    if (!uniDRIGetDeviceInfo(ctx.display, ctx.screen,
	    &ctx.fbHandle, &ctx.fbOrigin, &ctx.fbSize,
	    &ctx.fbStride, &ctx.driPrivSize, &ctx.driPriv)) {
	fprintf(stderr, "Could not get DRI device info.\n");
	return releaseContext(&ctx);
    }
    ctx.state = haveDriverName;

    if ((ctx.drmFD = drmOpen(NULL, ctx.curBusID)) < 0) {
	perror("DRM Device could not be opened");
	return releaseContext(&ctx);
    }
    ctx.state = haveDRM;

    drmGetMagic(ctx.drmFD, &magic);
    if (!uniDRIAuthConnection(ctx.display, ctx.screen, magic)) {
	fprintf(stderr, "Could not get X server to authenticate us.\n");
	return releaseContext(&ctx);
    }

    ret = XMatchVisualInfo(ctx.display, ctx.screen, 24, TrueColor,
	&ctx.visualInfo);
    if (!ret) {
	ret = XMatchVisualInfo(ctx.display, ctx.screen, 16, TrueColor,
	    &ctx.visualInfo);
	if (!ret) {
	    fprintf(stderr, "Could not find a matching visual.\n");
	    return releaseContext(&ctx);
	}
    }

    if (!uniDRICreateContext(ctx.display, ctx.screen, ctx.visualInfo.visual,
	    &ctx.id, &ctx.hwContext)) {
	fprintf(stderr, "Could not create DRI context.\n");
	return releaseContext(&ctx);
    }
    ctx.state = haveContext;

    testAGP(&ctx);

    releaseContext(&ctx);
    printf("Terminating normally\n");
    return 0;
}
