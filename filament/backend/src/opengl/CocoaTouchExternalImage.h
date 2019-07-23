/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef FILAMENT_DRIVER_OPENGL_COCOA_TOUCH_EXTERNAL_IMAGE
#define FILAMENT_DRIVER_OPENGL_COCOA_TOUCH_EXTERNAL_IMAGE

#include "gl_headers.h"

#include <CoreVideo/CoreVideo.h>

namespace filament {

class CocoaTouchExternalImage final {
public:

    /**
     * GL objects that can be shared across multiple instances of CocoaTouchExternalImage.
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

    CocoaTouchExternalImage(const CVOpenGLESTextureCacheRef textureCache,
            const SharedGl& sharedGl) noexcept;
    ~CocoaTouchExternalImage() noexcept;

    /**
     * Set this external image to the passed-in CVPixelBuffer.
     * Afterwards, calling glGetTexture returns the GL texture name backed by the CVPixelBuffer.
     *
     * Calling set with a YCbCr image performs a render pass to convert the image from YCbCr to RGB.
     */
    bool set(CVPixelBufferRef p) noexcept;

    GLuint getGlTexture() const noexcept;
    GLuint getInternalFormat() const noexcept;
    GLuint getTarget() const noexcept;

private:

    void release() noexcept;
    CVOpenGLESTextureRef createTextureFromImage(CVPixelBufferRef image, GLuint glFormat,
            size_t plane) noexcept;
    GLuint encodeColorConversionPass(GLuint yPlaneTexture, GLuint colorTexture, size_t width,
            size_t height) noexcept;

    class State {
    public:
        void save() noexcept;
        void restore() noexcept;

    private:
        GLint textureBinding[2] = { 0 };
        GLint framebuffer = 0;
        GLint array = 0;
        GLint vertexAttrib = 0;
        GLint vertexArray = 0;
        GLint viewport[4] = { 0 };
        GLint activeTexture = 0;
        GLint sampler[2] = { 0 };
    } mState;

    GLuint mFBO = 0;
    const SharedGl& mSharedGl;

    bool mEncodedToRgb = false;
    GLuint mRgbTexture = 0;

    const CVOpenGLESTextureCacheRef mTextureCache;
    CVPixelBufferRef mImage = nullptr;
    CVOpenGLESTextureRef mTexture = nullptr;

};

} // namespace filament

#endif
