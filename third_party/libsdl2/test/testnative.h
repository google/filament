/*
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/

/* Definitions for platform dependent windowing functions to test SDL
   integration with native windows
*/

#include "SDL.h"

/* This header includes all the necessary system headers for native windows */
#include "SDL_syswm.h"

typedef struct
{
    const char *tag;
    void *(*CreateNativeWindow) (int w, int h);
    void (*DestroyNativeWindow) (void *window);
} NativeWindowFactory;

#ifdef SDL_VIDEO_DRIVER_WINDOWS
#define TEST_NATIVE_WINDOWS
extern NativeWindowFactory WindowsWindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_X11
#define TEST_NATIVE_X11
extern NativeWindowFactory X11WindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_COCOA
/* Actually, we don't really do this, since it involves adding Objective C
   support to the build system, which is a little tricky.  You can uncomment
   it manually though and link testnativecocoa.m into the test application.
*/
#define TEST_NATIVE_COCOA
extern NativeWindowFactory CocoaWindowFactory;
#endif

#ifdef SDL_VIDEO_DRIVER_OS2
#define TEST_NATIVE_OS2
extern NativeWindowFactory OS2WindowFactory;
#endif
