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

#if defined(ANDROID) || defined(USE_EXTERNAL_GLES3) || defined(__EMSCRIPTEN__)

    #include <GLES3/gl31.h>
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
    }

    using namespace glext;

#elif defined(IOS)

    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>

    /* The iOS SDK only provides OpenGL ES headers up to 3.0. Filament works with OpenGL 3.0, but
     * requires 3.1 headers in order to compile. We fake it by adding the necessary 3.1 declarations
     * below. */

    #define GL_ES_VERSION_3_1 1
    #define GL_TEXTURE_2D_MULTISAMPLE         0x9100

    void glTexStorage2DMultisample (GLenum target, GLsizei samples, GLenum internalformat,
            GLsizei width, GLsizei height, GLboolean fixedsamplelocations);

    namespace glext {
        void glFramebufferTexture2DMultisampleEXT (GLenum target, GLenum attachment,
                GLenum textarget, GLuint texture, GLint level, GLsizei samples);

        void glRenderbufferStorageMultisampleEXT (GLenum target, GLsizei samples,
                GLenum internalformat, GLsizei width, GLsizei height);
    }

#else
    #include <bluegl/BlueGL.h>
#endif

// This is just to simplify the implementation (i.e. so we don't have to have #ifdefs everywhere)
#ifndef GL_OES_EGL_image_external
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#endif

#include "NullGLES.h"

#if (!defined(GL_ES_VERSION_3_1) && !defined(GL_VERSION_4_1))
#error "Minimum header version must be OpenGL ES 3.1 or OpenGL 4.1"
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
