/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef FILAMENT_DRIVER_OPENGL_COCOA_EXTERNAL_IMAGE
#define FILAMENT_DRIVER_OPENGL_COCOA_EXTERNAL_IMAGE

#include <backend/platforms/OpenGLPlatform.h>
#include <CoreVideo/CoreVideo.h>

#include "../gl_headers.h"

namespace filament::backend {

class CocoaExternalImage final : public OpenGLPlatform::ExternalTexture {
public:
    /**
     * GL objects that can be shared across multiple instances of CocoaExternalImage.
     */
    class SharedGl {
    public:
        SharedGl() noexcept;
        ~SharedGl() noexcept;

        SharedGl(const SharedGl&) = delete;
        SharedGl& operator=(const SharedGl&) = delete;

        GLuint program = 0;
        GLuint sampler = 0;
        GLuint fragmentShader = 0;
        GLuint vertexShader = 0;
    };

    CocoaExternalImage(const CVOpenGLTextureCacheRef textureCache,
            const SharedGl& sharedGl) noexcept;
    ~CocoaExternalImage() noexcept;

    /**
     * Set this external image to the passed-in CVPixelBuffer.
     * Afterwards, calling glGetTexture returns the GL texture name backed by the CVPixelBuffer.
     */
    bool set(CVPixelBufferRef p) noexcept;

    GLuint getGlTexture() const noexcept;
    GLuint getInternalFormat() const noexcept;
    GLuint getTarget() const noexcept;

private:
    void release() noexcept;
    CVOpenGLTextureRef createTextureFromImage(CVPixelBufferRef image) noexcept;
    GLuint encodeCopyRectangleToTexture2D(GLuint rectangle, size_t width, size_t height) noexcept;

    class State {
    public:
        void save() noexcept;
        void restore() noexcept;

    private:
        GLint activeTexture = 0;
        GLint textureBinding = { 0 };
        GLint samplerBinding = { 0 };
        GLint framebuffer = 0;
        GLint viewport[4] = { 0 };
        GLint vertexAttrib = 0;
    } mState;

    GLuint mFBO = 0;
    const SharedGl& mSharedGl;
    GLuint mRgbaTexture = 0;

    const CVOpenGLTextureCacheRef mTextureCache;
    CVPixelBufferRef mImage = nullptr;
    CVOpenGLTextureRef mTexture = nullptr;
};

} // namespace filament::backend

#endif
