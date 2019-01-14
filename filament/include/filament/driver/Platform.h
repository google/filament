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

#ifndef TNT_FILAMENT_DRIVER_PLATFORM_H
#define TNT_FILAMENT_DRIVER_PLATFORM_H

#include <filament/driver/DriverEnums.h>

#include <utils/compiler.h>

namespace filament {
namespace details {
class FEngine;
}

class Driver;

namespace driver {

class UTILS_PUBLIC Platform {
public:
    struct SwapChain {};
    struct Fence {};
    struct Stream {};
    struct ExternalTexture {
        uintptr_t image = 0;
    };

    virtual int getOSVersion() const noexcept = 0;

    virtual ~Platform() noexcept;

protected:
    // Creates and initializes the low-level API (e.g. an OpenGL context or Vulkan instance),
    // then creates the concrete Driver. Returns null on failure.
    // The caller takes ownership of the returned Driver* and must destroy it with delete.
    virtual Driver* createDriver(void* sharedContext) noexcept = 0;

private:
    friend class details::FEngine;
    static Platform* create(driver::Backend* backendHint) noexcept;
    static void destroy(Platform** context) noexcept;
};

class UTILS_PUBLIC OpenGLPlatform : public Platform {
public:
    ~OpenGLPlatform() noexcept override;

    // Called to destroy the OpenGL context. This should clean up any windows
    // or buffers from initialization.
    virtual void terminate() noexcept = 0;

    virtual SwapChain* createSwapChain(void* nativeWindow, uint64_t& flags) noexcept = 0;
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
    virtual driver::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept = 0;

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
};

class UTILS_PUBLIC VulkanPlatform : public Platform {
public:
    ~VulkanPlatform() noexcept override;

    // Given a Vulkan instance and native window handle, creates the platform-specific surface.
    virtual void* createVkSurfaceKHR(void* nativeWindow, void* instance,
            uint32_t* width, uint32_t* height) noexcept = 0;
};

class UTILS_PUBLIC MetalPlatform : public Platform {
public:
    ~MetalPlatform() noexcept override;

};

} // namespace driver
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_PLATFORM_H
