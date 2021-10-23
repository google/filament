/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_OFFSCREEN

/* Offscreen video driver is similar to dummy driver, however its purpose
 * is enabling applications to use some of the SDL video functionality
 * (notably context creation) while not requiring a display output.
 *
 * An example would be running a graphical program on a headless box
 * for automated testing.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_offscreenvideo.h"
#include "SDL_offscreenevents_c.h"
#include "SDL_offscreenframebuffer_c.h"
#include "SDL_offscreenopengl.h"

#define OFFSCREENVID_DRIVER_NAME "offscreen"

/* Initialization/Query functions */
static int OFFSCREEN_VideoInit(_THIS);
static int OFFSCREEN_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void OFFSCREEN_VideoQuit(_THIS);

/* OFFSCREEN driver bootstrap functions */

static void
OFFSCREEN_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
OFFSCREEN_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return (0);
    }

    /* General video */
    device->VideoInit = OFFSCREEN_VideoInit;
    device->VideoQuit = OFFSCREEN_VideoQuit;
    device->SetDisplayMode = OFFSCREEN_SetDisplayMode;
    device->PumpEvents = OFFSCREEN_PumpEvents;
    device->CreateWindowFramebuffer = SDL_OFFSCREEN_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = SDL_OFFSCREEN_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = SDL_OFFSCREEN_DestroyWindowFramebuffer;
    device->free = OFFSCREEN_DeleteDevice;

    /* GL context */
    device->GL_SwapWindow = OFFSCREEN_GL_SwapWindow;
    device->GL_MakeCurrent = OFFSCREEN_GL_MakeCurrent;
    device->GL_CreateContext = OFFSCREEN_GL_CreateContext;
    device->GL_DeleteContext = OFFSCREEN_GL_DeleteContext;
    device->GL_LoadLibrary = OFFSCREEN_GL_LoadLibrary;
    device->GL_UnloadLibrary = OFFSCREEN_GL_UnloadLibrary;
    device->GL_GetProcAddress = OFFSCREEN_GL_GetProcAddress;
    device->GL_GetSwapInterval = OFFSCREEN_GL_GetSwapInterval;
    device->GL_SetSwapInterval = OFFSCREEN_GL_SetSwapInterval;

    /* "Window" */
    device->CreateSDLWindow = OFFSCREEN_CreateWindow;
    device->DestroyWindow = OFFSCREEN_DestroyWindow;

    return device;
}

VideoBootStrap OFFSCREEN_bootstrap = {
    OFFSCREENVID_DRIVER_NAME, "SDL offscreen video driver",
    OFFSCREEN_CreateDevice
};

int
OFFSCREEN_VideoInit(_THIS)
{
    SDL_DisplayMode mode;
    SDL_Mouse *mouse = NULL;

    /* Use a fake 32-bpp desktop mode */
    mode.format = SDL_PIXELFORMAT_RGB888;
    mode.w = 1024;
    mode.h = 768;
    mode.refresh_rate = 0;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    /* We're done! */
    return 0;
}

static int
OFFSCREEN_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
OFFSCREEN_VideoQuit(_THIS)
{
}

#endif /* SDL_VIDEO_DRIVER_OFFSCREEN */

/* vi: set ts=4 sw=4 expandtab: */
