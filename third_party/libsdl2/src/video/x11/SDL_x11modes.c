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
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_X11

#include "SDL_hints.h"
#include "SDL_x11video.h"
#include "SDL_timer.h"
#include "edid.h"

/* #define X11MODES_DEBUG */

/* I'm becoming more and more convinced that the application should never
 * use XRandR, and it's the window manager's responsibility to track and
 * manage display modes for fullscreen windows.  Right now XRandR is completely
 * broken with respect to window manager behavior on every window manager that
 * I can find.  For example, on Unity 3D if you show a fullscreen window while
 * the resolution is changing (within ~250 ms) your window will retain the
 * fullscreen state hint but be decorated and windowed.
 *
 * However, many people swear by it, so let them swear at it. :)
*/
/* #define XRANDR_DISABLED_BY_DEFAULT */


static int
get_visualinfo(Display * display, int screen, XVisualInfo * vinfo)
{
    const char *visual_id = SDL_getenv("SDL_VIDEO_X11_VISUALID");
    int depth;

    /* Look for an exact visual, if requested */
    if (visual_id) {
        XVisualInfo *vi, template;
        int nvis;

        SDL_zero(template);
        template.visualid = SDL_strtol(visual_id, NULL, 0);
        vi = X11_XGetVisualInfo(display, VisualIDMask, &template, &nvis);
        if (vi) {
            *vinfo = *vi;
            X11_XFree(vi);
            return 0;
        }
    }

    depth = DefaultDepth(display, screen);
    if ((X11_UseDirectColorVisuals() &&
         X11_XMatchVisualInfo(display, screen, depth, DirectColor, vinfo)) ||
        X11_XMatchVisualInfo(display, screen, depth, TrueColor, vinfo) ||
        X11_XMatchVisualInfo(display, screen, depth, PseudoColor, vinfo) ||
        X11_XMatchVisualInfo(display, screen, depth, StaticColor, vinfo)) {
        return 0;
    }
    return -1;
}

int
X11_GetVisualInfoFromVisual(Display * display, Visual * visual, XVisualInfo * vinfo)
{
    XVisualInfo *vi;
    int nvis;

    vinfo->visualid = X11_XVisualIDFromVisual(visual);
    vi = X11_XGetVisualInfo(display, VisualIDMask, vinfo, &nvis);
    if (vi) {
        *vinfo = *vi;
        X11_XFree(vi);
        return 0;
    }
    return -1;
}

Uint32
X11_GetPixelFormatFromVisualInfo(Display * display, XVisualInfo * vinfo)
{
    if (vinfo->class == DirectColor || vinfo->class == TrueColor) {
        int bpp;
        Uint32 Rmask, Gmask, Bmask, Amask;

        Rmask = vinfo->visual->red_mask;
        Gmask = vinfo->visual->green_mask;
        Bmask = vinfo->visual->blue_mask;
        if (vinfo->depth == 32) {
            Amask = (0xFFFFFFFF & ~(Rmask | Gmask | Bmask));
        } else {
            Amask = 0;
        }

        bpp = vinfo->depth;
        if (bpp == 24) {
            int i, n;
            XPixmapFormatValues *p = X11_XListPixmapFormats(display, &n);
            if (p) {
                for (i = 0; i < n; ++i) {
                    if (p[i].depth == 24) {
                        bpp = p[i].bits_per_pixel;
                        break;
                    }
                }
                X11_XFree(p);
            }
        }

        return SDL_MasksToPixelFormatEnum(bpp, Rmask, Gmask, Bmask, Amask);
    }

    if (vinfo->class == PseudoColor || vinfo->class == StaticColor) {
        switch (vinfo->depth) {
        case 8:
            return SDL_PIXELTYPE_INDEX8;
        case 4:
            if (BitmapBitOrder(display) == LSBFirst) {
                return SDL_PIXELFORMAT_INDEX4LSB;
            } else {
                return SDL_PIXELFORMAT_INDEX4MSB;
            }
            /* break; -Wunreachable-code-break */
        case 1:
            if (BitmapBitOrder(display) == LSBFirst) {
                return SDL_PIXELFORMAT_INDEX1LSB;
            } else {
                return SDL_PIXELFORMAT_INDEX1MSB;
            }
            /* break; -Wunreachable-code-break */
        }
    }

    return SDL_PIXELFORMAT_UNKNOWN;
}

#if SDL_VIDEO_DRIVER_X11_XINERAMA
static SDL_bool
CheckXinerama(Display * display, int *major, int *minor)
{
    int event_base = 0;
    int error_base = 0;

    /* Default the extension not available */
    *major = *minor = 0;

    /* Allow environment override */
    if (!SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_XINERAMA, SDL_TRUE)) {
#ifdef X11MODES_DEBUG
        printf("Xinerama disabled due to hint\n");
#endif
        return SDL_FALSE;
    }

    if (!SDL_X11_HAVE_XINERAMA) {
#ifdef X11MODES_DEBUG
        printf("Xinerama support not available\n");
#endif
        return SDL_FALSE;
    }

    /* Query the extension version */
    if (!X11_XineramaQueryExtension(display, &event_base, &error_base) ||
        !X11_XineramaQueryVersion(display, major, minor) ||
        !X11_XineramaIsActive(display)) {
#ifdef X11MODES_DEBUG
        printf("Xinerama not active on the display\n");
#endif
        return SDL_FALSE;
    }
#ifdef X11MODES_DEBUG
    printf("Xinerama available at version %d.%d!\n", *major, *minor);
#endif
    return SDL_TRUE;
}

/* !!! FIXME: remove this later. */
/* we have a weird bug where XineramaQueryScreens() throws an X error, so this
   is here to help track it down (and not crash, too!). */
static SDL_bool xinerama_triggered_error = SDL_FALSE;
static int
X11_XineramaFailed(Display * d, XErrorEvent * e)
{
    xinerama_triggered_error = SDL_TRUE;
    fprintf(stderr, "XINERAMA X ERROR: type=%d serial=%lu err=%u req=%u minor=%u\n",
            e->type, e->serial, (unsigned int) e->error_code,
            (unsigned int) e->request_code, (unsigned int) e->minor_code);
    fflush(stderr);
    return 0;
}
#endif /* SDL_VIDEO_DRIVER_X11_XINERAMA */

#if SDL_VIDEO_DRIVER_X11_XRANDR
static SDL_bool
CheckXRandR(Display * display, int *major, int *minor)
{
    /* Default the extension not available */
    *major = *minor = 0;

    /* Allow environment override */
#ifdef XRANDR_DISABLED_BY_DEFAULT
    if (!SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_XRANDR, SDL_FALSE)) {
#ifdef X11MODES_DEBUG
        printf("XRandR disabled by default due to window manager issues\n");
#endif
        return SDL_FALSE;
    }
#else
    if (!SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_XRANDR, SDL_TRUE)) {
#ifdef X11MODES_DEBUG
        printf("XRandR disabled due to hint\n");
#endif
        return SDL_FALSE;
    }
#endif /* XRANDR_ENABLED_BY_DEFAULT */

    if (!SDL_X11_HAVE_XRANDR) {
#ifdef X11MODES_DEBUG
        printf("XRandR support not available\n");
#endif
        return SDL_FALSE;
    }

    /* Query the extension version */
    *major = 1; *minor = 3;  /* we want 1.3 */
    if (!X11_XRRQueryVersion(display, major, minor)) {
#ifdef X11MODES_DEBUG
        printf("XRandR not active on the display\n");
#endif
        *major = *minor = 0;
        return SDL_FALSE;
    }
#ifdef X11MODES_DEBUG
    printf("XRandR available at version %d.%d!\n", *major, *minor);
#endif
    return SDL_TRUE;
}

#define XRANDR_ROTATION_LEFT    (1 << 1)
#define XRANDR_ROTATION_RIGHT   (1 << 3)

static int
CalculateXRandRRefreshRate(const XRRModeInfo *info)
{
    return (info->hTotal && info->vTotal) ?
        round(((double)info->dotClock / (double)(info->hTotal * info->vTotal))) : 0;
}

static SDL_bool
SetXRandRModeInfo(Display *display, XRRScreenResources *res, RRCrtc crtc,
                  RRMode modeID, SDL_DisplayMode *mode)
{
    int i;
    for (i = 0; i < res->nmode; ++i) {
        const XRRModeInfo *info = &res->modes[i];
        if (info->id == modeID) {
            XRRCrtcInfo *crtcinfo;
            Rotation rotation = 0;

            crtcinfo = X11_XRRGetCrtcInfo(display, res, crtc);
            if (crtcinfo) {
                rotation = crtcinfo->rotation;
                X11_XRRFreeCrtcInfo(crtcinfo);
            }

            if (rotation & (XRANDR_ROTATION_LEFT|XRANDR_ROTATION_RIGHT)) {
                mode->w = info->height;
                mode->h = info->width;
            } else {
                mode->w = info->width;
                mode->h = info->height;
            }
            mode->refresh_rate = CalculateXRandRRefreshRate(info);
            ((SDL_DisplayModeData*)mode->driverdata)->xrandr_mode = modeID;
#ifdef X11MODES_DEBUG
            printf("XRandR mode %d: %dx%d@%dHz\n", (int) modeID, mode->w, mode->h, mode->refresh_rate);
#endif
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

static void
SetXRandRDisplayName(Display *dpy, Atom EDID, char *name, const size_t namelen, RROutput output, const unsigned long widthmm, const unsigned long heightmm)
{
    /* See if we can get the EDID data for the real monitor name */
    int inches;
    int nprop;
    Atom *props = X11_XRRListOutputProperties(dpy, output, &nprop);
    int i;

    for (i = 0; i < nprop; ++i) {
        unsigned char *prop;
        int actual_format;
        unsigned long nitems, bytes_after;
        Atom actual_type;

        if (props[i] == EDID) {
            if (X11_XRRGetOutputProperty(dpy, output, props[i], 0, 100, False,
                                         False, AnyPropertyType, &actual_type,
                                         &actual_format, &nitems, &bytes_after,
                                         &prop) == Success) {
                MonitorInfo *info = decode_edid(prop);
                if (info) {
#ifdef X11MODES_DEBUG
                    printf("Found EDID data for %s\n", name);
                    dump_monitor_info(info);
#endif
                    SDL_strlcpy(name, info->dsc_product_name, namelen);
                    free(info);
                }
                X11_XFree(prop);
            }
            break;
        }
    }

    if (props) {
        X11_XFree(props);
    }

    inches = (int)((SDL_sqrtf(widthmm * widthmm + heightmm * heightmm) / 25.4f) + 0.5f);
    if (*name && inches) {
        const size_t len = SDL_strlen(name);
        SDL_snprintf(&name[len], namelen-len, " %d\"", inches);
    }

#ifdef X11MODES_DEBUG
    printf("Display name: %s\n", name);
#endif
}


static int
X11_InitModes_XRandR(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    Display *dpy = data->display;
    const int screencount = ScreenCount(dpy);
    const int default_screen = DefaultScreen(dpy);
    RROutput primary = X11_XRRGetOutputPrimary(dpy, RootWindow(dpy, default_screen));
    Atom EDID = X11_XInternAtom(dpy, "EDID", False);
    XRRScreenResources *res = NULL;
    Uint32 pixelformat;
    XVisualInfo vinfo;
    XPixmapFormatValues *pixmapformats;
    int looking_for_primary;
    int scanline_pad;
    int output;
    int screen, i, n;

    for (looking_for_primary = 1; looking_for_primary >= 0; looking_for_primary--) {
        for (screen = 0; screen < screencount; screen++) {

            /* we want the primary output first, and then skipped later. */
            if (looking_for_primary && (screen != default_screen)) {
                continue;
            }

            if (get_visualinfo(dpy, screen, &vinfo) < 0) {
                continue;  /* uh, skip this screen? */
            }

            pixelformat = X11_GetPixelFormatFromVisualInfo(dpy, &vinfo);
            if (SDL_ISPIXELFORMAT_INDEXED(pixelformat)) {
                continue;  /* Palettized video modes are no longer supported */
            }

            scanline_pad = SDL_BYTESPERPIXEL(pixelformat) * 8;
            pixmapformats = X11_XListPixmapFormats(dpy, &n);
            if (pixmapformats) {
                for (i = 0; i < n; ++i) {
                    if (pixmapformats[i].depth == vinfo.depth) {
                        scanline_pad = pixmapformats[i].scanline_pad;
                        break;
                    }
                }
                X11_XFree(pixmapformats);
            }

            res = X11_XRRGetScreenResourcesCurrent(dpy, RootWindow(dpy, screen));
            if (!res || res->noutput == 0) {
                if (res) {
                    X11_XRRFreeScreenResources(res);
                }

                res = X11_XRRGetScreenResources(dpy, RootWindow(dpy, screen));
                if (!res) {
                    continue;
                }
            }

            for (output = 0; output < res->noutput; output++) {
                XRROutputInfo *output_info;
                int display_x, display_y;
                unsigned long display_mm_width, display_mm_height;
                SDL_DisplayData *displaydata;
                char display_name[128];
                SDL_DisplayMode mode;
                SDL_DisplayModeData *modedata;
                SDL_VideoDisplay display;
                RRMode modeID;
                RRCrtc output_crtc;
                XRRCrtcInfo *crtc;

                /* The primary output _should_ always be sorted first, but just in case... */
                if ((looking_for_primary && (res->outputs[output] != primary)) ||
                    (!looking_for_primary && (screen == default_screen) && (res->outputs[output] == primary))) {
                    continue;
                }

                output_info = X11_XRRGetOutputInfo(dpy, res, res->outputs[output]);
                if (!output_info || !output_info->crtc || output_info->connection == RR_Disconnected) {
                    X11_XRRFreeOutputInfo(output_info);
                    continue;
                }

                SDL_strlcpy(display_name, output_info->name, sizeof(display_name));
                display_mm_width = output_info->mm_width;
                display_mm_height = output_info->mm_height;
                output_crtc = output_info->crtc;
                X11_XRRFreeOutputInfo(output_info);

                crtc = X11_XRRGetCrtcInfo(dpy, res, output_crtc);
                if (!crtc) {
                    continue;
                }

                SDL_zero(mode);
                modeID = crtc->mode;
                mode.w = crtc->width;
                mode.h = crtc->height;
                mode.format = pixelformat;

                display_x = crtc->x;
                display_y = crtc->y;

                X11_XRRFreeCrtcInfo(crtc);

                displaydata = (SDL_DisplayData *) SDL_calloc(1, sizeof(*displaydata));
                if (!displaydata) {
                    return SDL_OutOfMemory();
                }

                modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
                if (!modedata) {
                    SDL_free(displaydata);
                    return SDL_OutOfMemory();
                }
                modedata->xrandr_mode = modeID;
                mode.driverdata = modedata;

                displaydata->screen = screen;
                displaydata->visual = vinfo.visual;
                displaydata->depth = vinfo.depth;
                displaydata->hdpi = display_mm_width ? (((float) mode.w) * 25.4f / display_mm_width) : 0.0f;
                displaydata->vdpi = display_mm_height ? (((float) mode.h) * 25.4f / display_mm_height) : 0.0f;
                displaydata->ddpi = SDL_ComputeDiagonalDPI(mode.w, mode.h, ((float) display_mm_width) / 25.4f,((float) display_mm_height) / 25.4f);
                displaydata->scanline_pad = scanline_pad;
                displaydata->x = display_x;
                displaydata->y = display_y;
                displaydata->use_xrandr = 1;
                displaydata->xrandr_output = res->outputs[output];

                SetXRandRModeInfo(dpy, res, output_crtc, modeID, &mode);
                SetXRandRDisplayName(dpy, EDID, display_name, sizeof (display_name), res->outputs[output], display_mm_width, display_mm_height);

                SDL_zero(display);
                if (*display_name) {
                    display.name = display_name;
                }
                display.desktop_mode = mode;
                display.current_mode = mode;
                display.driverdata = displaydata;
                SDL_AddVideoDisplay(&display);
            }

            X11_XRRFreeScreenResources(res);
        }
    }

    if (_this->num_displays == 0) {
        return SDL_SetError("No available displays");
    }

    return 0;
}
#endif /* SDL_VIDEO_DRIVER_X11_XRANDR */

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
static SDL_bool
CheckVidMode(Display * display, int *major, int *minor)
{
    int vm_event, vm_error = -1;
    /* Default the extension not available */
    *major = *minor = 0;

    /* Allow environment override */
    if (!SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_XVIDMODE, SDL_TRUE)) {
#ifdef X11MODES_DEBUG
        printf("XVidMode disabled due to hint\n");
#endif
        return SDL_FALSE;
    }

    if (!SDL_X11_HAVE_XVIDMODE) {
#ifdef X11MODES_DEBUG
        printf("XVidMode support not available\n");
#endif
        return SDL_FALSE;
    }

    /* Query the extension version */
    if (!X11_XF86VidModeQueryExtension(display, &vm_event, &vm_error)
        || !X11_XF86VidModeQueryVersion(display, major, minor)) {
#ifdef X11MODES_DEBUG
        printf("XVidMode not active on the display\n");
#endif
        return SDL_FALSE;
    }
#ifdef X11MODES_DEBUG
    printf("XVidMode available at version %d.%d!\n", *major, *minor);
#endif
    return SDL_TRUE;
}

static
Bool XF86VidModeGetModeInfo(Display * dpy, int scr,
                                       XF86VidModeModeInfo* info)
{
    Bool retval;
    int dotclock;
    XF86VidModeModeLine l;
    SDL_zerop(info);
    SDL_zero(l);
    retval = X11_XF86VidModeGetModeLine(dpy, scr, &dotclock, &l);
    info->dotclock = dotclock;
    info->hdisplay = l.hdisplay;
    info->hsyncstart = l.hsyncstart;
    info->hsyncend = l.hsyncend;
    info->htotal = l.htotal;
    info->hskew = l.hskew;
    info->vdisplay = l.vdisplay;
    info->vsyncstart = l.vsyncstart;
    info->vsyncend = l.vsyncend;
    info->vtotal = l.vtotal;
    info->flags = l.flags;
    info->privsize = l.privsize;
    info->private = l.private;
    return retval;
}

static int
CalculateXVidModeRefreshRate(const XF86VidModeModeInfo * info)
{
    return (info->htotal
            && info->vtotal) ? (1000 * info->dotclock / (info->htotal *
                                                         info->vtotal)) : 0;
}

static SDL_bool
SetXVidModeModeInfo(const XF86VidModeModeInfo *info, SDL_DisplayMode *mode)
{
    mode->w = info->hdisplay;
    mode->h = info->vdisplay;
    mode->refresh_rate = CalculateXVidModeRefreshRate(info);
    ((SDL_DisplayModeData*)mode->driverdata)->vm_mode = *info;
    return SDL_TRUE;
}
#endif /* SDL_VIDEO_DRIVER_X11_XVIDMODE */

int
X11_InitModes(_THIS)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    int snum, screen, screencount = 0;
#if SDL_VIDEO_DRIVER_X11_XINERAMA
    int xinerama_major, xinerama_minor;
    int use_xinerama = 0;
    XineramaScreenInfo *xinerama = NULL;
#endif
#if SDL_VIDEO_DRIVER_X11_XRANDR
    int xrandr_major, xrandr_minor;
#endif
#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    int vm_major, vm_minor;
    int use_vidmode = 0;
#endif

/* XRandR is the One True Modern Way to do this on X11. If it's enabled and
   available, don't even look at other ways of doing things. */
#if SDL_VIDEO_DRIVER_X11_XRANDR
    /* require at least XRandR v1.3 */
    if (CheckXRandR(data->display, &xrandr_major, &xrandr_minor) &&
        (xrandr_major >= 2 || (xrandr_major == 1 && xrandr_minor >= 3))) {
        if (X11_InitModes_XRandR(_this) == 0)
            return 0;
    }
#endif /* SDL_VIDEO_DRIVER_X11_XRANDR */

/* !!! FIXME: eventually remove support for Xinerama and XVidMode (everything below here). */

    /* This is a workaround for some apps (UnrealEngine4, for example) until
       we sort out the ramifications of removing XVidMode support outright.
       This block should be removed with the XVidMode support. */
    {
        if (SDL_GetHintBoolean("SDL_VIDEO_X11_REQUIRE_XRANDR", SDL_FALSE)) {
            #if SDL_VIDEO_DRIVER_X11_XRANDR
            return SDL_SetError("XRandR support is required but not available");
            #else
            return SDL_SetError("XRandR support is required but not built into SDL!");
            #endif
        }
    }

#if SDL_VIDEO_DRIVER_X11_XINERAMA
    /* Query Xinerama extention
     * NOTE: This works with Nvidia Twinview correctly, but you need version 302.17 (released on June 2012)
     *       or newer of the Nvidia binary drivers
     */
    if (CheckXinerama(data->display, &xinerama_major, &xinerama_minor)) {
        int (*handler) (Display *, XErrorEvent *);
        X11_XSync(data->display, False);
        handler = X11_XSetErrorHandler(X11_XineramaFailed);
        xinerama = X11_XineramaQueryScreens(data->display, &screencount);
        X11_XSync(data->display, False);
        X11_XSetErrorHandler(handler);
        if (xinerama_triggered_error) {
            xinerama = 0;
        }
        if (xinerama) {
            use_xinerama = xinerama_major * 100 + xinerama_minor;
        }
    }
    if (!xinerama) {
        screencount = ScreenCount(data->display);
    }
#else
    screencount = ScreenCount(data->display);
#endif /* SDL_VIDEO_DRIVER_X11_XINERAMA */

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    if (CheckVidMode(data->display, &vm_major, &vm_minor)) {
        use_vidmode = vm_major * 100 + vm_minor;
    }
#endif /* SDL_VIDEO_DRIVER_X11_XVIDMODE */

    for (snum = 0; snum < screencount; ++snum) {
        XVisualInfo vinfo;
        SDL_VideoDisplay display;
        SDL_DisplayData *displaydata;
        SDL_DisplayMode mode;
        SDL_DisplayModeData *modedata;
        XPixmapFormatValues *pixmapFormats;
        char display_name[128];
        int i, n;

        /* Re-order screens to always put default screen first */
        if (snum == 0) {
            screen = DefaultScreen(data->display);
        } else if (snum == DefaultScreen(data->display)) {
            screen = 0;
        } else {
            screen = snum;
        }

#if SDL_VIDEO_DRIVER_X11_XINERAMA
        if (xinerama) {
            if (get_visualinfo(data->display, 0, &vinfo) < 0) {
                continue;
            }
        } else {
            if (get_visualinfo(data->display, screen, &vinfo) < 0) {
                continue;
            }
        }
#else
        if (get_visualinfo(data->display, screen, &vinfo) < 0) {
            continue;
        }
#endif

        displaydata = (SDL_DisplayData *) SDL_calloc(1, sizeof(*displaydata));
        if (!displaydata) {
            continue;
        }
        display_name[0] = '\0';

        mode.format = X11_GetPixelFormatFromVisualInfo(data->display, &vinfo);
        if (SDL_ISPIXELFORMAT_INDEXED(mode.format)) {
            /* We don't support palettized modes now */
            SDL_free(displaydata);
            continue;
        }
#if SDL_VIDEO_DRIVER_X11_XINERAMA
        if (xinerama) {
            mode.w = xinerama[screen].width;
            mode.h = xinerama[screen].height;
        } else {
            mode.w = DisplayWidth(data->display, screen);
            mode.h = DisplayHeight(data->display, screen);
        }
#else
        mode.w = DisplayWidth(data->display, screen);
        mode.h = DisplayHeight(data->display, screen);
#endif
        mode.refresh_rate = 0;

        modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
        if (!modedata) {
            SDL_free(displaydata);
            continue;
        }
        mode.driverdata = modedata;

#if SDL_VIDEO_DRIVER_X11_XINERAMA
        /* Most of SDL's calls to X11 are unwaware of Xinerama, and to X11 standard calls, when Xinerama is active,
         * there's only one screen available. So we force the screen number to zero and
         * let Xinerama specific code handle specific functionality using displaydata->xinerama_info
         */
        if (use_xinerama) {
            displaydata->screen = 0;
            displaydata->use_xinerama = use_xinerama;
            displaydata->xinerama_info = xinerama[screen];
            displaydata->xinerama_screen = screen;
        }
        else displaydata->screen = screen;
#else
        displaydata->screen = screen;
#endif
        displaydata->visual = vinfo.visual;
        displaydata->depth = vinfo.depth;

        /* We use the displaydata screen index here so that this works
           for both the Xinerama case, where we get the overall DPI,
           and the regular X11 screen info case. */
        displaydata->hdpi = (float)DisplayWidth(data->display, displaydata->screen) * 25.4f /
            DisplayWidthMM(data->display, displaydata->screen);
        displaydata->vdpi = (float)DisplayHeight(data->display, displaydata->screen) * 25.4f /
            DisplayHeightMM(data->display, displaydata->screen);
        displaydata->ddpi = SDL_ComputeDiagonalDPI(DisplayWidth(data->display, displaydata->screen),
                                                   DisplayHeight(data->display, displaydata->screen),
                                                   (float)DisplayWidthMM(data->display, displaydata->screen) / 25.4f,
                                                   (float)DisplayHeightMM(data->display, displaydata->screen) / 25.4f);

        displaydata->scanline_pad = SDL_BYTESPERPIXEL(mode.format) * 8;
        pixmapFormats = X11_XListPixmapFormats(data->display, &n);
        if (pixmapFormats) {
            for (i = 0; i < n; ++i) {
                if (pixmapFormats[i].depth == displaydata->depth) {
                    displaydata->scanline_pad = pixmapFormats[i].scanline_pad;
                    break;
                }
            }
            X11_XFree(pixmapFormats);
        }

#if SDL_VIDEO_DRIVER_X11_XINERAMA
        if (use_xinerama) {
            displaydata->x = xinerama[screen].x_org;
            displaydata->y = xinerama[screen].y_org;
        }
        else
#endif
        {
            displaydata->x = 0;
            displaydata->y = 0;
        }

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
        if (!displaydata->use_xrandr &&
#if SDL_VIDEO_DRIVER_X11_XINERAMA
            /* XVidMode only works on the screen at the origin */
            (!displaydata->use_xinerama ||
             (displaydata->x == 0 && displaydata->y == 0)) &&
#endif
            use_vidmode) {
            displaydata->use_vidmode = use_vidmode;
            if (displaydata->use_xinerama) {
                displaydata->vidmode_screen = 0;
            } else {
                displaydata->vidmode_screen = screen;
            }
            XF86VidModeGetModeInfo(data->display, displaydata->vidmode_screen, &modedata->vm_mode);
        }
#endif /* SDL_VIDEO_DRIVER_X11_XVIDMODE */

        SDL_zero(display);
        if (*display_name) {
            display.name = display_name;
        }
        display.desktop_mode = mode;
        display.current_mode = mode;
        display.driverdata = displaydata;
        SDL_AddVideoDisplay(&display);
    }

#if SDL_VIDEO_DRIVER_X11_XINERAMA
    if (xinerama) X11_XFree(xinerama);
#endif

    if (_this->num_displays == 0) {
        return SDL_SetError("No available displays");
    }
    return 0;
}

void
X11_GetDisplayModes(_THIS, SDL_VideoDisplay * sdl_display)
{
    Display *display = ((SDL_VideoData *) _this->driverdata)->display;
    SDL_DisplayData *data = (SDL_DisplayData *) sdl_display->driverdata;
#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    int nmodes;
    XF86VidModeModeInfo ** modes;
#endif
    SDL_DisplayMode mode;

    /* Unfortunately X11 requires the window to be created with the correct
     * visual and depth ahead of time, but the SDL API allows you to create
     * a window before setting the fullscreen display mode.  This means that
     * we have to use the same format for all windows and all display modes.
     * (or support recreating the window with a new visual behind the scenes)
     */
    mode.format = sdl_display->current_mode.format;
    mode.driverdata = NULL;

#if SDL_VIDEO_DRIVER_X11_XINERAMA
    if (data->use_xinerama) {
        int screen_w;
        int screen_h;

        screen_w = DisplayWidth(display, data->screen);
        screen_h = DisplayHeight(display, data->screen);

        if (data->use_vidmode && !data->xinerama_info.x_org && !data->xinerama_info.y_org &&
           (screen_w > data->xinerama_info.width || screen_h > data->xinerama_info.height)) {
            SDL_DisplayModeData *modedata;
            /* Add the full (both screens combined) xinerama mode only on the display that starts at 0,0
             * if we're using vidmode.
             */
            mode.w = screen_w;
            mode.h = screen_h;
            mode.refresh_rate = 0;
            modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
            if (modedata) {
                *modedata = *(SDL_DisplayModeData *)sdl_display->desktop_mode.driverdata;
            }
            mode.driverdata = modedata;
            if (!SDL_AddDisplayMode(sdl_display, &mode)) {
                SDL_free(modedata);
            }
        }
        else if (!data->use_xrandr)
        {
            SDL_DisplayModeData *modedata;
            /* Add the current mode of each monitor otherwise if we can't get them from xrandr */
            mode.w = data->xinerama_info.width;
            mode.h = data->xinerama_info.height;
            mode.refresh_rate = 0;
            modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
            if (modedata) {
                *modedata = *(SDL_DisplayModeData *)sdl_display->desktop_mode.driverdata;
            }
            mode.driverdata = modedata;
            if (!SDL_AddDisplayMode(sdl_display, &mode)) {
                SDL_free(modedata);
            }
        }

    }
#endif /* SDL_VIDEO_DRIVER_X11_XINERAMA */

#if SDL_VIDEO_DRIVER_X11_XRANDR
    if (data->use_xrandr) {
        XRRScreenResources *res;

        res = X11_XRRGetScreenResources (display, RootWindow(display, data->screen));
        if (res) {
            SDL_DisplayModeData *modedata;
            XRROutputInfo *output_info;
            int i;

            output_info = X11_XRRGetOutputInfo(display, res, data->xrandr_output);
            if (output_info && output_info->connection != RR_Disconnected) {
                for (i = 0; i < output_info->nmode; ++i) {
                    modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
                    if (!modedata) {
                        continue;
                    }
                    mode.driverdata = modedata;

                    if (!SetXRandRModeInfo(display, res, output_info->crtc, output_info->modes[i], &mode) ||
                        !SDL_AddDisplayMode(sdl_display, &mode)) {
                        SDL_free(modedata);
                    }
                }
            }
            X11_XRRFreeOutputInfo(output_info);
            X11_XRRFreeScreenResources(res);
        }
        return;
    }
#endif /* SDL_VIDEO_DRIVER_X11_XRANDR */

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    if (data->use_vidmode &&
        X11_XF86VidModeGetAllModeLines(display, data->vidmode_screen, &nmodes, &modes)) {
        int i;
        SDL_DisplayModeData *modedata;

#ifdef X11MODES_DEBUG
        printf("VidMode modes: (unsorted)\n");
        for (i = 0; i < nmodes; ++i) {
            printf("Mode %d: %d x %d @ %d, flags: 0x%x\n", i,
                   modes[i]->hdisplay, modes[i]->vdisplay,
                   CalculateXVidModeRefreshRate(modes[i]), modes[i]->flags);
        }
#endif
        for (i = 0; i < nmodes; ++i) {
            modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
            if (!modedata) {
                continue;
            }
            mode.driverdata = modedata;

            if (!SetXVidModeModeInfo(modes[i], &mode) || !SDL_AddDisplayMode(sdl_display, &mode)) {
                SDL_free(modedata);
            }
        }
        X11_XFree(modes);
        return;
    }
#endif /* SDL_VIDEO_DRIVER_X11_XVIDMODE */

    if (!data->use_xrandr && !data->use_vidmode) {
        SDL_DisplayModeData *modedata;
        /* Add the desktop mode */
        mode = sdl_display->desktop_mode;
        modedata = (SDL_DisplayModeData *) SDL_calloc(1, sizeof(SDL_DisplayModeData));
        if (modedata) {
            *modedata = *(SDL_DisplayModeData *)sdl_display->desktop_mode.driverdata;
        }
        mode.driverdata = modedata;
        if (!SDL_AddDisplayMode(sdl_display, &mode)) {
            SDL_free(modedata);
        }
    }
}

int
X11_SetDisplayMode(_THIS, SDL_VideoDisplay * sdl_display, SDL_DisplayMode * mode)
{
    SDL_VideoData *viddata = (SDL_VideoData *) _this->driverdata;
    Display *display = viddata->display;
    SDL_DisplayData *data = (SDL_DisplayData *) sdl_display->driverdata;
    SDL_DisplayModeData *modedata = (SDL_DisplayModeData *)mode->driverdata;

    viddata->last_mode_change_deadline = SDL_GetTicks() + (PENDING_FOCUS_TIME * 2);

#if SDL_VIDEO_DRIVER_X11_XRANDR
    if (data->use_xrandr) {
        XRRScreenResources *res;
        XRROutputInfo *output_info;
        XRRCrtcInfo *crtc;
        Status status;

        res = X11_XRRGetScreenResources (display, RootWindow(display, data->screen));
        if (!res) {
            return SDL_SetError("Couldn't get XRandR screen resources");
        }

        output_info = X11_XRRGetOutputInfo(display, res, data->xrandr_output);
        if (!output_info || output_info->connection == RR_Disconnected) {
            X11_XRRFreeScreenResources(res);
            return SDL_SetError("Couldn't get XRandR output info");
        }

        crtc = X11_XRRGetCrtcInfo(display, res, output_info->crtc);
        if (!crtc) {
            X11_XRRFreeOutputInfo(output_info);
            X11_XRRFreeScreenResources(res);
            return SDL_SetError("Couldn't get XRandR crtc info");
        }

        status = X11_XRRSetCrtcConfig (display, res, output_info->crtc, CurrentTime,
          crtc->x, crtc->y, modedata->xrandr_mode, crtc->rotation,
          &data->xrandr_output, 1);

        X11_XRRFreeCrtcInfo(crtc);
        X11_XRRFreeOutputInfo(output_info);
        X11_XRRFreeScreenResources(res);

        if (status != Success) {
            return SDL_SetError("X11_XRRSetCrtcConfig failed");
        }
    }
#endif /* SDL_VIDEO_DRIVER_X11_XRANDR */

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    if (data->use_vidmode) {
        X11_XF86VidModeSwitchToMode(display, data->vidmode_screen, &modedata->vm_mode);
    }
#endif /* SDL_VIDEO_DRIVER_X11_XVIDMODE */

    return 0;
}

void
X11_QuitModes(_THIS)
{
}

int
X11_GetDisplayBounds(_THIS, SDL_VideoDisplay * sdl_display, SDL_Rect * rect)
{
    SDL_DisplayData *data = (SDL_DisplayData *) sdl_display->driverdata;

    rect->x = data->x;
    rect->y = data->y;
    rect->w = sdl_display->current_mode.w;
    rect->h = sdl_display->current_mode.h;

#if SDL_VIDEO_DRIVER_X11_XINERAMA
    /* Get the real current bounds of the display */
    if (data->use_xinerama) {
        Display *display = ((SDL_VideoData *) _this->driverdata)->display;
        int screencount;
        XineramaScreenInfo *xinerama = X11_XineramaQueryScreens(display, &screencount);
        if (xinerama) {
            rect->x = xinerama[data->xinerama_screen].x_org;
            rect->y = xinerama[data->xinerama_screen].y_org;
            X11_XFree(xinerama);
        }
    }
#endif /* SDL_VIDEO_DRIVER_X11_XINERAMA */
    return 0;
}

int
X11_GetDisplayDPI(_THIS, SDL_VideoDisplay * sdl_display, float * ddpi, float * hdpi, float * vdpi)
{
    SDL_DisplayData *data = (SDL_DisplayData *) sdl_display->driverdata;

    if (ddpi) {
        *ddpi = data->ddpi;
    }
    if (hdpi) {
        *hdpi = data->hdpi;
    }
    if (vdpi) {
        *vdpi = data->vdpi;
    }

    return data->ddpi != 0.0f ? 0 : SDL_SetError("Couldn't get DPI");
}

int
X11_GetDisplayUsableBounds(_THIS, SDL_VideoDisplay * sdl_display, SDL_Rect * rect)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    Display *display = data->display;
    Atom _NET_WORKAREA;
    int status, real_format;
    int retval = -1;
    Atom real_type;
    unsigned long items_read = 0, items_left = 0;
    unsigned char *propdata = NULL;

    if (X11_GetDisplayBounds(_this, sdl_display, rect) < 0) {
        return -1;
    }

    _NET_WORKAREA = X11_XInternAtom(display, "_NET_WORKAREA", False);
    status = X11_XGetWindowProperty(display, DefaultRootWindow(display),
                                    _NET_WORKAREA, 0L, 4L, False, XA_CARDINAL,
                                    &real_type, &real_format, &items_read,
                                    &items_left, &propdata);
    if ((status == Success) && (items_read >= 4)) {
        const long *p = (long*) propdata;
        const SDL_Rect usable = { (int)p[0], (int)p[1], (int)p[2], (int)p[3] };
        retval = 0;
        if (!SDL_IntersectRect(rect, &usable, rect)) {
            SDL_zerop(rect);
        }
    }

    if (propdata) {
        X11_XFree(propdata);
    }

    return retval;
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vi: set ts=4 sw=4 expandtab: */
