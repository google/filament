/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WGL_H
#define TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_WGL_H

#include <stdint.h>

#include <windows.h>
#include <utils/unwindows.h>

#include <backend/DriverEnums.h>

#include "private/backend/OpenGLPlatform.h"

namespace filament::backend {

class PlatformWGL final : public OpenGLPlatform {
public:
    Driver* createDriver(void* const sharedGLContext) noexcept override;
    void terminate() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept override;
    void commit(SwapChain* swapChain) noexcept override;

    Fence* createFence() noexcept override;
    void destroyFence(Fence* fence) noexcept override;
    FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept override;

    void setPresentationTime(int64_t time) noexcept final override {}

    Stream* createStream(void* nativeStream) noexcept final override { return nullptr; }
    void destroyStream(Stream* stream) noexcept final override {}
    void attach(Stream* stream, intptr_t tname) noexcept final override {}
    void detach(Stream* stream) noexcept final override {}
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept final override {}

    int getOSVersion() const noexcept final override { return 0; }

private:
    HGLRC mContext = NULL;
    HWND mHWnd = NULL;
    HDC mWhdc = NULL;
    PIXELFORMATDESCRIPTOR mPfd = {};
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_OPENGL_OPENGL_PLATFORM_GLX_H
