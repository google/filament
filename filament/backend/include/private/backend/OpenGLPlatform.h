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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_H

#include <backend/Platform.h>

#include "private/backend/AcquiredImage.h"

namespace filament {
namespace backend {

class Driver;

class OpenGLPlatform : public DefaultPlatform {
protected:

    /*
     * Derived classes can use this to instantiate the default OpenGLDriver backend.
     * This is typically called from your implementation of createDriver()
     */
    static Driver* createDefaultDriver(OpenGLPlatform* platform, void* sharedContext);

public:
    ~OpenGLPlatform() noexcept override;

    // Called to destroy the OpenGL context. This should clean up any windows
    // or buffers from initialization.
    virtual void terminate() noexcept = 0;

    virtual SwapChain* createSwapChain(void* nativeWindow, uint64_t& flags) noexcept = 0;

    // headless swapchain
    virtual SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept = 0;

    virtual void destroySwapChain(SwapChain* swapChain) noexcept = 0;

    virtual void createDefaultRenderTarget(uint32_t& framebuffer, uint32_t& colorbuffer,
            uint32_t& depthbuffer) noexcept {
        framebuffer = 0;
        colorbuffer = 0;
        depthbuffer = 0;
    }

    // Called to make the OpenGL context active on the calling thread.
    virtual void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept = 0;

    // Called once the current frame finishes drawing. Typically this should
    // swap draw buffers (i.e. for double-buffered rendering).
    virtual void commit(SwapChain* swapChain) noexcept = 0;

    virtual void setPresentationTime(int64_t presentationTimeInNanosecond) noexcept = 0;

    virtual bool canCreateFence() noexcept { return false; }
    virtual Fence* createFence() noexcept = 0;
    virtual void destroyFence(Fence* fence) noexcept = 0;
    virtual backend::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept = 0;

    // this is called synchronously in the application thread (NOT the Driver thread)
    virtual Stream* createStream(void* nativeStream) noexcept = 0;

    virtual void destroyStream(Stream* stream) noexcept = 0;

    // attach takes ownership of the texture (tname) object
    virtual void attach(Stream* stream, intptr_t tname) noexcept = 0;

    // detach destroys the texture associated to the stream
    virtual void detach(Stream* stream) noexcept = 0;
    virtual void updateTexImage(Stream* stream, int64_t* timestamp) noexcept = 0;

    // external texture storage
    virtual ExternalTexture* createExternalTextureStorage() noexcept = 0;

    // this is called synchronously in the application thread (NOT the Driver thread)
    virtual void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, TextureFormat format) noexcept = 0;

    virtual void destroyExternalTextureStorage(ExternalTexture* ets) noexcept = 0;

    // The method allows platforms to convert a user-supplied external image object into a new type
    // (e.g. HardwareBuffer => EGLImage). It makes sense for the default implementation to do nothing.
    virtual AcquiredImage transformAcquiredImage(AcquiredImage source) noexcept { return source; }

    // called to bind the platform-specific externalImage to a texture
    // texture points to a OpenGLDriver::GLTexture
    virtual bool setExternalImage(void* externalImage, void* texture) noexcept {
        return false;
    }

    // called on the application thread to allow Filament to take ownership of the image
    virtual void retainExternalImage(void* externalImage) noexcept {}

    // called to release ownership of the image
    virtual void releaseExternalImage(void* externalImage) noexcept {}

    // called once when a new SAMPLER_EXTERNAL texture is created.
    virtual void createExternalImageTexture(void* texture) noexcept {}

    // called once before a SAMPLER_EXTERNAL texture is destroyed.
    virtual void destroyExternalImage(void* texture) noexcept {}
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_H
