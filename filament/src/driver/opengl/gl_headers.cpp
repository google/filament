/*
 * Copyright (C) 2016 The Android Open Source Project
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

#if defined(ANDROID) || defined(USE_EXTERNAL_GLES3) || defined(__EMSCRIPTEN__)

#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>

namespace glext {
#ifdef GL_QCOM_tiled_rendering
PFNGLSTARTTILINGQCOMPROC glStartTilingQCOM;
PFNGLENDTILINGQCOMPROC glEndTilingQCOM;
#endif
#ifdef GL_OES_EGL_image
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES;
#endif
#if GL_EXT_debug_marker
PFNGLINSERTEVENTMARKEREXTPROC glInsertEventMarkerEXT;
PFNGLPUSHGROUPMARKEREXTPROC glPushGroupMarkerEXT;
PFNGLPOPGROUPMARKEREXTPROC glPopGroupMarkerEXT;
#endif
#if GL_EXT_multisampled_render_to_texture
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT;
#endif
}

using namespace glext;

namespace filament {
static class GLESExtInit {
public:
    GLESExtInit() noexcept {
#ifdef GL_QCOM_tiled_rendering
        glStartTilingQCOM =
                (PFNGLSTARTTILINGQCOMPROC)eglGetProcAddress(
                        "glStartTilingQCOM");

        glEndTilingQCOM =
                (PFNGLENDTILINGQCOMPROC)eglGetProcAddress(
                        "glEndTilingQCOM");
#endif

#ifdef GL_OES_EGL_image
        glEGLImageTargetTexture2DOES =
                (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress(
                        "glEGLImageTargetTexture2DOES");
#endif

#if GL_EXT_debug_marker
        glInsertEventMarkerEXT =
                (PFNGLINSERTEVENTMARKEREXTPROC)eglGetProcAddress(
                        "glInsertEventMarkerEXT");

        glPushGroupMarkerEXT =
                (PFNGLPUSHGROUPMARKEREXTPROC)eglGetProcAddress(
                        "glPushGroupMarkerEXT");

        glPopGroupMarkerEXT =
                (PFNGLPOPGROUPMARKEREXTPROC)eglGetProcAddress(
                        "glPopGroupMarkerEXT");
#endif
#if GL_EXT_multisampled_render_to_texture
        glFramebufferTexture2DMultisampleEXT =
                (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress(
                        "glFramebufferTexture2DMultisampleEXT");
#endif
    }
} instance;
} // namespace filament

#endif

#if defined(IOS)

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include <utils/Panic.h>

void glTexStorage2DMultisample (GLenum target, GLsizei samples, GLenum internalformat,
            GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
    PANIC_PRECONDITION("glTexStorage2DMultisample should not be called on iOS.");
}

namespace glext {
    void glFramebufferTexture2DMultisampleEXT (GLenum target, GLenum attachment,
            GLenum textarget, GLuint texture, GLint level, GLsizei samples) {
        PANIC_PRECONDITION("glFramebufferTexture2DMultisampleEXT should not be called on iOS.");
    }
}

#endif
