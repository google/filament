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

#if SDL_VIDEO_DRIVER_HAIKU


#ifdef __cplusplus
extern "C" {
#endif

#include "SDL_bkeyboard.h"
#include "SDL_bwindow.h"
#include "SDL_bclipboard.h"
#include "SDL_bvideo.h"
#include "SDL_bopengl.h"
#include "SDL_bmodes.h"
#include "SDL_bframebuffer.h"
#include "SDL_bevents.h"

/* FIXME: Undefined functions */
//    #define BE_PumpEvents NULL
    #define BE_StartTextInput NULL
    #define BE_StopTextInput NULL
    #define BE_SetTextInputRect NULL

//    #define BE_DeleteDevice NULL

/* End undefined functions */

static SDL_VideoDevice *
BE_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;
    /*SDL_VideoData *data;*/

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));

    device->driverdata = NULL; /* FIXME: Is this the cause of some of the
    							  SDL_Quit() errors? */

/* TODO: Figure out if any initialization needs to go here */

    /* Set the function pointers */
    device->VideoInit = BE_VideoInit;
    device->VideoQuit = BE_VideoQuit;
    device->GetDisplayBounds = BE_GetDisplayBounds;
    device->GetDisplayModes = BE_GetDisplayModes;
    device->SetDisplayMode = BE_SetDisplayMode;
    device->PumpEvents = BE_PumpEvents;

    device->CreateSDLWindow = BE_CreateWindow;
    device->CreateSDLWindowFrom = BE_CreateWindowFrom;
    device->SetWindowTitle = BE_SetWindowTitle;
    device->SetWindowIcon = BE_SetWindowIcon;
    device->SetWindowPosition = BE_SetWindowPosition;
    device->SetWindowSize = BE_SetWindowSize;
    device->ShowWindow = BE_ShowWindow;
    device->HideWindow = BE_HideWindow;
    device->RaiseWindow = BE_RaiseWindow;
    device->MaximizeWindow = BE_MaximizeWindow;
    device->MinimizeWindow = BE_MinimizeWindow;
    device->RestoreWindow = BE_RestoreWindow;
    device->SetWindowBordered = BE_SetWindowBordered;
    device->SetWindowResizable = BE_SetWindowResizable;
    device->SetWindowFullscreen = BE_SetWindowFullscreen;
    device->SetWindowGammaRamp = BE_SetWindowGammaRamp;
    device->GetWindowGammaRamp = BE_GetWindowGammaRamp;
    device->SetWindowGrab = BE_SetWindowGrab;
    device->DestroyWindow = BE_DestroyWindow;
    device->GetWindowWMInfo = BE_GetWindowWMInfo;
    device->CreateWindowFramebuffer = BE_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = BE_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = BE_DestroyWindowFramebuffer;
    
    device->shape_driver.CreateShaper = NULL;
    device->shape_driver.SetWindowShape = NULL;
    device->shape_driver.ResizeWindowShape = NULL;

#if SDL_VIDEO_OPENGL
    device->GL_LoadLibrary = BE_GL_LoadLibrary;
    device->GL_GetProcAddress = BE_GL_GetProcAddress;
    device->GL_UnloadLibrary = BE_GL_UnloadLibrary;
    device->GL_CreateContext = BE_GL_CreateContext;
    device->GL_MakeCurrent = BE_GL_MakeCurrent;
    device->GL_SetSwapInterval = BE_GL_SetSwapInterval;
    device->GL_GetSwapInterval = BE_GL_GetSwapInterval;
    device->GL_SwapWindow = BE_GL_SwapWindow;
    device->GL_DeleteContext = BE_GL_DeleteContext;
#endif

    device->StartTextInput = BE_StartTextInput;
    device->StopTextInput = BE_StopTextInput;
    device->SetTextInputRect = BE_SetTextInputRect;

    device->SetClipboardText = BE_SetClipboardText;
    device->GetClipboardText = BE_GetClipboardText;
    device->HasClipboardText = BE_HasClipboardText;

    device->free = BE_DeleteDevice;

    return device;
}

VideoBootStrap HAIKU_bootstrap = {
	"haiku", "Haiku graphics",
	BE_Available, BE_CreateDevice
};

void BE_DeleteDevice(SDL_VideoDevice * device)
{
	SDL_free(device->driverdata);
	SDL_free(device);
}

int BE_VideoInit(_THIS)
{
	/* Initialize the Be Application for appserver interaction */
	if (SDL_InitBeApp() < 0) {
		return -1;
	}
	
	/* Initialize video modes */
	BE_InitModes(_this);

	/* Init the keymap */
	BE_InitOSKeymap();
	
	
#if SDL_VIDEO_OPENGL
        /* testgl application doesn't load library, just tries to load symbols */
        /* is it correct? if so we have to load library here */
    BE_GL_LoadLibrary(_this, NULL);
#endif

        /* We're done! */
    return (0);
}

int BE_Available(void)
{
    return (1);
}

void BE_VideoQuit(_THIS)
{

    BE_QuitModes(_this);

    SDL_QuitBeApp();
}

#ifdef __cplusplus
}
#endif

#endif /* SDL_VIDEO_DRIVER_HAIKU */

/* vi: set ts=4 sw=4 expandtab: */
