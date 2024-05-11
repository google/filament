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

#include "SDL_assert.h"
#include "SDL_hints.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_keyboard_c.h"
#include "../../events/SDL_mouse_c.h"

#include "SDL_x11video.h"
#include "SDL_x11mouse.h"
#include "SDL_x11shape.h"
#include "SDL_x11xinput2.h"

#if SDL_VIDEO_OPENGL_EGL
#include "SDL_x11opengles.h"
#endif

#include "SDL_timer.h"
#include "SDL_syswm.h"
#include "SDL_log.h"

#define _NET_WM_STATE_REMOVE    0l
#define _NET_WM_STATE_ADD       1l

static Bool isMapNotify(Display *dpy, XEvent *ev, XPointer win)
{
    return ev->type == MapNotify && ev->xmap.window == *((Window*)win);
}
static Bool isUnmapNotify(Display *dpy, XEvent *ev, XPointer win)
{
    return ev->type == UnmapNotify && ev->xunmap.window == *((Window*)win);
}

/*
static Bool isConfigureNotify(Display *dpy, XEvent *ev, XPointer win)
{
    return ev->type == ConfigureNotify && ev->xconfigure.window == *((Window*)win);
}
static Bool
X11_XIfEventTimeout(Display *display, XEvent *event_return, Bool (*predicate)(), XPointer arg, int timeoutMS)
{
    Uint32 start = SDL_GetTicks();

    while (!X11_XCheckIfEvent(display, event_return, predicate, arg)) {
        if (SDL_TICKS_PASSED(SDL_GetTicks(), start + timeoutMS)) {
            return False;
        }
    }
    return True;
}
*/

static SDL_bool
X11_IsWindowLegacyFullscreen(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    return (data->fswindow != 0);
}

static SDL_bool
X11_IsWindowMapped(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    XWindowAttributes attr;

    X11_XGetWindowAttributes(videodata->display, data->xwindow, &attr);
    if (attr.map_state != IsUnmapped) {
        return SDL_TRUE;
    } else {
        return SDL_FALSE;
    }
}

#if 0
static SDL_bool
X11_IsActionAllowed(SDL_Window *window, Atom action)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Atom _NET_WM_ALLOWED_ACTIONS = data->videodata->_NET_WM_ALLOWED_ACTIONS;
    Atom type;
    Display *display = data->videodata->display;
    int form;
    unsigned long remain;
    unsigned long len, i;
    Atom *list;
    SDL_bool ret = SDL_FALSE;

    if (X11_XGetWindowProperty(display, data->xwindow, _NET_WM_ALLOWED_ACTIONS, 0, 1024, False, XA_ATOM, &type, &form, &len, &remain, (unsigned char **)&list) == Success)
    {
        for (i=0; i<len; ++i)
        {
            if (list[i] == action) {
                ret = SDL_TRUE;
                break;
            }
        }
        X11_XFree(list);
    }
    return ret;
}
#endif /* 0 */

void
X11_SetNetWMState(_THIS, Window xwindow, Uint32 flags)
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    Display *display = videodata->display;
    /* !!! FIXME: just dereference videodata below instead of copying to locals. */
    Atom _NET_WM_STATE = videodata->_NET_WM_STATE;
    /* Atom _NET_WM_STATE_HIDDEN = videodata->_NET_WM_STATE_HIDDEN; */
    Atom _NET_WM_STATE_FOCUSED = videodata->_NET_WM_STATE_FOCUSED;
    Atom _NET_WM_STATE_MAXIMIZED_VERT = videodata->_NET_WM_STATE_MAXIMIZED_VERT;
    Atom _NET_WM_STATE_MAXIMIZED_HORZ = videodata->_NET_WM_STATE_MAXIMIZED_HORZ;
    Atom _NET_WM_STATE_FULLSCREEN = videodata->_NET_WM_STATE_FULLSCREEN;
    Atom _NET_WM_STATE_ABOVE = videodata->_NET_WM_STATE_ABOVE;
    Atom _NET_WM_STATE_SKIP_TASKBAR = videodata->_NET_WM_STATE_SKIP_TASKBAR;
    Atom _NET_WM_STATE_SKIP_PAGER = videodata->_NET_WM_STATE_SKIP_PAGER;
    Atom atoms[16];
    int count = 0;

    /* The window manager sets this property, we shouldn't set it.
       If we did, this would indicate to the window manager that we don't
       actually want to be mapped during X11_XMapRaised(), which would be bad.
     *
    if (flags & SDL_WINDOW_HIDDEN) {
        atoms[count++] = _NET_WM_STATE_HIDDEN;
    }
    */

    if (flags & SDL_WINDOW_ALWAYS_ON_TOP) {
        atoms[count++] = _NET_WM_STATE_ABOVE;
    }
    if (flags & SDL_WINDOW_SKIP_TASKBAR) {
        atoms[count++] = _NET_WM_STATE_SKIP_TASKBAR;
        atoms[count++] = _NET_WM_STATE_SKIP_PAGER;
    }
    if (flags & SDL_WINDOW_INPUT_FOCUS) {
        atoms[count++] = _NET_WM_STATE_FOCUSED;
    }
    if (flags & SDL_WINDOW_MAXIMIZED) {
        atoms[count++] = _NET_WM_STATE_MAXIMIZED_VERT;
        atoms[count++] = _NET_WM_STATE_MAXIMIZED_HORZ;
    }
    if (flags & SDL_WINDOW_FULLSCREEN) {
        atoms[count++] = _NET_WM_STATE_FULLSCREEN;
    }

    SDL_assert(count <= SDL_arraysize(atoms));

    if (count > 0) {
        X11_XChangeProperty(display, xwindow, _NET_WM_STATE, XA_ATOM, 32,
                        PropModeReplace, (unsigned char *)atoms, count);
    } else {
        X11_XDeleteProperty(display, xwindow, _NET_WM_STATE);
    }
}

Uint32
X11_GetNetWMState(_THIS, Window xwindow)
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    Display *display = videodata->display;
    Atom _NET_WM_STATE = videodata->_NET_WM_STATE;
    Atom _NET_WM_STATE_HIDDEN = videodata->_NET_WM_STATE_HIDDEN;
    Atom _NET_WM_STATE_FOCUSED = videodata->_NET_WM_STATE_FOCUSED;
    Atom _NET_WM_STATE_MAXIMIZED_VERT = videodata->_NET_WM_STATE_MAXIMIZED_VERT;
    Atom _NET_WM_STATE_MAXIMIZED_HORZ = videodata->_NET_WM_STATE_MAXIMIZED_HORZ;
    Atom _NET_WM_STATE_FULLSCREEN = videodata->_NET_WM_STATE_FULLSCREEN;
    Atom actualType;
    int actualFormat;
    unsigned long i, numItems, bytesAfter;
    unsigned char *propertyValue = NULL;
    long maxLength = 1024;
    Uint32 flags = 0;

    if (X11_XGetWindowProperty(display, xwindow, _NET_WM_STATE,
                           0l, maxLength, False, XA_ATOM, &actualType,
                           &actualFormat, &numItems, &bytesAfter,
                           &propertyValue) == Success) {
        Atom *atoms = (Atom *) propertyValue;
        int maximized = 0;
        int fullscreen = 0;

        for (i = 0; i < numItems; ++i) {
            if (atoms[i] == _NET_WM_STATE_HIDDEN) {
                flags |= SDL_WINDOW_HIDDEN;
            } else if (atoms[i] == _NET_WM_STATE_FOCUSED) {
                flags |= SDL_WINDOW_INPUT_FOCUS;
            } else if (atoms[i] == _NET_WM_STATE_MAXIMIZED_VERT) {
                maximized |= 1;
            } else if (atoms[i] == _NET_WM_STATE_MAXIMIZED_HORZ) {
                maximized |= 2;
            } else if ( atoms[i] == _NET_WM_STATE_FULLSCREEN) {
                fullscreen = 1;
            }
        }
        if (maximized == 3) {
            flags |= SDL_WINDOW_MAXIMIZED;
        }

        if (fullscreen == 1) {
            flags |= SDL_WINDOW_FULLSCREEN;
        }

        /* If the window is unmapped, numItems will be zero and _NET_WM_STATE_HIDDEN
         * will not be set. Do an additional check to see if the window is unmapped
         * and mark it as SDL_WINDOW_HIDDEN if it is.
         */
        {
            XWindowAttributes attr;
            SDL_memset(&attr,0,sizeof(attr));
            X11_XGetWindowAttributes(videodata->display, xwindow, &attr);
            if (attr.map_state == IsUnmapped) {
                flags |= SDL_WINDOW_HIDDEN;
            }
        }
        X11_XFree(propertyValue);
    }

    /* FIXME, check the size hints for resizable */
    /* flags |= SDL_WINDOW_RESIZABLE; */

    return flags;
}

static int
SetupWindowData(_THIS, SDL_Window * window, Window w, BOOL created)
{
    SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
    SDL_WindowData *data;
    int numwindows = videodata->numwindows;
    int windowlistlength = videodata->windowlistlength;
    SDL_WindowData **windowlist = videodata->windowlist;

    /* Allocate the window data */
    data = (SDL_WindowData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        return SDL_OutOfMemory();
    }
    data->window = window;
    data->xwindow = w;
#ifdef X_HAVE_UTF8_STRING
    if (SDL_X11_HAVE_UTF8 && videodata->im) {
        data->ic =
            X11_XCreateIC(videodata->im, XNClientWindow, w, XNFocusWindow, w,
                       XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                       NULL);
    }
#endif
    data->created = created;
    data->videodata = videodata;

    /* Associate the data with the window */

    if (numwindows < windowlistlength) {
        windowlist[numwindows] = data;
        videodata->numwindows++;
    } else {
        windowlist =
            (SDL_WindowData **) SDL_realloc(windowlist,
                                            (numwindows +
                                             1) * sizeof(*windowlist));
        if (!windowlist) {
            SDL_free(data);
            return SDL_OutOfMemory();
        }
        windowlist[numwindows] = data;
        videodata->numwindows++;
        videodata->windowlistlength++;
        videodata->windowlist = windowlist;
    }

    /* Fill in the SDL window with the window data */
    {
        XWindowAttributes attrib;

        X11_XGetWindowAttributes(data->videodata->display, w, &attrib);
        window->x = attrib.x;
        window->y = attrib.y;
        window->w = attrib.width;
        window->h = attrib.height;
        if (attrib.map_state != IsUnmapped) {
            window->flags |= SDL_WINDOW_SHOWN;
        } else {
            window->flags &= ~SDL_WINDOW_SHOWN;
        }
        data->visual = attrib.visual;
        data->colormap = attrib.colormap;
    }

    window->flags |= X11_GetNetWMState(_this, w);

    {
        Window FocalWindow;
        int RevertTo=0;
        X11_XGetInputFocus(data->videodata->display, &FocalWindow, &RevertTo);
        if (FocalWindow==w)
        {
            window->flags |= SDL_WINDOW_INPUT_FOCUS;
        }

        if (window->flags & SDL_WINDOW_INPUT_FOCUS) {
            SDL_SetKeyboardFocus(data->window);
        }

        if (window->flags & SDL_WINDOW_INPUT_GRABBED) {
            /* Tell x11 to clip mouse */
        }
    }

    /* All done! */
    window->driverdata = data;
    return 0;
}

static void
SetWindowBordered(Display *display, int screen, Window window, SDL_bool border)
{
    /*
     * this code used to check for KWM_WIN_DECORATION, but KDE hasn't
     *  supported it for years and years. It now respects _MOTIF_WM_HINTS.
     *  Gnome is similar: just use the Motif atom.
     */

    Atom WM_HINTS = X11_XInternAtom(display, "_MOTIF_WM_HINTS", True);
    if (WM_HINTS != None) {
        /* Hints used by Motif compliant window managers */
        struct
        {
            unsigned long flags;
            unsigned long functions;
            unsigned long decorations;
            long input_mode;
            unsigned long status;
        } MWMHints = {
            (1L << 1), 0, border ? 1 : 0, 0, 0
        };

        X11_XChangeProperty(display, window, WM_HINTS, WM_HINTS, 32,
                        PropModeReplace, (unsigned char *) &MWMHints,
                        sizeof(MWMHints) / sizeof(long));
    } else {  /* set the transient hints instead, if necessary */
        X11_XSetTransientForHint(display, window, RootWindow(display, screen));
    }
}

int
X11_CreateWindow(_THIS, SDL_Window * window)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    SDL_DisplayData *displaydata =
        (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    SDL_WindowData *windowdata;
    Display *display = data->display;
    int screen = displaydata->screen;
    Visual *visual;
    int depth;
    XSetWindowAttributes xattr;
    Window w;
    XSizeHints *sizehints;
    XWMHints *wmhints;
    XClassHint *classhints;
    Atom _NET_WM_BYPASS_COMPOSITOR;
    Atom _NET_WM_WINDOW_TYPE;
    Atom wintype;
    const char *wintype_name = NULL;
    long compositor = 1;
    Atom _NET_WM_PID;
    Atom XdndAware, xdnd_version = 5;
    long fevent = 0;

#if SDL_VIDEO_OPENGL_GLX || SDL_VIDEO_OPENGL_EGL
    if ((window->flags & SDL_WINDOW_OPENGL) &&
        !SDL_getenv("SDL_VIDEO_X11_VISUALID")) {
        XVisualInfo *vinfo = NULL;

#if SDL_VIDEO_OPENGL_EGL
        if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES 
#if SDL_VIDEO_OPENGL_GLX            
            && ( !_this->gl_data || X11_GL_UseEGL(_this) )
#endif
        ) {
            vinfo = X11_GLES_GetVisual(_this, display, screen);
        } else
#endif
        {
#if SDL_VIDEO_OPENGL_GLX
            vinfo = X11_GL_GetVisual(_this, display, screen);
#endif
        }

        if (!vinfo) {
            return -1;
        }
        visual = vinfo->visual;
        depth = vinfo->depth;
        X11_XFree(vinfo);
    } else
#endif
    {
        visual = displaydata->visual;
        depth = displaydata->depth;
    }

    xattr.override_redirect = ((window->flags & SDL_WINDOW_TOOLTIP) || (window->flags & SDL_WINDOW_POPUP_MENU)) ? True : False;
    xattr.background_pixmap = None;
    xattr.border_pixel = 0;

    if (visual->class == DirectColor) {
        XColor *colorcells;
        int i;
        int ncolors;
        int rmax, gmax, bmax;
        int rmask, gmask, bmask;
        int rshift, gshift, bshift;

        xattr.colormap =
            X11_XCreateColormap(display, RootWindow(display, screen),
                            visual, AllocAll);

        /* If we can't create a colormap, then we must die */
        if (!xattr.colormap) {
            return SDL_SetError("Could not create writable colormap");
        }

        /* OK, we got a colormap, now fill it in as best as we can */
        colorcells = SDL_malloc(visual->map_entries * sizeof(XColor));
        if (!colorcells) {
            return SDL_OutOfMemory();
        }
        ncolors = visual->map_entries;
        rmax = 0xffff;
        gmax = 0xffff;
        bmax = 0xffff;

        rshift = 0;
        rmask = visual->red_mask;
        while (0 == (rmask & 1)) {
            rshift++;
            rmask >>= 1;
        }

        gshift = 0;
        gmask = visual->green_mask;
        while (0 == (gmask & 1)) {
            gshift++;
            gmask >>= 1;
        }

        bshift = 0;
        bmask = visual->blue_mask;
        while (0 == (bmask & 1)) {
            bshift++;
            bmask >>= 1;
        }

        /* build the color table pixel values */
        for (i = 0; i < ncolors; i++) {
            Uint32 red = (rmax * i) / (ncolors - 1);
            Uint32 green = (gmax * i) / (ncolors - 1);
            Uint32 blue = (bmax * i) / (ncolors - 1);

            Uint32 rbits = (rmask * i) / (ncolors - 1);
            Uint32 gbits = (gmask * i) / (ncolors - 1);
            Uint32 bbits = (bmask * i) / (ncolors - 1);

            Uint32 pix =
                (rbits << rshift) | (gbits << gshift) | (bbits << bshift);

            colorcells[i].pixel = pix;

            colorcells[i].red = red;
            colorcells[i].green = green;
            colorcells[i].blue = blue;

            colorcells[i].flags = DoRed | DoGreen | DoBlue;
        }

        X11_XStoreColors(display, xattr.colormap, colorcells, ncolors);

        SDL_free(colorcells);
    } else {
        xattr.colormap =
            X11_XCreateColormap(display, RootWindow(display, screen),
                            visual, AllocNone);
    }

    w = X11_XCreateWindow(display, RootWindow(display, screen),
                      window->x, window->y, window->w, window->h,
                      0, depth, InputOutput, visual,
                      (CWOverrideRedirect | CWBackPixmap | CWBorderPixel |
                       CWColormap), &xattr);
    if (!w) {
        return SDL_SetError("Couldn't create window");
    }

    SetWindowBordered(display, screen, w,
                      (window->flags & SDL_WINDOW_BORDERLESS) == 0);

    sizehints = X11_XAllocSizeHints();
    /* Setup the normal size hints */
    sizehints->flags = 0;
    if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
        sizehints->min_width = sizehints->max_width = window->w;
        sizehints->min_height = sizehints->max_height = window->h;
        sizehints->flags |= (PMaxSize | PMinSize);
    }
    sizehints->x = window->x;
    sizehints->y = window->y;
    sizehints->flags |= USPosition;

    /* Setup the input hints so we get keyboard input */
    wmhints = X11_XAllocWMHints();
    wmhints->input = True;
    wmhints->window_group = data->window_group;
    wmhints->flags = InputHint | WindowGroupHint;

    /* Setup the class hints so we can get an icon (AfterStep) */
    classhints = X11_XAllocClassHint();
    classhints->res_name = data->classname;
    classhints->res_class = data->classname;

    /* Set the size, input and class hints, and define WM_CLIENT_MACHINE and WM_LOCALE_NAME */
    X11_XSetWMProperties(display, w, NULL, NULL, NULL, 0, sizehints, wmhints, classhints);

    X11_XFree(sizehints);
    X11_XFree(wmhints);
    X11_XFree(classhints);
    /* Set the PID related to the window for the given hostname, if possible */
    if (data->pid > 0) {
        long pid = (long) data->pid;
        _NET_WM_PID = X11_XInternAtom(display, "_NET_WM_PID", False);
        X11_XChangeProperty(display, w, _NET_WM_PID, XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *) &pid, 1);
    }

    /* Set the window manager state */
    X11_SetNetWMState(_this, w, window->flags);

    compositor = 2;  /* don't disable compositing except for "normal" windows */

    if (window->flags & SDL_WINDOW_UTILITY) {
        wintype_name = "_NET_WM_WINDOW_TYPE_UTILITY";
    } else if (window->flags & SDL_WINDOW_TOOLTIP) {
        wintype_name = "_NET_WM_WINDOW_TYPE_TOOLTIP";
    } else if (window->flags & SDL_WINDOW_POPUP_MENU) {
        wintype_name = "_NET_WM_WINDOW_TYPE_POPUP_MENU";
    } else {
        wintype_name = "_NET_WM_WINDOW_TYPE_NORMAL";
        compositor = 1;  /* disable compositing for "normal" windows */
    }

    /* Let the window manager know what type of window we are. */
    _NET_WM_WINDOW_TYPE = X11_XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    wintype = X11_XInternAtom(display, wintype_name, False);
    X11_XChangeProperty(display, w, _NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&wintype, 1);
    if (SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, SDL_TRUE)) {
        _NET_WM_BYPASS_COMPOSITOR = X11_XInternAtom(display, "_NET_WM_BYPASS_COMPOSITOR", False);
        X11_XChangeProperty(display, w, _NET_WM_BYPASS_COMPOSITOR, XA_CARDINAL, 32,
                        PropModeReplace,
                        (unsigned char *)&compositor, 1);
    }

    {
        Atom protocols[3];
        int proto_count = 0;

        protocols[proto_count++] = data->WM_DELETE_WINDOW; /* Allow window to be deleted by the WM */
        protocols[proto_count++] = data->WM_TAKE_FOCUS; /* Since we will want to set input focus explicitly */

        /* Default to using ping if there is no hint */
        if (SDL_GetHintBoolean(SDL_HINT_VIDEO_X11_NET_WM_PING, SDL_TRUE)) {
            protocols[proto_count++] = data->_NET_WM_PING; /* Respond so WM knows we're alive */
        }

        SDL_assert(proto_count <= sizeof(protocols) / sizeof(protocols[0]));

        X11_XSetWMProtocols(display, w, protocols, proto_count);
    }

    if (SetupWindowData(_this, window, w, SDL_TRUE) < 0) {
        X11_XDestroyWindow(display, w);
        return -1;
    }
    windowdata = (SDL_WindowData *) window->driverdata;

#if SDL_VIDEO_OPENGL_ES || SDL_VIDEO_OPENGL_ES2
    if ((window->flags & SDL_WINDOW_OPENGL) && 
        _this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES
#if SDL_VIDEO_OPENGL_GLX            
        && ( !_this->gl_data || X11_GL_UseEGL(_this) )
#endif  
    ) {
#if SDL_VIDEO_OPENGL_EGL  
        if (!_this->egl_data) {
            X11_XDestroyWindow(display, w);
            return -1;
        }

        /* Create the GLES window surface */
        windowdata->egl_surface = SDL_EGL_CreateSurface(_this, (NativeWindowType) w);

        if (windowdata->egl_surface == EGL_NO_SURFACE) {
            X11_XDestroyWindow(display, w);
            return SDL_SetError("Could not create GLES window surface");
        }
#else
        return SDL_SetError("Could not create GLES window surface (EGL support not configured)");
#endif /* SDL_VIDEO_OPENGL_EGL */
    }
#endif
    

#ifdef X_HAVE_UTF8_STRING
    if (SDL_X11_HAVE_UTF8 && windowdata->ic) {
        X11_XGetICValues(windowdata->ic, XNFilterEvents, &fevent, NULL);
    }
#endif

    X11_Xinput2SelectTouch(_this, window);

    X11_XSelectInput(display, w,
                 (FocusChangeMask | EnterWindowMask | LeaveWindowMask |
                 ExposureMask | ButtonPressMask | ButtonReleaseMask |
                 PointerMotionMask | KeyPressMask | KeyReleaseMask |
                 PropertyChangeMask | StructureNotifyMask |
                 KeymapStateMask | fevent));

    XdndAware = X11_XInternAtom(display, "XdndAware", False);
    X11_XChangeProperty(display, w, XdndAware, XA_ATOM, 32,
                 PropModeReplace,
                 (unsigned char*)&xdnd_version, 1);

    X11_XFlush(display);

    return 0;
}

int
X11_CreateWindowFrom(_THIS, SDL_Window * window, const void *data)
{
    Window w = (Window) data;

    window->title = X11_GetWindowTitle(_this, w);

    if (SetupWindowData(_this, window, w, SDL_FALSE) < 0) {
        return -1;
    }
    return 0;
}

char *
X11_GetWindowTitle(_THIS, Window xwindow)
{
    SDL_VideoData *data = (SDL_VideoData *) _this->driverdata;
    Display *display = data->display;
    int status, real_format;
    Atom real_type;
    unsigned long items_read, items_left;
    unsigned char *propdata;
    char *title = NULL;

    status = X11_XGetWindowProperty(display, xwindow, data->_NET_WM_NAME,
                0L, 8192L, False, data->UTF8_STRING, &real_type, &real_format,
                &items_read, &items_left, &propdata);
    if (status == Success && propdata) {
        title = SDL_strdup(SDL_static_cast(char*, propdata));
        X11_XFree(propdata);
    } else {
        status = X11_XGetWindowProperty(display, xwindow, XA_WM_NAME,
                    0L, 8192L, False, XA_STRING, &real_type, &real_format,
                    &items_read, &items_left, &propdata);
        if (status == Success && propdata) {
            title = SDL_iconv_string("UTF-8", "", SDL_static_cast(char*, propdata), items_read+1);
            X11_XFree(propdata);
        } else {
            title = SDL_strdup("");
        }
    }
    return title;
}

void
X11_SetWindowTitle(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    XTextProperty titleprop;
    Status status;
    const char *title = window->title ? window->title : "";
    char *title_locale = NULL;

#ifdef X_HAVE_UTF8_STRING
    Atom _NET_WM_NAME = data->videodata->_NET_WM_NAME;
#endif

    title_locale = SDL_iconv_utf8_locale(title);
    if (!title_locale) {
        SDL_OutOfMemory();
        return;
    }

    status = X11_XStringListToTextProperty(&title_locale, 1, &titleprop);
    SDL_free(title_locale);
    if (status) {
        X11_XSetTextProperty(display, data->xwindow, &titleprop, XA_WM_NAME);
        X11_XFree(titleprop.value);
    }
#ifdef X_HAVE_UTF8_STRING
    if (SDL_X11_HAVE_UTF8) {
        status = X11_Xutf8TextListToTextProperty(display, (char **) &title, 1,
                                            XUTF8StringStyle, &titleprop);
        if (status == Success) {
            X11_XSetTextProperty(display, data->xwindow, &titleprop,
                                 _NET_WM_NAME);
            X11_XFree(titleprop.value);
        }
    }
#endif

    X11_XFlush(display);
}

void
X11_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    Atom _NET_WM_ICON = data->videodata->_NET_WM_ICON;

    if (icon) {
        int propsize;
        long *propdata;

        /* Set the _NET_WM_ICON property */
        SDL_assert(icon->format->format == SDL_PIXELFORMAT_ARGB8888);
        propsize = 2 + (icon->w * icon->h);
        propdata = SDL_malloc(propsize * sizeof(long));
        if (propdata) {
            int x, y;
            Uint32 *src;
            long *dst;

            propdata[0] = icon->w;
            propdata[1] = icon->h;
            dst = &propdata[2];
            for (y = 0; y < icon->h; ++y) {
                src = (Uint32*)((Uint8*)icon->pixels + y * icon->pitch);
                for (x = 0; x < icon->w; ++x) {
                    *dst++ = *src++;
                }
            }
            X11_XChangeProperty(display, data->xwindow, _NET_WM_ICON, XA_CARDINAL,
                            32, PropModeReplace, (unsigned char *) propdata,
                            propsize);
        }
        SDL_free(propdata);
    } else {
        X11_XDeleteProperty(display, data->xwindow, _NET_WM_ICON);
    }
    X11_XFlush(display);
}

void
X11_SetWindowPosition(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    X11_XMoveWindow(display, data->xwindow, window->x - data->border_left, window->y - data->border_top);
    X11_XFlush(display);
}

void
X11_SetWindowMinimumSize(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    if (window->flags & SDL_WINDOW_RESIZABLE) {
         XSizeHints *sizehints = X11_XAllocSizeHints();
         long userhints;

         X11_XGetWMNormalHints(display, data->xwindow, sizehints, &userhints);

         sizehints->min_width = window->min_w;
         sizehints->min_height = window->min_h;
         sizehints->flags |= PMinSize;

         X11_XSetWMNormalHints(display, data->xwindow, sizehints);

         X11_XFree(sizehints);

        /* See comment in X11_SetWindowSize. */
        X11_XResizeWindow(display, data->xwindow, window->w, window->h);
        X11_XMoveWindow(display, data->xwindow, window->x - data->border_left, window->y - data->border_top);
        X11_XRaiseWindow(display, data->xwindow);
    }

    X11_XFlush(display);
}

void
X11_SetWindowMaximumSize(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    if (window->flags & SDL_WINDOW_RESIZABLE) {
         XSizeHints *sizehints = X11_XAllocSizeHints();
         long userhints;

         X11_XGetWMNormalHints(display, data->xwindow, sizehints, &userhints);

         sizehints->max_width = window->max_w;
         sizehints->max_height = window->max_h;
         sizehints->flags |= PMaxSize;

         X11_XSetWMNormalHints(display, data->xwindow, sizehints);

         X11_XFree(sizehints);

        /* See comment in X11_SetWindowSize. */
        X11_XResizeWindow(display, data->xwindow, window->w, window->h);
        X11_XMoveWindow(display, data->xwindow, window->x - data->border_left, window->y - data->border_top);
        X11_XRaiseWindow(display, data->xwindow);
    }

    X11_XFlush(display);
}

void
X11_SetWindowSize(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    if (SDL_IsShapedWindow(window)) {
        X11_ResizeWindowShape(window);
    }
    if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
         /* Apparently, if the X11 Window is set to a 'non-resizable' window, you cannot resize it using the X11_XResizeWindow, thus
            we must set the size hints to adjust the window size. */
         XSizeHints *sizehints = X11_XAllocSizeHints();
         long userhints;

         X11_XGetWMNormalHints(display, data->xwindow, sizehints, &userhints);

         sizehints->min_width = sizehints->max_width = window->w;
         sizehints->min_height = sizehints->max_height = window->h;
         sizehints->flags |= PMinSize | PMaxSize;

         X11_XSetWMNormalHints(display, data->xwindow, sizehints);

         X11_XFree(sizehints);

        /* From Pierre-Loup:
           WMs each have their little quirks with that.  When you change the
           size hints, they get a ConfigureNotify event with the
           WM_NORMAL_SIZE_HINTS Atom.  They all save the hints then, but they
           don't all resize the window right away to enforce the new hints.

           Some of them resize only after:
            - A user-initiated move or resize
            - A code-initiated move or resize
            - Hiding & showing window (Unmap & map)

           The following move & resize seems to help a lot of WMs that didn't
           properly update after the hints were changed. We don't do a
           hide/show, because there are supposedly subtle problems with doing so
           and transitioning from windowed to fullscreen in Unity.
         */
        X11_XResizeWindow(display, data->xwindow, window->w, window->h);
        X11_XMoveWindow(display, data->xwindow, window->x - data->border_left, window->y - data->border_top);
        X11_XRaiseWindow(display, data->xwindow);
    } else {
        X11_XResizeWindow(display, data->xwindow, window->w, window->h);
    }

    X11_XFlush(display);
}

int
X11_GetWindowBordersSize(_THIS, SDL_Window * window, int *top, int *left, int *bottom, int *right)
{
    SDL_WindowData *data = (SDL_WindowData *)window->driverdata;

    *left = data->border_left;
    *right = data->border_right;
    *top = data->border_top;
    *bottom = data->border_bottom;

    return 0;
}

int
X11_SetWindowOpacity(_THIS, SDL_Window * window, float opacity)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    Atom _NET_WM_WINDOW_OPACITY = data->videodata->_NET_WM_WINDOW_OPACITY;

    if (opacity == 1.0f) {
        X11_XDeleteProperty(display, data->xwindow, _NET_WM_WINDOW_OPACITY);
    } else  {
        const Uint32 FullyOpaque = 0xFFFFFFFF;
        const long alpha = (long) ((double)opacity * (double)FullyOpaque);
        X11_XChangeProperty(display, data->xwindow, _NET_WM_WINDOW_OPACITY, XA_CARDINAL, 32,
            PropModeReplace, (unsigned char *)&alpha, 1);
    }

    return 0;
}

int 
X11_SetWindowModalFor(_THIS, SDL_Window * modal_window, SDL_Window * parent_window) {
    SDL_WindowData *data = (SDL_WindowData *) modal_window->driverdata;
    SDL_WindowData *parent_data = (SDL_WindowData *) parent_window->driverdata;
    Display *display = data->videodata->display;

    X11_XSetTransientForHint(display, data->xwindow, parent_data->xwindow);
    return 0;
}

int
X11_SetWindowInputFocus(_THIS, SDL_Window * window) 
{
    if (X11_IsWindowMapped(_this, window)) {
        SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
        Display *display = data->videodata->display;
        X11_XSetInputFocus(display, data->xwindow, RevertToNone, CurrentTime);
        X11_XFlush(display);
        return 0;
    }
    return -1;
}

void
X11_SetWindowBordered(_THIS, SDL_Window * window, SDL_bool bordered)
{
    const SDL_bool focused = ((window->flags & SDL_WINDOW_INPUT_FOCUS) != 0);
    const SDL_bool visible = ((window->flags & SDL_WINDOW_HIDDEN) == 0);
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata =
        (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    Display *display = data->videodata->display;
    XEvent event;

    SetWindowBordered(display, displaydata->screen, data->xwindow, bordered);
    X11_XFlush(display);

    if (visible) {
        XWindowAttributes attr;
        do {
            X11_XSync(display, False);
            X11_XGetWindowAttributes(display, data->xwindow, &attr);
        } while (attr.map_state != IsViewable);

        if (focused) {
            X11_XSetInputFocus(display, data->xwindow, RevertToParent, CurrentTime);
        }
    }

    /* make sure these don't make it to the real event queue if they fired here. */
    X11_XSync(display, False);
    X11_XCheckIfEvent(display, &event, &isUnmapNotify, (XPointer)&data->xwindow);
    X11_XCheckIfEvent(display, &event, &isMapNotify, (XPointer)&data->xwindow);
}

void
X11_SetWindowResizable(_THIS, SDL_Window * window, SDL_bool resizable)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    XSizeHints *sizehints = X11_XAllocSizeHints();
    long userhints;

    X11_XGetWMNormalHints(display, data->xwindow, sizehints, &userhints);

    if (resizable) {
        /* FIXME: Is there a better way to get max window size from X? -flibit */
        const int maxsize = 0x7FFFFFFF;
        sizehints->min_width = window->min_w;
        sizehints->min_height = window->min_h;
        sizehints->max_width = (window->max_w == 0) ? maxsize : window->max_w;
        sizehints->max_height = (window->max_h == 0) ? maxsize : window->max_h;
    } else {
        sizehints->min_width = window->w;
        sizehints->min_height = window->h;
        sizehints->max_width = window->w;
        sizehints->max_height = window->h;
    }
    sizehints->flags |= PMinSize | PMaxSize;

    X11_XSetWMNormalHints(display, data->xwindow, sizehints);

    X11_XFree(sizehints);

    /* See comment in X11_SetWindowSize. */
    X11_XResizeWindow(display, data->xwindow, window->w, window->h);
    X11_XMoveWindow(display, data->xwindow, window->x - data->border_left, window->y - data->border_top);
    X11_XRaiseWindow(display, data->xwindow);

    X11_XFlush(display);
}

void
X11_ShowWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    XEvent event;

    if (!X11_IsWindowMapped(_this, window)) {
        X11_XMapRaised(display, data->xwindow);
        /* Blocking wait for "MapNotify" event.
         * We use X11_XIfEvent because pXWindowEvent takes a mask rather than a type,
         * and XCheckTypedWindowEvent doesn't block */
        if(!(window->flags & SDL_WINDOW_FOREIGN))
            X11_XIfEvent(display, &event, &isMapNotify, (XPointer)&data->xwindow);
        X11_XFlush(display);
    }

    if (!data->videodata->net_wm) {
        /* no WM means no FocusIn event, which confuses us. Force it. */
        X11_XSetInputFocus(display, data->xwindow, RevertToNone, CurrentTime);
        X11_XFlush(display);
    }
}

void
X11_HideWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata = (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    Display *display = data->videodata->display;
    XEvent event;

    if (X11_IsWindowMapped(_this, window)) {
        X11_XWithdrawWindow(display, data->xwindow, displaydata->screen);
        /* Blocking wait for "UnmapNotify" event */
        if(!(window->flags & SDL_WINDOW_FOREIGN))
            X11_XIfEvent(display, &event, &isUnmapNotify, (XPointer)&data->xwindow);
        X11_XFlush(display);
    }
}

static void
SetWindowActive(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata =
        (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    Display *display = data->videodata->display;
    Atom _NET_ACTIVE_WINDOW = data->videodata->_NET_ACTIVE_WINDOW;

    if (X11_IsWindowMapped(_this, window)) {
        XEvent e;

        /*printf("SDL Window %p: sending _NET_ACTIVE_WINDOW with timestamp %lu\n", window, data->user_time);*/

        SDL_zero(e);
        e.xany.type = ClientMessage;
        e.xclient.message_type = _NET_ACTIVE_WINDOW;
        e.xclient.format = 32;
        e.xclient.window = data->xwindow;
        e.xclient.data.l[0] = 1;  /* source indication. 1 = application */
        e.xclient.data.l[1] = data->user_time;
        e.xclient.data.l[2] = 0;

        X11_XSendEvent(display, RootWindow(display, displaydata->screen), 0,
                   SubstructureNotifyMask | SubstructureRedirectMask, &e);

        X11_XFlush(display);
    }
}

void
X11_RaiseWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    X11_XRaiseWindow(display, data->xwindow);
    SetWindowActive(_this, window);
    X11_XFlush(display);
}

static void
SetWindowMaximized(_THIS, SDL_Window * window, SDL_bool maximized)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata =
        (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    Display *display = data->videodata->display;
    Atom _NET_WM_STATE = data->videodata->_NET_WM_STATE;
    Atom _NET_WM_STATE_MAXIMIZED_VERT = data->videodata->_NET_WM_STATE_MAXIMIZED_VERT;
    Atom _NET_WM_STATE_MAXIMIZED_HORZ = data->videodata->_NET_WM_STATE_MAXIMIZED_HORZ;

    if (maximized) {
        window->flags |= SDL_WINDOW_MAXIMIZED;
    } else {
        window->flags &= ~SDL_WINDOW_MAXIMIZED;
    }

    if (X11_IsWindowMapped(_this, window)) {
        XEvent e;

        SDL_zero(e);
        e.xany.type = ClientMessage;
        e.xclient.message_type = _NET_WM_STATE;
        e.xclient.format = 32;
        e.xclient.window = data->xwindow;
        e.xclient.data.l[0] =
            maximized ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        e.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
        e.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
        e.xclient.data.l[3] = 0l;

        X11_XSendEvent(display, RootWindow(display, displaydata->screen), 0,
                   SubstructureNotifyMask | SubstructureRedirectMask, &e);
    } else {
        X11_SetNetWMState(_this, data->xwindow, window->flags);
    }
    X11_XFlush(display);
}

void
X11_MaximizeWindow(_THIS, SDL_Window * window)
{
    SetWindowMaximized(_this, window, SDL_TRUE);
}

void
X11_MinimizeWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata =
        (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    Display *display = data->videodata->display;

    X11_XIconifyWindow(display, data->xwindow, displaydata->screen);
    X11_XFlush(display);
}

void
X11_RestoreWindow(_THIS, SDL_Window * window)
{
    SetWindowMaximized(_this, window, SDL_FALSE);
    X11_ShowWindow(_this, window);
    SetWindowActive(_this, window);
}

/* This asks the Window Manager to handle fullscreen for us. This is the modern way. */
static void
X11_SetWindowFullscreenViaWM(_THIS, SDL_Window * window, SDL_VideoDisplay * _display, SDL_bool fullscreen)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata = (SDL_DisplayData *) _display->driverdata;
    Display *display = data->videodata->display;
    Atom _NET_WM_STATE = data->videodata->_NET_WM_STATE;
    Atom _NET_WM_STATE_FULLSCREEN = data->videodata->_NET_WM_STATE_FULLSCREEN;

    if (X11_IsWindowMapped(_this, window)) {
        XEvent e;

        if (!(window->flags & SDL_WINDOW_RESIZABLE)) {
            /* Compiz refuses fullscreen toggle if we're not resizable, so update the hints so we
               can be resized to the fullscreen resolution (or reset so we're not resizable again) */
            XSizeHints *sizehints = X11_XAllocSizeHints();
            long flags = 0;
            X11_XGetWMNormalHints(display, data->xwindow, sizehints, &flags);
            /* set the resize flags on */
            if (fullscreen) {
                /* we are going fullscreen so turn the flags off */
                sizehints->flags &= ~(PMinSize | PMaxSize);
            } else {
                /* Reset the min/max width height to make the window non-resizable again */
                sizehints->flags |= PMinSize | PMaxSize;
                sizehints->min_width = sizehints->max_width = window->windowed.w;
                sizehints->min_height = sizehints->max_height = window->windowed.h;
            }
            X11_XSetWMNormalHints(display, data->xwindow, sizehints);
            X11_XFree(sizehints);
        }

        SDL_zero(e);
        e.xany.type = ClientMessage;
        e.xclient.message_type = _NET_WM_STATE;
        e.xclient.format = 32;
        e.xclient.window = data->xwindow;
        e.xclient.data.l[0] =
            fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
        e.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
        e.xclient.data.l[3] = 0l;

        X11_XSendEvent(display, RootWindow(display, displaydata->screen), 0,
                   SubstructureNotifyMask | SubstructureRedirectMask, &e);

        /* Fullscreen windows sometimes end up being marked maximized by
            window managers. Force it back to how we expect it to be. */
        if (!fullscreen && ((window->flags & SDL_WINDOW_MAXIMIZED) == 0)) {
            SDL_zero(e);
            e.xany.type = ClientMessage;
            e.xclient.message_type = _NET_WM_STATE;
            e.xclient.format = 32;
            e.xclient.window = data->xwindow;
            e.xclient.data.l[0] = _NET_WM_STATE_REMOVE;
            e.xclient.data.l[1] = data->videodata->_NET_WM_STATE_MAXIMIZED_VERT;
            e.xclient.data.l[2] = data->videodata->_NET_WM_STATE_MAXIMIZED_HORZ;
            e.xclient.data.l[3] = 0l;
            X11_XSendEvent(display, RootWindow(display, displaydata->screen), 0,
                   SubstructureNotifyMask | SubstructureRedirectMask, &e);
        }
    } else {
        Uint32 flags;

        flags = window->flags;
        if (fullscreen) {
            flags |= SDL_WINDOW_FULLSCREEN;
        } else {
            flags &= ~SDL_WINDOW_FULLSCREEN;
        }
        X11_SetNetWMState(_this, data->xwindow, flags);
    }

    if (data->visual->class == DirectColor) {
        if ( fullscreen ) {
            X11_XInstallColormap(display, data->colormap);
        } else {
            X11_XUninstallColormap(display, data->colormap);
        }
    }

    X11_XFlush(display);
}

/* This handles fullscreen itself, outside the Window Manager. */
static void
X11_BeginWindowFullscreenLegacy(_THIS, SDL_Window * window, SDL_VideoDisplay * _display)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata = (SDL_DisplayData *) _display->driverdata;
    Visual *visual = data->visual;
    Display *display = data->videodata->display;
    const int screen = displaydata->screen;
    Window root = RootWindow(display, screen);
    const int def_vis = (visual == DefaultVisual(display, screen));
    unsigned long xattrmask = 0;
    XSetWindowAttributes xattr;
    XEvent ev;
    SDL_Rect rect;

    if ( data->fswindow ) {
        return;  /* already fullscreen, I hope. */
    }

    X11_GetDisplayBounds(_this, _display, &rect);

    SDL_zero(xattr);
    xattr.override_redirect = True;
    xattrmask |= CWOverrideRedirect;
    xattr.background_pixel = def_vis ? BlackPixel(display, screen) : 0;
    xattrmask |= CWBackPixel;
    xattr.border_pixel = 0;
    xattrmask |= CWBorderPixel;
    xattr.colormap = data->colormap;
    xattrmask |= CWColormap;

    data->fswindow = X11_XCreateWindow(display, root,
                                   rect.x, rect.y, rect.w, rect.h, 0,
                                   displaydata->depth, InputOutput,
                                   visual, xattrmask, &xattr);

    X11_XSelectInput(display, data->fswindow, StructureNotifyMask);
    X11_XSetWindowBackground(display, data->fswindow, 0);
    X11_XInstallColormap(display, data->colormap);
    X11_XClearWindow(display, data->fswindow);
    X11_XMapRaised(display, data->fswindow);

    /* Make sure the fswindow is in view by warping mouse to the corner */
    X11_XUngrabPointer(display, CurrentTime);
    X11_XWarpPointer(display, None, root, 0, 0, 0, 0, rect.x, rect.y);

    /* Wait to be mapped, filter Unmap event out if it arrives. */
    X11_XIfEvent(display, &ev, &isMapNotify, (XPointer)&data->fswindow);
    X11_XCheckIfEvent(display, &ev, &isUnmapNotify, (XPointer)&data->fswindow);

#if SDL_VIDEO_DRIVER_X11_XVIDMODE
    if ( displaydata->use_vidmode ) {
        X11_XF86VidModeLockModeSwitch(display, screen, True);
    }
#endif

    SetWindowBordered(display, displaydata->screen, data->xwindow, SDL_FALSE);

    /* Center actual window within our cover-the-screen window. */
    X11_XReparentWindow(display, data->xwindow, data->fswindow,
                    (rect.w - window->w) / 2, (rect.h - window->h) / 2);

    /* Move the mouse to the upper left to make sure it's on-screen */
    X11_XWarpPointer(display, None, root, 0, 0, 0, 0, rect.x, rect.y);

    /* Center mouse in the fullscreen window. */
    rect.x += (rect.w / 2);
    rect.y += (rect.h / 2);
    X11_XWarpPointer(display, None, root, 0, 0, 0, 0, rect.x, rect.y);

    /* Wait to be mapped, filter Unmap event out if it arrives. */
    X11_XIfEvent(display, &ev, &isMapNotify, (XPointer)&data->xwindow);
    X11_XCheckIfEvent(display, &ev, &isUnmapNotify, (XPointer)&data->xwindow);

    SDL_UpdateWindowGrab(window);
}

static void
X11_EndWindowFullscreenLegacy(_THIS, SDL_Window * window, SDL_VideoDisplay * _display)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    SDL_DisplayData *displaydata = (SDL_DisplayData *) _display->driverdata;
    Display *display = data->videodata->display;
    const int screen = displaydata->screen;
    Window root = RootWindow(display, screen);
    Window fswindow = data->fswindow;
    XEvent ev;

    if (!data->fswindow) {
        return;  /* already not fullscreen, I hope. */
    }

    data->fswindow = None;

#if SDL_VIDEO_DRIVER_X11_VIDMODE
    if ( displaydata->use_vidmode ) {
        X11_XF86VidModeLockModeSwitch(display, screen, False);
    }
#endif

    SDL_UpdateWindowGrab(window);

    X11_XReparentWindow(display, data->xwindow, root, window->x, window->y);

    /* flush these events so they don't confuse normal event handling */
    X11_XSync(display, False);
    X11_XCheckIfEvent(display, &ev, &isMapNotify, (XPointer)&data->xwindow);
    X11_XCheckIfEvent(display, &ev, &isUnmapNotify, (XPointer)&data->xwindow);

    SetWindowBordered(display, screen, data->xwindow,
                      (window->flags & SDL_WINDOW_BORDERLESS) == 0);

    X11_XWithdrawWindow(display, fswindow, screen);

    /* Wait to be unmapped. */
    X11_XIfEvent(display, &ev, &isUnmapNotify, (XPointer)&fswindow);
    X11_XDestroyWindow(display, fswindow);
}


void
X11_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * _display, SDL_bool fullscreen)
{
    /* !!! FIXME: SDL_Hint? */
    SDL_bool legacy = SDL_FALSE;
    const char *env = SDL_getenv("SDL_VIDEO_X11_LEGACY_FULLSCREEN");
    if (env) {
        legacy = SDL_atoi(env);
    } else {
        SDL_VideoData *videodata = (SDL_VideoData *) _this->driverdata;
        SDL_DisplayData *displaydata = (SDL_DisplayData *) _display->driverdata;
        if ( displaydata->use_vidmode ) {
            legacy = SDL_TRUE;  /* the new stuff only works with XRandR. */
        } else if ( !videodata->net_wm ) {
            legacy = SDL_TRUE;  /* The window manager doesn't support it */
        } else {
            /* !!! FIXME: look at the window manager name, and blacklist certain ones? */
            /* http://stackoverflow.com/questions/758648/find-the-name-of-the-x-window-manager */
            legacy = SDL_FALSE;  /* try the new way. */
        }
    }

    if (legacy) {
        if (fullscreen) {
            X11_BeginWindowFullscreenLegacy(_this, window, _display);
        } else {
            X11_EndWindowFullscreenLegacy(_this, window, _display);
        }
    } else {
        X11_SetWindowFullscreenViaWM(_this, window, _display, fullscreen);
    }
}


int
X11_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    Visual *visual = data->visual;
    Colormap colormap = data->colormap;
    XColor *colorcells;
    int ncolors;
    int rmask, gmask, bmask;
    int rshift, gshift, bshift;
    int i;

    if (visual->class != DirectColor) {
        return SDL_SetError("Window doesn't have DirectColor visual");
    }

    ncolors = visual->map_entries;
    colorcells = SDL_malloc(ncolors * sizeof(XColor));
    if (!colorcells) {
        return SDL_OutOfMemory();
    }

    rshift = 0;
    rmask = visual->red_mask;
    while (0 == (rmask & 1)) {
        rshift++;
        rmask >>= 1;
    }

    gshift = 0;
    gmask = visual->green_mask;
    while (0 == (gmask & 1)) {
        gshift++;
        gmask >>= 1;
    }

    bshift = 0;
    bmask = visual->blue_mask;
    while (0 == (bmask & 1)) {
        bshift++;
        bmask >>= 1;
    }

    /* build the color table pixel values */
    for (i = 0; i < ncolors; i++) {
        Uint32 rbits = (rmask * i) / (ncolors - 1);
        Uint32 gbits = (gmask * i) / (ncolors - 1);
        Uint32 bbits = (bmask * i) / (ncolors - 1);
        Uint32 pix = (rbits << rshift) | (gbits << gshift) | (bbits << bshift);

        colorcells[i].pixel = pix;

        colorcells[i].red = ramp[(0 * 256) + i];
        colorcells[i].green = ramp[(1 * 256) + i];
        colorcells[i].blue = ramp[(2 * 256) + i];

        colorcells[i].flags = DoRed | DoGreen | DoBlue;
    }

    X11_XStoreColors(display, colormap, colorcells, ncolors);
    X11_XFlush(display);
    SDL_free(colorcells);

    return 0;
}

void
X11_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;
    SDL_bool oldstyle_fullscreen;
    SDL_bool grab_keyboard;

    /* ICCCM2.0-compliant window managers can handle fullscreen windows
       If we're using XVidMode to change resolution we need to confine
       the cursor so we don't pan around the virtual desktop.
     */
    oldstyle_fullscreen = X11_IsWindowLegacyFullscreen(_this, window);

    if (oldstyle_fullscreen || grabbed) {
        /* Try to grab the mouse */
        if (!data->videodata->broken_pointer_grab) {
            const unsigned int mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | FocusChangeMask;
            int attempts;
            int result;

            /* Try for up to 5000ms (5s) to grab. If it still fails, stop trying. */
            for (attempts = 0; attempts < 100; attempts++) {
                result = X11_XGrabPointer(display, data->xwindow, True, mask, GrabModeAsync,
                                 GrabModeAsync, data->xwindow, None, CurrentTime);
                if (result == GrabSuccess) {
                    break;
                }
                SDL_Delay(50);
            }

            if (result != GrabSuccess) {
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "The X server refused to let us grab the mouse. You might experience input bugs.");
                data->videodata->broken_pointer_grab = SDL_TRUE;  /* don't try again. */
            }
        }

        /* Raise the window if we grab the mouse */
        X11_XRaiseWindow(display, data->xwindow);

        /* Now grab the keyboard */
        if (SDL_GetHintBoolean(SDL_HINT_GRAB_KEYBOARD, SDL_FALSE)) {
            grab_keyboard = SDL_TRUE;
        } else {
            /* We need to do this with the old style override_redirect
               fullscreen window otherwise we won't get keyboard focus.
            */
            grab_keyboard = oldstyle_fullscreen;
        }
        if (grab_keyboard) {
            X11_XGrabKeyboard(display, data->xwindow, True, GrabModeAsync,
                          GrabModeAsync, CurrentTime);
        }
    } else {
        X11_XUngrabPointer(display, CurrentTime);
        X11_XUngrabKeyboard(display, CurrentTime);
    }
    X11_XSync(display, False);
}

void
X11_DestroyWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;

    if (data) {
        SDL_VideoData *videodata = (SDL_VideoData *) data->videodata;
        Display *display = videodata->display;
        int numwindows = videodata->numwindows;
        SDL_WindowData **windowlist = videodata->windowlist;
        int i;

        if (windowlist) {
            for (i = 0; i < numwindows; ++i) {
                if (windowlist[i] && (windowlist[i]->window == window)) {
                    windowlist[i] = windowlist[numwindows - 1];
                    windowlist[numwindows - 1] = NULL;
                    videodata->numwindows--;
                    break;
                }
            }
        }
#ifdef X_HAVE_UTF8_STRING
        if (data->ic) {
            X11_XDestroyIC(data->ic);
        }
#endif
        if (data->created) {
            X11_XDestroyWindow(display, data->xwindow);
            X11_XFlush(display);
        }
        SDL_free(data);
    }
    window->driverdata = NULL;
}

SDL_bool
X11_GetWindowWMInfo(_THIS, SDL_Window * window, SDL_SysWMinfo * info)
{
    SDL_WindowData *data = (SDL_WindowData *) window->driverdata;
    Display *display = data->videodata->display;

    if (info->version.major == SDL_MAJOR_VERSION &&
        info->version.minor == SDL_MINOR_VERSION) {
        info->subsystem = SDL_SYSWM_X11;
        info->info.x11.display = display;
        info->info.x11.window = data->xwindow;
        return SDL_TRUE;
    } else {
        SDL_SetError("Application not compiled with SDL %d.%d",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }
}

int
X11_SetWindowHitTest(SDL_Window *window, SDL_bool enabled)
{
    return 0;  /* just succeed, the real work is done elsewhere. */
}

#endif /* SDL_VIDEO_DRIVER_X11 */

/* vi: set ts=4 sw=4 expandtab: */
