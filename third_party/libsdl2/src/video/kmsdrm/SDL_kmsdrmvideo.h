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

#ifndef __SDL_KMSDRMVIDEO_H__
#define __SDL_KMSDRMVIDEO_H__

#include "../SDL_sysvideo.h"

#include <fcntl.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
#include <EGL/egl.h>

typedef struct SDL_VideoData
{
    int devindex;               /* device index that was passed on creation */
    int drm_fd;                 /* DRM file desc */
    char devpath[32];           /* DRM dev path. */

    struct gbm_device *gbm_dev;

    SDL_bool video_init;             /* Has VideoInit succeeded? */
    SDL_bool vulkan_mode;            /* Are we in Vulkan mode? One VK window is enough to be. */
    SDL_bool async_pageflip_support; /* Does the hardware support async. pageflips? */ 

    SDL_Window **windows;
    int max_windows;
    int num_windows;

    /* Even if we have several displays, we only have to
       open 1 FD and create 1 gbm device. */
    SDL_bool gbm_init;

} SDL_VideoData;


typedef struct SDL_DisplayModeData
{
    int mode_index;
} SDL_DisplayModeData;


typedef struct SDL_DisplayData
{
    drmModeConnector *connector;
    drmModeCrtc *crtc;
    drmModeModeInfo mode;
    drmModeModeInfo original_mode;
    drmModeModeInfo fullscreen_mode;

    drmModeCrtc *saved_crtc;    /* CRTC to restore on quit */

    /* DRM & GBM cursor stuff lives here, not in an SDL_Cursor's driverdata struct,
       because setting/unsetting up these is done on window creation/destruction,
       where we may not have an SDL_Cursor at all (so no SDL_Cursor driverdata).
       There's only one cursor GBM BO because we only support one cursor. */
    struct gbm_bo *cursor_bo;
    uint64_t cursor_w, cursor_h;

    SDL_bool default_cursor_init;
} SDL_DisplayData;

typedef struct SDL_WindowData
{
    SDL_VideoData *viddata;
    /* SDL internals expect EGL surface to be here, and in KMSDRM the GBM surface is
       what supports the EGL surface on the driver side, so all these surfaces and buffers
       are expected to be here, in the struct pointed by SDL_Window driverdata pointer:
       this one. So don't try to move these to dispdata!  */
    struct gbm_surface *gs;
    struct gbm_bo *bo;
    struct gbm_bo *next_bo;

    SDL_bool waiting_for_flip;
    SDL_bool double_buffer;

    EGLSurface egl_surface;
    SDL_bool egl_surface_dirty;
} SDL_WindowData;

typedef struct KMSDRM_FBInfo
{
    int drm_fd;         /* DRM file desc */
    uint32_t fb_id;     /* DRM framebuffer ID */
} KMSDRM_FBInfo;

/* Helper functions */
int KMSDRM_CreateSurfaces(_THIS, SDL_Window * window);
KMSDRM_FBInfo *KMSDRM_FBFromBO(_THIS, struct gbm_bo *bo);
KMSDRM_FBInfo *KMSDRM_FBFromBO2(_THIS, struct gbm_bo *bo, int w, int h);
SDL_bool KMSDRM_WaitPageflip(_THIS, SDL_WindowData *windata);

/****************************************************************************/
/* SDL_VideoDevice functions declaration                                    */
/****************************************************************************/

/* Display and window functions */
int KMSDRM_VideoInit(_THIS);
void KMSDRM_VideoQuit(_THIS);
void KMSDRM_GetDisplayModes(_THIS, SDL_VideoDisplay * display);
int KMSDRM_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
int KMSDRM_CreateWindow(_THIS, SDL_Window * window);
int KMSDRM_CreateWindowFrom(_THIS, SDL_Window * window, const void *data);
void KMSDRM_SetWindowTitle(_THIS, SDL_Window * window);
void KMSDRM_SetWindowIcon(_THIS, SDL_Window * window, SDL_Surface * icon);
void KMSDRM_SetWindowPosition(_THIS, SDL_Window * window);
void KMSDRM_SetWindowSize(_THIS, SDL_Window * window);
void KMSDRM_SetWindowFullscreen(_THIS, SDL_Window * window, SDL_VideoDisplay * _display, SDL_bool fullscreen);
int KMSDRM_SetWindowGammaRamp(_THIS, SDL_Window * window, const Uint16 * ramp);
int KMSDRM_GetWindowGammaRamp(_THIS, SDL_Window * window, Uint16 * ramp);
void KMSDRM_ShowWindow(_THIS, SDL_Window * window);
void KMSDRM_HideWindow(_THIS, SDL_Window * window);
void KMSDRM_RaiseWindow(_THIS, SDL_Window * window);
void KMSDRM_MaximizeWindow(_THIS, SDL_Window * window);
void KMSDRM_MinimizeWindow(_THIS, SDL_Window * window);
void KMSDRM_RestoreWindow(_THIS, SDL_Window * window);
void KMSDRM_DestroyWindow(_THIS, SDL_Window * window);

/* Window manager function */
SDL_bool KMSDRM_GetWindowWMInfo(_THIS, SDL_Window * window,
                             struct SDL_SysWMinfo *info);

/* OpenGL/OpenGL ES functions */
int KMSDRM_GLES_LoadLibrary(_THIS, const char *path);
void *KMSDRM_GLES_GetProcAddress(_THIS, const char *proc);
void KMSDRM_GLES_UnloadLibrary(_THIS);
SDL_GLContext KMSDRM_GLES_CreateContext(_THIS, SDL_Window * window);
int KMSDRM_GLES_MakeCurrent(_THIS, SDL_Window * window, SDL_GLContext context);
int KMSDRM_GLES_SetSwapInterval(_THIS, int interval);
int KMSDRM_GLES_GetSwapInterval(_THIS);
int KMSDRM_GLES_SwapWindow(_THIS, SDL_Window * window);
void KMSDRM_GLES_DeleteContext(_THIS, SDL_GLContext context);

#endif /* __SDL_KMSDRMVIDEO_H__ */

/* vi: set ts=4 sw=4 expandtab: */
