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

#ifndef TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H
#define TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H

#include <stdint.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <backend/DriverEnums.h>

#include "private/backend/OpenGLPlatform.h"

namespace filament {

class PlatformEGL : public backend::OpenGLPlatform {
public:

    PlatformEGL() noexcept;

    backend::Driver* createDriver(void* sharedContext) noexcept override;
    void terminate() noexcept override;

    SwapChain* createSwapChain(void* nativewindow, uint64_t& flags) noexcept override;
    SwapChain* createSwapChain(uint32_t width, uint32_t height, uint64_t& flags) noexcept override;
    void destroySwapChain(SwapChain* swapChain) noexcept override;
    void makeCurrent(SwapChain* drawSwapChain, SwapChain* readSwapChain) noexcept override;
    void commit(SwapChain* swapChain) noexcept override;

    bool canCreateFence() noexcept override { return true; }
    Fence* createFence() noexcept override;
    void destroyFence(Fence* fence) noexcept override;
    backend::FenceStatus waitFence(Fence* fence, uint64_t timeout) noexcept override;

    void createExternalImageTexture(void* texture) noexcept override;
    void destroyExternalImage(void* texture) noexcept override;

    /* default no-op implementations... */

    int getOSVersion() const noexcept override { return 0; }

    void setPresentationTime(int64_t presentationTimeInNanosecond) noexcept override {}

    Stream* createStream(void* nativeStream) noexcept override { return nullptr; }
    void destroyStream(Stream* stream) noexcept override {}
    void attach(Stream* stream, intptr_t tname) noexcept override {}
    void detach(Stream* stream) noexcept override {}
    void updateTexImage(Stream* stream, int64_t* timestamp) noexcept override {}

    ExternalTexture* createExternalTextureStorage() noexcept override { return nullptr; }
    void reallocateExternalStorage(ExternalTexture* ets,
            uint32_t w, uint32_t h, backend::TextureFormat format) noexcept override {}
    void destroyExternalTextureStorage(ExternalTexture* ets) noexcept override {}

protected:
    static void logEglError(const char* name) noexcept;

    EGLBoolean makeCurrent(EGLSurface drawSurface, EGLSurface readSurface) noexcept;
    void initializeGlExtensions() noexcept;

    EGLDisplay mEGLDisplay = EGL_NO_DISPLAY;
    EGLContext mEGLContext = EGL_NO_CONTEXT;
    EGLSurface mCurrentDrawSurface = EGL_NO_SURFACE;
    EGLSurface mCurrentReadSurface = EGL_NO_SURFACE;
    EGLSurface mEGLDummySurface = EGL_NO_SURFACE;
    EGLConfig mEGLConfig = EGL_NO_CONFIG_KHR;
    EGLConfig mEGLTransparentConfig = EGL_NO_CONFIG_KHR;

    // supported extensions detected at runtime
    struct {
        bool OES_EGL_image_external_essl3 = false;
    } ext;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_OPENGL_PLATFORM_EGL_H
