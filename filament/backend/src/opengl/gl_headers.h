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
//    #   include <GLES2/gl2.h>
    #   include <GLES3/gl31.h>
    #endif
    #include <GLES2/gl2ext.h>

    // For development and debugging purpose only, we want to support compiling this backend
    // with ES2 only headers, in this case (i.e. we have VERSION_2 but not VERSION_3+),
    // we define FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2 with the purpose of compiling out
    // code that cannot be compiled with ES2 headers. In production, this code is compiled in but
    // is never executed thanks to runtime checks or asserts.
    #if defined(GL_ES_VERSION_2_0) && !defined(GL_ES_VERSION_3_0)
    #   define FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2
    #endif

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

#ifndef __EMSCRIPTEN__
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
extern PFNGLGENQUERIESEXTPROC glGenQueriesEXT;
extern PFNGLDELETEQUERIESEXTPROC glDeleteQueriesEXT;
extern PFNGLBEGINQUERYEXTPROC glBeginQueryEXT;
extern PFNGLENDQUERYEXTPROC glEndQueryEXT;
extern PFNGLGETQUERYOBJECTUIVEXTPROC glGetQueryObjectuivEXT;
extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64vEXT;
#endif
#ifdef GL_OES_vertex_array_object
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
#endif
#ifdef GL_EXT_discard_framebuffer
extern PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT;
#endif
#ifdef GL_KHR_parallel_shader_compile
extern PFNGLMAXSHADERCOMPILERTHREADSKHRPROC glMaxShaderCompilerThreadsKHR;
#endif
#if defined(__ANDROID__) && !defined(FILAMENT_SILENCE_NOT_SUPPORTED_BY_ES2)
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLFRAMEBUFFERTEXTUREMULTIVIEWOVRPROC glFramebufferTextureMultiviewOVR;
#endif
#endif // __EMSCRIPTEN__
} // namespace glext

using namespace glext;

#endif

// Prevent lots of #ifdef's between desktop and mobile

#ifdef GL_EXT_disjoint_timer_query
#   define GL_TIME_ELAPSED                          GL_TIME_ELAPSED_EXT
#   ifndef GL_ES_VERSION_3_0
#       define GL_QUERY_RESULT_AVAILABLE            GL_QUERY_RESULT_AVAILABLE_EXT
#       define GL_QUERY_RESULT                      GL_QUERY_RESULT_EXT
#   endif
#endif

#ifdef GL_EXT_clip_control
#   define GL_LOWER_LEFT                            GL_LOWER_LEFT_EXT
#   define GL_ZERO_TO_ONE                           GL_ZERO_TO_ONE_EXT
#endif

#ifdef GL_KHR_parallel_shader_compile
#   define GL_COMPLETION_STATUS                     GL_COMPLETION_STATUS_KHR
#else
#   define GL_COMPLETION_STATUS                     0x91B1
#endif

// we need GL_TEXTURE_CUBE_MAP_ARRAY defined, but we won't use it if the extension/feature
// is not available.
#if defined(GL_EXT_texture_cube_map_array)
#   define GL_TEXTURE_CUBE_MAP_ARRAY                GL_TEXTURE_CUBE_MAP_ARRAY_EXT
#else
#   define GL_TEXTURE_CUBE_MAP_ARRAY                0x9009
#endif

#if defined(GL_EXT_clip_cull_distance)
#   define GL_CLIP_DISTANCE0                        GL_CLIP_DISTANCE0_EXT
#   define GL_CLIP_DISTANCE1                        GL_CLIP_DISTANCE1_EXT
#else
#   define GL_CLIP_DISTANCE0                        0x3000
#   define GL_CLIP_DISTANCE1                        0x3001
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

// token that exist in ES3 core but are extensions (mandatory or not) in ES2
#ifndef GL_ES_VERSION_3_0
#   ifdef GL_OES_vertex_array_object
#       define GL_VERTEX_ARRAY_BINDING             GL_VERTEX_ARRAY_BINDING_OES
#   endif
#   ifdef GL_OES_rgb8_rgba8
#       define GL_RGB8                             0x8051
#       define GL_RGBA8                            0x8058
#   endif
#   ifdef GL_OES_depth24
#       define GL_DEPTH_COMPONENT24                GL_DEPTH_COMPONENT24_OES
#   endif
#   ifdef GL_EXT_discard_framebuffer
#       define GL_COLOR                             GL_COLOR_EXT
#       define GL_DEPTH                             GL_DEPTH_EXT
#       define GL_STENCIL                           GL_STENCIL_EXT
#   endif
#   ifdef GL_OES_packed_depth_stencil
#       define GL_DEPTH_STENCIL                     GL_DEPTH_STENCIL_OES
#       define GL_UNSIGNED_INT_24_8                 GL_UNSIGNED_INT_24_8_OES
#       define GL_DEPTH24_STENCIL8                  GL_DEPTH24_STENCIL8_OES
#   endif
#endif

#else // All version OpenGL below

#ifdef GL_ARB_parallel_shader_compile
#   define GL_COMPLETION_STATUS                     GL_COMPLETION_STATUS_ARB
#else
#   define GL_COMPLETION_STATUS                     0x91B1
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

#ifdef GL_ES_VERSION_2_0
#   ifndef IOS
#      ifndef GL_OES_vertex_array_object
#          error "Headers with GL_OES_vertex_array_object are mandatory unless on iOS"
#      endif
#      ifndef GL_EXT_disjoint_timer_query
#          error "Headers with GL_EXT_disjoint_timer_query are mandatory unless on iOS"
#      endif
#      ifndef GL_OES_rgb8_rgba8
#          error "Headers with GL_OES_rgb8_rgba8 are mandatory unless on iOS"
#      endif
#   endif
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
