/*
 * Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include <dlfcn.h>
#include <stdlib.h>

#include <X11/Xutil.h>

typedef int (*PFN_XDestroyWindow)(Display*, Window);
typedef Display* (*PFN_XOpenDisplay)(_Xconst char*);
typedef Colormap (*PFN_XCreateColormap)(Display*, Window, Visual*, int);
typedef Window (*PFN_XCreateWindow)(Display*, Window, int, int, unsigned int, unsigned int, unsigned int, int, unsigned int,
                                    Visual*, unsigned long, XSetWindowAttributes*);
typedef int (*PFN_XSelectInput)(Display*, Window, long);
typedef int (*PFN_XMapWindow)(Display*, Window);
typedef Atom (*PFN_XInternAtom)(Display*, _Xconst char*, Bool);
typedef int (*PFN_XNextEvent)(Display*, XEvent*);
typedef int (*PFN_XPending)(Display*);
typedef XVisualInfo* (*PFN_XGetVisualInfo)(Display*, long, XVisualInfo*, int*);
typedef int (*PFN_XCloseDisplay)(Display* /* display */
);
typedef Status (*PFN_XInitThreads)(void);
typedef int (*PFN_XFlush)(Display* /* display */
);

static PFN_XDestroyWindow cube_XDestroyWindow = NULL;
static PFN_XOpenDisplay cube_XOpenDisplay = NULL;
static PFN_XCreateColormap cube_XCreateColormap = NULL;
static PFN_XCreateWindow cube_XCreateWindow = NULL;
static PFN_XSelectInput cube_XSelectInput = NULL;
static PFN_XMapWindow cube_XMapWindow = NULL;
static PFN_XInternAtom cube_XInternAtom = NULL;
static PFN_XNextEvent cube_XNextEvent = NULL;
static PFN_XPending cube_XPending = NULL;
static PFN_XGetVisualInfo cube_XGetVisualInfo = NULL;
static PFN_XCloseDisplay cube_XCloseDisplay = NULL;
static PFN_XInitThreads cube_XInitThreads = NULL;
static PFN_XFlush cube_XFlush = NULL;

#define XDestroyWindow cube_XDestroyWindow
#define XOpenDisplay cube_XOpenDisplay
#define XCreateColormap cube_XCreateColormap
#define XCreateWindow cube_XCreateWindow
#define XSelectInput cube_XSelectInput
#define XMapWindow cube_XMapWindow
#define XInternAtom cube_XInternAtom
#define XNextEvent cube_XNextEvent
#define XPending cube_XPending
#define XGetVisualInfo cube_XGetVisualInfo
#define XCloseDisplay cube_XCloseDisplay
#define XInitThreads cube_XInitThreads
#define XFlush cube_XFlush

void* initialize_xlib() {
    void* xlib_library = NULL;
#if defined(XLIB_LIBRARY)
    xlib_library = dlopen(XLIB_LIBRARY, RTLD_NOW | RTLD_LOCAL);
#endif
    if (NULL == xlib_library) {
        xlib_library = dlopen("libX11.so.6", RTLD_NOW | RTLD_LOCAL);
    }
    if (NULL == xlib_library) {
        xlib_library = dlopen("libX11.so", RTLD_NOW | RTLD_LOCAL);
    }
    if (NULL == xlib_library) {
        return NULL;
    }

#ifdef __cplusplus
#define TYPE_CONVERSION(type) reinterpret_cast<type>
#else
#define TYPE_CONVERSION(type)
#endif

    cube_XDestroyWindow = TYPE_CONVERSION(PFN_XDestroyWindow)(dlsym(xlib_library, "XDestroyWindow"));
    cube_XOpenDisplay = TYPE_CONVERSION(PFN_XOpenDisplay)(dlsym(xlib_library, "XOpenDisplay"));
    cube_XCreateColormap = TYPE_CONVERSION(PFN_XCreateColormap)(dlsym(xlib_library, "XCreateColormap"));
    cube_XCreateWindow = TYPE_CONVERSION(PFN_XCreateWindow)(dlsym(xlib_library, "XCreateWindow"));
    cube_XSelectInput = TYPE_CONVERSION(PFN_XSelectInput)(dlsym(xlib_library, "XSelectInput"));
    cube_XMapWindow = TYPE_CONVERSION(PFN_XMapWindow)(dlsym(xlib_library, "XMapWindow"));
    cube_XInternAtom = TYPE_CONVERSION(PFN_XInternAtom)(dlsym(xlib_library, "XInternAtom"));
    cube_XNextEvent = TYPE_CONVERSION(PFN_XNextEvent)(dlsym(xlib_library, "XNextEvent"));
    cube_XPending = TYPE_CONVERSION(PFN_XPending)(dlsym(xlib_library, "XPending"));
    cube_XGetVisualInfo = TYPE_CONVERSION(PFN_XGetVisualInfo)(dlsym(xlib_library, "XGetVisualInfo"));
    cube_XCloseDisplay = TYPE_CONVERSION(PFN_XCloseDisplay)(dlsym(xlib_library, "XCloseDisplay"));
    cube_XInitThreads = TYPE_CONVERSION(PFN_XInitThreads)(dlsym(xlib_library, "XInitThreads"));
    cube_XFlush = TYPE_CONVERSION(PFN_XFlush)(dlsym(xlib_library, "XFlush"));

    return xlib_library;
}
