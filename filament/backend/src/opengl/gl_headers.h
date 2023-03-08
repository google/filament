/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_GL_HEADERS_H
#define TNT_FILAMENT_BACKEND_OPENGL_GL_HEADERS_H

/*
 * Configuration we aim to support:
 *
 * GL 4.5 headers
 *      - GL 4.1 runtime (for macOS)
 *      - GL 4.5 runtime
 *
 * GLES 2.0 headers
 *      - GLES 2.0 runtime Android only
 *
 * GLES 3.0 headers
 *      - GLES 3.0 runtime iOS and WebGL2 only
 *
 * GLES 3.1 headers
 *      - GLES 2.0 runtime
 *      - GLES 3.0 runtime
 *      - GLES 3.1 runtime
 */


#if defined(__ANDROID__) || defined(FILAMENT_USE_EXTERNAL_GLES3) || defined(__EMSCRIPTEN__)

    #if defined(__EMSCRIPTEN__)
    #   include <GLES3/gl3.h>
    #else
    #   include <GLES3/gl31.h>
    #endif
    #include <GLES2/gl2ext.h>

#elif defined(IOS)

    #define GLES_SILENCE_DEPRECATION

    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>

#else

    // bluegl exposes symbols prefixed with bluegl_ to avoid clashing with clients that also link
    // against GL.
    // This header re-defines GL function names with the bluegl_ prefix.
    // For example:
    //   #define glFunction bluegl_glFunction
    // This header must come before <bluegl/BlueGL.h>.
    #include <bluegl/BlueGLDefines.h>
    #include <bluegl/BlueGL.h>

#endif

/* Validate the header configurations we aim to support */

#if defined(GL_VERSION_4_5)
#elif defined(GL_ES_VERSION_3_1)
#elif defined(GL_ES_VERSION_3_0)
#   if !defined(IOS) && !defined(__EMSCRIPTEN__)
#       error "GLES 3.0 headers only supported on iOS and WebGL2"
#   endif
#elif defined(GL_ES_VERSION_2_0)
#   if !defined(__ANDROID__)
#       error "GLES 2.0 headers only supported on Android"
#   endif
#else
#   error "Minimum header version must be OpenGL ES 2.0 or OpenGL 4.5"
#endif

/*
 * GLES extensions
 */

#if defined(GL_ES_VERSION_2_0)  // this basically means all versions of GLES

#if defined(IOS)

// iOS headers only provide prototypes, nothing to do.

#else

#define FILAMENT_IMPORT_ENTRY_POINTS

/* The Android NDK doesn't expose extensions, fake it with eglGetProcAddress */
namespace glext {
// importGLESExtensionsEntryPoints is thread-safe and can be called multiple times.
// it is currently called from PlatformEGL.
void importGLESExtensionsEntryPoints();

#ifdef GL_OES_EGL_image
extern PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
#endif
#ifdef GL_EXT_debug_marker
extern PFNGLINSERTEVENTMARKEREXTPROC glInsertEventMarkerEXT;
extern PFNGLPUSHGROUPMARKEREXTPROC glPushGroupMarkerEXT;
extern PFNGLPOPGROUPMARKEREXTPROC glPopGroupMarkerEXT;
#endif
#ifdef GL_EXT_multisampled_render_to_texture
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
#endif
#ifdef GL_KHR_debug
extern PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
extern PFNGLGETDEBUGMESSAGELOGKHRPROC glGetDebugMessageLogKHR;
#endif
#ifdef GL_EXT_clip_control
extern PFNGLCLIPCONTROLEXTPROC glClipControlEXT;
#endif
#ifdef GL_EXT_disjoint_timer_query
extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v;
#endif
#if defined(__ANDROID__)
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
#endif
} // namespace glext

using namespace glext;

#endif

// Prevent lots of #ifdef's between desktop and mobile

#ifdef GL_EXT_disjoint_timer_query
#   define GL_TIME_ELAPSED                          GL_TIME_ELAPSED_EXT
#endif

#ifdef GL_EXT_clip_control
#   define GL_LOWER_LEFT                            GL_LOWER_LEFT_EXT
#   define GL_ZERO_TO_ONE                           GL_ZERO_TO_ONE_EXT
#endif

// we need GL_TEXTURE_CUBE_MAP_ARRAY defined, but we won't use it if the extension/feature
// is not available.
#if defined(GL_EXT_texture_cube_map_array)
#   define GL_TEXTURE_CUBE_MAP_ARRAY                GL_TEXTURE_CUBE_MAP_ARRAY_EXT
#else
#   define GL_TEXTURE_CUBE_MAP_ARRAY                0x9009
#endif

#if defined(GL_KHR_debug)
#   define GL_DEBUG_OUTPUT                          GL_DEBUG_OUTPUT_KHR
#   define GL_DEBUG_OUTPUT_SYNCHRONOUS              GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR
#   define GL_DEBUG_SEVERITY_HIGH                   GL_DEBUG_SEVERITY_HIGH_KHR
#   define GL_DEBUG_SEVERITY_MEDIUM                 GL_DEBUG_SEVERITY_MEDIUM_KHR
#   define GL_DEBUG_SEVERITY_LOW                    GL_DEBUG_SEVERITY_LOW_KHR
#   define GL_DEBUG_SEVERITY_NOTIFICATION           GL_DEBUG_SEVERITY_NOTIFICATION_KHR
#   define GL_DEBUG_TYPE_MARKER                     GL_DEBUG_TYPE_MARKER_KHR
#   define GL_DEBUG_TYPE_ERROR                      GL_DEBUG_TYPE_ERROR_KHR
#   define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR        GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR
#   define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR         GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR
#   define GL_DEBUG_TYPE_PORTABILITY                GL_DEBUG_TYPE_PORTABILITY_KHR
#   define GL_DEBUG_TYPE_PERFORMANCE                GL_DEBUG_TYPE_PERFORMANCE_KHR
#   define GL_DEBUG_TYPE_OTHER                      GL_DEBUG_TYPE_OTHER_KHR
#   define glDebugMessageCallback                   glDebugMessageCallbackKHR
#endif

#endif // GL_ES_VERSION_2_0

// This is just to simplify the implementation (i.e. so we don't have to have #ifdefs everywhere)
#ifndef GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

// This is an odd duck function that exists in WebGL 2.0 but not in OpenGL ES.
#if defined(__EMSCRIPTEN__)
extern "C" {
void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
}
#endif

#if defined(GL_ES_VERSION_2_0)
#   define BACKEND_OPENGL_VERSION_GLES
#elif defined(GL_VERSION_4_5)
#   define BACKEND_OPENGL_VERSION_GL
#else
#   error "Unsupported header version"
#endif

#if defined(GL_VERSION_4_5) || defined(GL_ES_VERSION_3_1)
#   define BACKEND_OPENGL_LEVEL_GLES31
#   ifdef __EMSCRIPTEN__
#       error "__EMSCRIPTEN__ shouldn't be defined with GLES 3.1 headers"
#   endif
#endif
#if defined(GL_VERSION_4_5) || defined(GL_ES_VERSION_3_0)
#   define BACKEND_OPENGL_LEVEL_GLES30
#endif
#if defined(GL_VERSION_4_5) || defined(GL_ES_VERSION_2_0)
#   define BACKEND_OPENGL_LEVEL_GLES20
#endif

#include "NullGLES.h"

#endif // TNT_FILAMENT_BACKEND_OPENGL_GL_HEADERS_H
