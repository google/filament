/* Copyright 2006-2020 The Khronos Group Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __egl_h_
#define __egl_h_

#include <GLES/gl.h>
#include <GLES/egltypes.h>

/*
** egltypes.h is platform dependent. It defines:
**
**     - EGL types and resources
**     - Native types
**     - EGL and native handle values
**
** EGL types and resources are to be typedef'ed with appropriate platform
** dependent resource handle types. EGLint must be an integer of at least
** 32-bit.
**
** NativeDisplayType, NativeWindowType and NativePixmapType are to be
** replaced with corresponding types of the native window system in egl.h.
**
** EGL and native handle values must match their types.
**
** Example egltypes.h:
*/

#if 0

#include <sys/types.h>
#include <native_window_system.h>

/*
** Types and resources
*/
typedef int EGLBoolean;
typedef int32_t EGLint;
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;

/*
** EGL and native handle values
*/
#define EGL_DEFAULT_DISPLAY ((NativeDisplayType)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_SURFACE ((EGLSurface)0)

#endif

/*
** Versioning and extensions
*/
#define EGL_VERSION_1_0		       1
#define EGL_VERSION_1_1		       1

/*
** Boolean
*/
#define EGL_FALSE		       0
#define EGL_TRUE		       1

/*
** Errors
*/
#define EGL_SUCCESS		       0x3000
#define EGL_NOT_INITIALIZED	       0x3001
#define EGL_BAD_ACCESS		       0x3002
#define EGL_BAD_ALLOC		       0x3003
#define EGL_BAD_ATTRIBUTE	       0x3004
#define EGL_BAD_CONFIG		       0x3005
#define EGL_BAD_CONTEXT		       0x3006
#define EGL_BAD_CURRENT_SURFACE        0x3007
#define EGL_BAD_DISPLAY		       0x3008
#define EGL_BAD_MATCH		       0x3009
#define EGL_BAD_NATIVE_PIXMAP	       0x300A
#define EGL_BAD_NATIVE_WINDOW	       0x300B
#define EGL_BAD_PARAMETER	       0x300C
#define EGL_BAD_SURFACE		       0x300D
#define EGL_CONTEXT_LOST	       0x300E
/* 0x300F - 0x301F reserved for additional errors. */

/*
** Config attributes
*/
#define EGL_BUFFER_SIZE		       0x3020
#define EGL_ALPHA_SIZE		       0x3021
#define EGL_BLUE_SIZE		       0x3022
#define EGL_GREEN_SIZE		       0x3023
#define EGL_RED_SIZE		       0x3024
#define EGL_DEPTH_SIZE		       0x3025
#define EGL_STENCIL_SIZE	       0x3026
#define EGL_CONFIG_CAVEAT	       0x3027
#define EGL_CONFIG_ID		       0x3028
#define EGL_LEVEL		       0x3029
#define EGL_MAX_PBUFFER_HEIGHT	       0x302A
#define EGL_MAX_PBUFFER_PIXELS	       0x302B
#define EGL_MAX_PBUFFER_WIDTH	       0x302C
#define EGL_NATIVE_RENDERABLE	       0x302D
#define EGL_NATIVE_VISUAL_ID	       0x302E
#define EGL_NATIVE_VISUAL_TYPE	       0x302F
/*#define EGL_PRESERVED_RESOURCES	 0x3030*/
#define EGL_SAMPLES		       0x3031
#define EGL_SAMPLE_BUFFERS	       0x3032
#define EGL_SURFACE_TYPE	       0x3033
#define EGL_TRANSPARENT_TYPE	       0x3034
#define EGL_TRANSPARENT_BLUE_VALUE     0x3035
#define EGL_TRANSPARENT_GREEN_VALUE    0x3036
#define EGL_TRANSPARENT_RED_VALUE      0x3037
#define EGL_NONE		       0x3038	/* Also a config value */
#define EGL_BIND_TO_TEXTURE_RGB        0x3039
#define EGL_BIND_TO_TEXTURE_RGBA       0x303A
#define EGL_MIN_SWAP_INTERVAL	       0x303B
#define EGL_MAX_SWAP_INTERVAL	       0x303C

/*
** Config values
*/
#define EGL_DONT_CARE		       ((EGLint) -1)

#define EGL_SLOW_CONFIG		       0x3050	/* EGL_CONFIG_CAVEAT value */
#define EGL_NON_CONFORMANT_CONFIG      0x3051	/* " */
#define EGL_TRANSPARENT_RGB	       0x3052	/* EGL_TRANSPARENT_TYPE value */
#define EGL_NO_TEXTURE		       0x305C	/* EGL_TEXTURE_FORMAT/TARGET value */
#define EGL_TEXTURE_RGB		       0x305D	/* EGL_TEXTURE_FORMAT value */
#define EGL_TEXTURE_RGBA	       0x305E	/* " */
#define EGL_TEXTURE_2D		       0x305F	/* EGL_TEXTURE_TARGET value */

/*
** Config attribute mask bits
*/
#define EGL_PBUFFER_BIT		       0x01	/* EGL_SURFACE_TYPE mask bit */
#define EGL_PIXMAP_BIT		       0x02	/* " */
#define EGL_WINDOW_BIT		       0x04	/* " */

/*
** String names
*/
#define EGL_VENDOR		       0x3053	/* eglQueryString target */
#define EGL_VERSION		       0x3054	/* " */
#define EGL_EXTENSIONS		       0x3055	/* " */

/*
** Surface attributes
*/
#define EGL_HEIGHT		       0x3056
#define EGL_WIDTH		       0x3057
#define EGL_LARGEST_PBUFFER	       0x3058
#define EGL_TEXTURE_FORMAT	       0x3080	/* For pbuffers bound as textures */
#define EGL_TEXTURE_TARGET	       0x3081	/* " */
#define EGL_MIPMAP_TEXTURE	       0x3082	/* " */
#define EGL_MIPMAP_LEVEL	       0x3083	/* " */

/*
** BindTexImage / ReleaseTexImage buffer target
*/
#define EGL_BACK_BUFFER		       0x3084

/*
** Current surfaces
*/
#define EGL_DRAW		       0x3059
#define EGL_READ		       0x305A

/*
** Engines
*/
#define EGL_CORE_NATIVE_ENGINE	       0x305B

/* 0x305C-0x3FFFF reserved for future use */

/*
** Functions
*/
#ifdef __cplusplus
extern "C" {
#endif

GLAPI EGLint APIENTRY eglGetError (void);

GLAPI EGLDisplay APIENTRY eglGetDisplay (NativeDisplayType display);
GLAPI EGLBoolean APIENTRY eglInitialize (EGLDisplay dpy, EGLint *major, EGLint *minor);
GLAPI EGLBoolean APIENTRY eglTerminate (EGLDisplay dpy);
GLAPI const char * APIENTRY eglQueryString (EGLDisplay dpy, EGLint name);
GLAPI void (* APIENTRY eglGetProcAddress (const char *procname))();

GLAPI EGLBoolean APIENTRY eglGetConfigs (EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
GLAPI EGLBoolean APIENTRY eglChooseConfig (EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
GLAPI EGLBoolean APIENTRY eglGetConfigAttrib (EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);

GLAPI EGLSurface APIENTRY eglCreateWindowSurface (EGLDisplay dpy, EGLConfig config, NativeWindowType window, const EGLint *attrib_list);
GLAPI EGLSurface APIENTRY eglCreatePixmapSurface (EGLDisplay dpy, EGLConfig config, NativePixmapType pixmap, const EGLint *attrib_list);
GLAPI EGLSurface APIENTRY eglCreatePbufferSurface (EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
GLAPI EGLBoolean APIENTRY eglDestroySurface (EGLDisplay dpy, EGLSurface surface);
GLAPI EGLBoolean APIENTRY eglQuerySurface (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);

/* EGL 1.1 render-to-texture APIs */
GLAPI EGLBoolean APIENTRY eglSurfaceAttrib (EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
GLAPI EGLBoolean APIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
GLAPI EGLBoolean APIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);

/* EGL 1.1 swap control API */
GLAPI EGLBoolean APIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval);

GLAPI EGLContext APIENTRY eglCreateContext (EGLDisplay dpy, EGLConfig config, EGLContext share_list, const EGLint *attrib_list);
GLAPI EGLBoolean APIENTRY eglDestroyContext (EGLDisplay dpy, EGLContext ctx);
GLAPI EGLBoolean APIENTRY eglMakeCurrent (EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
GLAPI EGLContext APIENTRY eglGetCurrentContext (void);
GLAPI EGLSurface APIENTRY eglGetCurrentSurface (EGLint readdraw);
GLAPI EGLDisplay APIENTRY eglGetCurrentDisplay (void);
GLAPI EGLBoolean APIENTRY eglQueryContext (EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);

GLAPI EGLBoolean APIENTRY eglWaitGL (void);
GLAPI EGLBoolean APIENTRY eglWaitNative (EGLint engine);
GLAPI EGLBoolean APIENTRY eglSwapBuffers (EGLDisplay dpy, EGLSurface draw);
GLAPI EGLBoolean APIENTRY eglCopyBuffers (EGLDisplay dpy, EGLSurface surface, NativePixmapType target);

#ifdef __cplusplus
}
#endif

#endif /* ___egl_h_ */
