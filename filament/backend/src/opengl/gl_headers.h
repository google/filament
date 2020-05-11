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

#ifndef TNT_FILAMENT_DRIVER_GL_HEADERS_H
#define TNT_FILAMENT_DRIVER_GL_HEADERS_H

#if defined(ANDROID) || defined(SWIFTSHADER) || defined(FILAMENT_USE_EXTERNAL_GLES3) || defined(__EMSCRIPTEN__)

    #include <GLES3/gl3.h>
    #include <GLES2/gl2ext.h>

    /* The Android NDK doesn't exposes extensions, fake it with eglGetProcAddress */
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
#if GL_EXT_multisampled_render_to_texture
        extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
        extern PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
#endif
#ifdef GL_KHR_debug
        extern PFNGLDEBUGMESSAGECALLBACKKHRPROC glDebugMessageCallbackKHR;
        extern PFNGLGETDEBUGMESSAGELOGKHRPROC glGetDebugMessageLogKHR;
#endif
#ifdef GL_EXT_disjoint_timer_query
        extern PFNGLGETQUERYOBJECTUI64VEXTPROC glGetQueryObjectui64v;
        #define GL_TIME_ELAPSED               0x88BF
#endif
    }

    // Prevent lots of #ifdef's between desktop and mobile by providing some suffix-free constants:
    #define GL_DEBUG_OUTPUT                   0x92E0
    #define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242

    #define GL_DEBUG_SEVERITY_HIGH            0x9146
    #define GL_DEBUG_SEVERITY_MEDIUM          0x9147
    #define GL_DEBUG_SEVERITY_LOW             0x9148
    #define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B

    #define GL_DEBUG_TYPE_MARKER              0x8268
    #define GL_DEBUG_TYPE_ERROR               0x824C
    #define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 0x824D
    #define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  0x824E
    #define GL_DEBUG_TYPE_PORTABILITY         0x824F
    #define GL_DEBUG_TYPE_PERFORMANCE         0x8250
    #define GL_DEBUG_TYPE_OTHER               0x8251

    #define glDebugMessageCallback            glext::glDebugMessageCallbackKHR

    using namespace glext;

#elif defined(IOS)

    #define GLES_SILENCE_DEPRECATION

    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>

    /* The iOS SDK only provides OpenGL ES headers up to 3.0. Filament works with OpenGL 3.0, but
     * requires the following 3.1 define in order to compile. */

    #define GL_TEXTURE_2D_MULTISAMPLE         0x9100
    #define GL_TIME_ELAPSED                   0x88BF

#else
    #include <bluegl/BlueGL.h>
#endif

// This is just to simplify the implementation (i.e. so we don't have to have #ifdefs everywhere)
#ifndef GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

#include "NullGLES.h"

#if (!defined(GL_ES_VERSION_3_0) && !defined(GL_VERSION_4_1))
#error "Minimum header version must be OpenGL ES 3.0 or OpenGL 4.1"
#endif

#if defined(GL_ES_VERSION_3_0)
#define GLES30_HEADERS true
#else
#define GLES30_HEADERS false
#endif

#if defined(GL_ES_VERSION_3_1)
#define GLES31_HEADERS true
#else
#define GLES31_HEADERS false
#endif

#if defined(GL_VERSION_4_1)
#define GL41_HEADERS true
#else
#define GL41_HEADERS false
#endif

#endif // TNT_FILAMENT_DRIVER_GL_HEADERS_H
