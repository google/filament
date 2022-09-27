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

#if defined(__ANDROID__) || defined(FILAMENT_USE_EXTERNAL_GLES3) || defined(__EMSCRIPTEN__)

    #if defined(__EMSCRIPTEN__)
    #   include <GLES3/gl3.h>
    #else
    #   include <GLES3/gl31.h>
    #endif
    #include <GLES2/gl2ext.h>

    /* The Android NDK doesn't expose extensions, fake it with eglGetProcAddress */
    namespace glext {
        // importGLESExtensionsEntryPoints is thread-safe and can be called multiple times.
        // it is currently called from PlatformEGL.
        void importGLESExtensionsEntryPoints();

#ifdef GL_QCOM_tiled_rendering
        extern PFNGLSTARTTILINGQCOMPROC glStartTilingQCOM;
        extern PFNGLENDTILINGQCOMPROC glEndTilingQCOM;
#endif
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
#ifdef GL_EXT_disjoint_timer_query
        extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v;
#endif
#ifdef GL_EXT_clip_control
        extern PFNGLCLIPCONTROLEXTPROC glClipControl;
#endif
    }

    using namespace glext;

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

#if (!defined(GL_ES_VERSION_2_0) && !defined(GL_VERSION_4_1))
#error "Minimum header version must be OpenGL ES 2.0 or OpenGL 4.1"
#endif


/*
 * Since we need ES3.1 headers and iOS only has ES3.0, we also define the constants we
 * need to avoid many #ifdef in the actual code.
 */

#if defined(GL_ES_VERSION_2_0)
#ifdef GL_EXT_disjoint_timer_query
#       define GL_TIME_ELAPSED              GL_TIME_ELAPSED_EXT
#endif

#ifdef GL_EXT_clip_control
#   ifndef GL_LOWER_LEFT
#      define GL_LOWER_LEFT                 GL_LOWER_LEFT_EXT
#   endif
#   ifndef GL_ZERO_TO_ONE
#      define GL_ZERO_TO_ONE                GL_ZERO_TO_ONE_EXT
#   endif
#endif

#ifndef GL_TEXTURE_CUBE_MAP_ARRAY
#   define GL_TEXTURE_CUBE_MAP_ARRAY        0x9009
#endif

// Prevent lots of #ifdef's between desktop and mobile

#if defined(GL_KHR_debug)
#   ifndef GL_DEBUG_OUTPUT
#      define GL_DEBUG_OUTPUT                   GL_DEBUG_OUTPUT_KHR
#   endif
#   ifndef GL_DEBUG_OUTPUT_SYNCHRONOUS
#      define GL_DEBUG_OUTPUT_SYNCHRONOUS       GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR
#   endif

#   ifndef GL_DEBUG_SEVERITY_HIGH
#      define GL_DEBUG_SEVERITY_HIGH            GL_DEBUG_SEVERITY_HIGH_KHR
#   endif
#   ifndef GL_DEBUG_SEVERITY_MEDIUM
#      define GL_DEBUG_SEVERITY_MEDIUM          GL_DEBUG_SEVERITY_MEDIUM_KHR
#   endif
#   ifndef GL_DEBUG_SEVERITY_LOW
#      define GL_DEBUG_SEVERITY_LOW             GL_DEBUG_SEVERITY_LOW_KHR
#   endif
#   ifndef GL_DEBUG_SEVERITY_NOTIFICATION
#      define GL_DEBUG_SEVERITY_NOTIFICATION    GL_DEBUG_SEVERITY_NOTIFICATION_KHR
#   endif

#   ifndef GL_DEBUG_TYPE_MARKER
#      define GL_DEBUG_TYPE_MARKER              GL_DEBUG_TYPE_MARKER_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_ERROR
#      define GL_DEBUG_TYPE_ERROR               GL_DEBUG_TYPE_ERROR_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#      define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#      define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_PORTABILITY
#      define GL_DEBUG_TYPE_PORTABILITY         GL_DEBUG_TYPE_PORTABILITY_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_PERFORMANCE
#      define GL_DEBUG_TYPE_PERFORMANCE         GL_DEBUG_TYPE_PERFORMANCE_KHR
#   endif
#   ifndef GL_DEBUG_TYPE_OTHER
#      define GL_DEBUG_TYPE_OTHER               GL_DEBUG_TYPE_OTHER_KHR
#   endif

#   define glDebugMessageCallback            glDebugMessageCallbackKHR
#endif

/* The iOS SDK only provides OpenGL ES headers up to 3.0. Filament works with OpenGL 3.0, but
 * requires ES3.1 headers */
#if !defined(GL_ES_VERSION_3_1)
    #define GL_SHADER_STORAGE_BUFFER                0x90D2
    #define GL_COMPUTE_SHADER                       0x91B9

    #define GL_TEXTURE_2D_MULTISAMPLE               0x9100
    #define GL_TIME_ELAPSED                         0x88BF

    #define GL_TEXTURE_BINDING_CUBE_MAP_ARRAY       0x900A
    #define GL_SAMPLER_CUBE_MAP_ARRAY               0x900C
    #define GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW        0x900D
    #define GL_INT_SAMPLER_CUBE_MAP_ARRAY           0x900E
    #define GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY  0x900F
    #define GL_IMAGE_CUBE_MAP_ARRAY                 0x9054
    #define GL_INT_IMAGE_CUBE_MAP_ARRAY             0x905F
    #define GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY    0x906A

#endif
#endif // GL_ES_VERSION_2_0

// This is an odd duck function that exists in WebGL 2.0 but not in OpenGL ES.
#if defined(__EMSCRIPTEN__)
extern "C" {
void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
}
#endif


#define BACKEND_OPENGL_VERSION_GLES     0
#define BACKEND_OPENGL_VERSION_GL       1
#if defined(GL_ES_VERSION_2_0)
#   define BACKEND_OPENGL_VERSION      BACKEND_OPENGL_VERSION_GLES
#elif defined(GL_VERSION_4_1)
#   define BACKEND_OPENGL_VERSION      BACKEND_OPENGL_VERSION_GL
#endif

#define BACKEND_OPENGL_LEVEL_GLES20     0
#define BACKEND_OPENGL_LEVEL_GLES30     1
#define BACKEND_OPENGL_LEVEL_GLES31     2

#if defined(GL_VERSION_4_1)
#   define BACKEND_OPENGL_LEVEL        BACKEND_OPENGL_LEVEL_GLES30
#endif

#if defined(GL_ES_VERSION_3_1)
#   define BACKEND_OPENGL_LEVEL        BACKEND_OPENGL_LEVEL_GLES31
#elif defined(GL_ES_VERSION_3_0)
#   define BACKEND_OPENGL_LEVEL        BACKEND_OPENGL_LEVEL_GLES30
#elif defined(GL_ES_VERSION_2_0)
#   define BACKEND_OPENGL_LEVEL        BACKEND_OPENGL_LEVEL_GLES20
#endif

// This is just to simplify the implementation (i.e. so we don't have to have #ifdefs everywhere)
#ifndef GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

#include "NullGLES.h"

#endif // TNT_FILAMENT_BACKEND_OPENGL_GL_HEADERS_H
