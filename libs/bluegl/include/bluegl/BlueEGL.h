/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_BLUEGL_BLUEEGL_H
#define TNT_BLUEGL_BLUEEGL_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EGL_PLATFORM_SURFACELESS_MESA     0x31DD

extern void* bluegl_eglGetProcAddress;
extern void* bluegl_eglGetDisplay;
extern void* bluegl_eglInitialize;
extern void* bluegl_eglTerminate;
extern void* bluegl_eglBindAPI;
extern void* bluegl_eglCreateContext;
extern void* bluegl_eglDestroyContext;
extern void* bluegl_eglMakeCurrent;
extern void* bluegl_eglCreatePbufferSurface;
extern void* bluegl_eglCreateWindowSurface;
extern void* bluegl_eglDestroySurface;
extern void* bluegl_eglSurfaceAttrib;
extern void* bluegl_eglSwapBuffers;
extern void* bluegl_eglQueryString;
extern void* bluegl_eglChooseConfig;
extern void* bluegl_eglGetConfigAttrib;
extern void* bluegl_eglGetConfigs;
extern void* bluegl_eglGetCurrentContext;
extern void* bluegl_eglReleaseThread;
extern void* bluegl_eglGetError;
extern void* bluegl_eglGetPlatformDisplayEXT;

#if !defined(__ANDROID__)

#ifdef eglGetProcAddress
#undef eglGetProcAddress
#endif
#define eglGetProcAddress ((PFNEGLGETPROCADDRESSPROC)bluegl_eglGetProcAddress)

#ifdef eglGetDisplay
#undef eglGetDisplay
#endif
#define eglGetDisplay ((PFNEGLGETDISPLAYPROC)bluegl_eglGetDisplay)

#ifdef eglInitialize
#undef eglInitialize
#endif
#define eglInitialize ((PFNEGLINITIALIZEPROC)bluegl_eglInitialize)

#ifdef eglTerminate
#undef eglTerminate
#endif
#define eglTerminate ((PFNEGLTERMINATEPROC)bluegl_eglTerminate)

#ifdef eglBindAPI
#undef eglBindAPI
#endif
#define eglBindAPI ((PFNEGLBINDAPIPROC)bluegl_eglBindAPI)

#ifdef eglCreateContext
#undef eglCreateContext
#endif
#define eglCreateContext ((PFNEGLCREATECONTEXTPROC)bluegl_eglCreateContext)

#ifdef eglDestroyContext
#undef eglDestroyContext
#endif
#define eglDestroyContext ((PFNEGLDESTROYCONTEXTPROC)bluegl_eglDestroyContext)

#ifdef eglMakeCurrent
#undef eglMakeCurrent
#endif
#define eglMakeCurrent ((PFNEGLMAKECURRENTPROC)bluegl_eglMakeCurrent)

#ifdef eglCreatePbufferSurface
#undef eglCreatePbufferSurface
#endif
#define eglCreatePbufferSurface ((PFNEGLCREATEPBUFFERSURFACEPROC)bluegl_eglCreatePbufferSurface)

#ifdef eglCreateWindowSurface
#undef eglCreateWindowSurface
#endif
#define eglCreateWindowSurface ((PFNEGLCREATEWINDOWSURFACEPROC)bluegl_eglCreateWindowSurface)

#ifdef eglDestroySurface
#undef eglDestroySurface
#endif
#define eglDestroySurface ((PFNEGLDESTROYSURFACEPROC)bluegl_eglDestroySurface)

#ifdef eglSurfaceAttrib
#undef eglSurfaceAttrib
#endif
#define eglSurfaceAttrib ((PFNEGLSURFACEATTRIBPROC)bluegl_eglSurfaceAttrib)

#ifdef eglSwapBuffers
#undef eglSwapBuffers
#endif
#define eglSwapBuffers ((PFNEGLSWAPBUFFERSPROC)bluegl_eglSwapBuffers)

#ifdef eglQueryString
#undef eglQueryString
#endif
#define eglQueryString ((PFNEGLQUERYSTRINGPROC)bluegl_eglQueryString)

#ifdef eglChooseConfig
#undef eglChooseConfig
#endif
#define eglChooseConfig ((PFNEGLCHOOSECONFIGPROC)bluegl_eglChooseConfig)

#ifdef eglGetConfigAttrib
#undef eglGetConfigAttrib
#endif
#define eglGetConfigAttrib ((PFNEGLGETCONFIGATTRIBPROC)bluegl_eglGetConfigAttrib)

#ifdef eglGetConfigs
#undef eglGetConfigs
#endif
#define eglGetConfigs ((PFNEGLGETCONFIGSPROC)bluegl_eglGetConfigs)

#ifdef eglGetCurrentContext
#undef eglGetCurrentContext
#endif
#define eglGetCurrentContext ((PFNEGLGETCURRENTCONTEXTPROC)bluegl_eglGetCurrentContext)

#ifdef eglReleaseThread
#undef eglReleaseThread
#endif
#define eglReleaseThread ((PFNEGLRELEASETHREADPROC)bluegl_eglReleaseThread)

#ifdef eglGetError
#undef eglGetError
#endif
#define eglGetError ((PFNEGLGETERRORPROC)bluegl_eglGetError)

#define eglGetPlatformDisplayEXT ((PFNEGLGETPLATFORMDISPLAYEXTPROC)bluegl_eglGetPlatformDisplayEXT)

#endif // !defined(__ANDROID__)

#ifdef __cplusplus
}
#endif

#endif // TNT_BLUEGL_BLUEEGL_H
