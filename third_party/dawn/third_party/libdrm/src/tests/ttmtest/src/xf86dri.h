/* $XFree86: xc/lib/GL/dri/xf86dri.h,v 1.8 2002/10/30 12:51:25 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/**
 * \file xf86dri.h
 * Protocol numbers and function prototypes for DRI X protocol.
 *
 * \author Kevin E. Martin <martin@valinux.com>
 * \author Jens Owen <jens@tungstengraphics.com>
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 */

#ifndef _XF86DRI_H_
#define _XF86DRI_H_

#include <X11/Xfuncproto.h>
#include <drm/drm.h>

#define X_XF86DRIQueryVersion			0
#define X_XF86DRIQueryDirectRenderingCapable	1
#define X_XF86DRIOpenConnection			2
#define X_XF86DRICloseConnection		3
#define X_XF86DRIGetClientDriverName		4
#define X_XF86DRICreateContext			5
#define X_XF86DRIDestroyContext			6
#define X_XF86DRICreateDrawable			7
#define X_XF86DRIDestroyDrawable		8
#define X_XF86DRIGetDrawableInfo		9
#define X_XF86DRIGetDeviceInfo			10
#define X_XF86DRIAuthConnection                 11
#define X_XF86DRIOpenFullScreen                 12	/* Deprecated */
#define X_XF86DRICloseFullScreen                13	/* Deprecated */

#define XF86DRINumberEvents		0

#define XF86DRIClientNotLocal		0
#define XF86DRIOperationNotSupported	1
#define XF86DRINumberErrors		(XF86DRIOperationNotSupported + 1)

#ifndef _XF86DRI_SERVER_

_XFUNCPROTOBEGIN
    Bool uniDRIQueryExtension(Display * dpy, int *event_base,
    int *error_base);

Bool uniDRIQueryVersion(Display * dpy, int *majorVersion, int *minorVersion,
    int *patchVersion);

Bool uniDRIQueryDirectRenderingCapable(Display * dpy, int screen,
    Bool * isCapable);

Bool uniDRIOpenConnection(Display * dpy, int screen, drm_handle_t * hSAREA,
    char **busIDString);

Bool uniDRIAuthConnection(Display * dpy, int screen, drm_magic_t magic);

Bool uniDRICloseConnection(Display * dpy, int screen);

Bool uniDRIGetClientDriverName(Display * dpy, int screen,
    int *ddxDriverMajorVersion, int *ddxDriverMinorVersion,
    int *ddxDriverPatchVersion, char **clientDriverName);

Bool uniDRICreateContext(Display * dpy, int screen, Visual * visual,
    XID * ptr_to_returned_context_id, drm_context_t * hHWContext);

Bool uniDRICreateContextWithConfig(Display * dpy, int screen, int configID,
    XID * ptr_to_returned_context_id, drm_context_t * hHWContext);

extern Bool uniDRIDestroyContext(Display * dpy, int screen, XID context_id);

extern Bool uniDRICreateDrawable(Display * dpy, int screen,
    Drawable drawable, drm_drawable_t * hHWDrawable);

extern Bool uniDRIDestroyDrawable(Display * dpy, int screen,
    Drawable drawable);

Bool uniDRIGetDrawableInfo(Display * dpy, int screen, Drawable drawable,
    unsigned int *index, unsigned int *stamp,
    int *X, int *Y, int *W, int *H,
    int *numClipRects, drm_clip_rect_t ** pClipRects,
    int *backX, int *backY,
    int *numBackClipRects, drm_clip_rect_t ** pBackClipRects);

Bool uniDRIGetDeviceInfo(Display * dpy, int screen,
    drm_handle_t * hFrameBuffer, int *fbOrigin, int *fbSize,
    int *fbStride, int *devPrivateSize, void **pDevPrivate);

_XFUNCPROTOEND
#endif /* _XF86DRI_SERVER_ */
#endif /* _XF86DRI_H_ */
