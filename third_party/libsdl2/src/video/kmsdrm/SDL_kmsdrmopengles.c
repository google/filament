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

#if SDL_VIDEO_DRIVER_KMSDRM && SDL_VIDEO_OPENGL_EGL

#include "SDL_log.h"

#include "SDL_kmsdrmvideo.h"
#include "SDL_kmsdrmopengles.h"
#include "SDL_kmsdrmdyn.h"

#ifndef EGL_PLATFORM_GBM_MESA
#define EGL_PLATFORM_GBM_MESA 0x31D7
#endif

/* EGL implementation of SDL OpenGL support */

int
KMSDRM_GLES_LoadLibrary(_THIS, const char *path) {
    return SDL_EGL_LoadLibrary(_this, path, ((SDL_VideoData *)_this->driverdata)->gbm, EGL_PLATFORM_GBM_MESA);
}

SDL_EGL_CreateContext_impl(KMSDRM)

SDL_bool
KMSDRM_GLES_SetupCrtc(_THIS, SDL_Window * window) {
    SDL_WindowData *wdata = ((SDL_WindowData *) window->driverdata);
    SDL_DisplayData *displaydata = (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    SDL_VideoData *vdata = ((SDL_VideoData *)_this->driverdata);
    KMSDRM_FBInfo *fb_info;

    if (!(_this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, wdata->egl_surface))) {
	SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "eglSwapBuffers failed on CRTC setup");
	return SDL_FALSE;
    }

    wdata->next_bo = KMSDRM_gbm_surface_lock_front_buffer(wdata->gs);
    if (wdata->next_bo == NULL) {
	SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not lock GBM surface front buffer on CRTC setup");
	return SDL_FALSE;
    }

    fb_info = KMSDRM_FBFromBO(_this, wdata->next_bo);
    if (fb_info == NULL) {
	return SDL_FALSE;
    }

    if(KMSDRM_drmModeSetCrtc(vdata->drm_fd, displaydata->crtc_id, fb_info->fb_id,
			    0, 0, &vdata->saved_conn_id, 1, &displaydata->cur_mode) != 0) {
       SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Could not set up CRTC to a GBM buffer");
       return SDL_FALSE;

    }

    wdata->crtc_ready = SDL_TRUE;
    return SDL_TRUE;
}

int KMSDRM_GLES_SetSwapInterval(_THIS, int interval) {
    if (!_this->egl_data) {
        return SDL_SetError("EGL not initialized");
    }

    if (interval == 0 || interval == 1) {
        _this->egl_data->egl_swapinterval = interval;
    } else {
        return SDL_SetError("Only swap intervals of 0 or 1 are supported");
    }

    return 0;
}

int
KMSDRM_GLES_SwapWindow(_THIS, SDL_Window * window) {
    SDL_WindowData *wdata = ((SDL_WindowData *) window->driverdata);
    SDL_DisplayData *displaydata = (SDL_DisplayData *) SDL_GetDisplayForWindow(window)->driverdata;
    SDL_VideoData *vdata = ((SDL_VideoData *)_this->driverdata);
    KMSDRM_FBInfo *fb_info;
    int ret;

    /* Do we still need to wait for a flip? */
    int timeout = 0;
    if (_this->egl_data->egl_swapinterval == 1) {
        timeout = -1;
    }
    if (!KMSDRM_WaitPageFlip(_this, wdata, timeout)) {
        return 0;
    }

    /* Release previously displayed buffer (which is now the backbuffer) and lock a new one */
    if (wdata->next_bo != NULL) {
        KMSDRM_gbm_surface_release_buffer(wdata->gs, wdata->current_bo);
        /* SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Released GBM surface %p", (void *)wdata->next_bo); */

        wdata->current_bo = wdata->next_bo;
        wdata->next_bo = NULL;
    }

    if (!(_this->egl_data->eglSwapBuffers(_this->egl_data->egl_display, wdata->egl_surface))) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "eglSwapBuffers failed.");
        return 0;
    }

    if (wdata->current_bo == NULL) {
        wdata->current_bo = KMSDRM_gbm_surface_lock_front_buffer(wdata->gs);
        if (wdata->current_bo == NULL) {
            return 0;
        }
    }

    wdata->next_bo = KMSDRM_gbm_surface_lock_front_buffer(wdata->gs);
    if (wdata->next_bo == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not lock GBM surface front buffer");
        return 0;
    /* } else {
        SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "Locked GBM surface %p", (void *)wdata->next_bo); */
    }

    fb_info = KMSDRM_FBFromBO(_this, wdata->next_bo);
    if (fb_info == NULL) {
        return 0;
    }
    if (_this->egl_data->egl_swapinterval == 0) {
        /* Swap buffers instantly, possible tearing */
        /* SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "drmModeSetCrtc(%d, %u, %u, 0, 0, &%u, 1, &%ux%u@%u)",
            vdata->drm_fd, displaydata->crtc_id, fb_info->fb_id, vdata->saved_conn_id,
            displaydata->cur_mode.hdisplay, displaydata->cur_mode.vdisplay, displaydata->cur_mode.vrefresh); */
        ret = KMSDRM_drmModeSetCrtc(vdata->drm_fd, displaydata->crtc_id, fb_info->fb_id,
                                    0, 0, &vdata->saved_conn_id, 1, &displaydata->cur_mode);
        if(ret != 0) {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not pageflip with drmModeSetCrtc: %d", ret);
        }
    } else {
        /* Queue page flip at vsync */

	/* Have we already setup the CRTC to one of the GBM buffers? Do so if we have not,
           or FlipPage won't work in some cases. */
	if (!wdata->crtc_ready) {
            if(!KMSDRM_GLES_SetupCrtc(_this, window)) {
                SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not set up CRTC for doing vsync-ed pageflips");
                return 0;
            }
	}

        /* SDL_LogDebug(SDL_LOG_CATEGORY_VIDEO, "drmModePageFlip(%d, %u, %u, DRM_MODE_PAGE_FLIP_EVENT, &wdata->waiting_for_flip)",
            vdata->drm_fd, displaydata->crtc_id, fb_info->fb_id); */
        ret = KMSDRM_drmModePageFlip(vdata->drm_fd, displaydata->crtc_id, fb_info->fb_id,
                                     DRM_MODE_PAGE_FLIP_EVENT, &wdata->waiting_for_flip);
        if (ret == 0) {
            wdata->waiting_for_flip = SDL_TRUE;
        } else {
            SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not queue pageflip: %d", ret);
        }

        /* Wait immediately for vsync (as if we only had two buffers), for low input-lag scenarios.
           Run your SDL2 program with "SDL_KMSDRM_DOUBLE_BUFFER=1 <program_name>" to enable this. */
        if (wdata->double_buffer) {
            KMSDRM_WaitPageFlip(_this, wdata, -1);
        }
    }

    return 0;
}

SDL_EGL_MakeCurrent_impl(KMSDRM)

#endif /* SDL_VIDEO_DRIVER_KMSDRM && SDL_VIDEO_OPENGL_EGL */

/* vi: set ts=4 sw=4 expandtab: */
