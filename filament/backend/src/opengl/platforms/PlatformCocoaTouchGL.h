/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_COCOA_TOUCH_GL_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_COCOA_TOUCH_GL_H

#include <stdint.h>

#include <backend/DriverEnums.h>

#include "private/backend/OpenGLPlatform.h"

namespace filament {

struct PlatformCocoaTouchGLImpl;

class PlatformCocoaTouchGL final : public backend::OpenGLPlatform {
public:
    PlatformCocoaTouchGL();
    ~PlatformCocoaTouchGL() noexcept final;

    backend::Driver* createDriver(void* sharedGLContext) noexcept override;
    void terminate() noexcept final;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept final;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept final;
    void destroySwapChain(SwapChain* swapChain) noexcept final;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept final;
    void commit(SwapChain* swapChain) noexcept final;
    void createDefaultRenderTarget(uint32_t& framebuffer, uint32_t& colorbuffer,
            uint32_t& depthbuffer) noexcept final;
    Fence* createFence() noexcept final { return nullptr; }
    void destroyFence(Fence* fence) noexcept final {}
    backend::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept final {
        return backend::FenceStatus::ERROR;
    }

    void setPresentationTime(int64_t time) noexcept final override {}

    Stream* createStream(void* nativeStream) noexcept final { return nullptr; }
    void destroyStream(Stream* stream) noexcept final {}
    void attach(Stream* stream, intptr_t tname) noexcept final {}
    void detach(Stream* stream) noexcept final {}
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept final {}

    ExternalTexture* createExternalTextureStorage() noexcept final { return nullptr; }
    void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, backend::TextureFormat format) noexcept final { }
    void destroyExternalTextureStorage(ExternalTexture* ets) noexcept final { }

    int getOSVersion() const noexcept final { return 0; }

    bool setExternalImage(void* externalImage, void* texture) noexcept final;
    void retainExternalImage(void* externalImage) noexcept final;
    void releaseExternalImage(void* externalImage) noexcept final;
    void createExternalImageTexture(void* texture) noexcept final;
    void destroyExternalImage(void* texture) noexcept final;

private:
    PlatformCocoaTouchGLImpl* pImpl = nullptr;
};

using ContextManager = filament::PlatformCocoaTouchGL;

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_COCOA_TOUCH_GL_H
