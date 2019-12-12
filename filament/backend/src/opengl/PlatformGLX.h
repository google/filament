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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_GLX_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_GLX_H

#include <stdint.h>

#include <bluegl/BlueGL.h>
#include <GL/glx.h>

#include <backend/DriverEnums.h>

#include "private/backend/OpenGLPlatform.h"

#include <vector>

namespace filament {

class PlatformGLX final : public backend::OpenGLPlatform {
public:

    backend::Driver* createDriver(void* const sharedGLContext) noexcept override;

    void terminate() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept override;
    void commit(SwapChain* swapChain) noexcept override;

    Fence* createFence() noexcept override;
    void destroyFence(Fence* fence) noexcept override;
    backend::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept override;

    void setPresentationTime(int64_t time) noexcept final override {}

    Stream* createStream(void* nativeStream) noexcept final override { return nullptr; }
    void destroyStream(Stream* stream) noexcept final override {}
    void attach(Stream* stream, intptr_t tname) noexcept final override {}
    void detach(Stream* stream) noexcept final override {}
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept final override {}

    ExternalTexture* createExternalTextureStorage() noexcept final override { return nullptr; }
    void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, backend::TextureFormat format) noexcept final override { }
    void destroyExternalTextureStorage(ExternalTexture* ets) noexcept final override { }

    int getOSVersion() const noexcept final override { return 0; }

private:
    Display *mGLXDisplay;
    GLXContext mGLXContext;
    GLXFBConfig* mGLXConfig;
    GLXPbuffer mDummySurface;
    std::vector<GLXPbuffer> mPBuffers;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_GLX_H
