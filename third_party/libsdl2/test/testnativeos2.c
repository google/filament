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

#ifdef TEST_NATIVE_OS2

#define WIN_CLIENT_CLASS         "SDL Test"

static void *CreateWindowNative(int w, int h);
static void DestroyWindowNative(void *window);

NativeWindowFactory OS2WindowFactory = {
  "DIVE",
  CreateWindowNative,
  DestroyWindowNative
};

static void *CreateWindowNative(int w, int h)
{
  HWND     hwnd;
  HWND     hwndFrame;
  ULONG    ulFrameFlags = FCF_TASKLIST | FCF_DLGBORDER | FCF_TITLEBAR |
                          FCF_SYSMENU | FCF_SHELLPOSITION |
                          FCF_SIZEBORDER | FCF_MINBUTTON | FCF_MAXBUTTON;

  WinRegisterClass( 0, WIN_CLIENT_CLASS, WinDefWindowProc,
                    CS_SIZEREDRAW | CS_MOVENOTIFY,
                    sizeof(ULONG) );        // We should have minimum 4 bytes.

  hwndFrame = WinCreateStdWindow( HWND_DESKTOP, 0, &ulFrameFlags,
                              WIN_CLIENT_CLASS, "SDL Test", 0, 0, 1, &hwnd );
  if ( hwndFrame == NULLHANDLE )
  {
    return 0;
  }

  WinSetWindowPos( hwndFrame, HWND_TOP, 0, 0, w, h,
                   SWP_ZORDER | SWP_ACTIVATE | SWP_SIZE | SWP_SHOW );

  return (void *)hwndFrame; // We may returns client or frame window handle
                            // for SDL_CreateWindowFrom().
}

static void DestroyWindowNative(void *window)
{
  WinDestroyWindow( (HWND)window );
}

#endif /* TEST_NATIVE_OS2 */
