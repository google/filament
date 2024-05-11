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

#ifndef SDL_windowsopengl_h_
#define SDL_windowsopengl_h_

#if SDL_VIDEO_OPENGL_WGL

struct SDL_GLDriverData
{
    SDL_bool HAS_WGL_ARB_pixel_format;
    SDL_bool HAS_WGL_EXT_swap_control_tear;
    SDL_bool HAS_WGL_ARB_context_flush_control;
    SDL_bool HAS_WGL_ARB_create_context_robustness;
    SDL_bool HAS_WGL_ARB_create_context_no_error;

	/* Max version of OpenGL ES context that can be created if the
	   implementation supports WGL_EXT_create_context_es2_profile.
	   major = minor = 0 when unsupported.
	 */
	struct {
		int major;
		int minor;
	} es_profile_max_supported_version;

	void *(WINAPI * wglGetProcAddress) (const char *proc);
      HGLRC(WINAPI * wglCreateContext) (HDC hdc);
      BOOL(WINAPI * wglDeleteContext) (HGLRC hglrc);
      BOOL(WINAPI * wglMakeCurrent) (HDC hdc, HGLRC hglrc);
      BOOL(WINAPI * wglShareLists) (HGLRC hglrc1, HGLRC hglrc2);
      BOOL(WINAPI * wglChoosePixelFormatARB) (HDC hdc,
                                              const int *piAttribIList,
                                              const FLOAT * pfAttribFList,
                                              UINT nMaxFormats,
                                              int *piFormats,
                                              UINT * nNumFormats);
      BOOL(WINAPI * wglGetPixelFormatAttribivARB) (HDC hdc, int iPixelFormat,
                                                   int iLayerPlane,
                                                   UINT nAttributes,
                                                   const int *piAttributes,
                                                   int *piValues);
    BOOL (WINAPI * wglSwapIntervalEXT) (int interval);
    int (WINAPI * wglGetSwapIntervalEXT) (void);
};

/* OpenGL functions */
extern int WIN_GL_LoadLibrary(_THIS, const char *path);
extern void *WIN_GL_GetProcAddress(_THIS, const char *proc);
extern void WIN_GL_UnloadLibrary(_THIS);
extern SDL_bool WIN_GL_UseEGL(_THIS);
extern int WIN_GL_SetupWindow(_THIS, SDL_Window * window);
extern SDL_GLContext WIN_GL_CreateContext(_THIS, SDL_Window * window);
extern int WIN_GL_MakeCurrent(_THIS, SDL_Window * window,
                              SDL_GLContext context);
extern int WIN_GL_SetSwapInterval(_THIS, int interval);
extern int WIN_GL_GetSwapInterval(_THIS);
extern int WIN_GL_SwapWindow(_THIS, SDL_Window * window);
extern void WIN_GL_DeleteContext(_THIS, SDL_GLContext context);
extern void WIN_GL_InitExtensions(_THIS);
extern SDL_bool WIN_GL_SetPixelFormatFrom(_THIS, SDL_Window * fromWindow, SDL_Window * toWindow);

#ifndef WGL_ARB_pixel_format
#define WGL_NUMBER_PIXEL_FORMATS_ARB   0x2000
#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DRAW_TO_BITMAP_ARB         0x2002
#define WGL_ACCELERATION_ARB           0x2003
#define WGL_NEED_PALETTE_ARB           0x2004
#define WGL_NEED_SYSTEM_PALETTE_ARB    0x2005
#define WGL_SWAP_LAYER_BUFFERS_ARB     0x2006
#define WGL_SWAP_METHOD_ARB            0x2007
#define WGL_NUMBER_OVERLAYS_ARB        0x2008
#define WGL_NUMBER_UNDERLAYS_ARB       0x2009
#define WGL_TRANSPARENT_ARB            0x200A
#define WGL_TRANSPARENT_RED_VALUE_ARB  0x2037
#define WGL_TRANSPARENT_GREEN_VALUE_ARB 0x2038
#define WGL_TRANSPARENT_BLUE_VALUE_ARB 0x2039
#define WGL_TRANSPARENT_ALPHA_VALUE_ARB 0x203A
#define WGL_TRANSPARENT_INDEX_VALUE_ARB 0x203B
#define WGL_SHARE_DEPTH_ARB            0x200C
#define WGL_SHARE_STENCIL_ARB          0x200D
#define WGL_SHARE_ACCUM_ARB            0x200E
#define WGL_SUPPORT_GDI_ARB            0x200F
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_STEREO_ARB                 0x2012
#define WGL_PIXEL_TYPE_ARB             0x2013
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_RED_SHIFT_ARB              0x2016
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_GREEN_SHIFT_ARB            0x2018
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_BLUE_SHIFT_ARB             0x201A
#define WGL_ALPHA_BITS_ARB             0x201B
#define WGL_ALPHA_SHIFT_ARB            0x201C
#define WGL_ACCUM_BITS_ARB             0x201D
#define WGL_ACCUM_RED_BITS_ARB         0x201E
#define WGL_ACCUM_GREEN_BITS_ARB       0x201F
#define WGL_ACCUM_BLUE_BITS_ARB        0x2020
#define WGL_ACCUM_ALPHA_BITS_ARB       0x2021
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#define WGL_AUX_BUFFERS_ARB            0x2024
#define WGL_NO_ACCELERATION_ARB        0x2025
#define WGL_GENERIC_ACCELERATION_ARB   0x2026
#define WGL_FULL_ACCELERATION_ARB      0x2027
#define WGL_SWAP_EXCHANGE_ARB          0x2028
#define WGL_SWAP_COPY_ARB              0x2029
#define WGL_SWAP_UNDEFINED_ARB         0x202A
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_TYPE_COLORINDEX_ARB        0x202C
#endif

#ifndef WGL_ARB_multisample
#define WGL_SAMPLE_BUFFERS_ARB         0x2041
#define WGL_SAMPLES_ARB                0x2042
#endif

#endif /* SDL_VIDEO_OPENGL_WGL */

#endif /* SDL_windowsopengl_h_ */

/* vi: set ts=4 sw=4 expandtab: */
