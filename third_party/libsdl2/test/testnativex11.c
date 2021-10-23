/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

#include "testnative.h"

#ifdef TEST_NATIVE_X11

static void *CreateWindowX11(int w, int h);
static void DestroyWindowX11(void *window);

NativeWindowFactory X11WindowFactory = {
    "x11",
    CreateWindowX11,
    DestroyWindowX11
};

static Display *dpy;

static void *
CreateWindowX11(int w, int h)
{
    Window window = 0;

    dpy = XOpenDisplay(NULL);
    if (dpy) {
        window =
            XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, w, h, 0, 0,
                                0);
        XMapRaised(dpy, window);
        XSync(dpy, False);
    }
    return (void *) window;
}

static void
DestroyWindowX11(void *window)
{
    if (dpy) {
        XDestroyWindow(dpy, (Window) window);
        XCloseDisplay(dpy);
    }
}

#endif
