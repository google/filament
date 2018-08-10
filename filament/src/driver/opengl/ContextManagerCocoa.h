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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_CONTEXT_MANAGER_COCOA_H
#define TNT_FILAMENT_DRIVER_OPENGL_CONTEXT_MANAGER_COCOA_H

#include <stdint.h>

#include <filament/driver/DriverEnums.h>
#include <filament/driver/ExternalContext.h>

namespace filament {

struct ContextManagerCocoaImpl;

class ContextManagerCocoa final : public driver::ContextManagerGL {
public:
    ContextManagerCocoa();
    ~ContextManagerCocoa() noexcept final;

    std::unique_ptr<Driver> createDriver(void* sharedGLContext) noexcept override;
    void terminate() noexcept final;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept final;
    void destroySwapChain(SwapChain* swapChain) noexcept final;
    void makeCurrent(SwapChain* swapChain) noexcept final;
    void commit(SwapChain* swapChain) noexcept final;

    Fence* createFence() noexcept final { return nullptr; }
    void destroyFence(Fence* fence) noexcept final {}
    driver::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept final {
        return driver::FenceStatus::ERROR;
    }

    Stream* createStream(void* nativeStream) noexcept final { return nullptr; }
    void destroyStream(Stream* stream) noexcept final {}
    void attach(Stream* stream, intptr_t tname) noexcept final {}
    void detach(Stream* stream) noexcept final {}
    void updateTexImage(Stream* stream) noexcept final {}

    ExternalTexture* createExternalTextureStorage() noexcept final { return nullptr; }
    void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, driver::TextureFormat format) noexcept final { }
    void destroyExternalTextureStorage(ExternalTexture* ets) noexcept final { }

    int getOSVersion() const noexcept final { return 0; }

private:
    ContextManagerCocoaImpl* pImpl = nullptr;
};

using ContextManager = filament::ContextManagerCocoa;

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_CONTEXT_MANAGER_COCOA_H
