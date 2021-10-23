/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2015 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_DRIVER_VITA

/* SDL internals */
#include "../SDL_sysvideo.h"
#include "SDL_version.h"
#include "SDL_syswm.h"
#include "SDL_loadso.h"
#include "SDL_events.h"
#include "../../events/SDL_mouse_c.h"
#include "../../events/SDL_keyboard_c.h"

/* VITA declarations */
#include "SDL_vitavideo.h"
#include "SDL_vitatouch.h"
#include "SDL_vitakeyboard.h"
#include "SDL_vitamouse_c.h"
#include "SDL_vitaframebuffer.h"
#if SDL_VIDEO_VITA_PIB
#include "SDL_vitagl_c.h"
#endif
#include <psp2/ime_dialog.h>

SDL_Window *Vita_Window;

static void
VITA_Destroy(SDL_VideoDevice * device)
{
/*    SDL_VideoData *phdata = (SDL_VideoData *) device->driverdata; */

    SDL_free(device->driverdata);
    SDL_free(device);
//    if (device->driverdata != NULL) {
//        device->driverdata = NULL;
//    }
}

static SDL_VideoDevice *
VITA_Create()
{
    SDL_VideoDevice *device;
    SDL_VideoData *phdata;
#if SDL_VIDEO_VITA_PIB
    SDL_GLDriverData *gldata;
#endif
    /* Initialize SDL_VideoDevice structure */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (device == NULL) {
        SDL_OutOfMemory();
        return NULL;
    }

    /* Initialize internal VITA specific data */
    phdata = (SDL_VideoData *) SDL_calloc(1, sizeof(SDL_VideoData));
    if (phdata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        return NULL;
    }
#if SDL_VIDEO_VITA_PIB

    gldata = (SDL_GLDriverData *) SDL_calloc(1, sizeof(SDL_GLDriverData));
    if (gldata == NULL) {
        SDL_OutOfMemory();
        SDL_free(device);
        SDL_free(phdata);
        return NULL;
    }
    device->gl_data = gldata;
    phdata->egl_initialized = SDL_TRUE;
#endif
    phdata->ime_active = SDL_FALSE;

    device->driverdata = phdata;

    /* Setup amount of available displays and current display */
    device->num_displays = 0;

    /* Set device free function */
    device->free = VITA_Destroy;

    /* Setup all functions which we can handle */
    device->VideoInit = VITA_VideoInit;
    device->VideoQuit = VITA_VideoQuit;
    device->GetDisplayModes = VITA_GetDisplayModes;
    device->SetDisplayMode = VITA_SetDisplayMode;
    device->CreateSDLWindow = VITA_CreateWindow;
    device->CreateSDLWindowFrom = VITA_CreateWindowFrom;
    device->SetWindowTitle = VITA_SetWindowTitle;
    device->SetWindowIcon = VITA_SetWindowIcon;
    device->SetWindowPosition = VITA_SetWindowPosition;
    device->SetWindowSize = VITA_SetWindowSize;
    device->ShowWindow = VITA_ShowWindow;
    device->HideWindow = VITA_HideWindow;
    device->RaiseWindow = VITA_RaiseWindow;
    device->MaximizeWindow = VITA_MaximizeWindow;
    device->MinimizeWindow = VITA_MinimizeWindow;
    device->RestoreWindow = VITA_RestoreWindow;
    device->SetWindowMouseGrab = VITA_SetWindowGrab;
    device->SetWindowKeyboardGrab = VITA_SetWindowGrab;
    device->DestroyWindow = VITA_DestroyWindow;
    device->GetWindowWMInfo = VITA_GetWindowWMInfo;

/*
    // Disabled, causes issues on high-framerate updates. SDL still emulates this.
    device->CreateWindowFramebuffer = VITA_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = VITA_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = VITA_DestroyWindowFramebuffer;
*/

#if SDL_VIDEO_VITA_PIB
    device->GL_LoadLibrary = VITA_GL_LoadLibrary;
    device->GL_GetProcAddress = VITA_GL_GetProcAddress;
    device->GL_UnloadLibrary = VITA_GL_UnloadLibrary;
    device->GL_CreateContext = VITA_GL_CreateContext;
    device->GL_MakeCurrent = VITA_GL_MakeCurrent;
    device->GL_SetSwapInterval = VITA_GL_SetSwapInterval;
    device->GL_GetSwapInterval = VITA_GL_GetSwapInterval;
    device->GL_SwapWindow = VITA_GL_SwapWindow;
    device->GL_DeleteContext = VITA_GL_DeleteContext;
#endif

    device->HasScreenKeyboardSupport = VITA_HasScreenKeyboardSupport;
    device->ShowScreenKeyboard = VITA_ShowScreenKeyboard;
    device->HideScreenKeyboard = VITA_HideScreenKeyboard;
    device->IsScreenKeyboardShown = VITA_IsScreenKeyboardShown;

    device->PumpEvents = VITA_PumpEvents;

    return device;
}

VideoBootStrap VITA_bootstrap = {
    "VITA",
    "VITA Video Driver",
    VITA_Create
};

/*****************************************************************************/
/* SDL Video and Display initialization/handling functions                   */
/*****************************************************************************/
int
VITA_VideoInit(_THIS)
{
    SDL_VideoDisplay display;
    SDL_DisplayMode current_mode;

    SDL_zero(current_mode);

    current_mode.w = 960;
    current_mode.h = 544;

    current_mode.refresh_rate = 60;
    /* 32 bpp for default */
    current_mode.format = SDL_PIXELFORMAT_ABGR8888;

    current_mode.driverdata = NULL;

    SDL_zero(display);
    display.desktop_mode = current_mode;
    display.current_mode = current_mode;
    display.driverdata = NULL;

    SDL_AddVideoDisplay(&display, SDL_FALSE);
    VITA_InitTouch();
    VITA_InitKeyboard();
    VITA_InitMouse();

    return 1;
}

void
VITA_VideoQuit(_THIS)
{
    VITA_QuitTouch();
}

void
VITA_GetDisplayModes(_THIS, SDL_VideoDisplay * display)
{
    SDL_AddDisplayMode(display, &display->current_mode);
}

int
VITA_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

int
VITA_CreateWindow(_THIS, SDL_Window * window)
{
    SDL_WindowData *wdata;

    /* Allocate window internal data */
    wdata = (SDL_WindowData *) SDL_calloc(1, sizeof(SDL_WindowData));
    if (wdata == NULL) {
        return SDL_OutOfMemory();
    }

    /* Setup driver data for this window */
    window->driverdata = wdata;

    // Vita can only have one window
    if (Vita_Window != NULL)
    {
        SDL_SetError("Only one window supported");
        return -1;
    }

    Vita_Window = window;

    // fix input, we need to find a better way
    SDL_SetKeyboardFocus(window);

    /* Window has been successfully created */
    return 0;
}

int
VITA_CreateWindowFrom(_THIS, SDL_Window * window, const void *data)
{
    return -1;
}

void
VITA_SetWindowTitle(_THIS, SDL_Window * window)
{
}
void
VITA_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon)
{
}
void
VITA_SetWindowPosition(_THIS, SDL_Window * window)
{
}
void
VITA_SetWindowSize(_THIS, SDL_Window * window)
{
}
void
VITA_ShowWindow(_THIS, SDL_Window * window)
{
}
void
VITA_HideWindow(_THIS, SDL_Window * window)
{
}
void
VITA_RaiseWindow(_THIS, SDL_Window * window)
{
}
void
VITA_MaximizeWindow(_THIS, SDL_Window * window)
{
}
void
VITA_MinimizeWindow(_THIS, SDL_Window * window)
{
}
void
VITA_RestoreWindow(_THIS, SDL_Window * window)
{
}
void
VITA_SetWindowGrab(_THIS, SDL_Window * window, SDL_bool grabbed)
{

}
void
VITA_DestroyWindow(_THIS, SDL_Window * window)
{
//    SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;
    SDL_WindowData *data;

    data = window->driverdata;
    if (data) {
        // TODO: should we destroy egl context? No one sane should recreate ogl window as non-ogl
        SDL_free(data);
    }

    window->driverdata = NULL;
    Vita_Window = NULL;
}

/*****************************************************************************/
/* SDL Window Manager function                                               */
/*****************************************************************************/
SDL_bool
VITA_GetWindowWMInfo(_THIS, SDL_Window * window, struct SDL_SysWMinfo *info)
{
    if (info->version.major <= SDL_MAJOR_VERSION) {
        return SDL_TRUE;
    } else {
        SDL_SetError("application not compiled with SDL %d.%d\n",
                     SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
        return SDL_FALSE;
    }

    /* Failed to get window manager information */
    return SDL_FALSE;
}

SDL_bool VITA_HasScreenKeyboardSupport(_THIS)
{
    return SDL_TRUE;
}

#if !defined(SCE_IME_LANGUAGE_ENGLISH_US)
#define SCE_IME_LANGUAGE_ENGLISH_US SCE_IME_LANGUAGE_ENGLISH
#endif

void VITA_ShowScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;

    SceWChar16 *title = u"";
    SceWChar16 *text = u"";
    SceInt32 res;

    SceImeDialogParam param;
    sceImeDialogParamInit(&param);

    param.supportedLanguages = SCE_IME_LANGUAGE_ENGLISH_US;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = 0;
    param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
    param.maxTextLength = SCE_IME_DIALOG_MAX_TEXT_LENGTH;

    param.title = title;
    param.initialText = text;
    param.inputTextBuffer = videodata->ime_buffer;

    res = sceImeDialogInit(&param);
    if (res < 0) {
        SDL_SetError("Failed to init IME dialog");
        return;
    }

    videodata->ime_active = SDL_TRUE;
}

void VITA_HideScreenKeyboard(_THIS, SDL_Window *window)
{
    SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;

    SceCommonDialogStatus dialogStatus = sceImeDialogGetStatus();

    switch (dialogStatus) {
        default:
        case SCE_COMMON_DIALOG_STATUS_NONE:
        case SCE_COMMON_DIALOG_STATUS_RUNNING:
                break;
        case SCE_COMMON_DIALOG_STATUS_FINISHED:
                sceImeDialogTerm();
                break;
    }

    videodata->ime_active = SDL_FALSE;
}

SDL_bool VITA_IsScreenKeyboardShown(_THIS, SDL_Window *window)
{
    SceCommonDialogStatus dialogStatus = sceImeDialogGetStatus();
    return (dialogStatus == SCE_COMMON_DIALOG_STATUS_RUNNING);
}


static void utf16_to_utf8(const uint16_t *src, uint8_t *dst) {
  int i;
  for (i = 0; src[i]; i++) {
    if ((src[i] & 0xFF80) == 0) {
      *(dst++) = src[i] & 0xFF;
    } else if((src[i] & 0xF800) == 0) {
      *(dst++) = ((src[i] >> 6) & 0xFF) | 0xC0;
      *(dst++) = (src[i] & 0x3F) | 0x80;
    } else if((src[i] & 0xFC00) == 0xD800 && (src[i + 1] & 0xFC00) == 0xDC00) {
      *(dst++) = (((src[i] + 64) >> 8) & 0x3) | 0xF0;
      *(dst++) = (((src[i] >> 2) + 16) & 0x3F) | 0x80;
      *(dst++) = ((src[i] >> 4) & 0x30) | 0x80 | ((src[i + 1] << 2) & 0xF);
      *(dst++) = (src[i + 1] & 0x3F) | 0x80;
      i += 1;
    } else {
      *(dst++) = ((src[i] >> 12) & 0xF) | 0xE0;
      *(dst++) = ((src[i] >> 6) & 0x3F) | 0x80;
      *(dst++) = (src[i] & 0x3F) | 0x80;
    }
  }

  *dst = '\0';
}

void VITA_PumpEvents(_THIS)
{
    SDL_VideoData *videodata = (SDL_VideoData *)_this->driverdata;


    VITA_PollTouch();
    VITA_PollKeyboard();
    VITA_PollMouse();

    if (videodata->ime_active == SDL_TRUE) {
        // update IME status. Terminate, if finished
        SceCommonDialogStatus dialogStatus = sceImeDialogGetStatus();
         if (dialogStatus == SCE_COMMON_DIALOG_STATUS_FINISHED) {
            uint8_t utf8_buffer[SCE_IME_DIALOG_MAX_TEXT_LENGTH];

            SceImeDialogResult result;
            SDL_memset(&result, 0, sizeof(SceImeDialogResult));
            sceImeDialogGetResult(&result);

            // Convert UTF16 to UTF8
            utf16_to_utf8(videodata->ime_buffer, utf8_buffer);

            // Send SDL event
            SDL_SendKeyboardText((const char*)utf8_buffer);

            // Send enter key only on enter
            if (result.button == SCE_IME_DIALOG_BUTTON_ENTER)
                SDL_SendKeyboardKeyAutoRelease(SDL_SCANCODE_RETURN);

            sceImeDialogTerm();

            videodata->ime_active = SDL_FALSE;
        }

    }
}

#endif /* SDL_VIDEO_DRIVER_VITA */

/* vi: set ts=4 sw=4 expandtab: */

