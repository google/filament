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

#ifndef TNT_FILAMENT_DETAILS_SWAPCHAIN_H
#define TNT_FILAMENT_DETAILS_SWAPCHAIN_H

#include "downcast.h"

#include "private/backend/DriverApi.h"

#include <filament/SwapChain.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <utils/Invocable.h>

#include <stdint.h>

namespace filament {

class FEngine;

class FSwapChain : public SwapChain {
public:
    FSwapChain(FEngine& engine, void* nativeWindow, uint64_t flags);
    FSwapChain(FEngine& engine, uint32_t width, uint32_t height, uint64_t flags);
    void terminate(FEngine& engine) noexcept;

    void makeCurrent(backend::DriverApi& driverApi) noexcept {
        driverApi.makeCurrent(mHwSwapChain, mHwSwapChain);
    }

    void commit(backend::DriverApi& driverApi) noexcept {
        driverApi.commit(mHwSwapChain);
    }

    void* getNativeWindow() const noexcept {
        return mNativeWindow;
    }

    bool isTransparent() const noexcept {
        return (mConfigFlags & CONFIG_TRANSPARENT) != 0;
    }

    bool isReadable() const noexcept {
        return (mConfigFlags & CONFIG_READABLE) != 0;
    }

    bool hasStencilBuffer() const noexcept {
        return (mConfigFlags & CONFIG_HAS_STENCIL_BUFFER) != 0;
    }

    bool isProtected() const noexcept {
        return (mConfigFlags & CONFIG_PROTECTED_CONTENT) != 0;
    }

    // This returns the effective flags. Unsupported flags are cleared automatically.
    uint64_t getFlags() const noexcept {
        return mConfigFlags;
    }

    backend::Handle<backend::HwSwapChain> getHwHandle() const noexcept {
      return mHwSwapChain;
    }

    void setFrameScheduledCallback(
            backend::CallbackHandler* handler, FrameScheduledCallback&& callback);

    bool isFrameScheduledCallbackSet() const noexcept;

    void setFrameCompletedCallback(backend::CallbackHandler* handler,
                utils::Invocable<void(SwapChain*)>&& callback) noexcept;

    static bool isSRGBSwapChainSupported(FEngine& engine) noexcept;

    static bool isProtectedContentSupported(FEngine& engine) noexcept;

    // This is currently only used for debugging. This allows to recreate the HwSwapChain with
    // new flags.
    void recreateWithNewFlags(FEngine& engine, uint64_t flags) noexcept;

private:
    FEngine& mEngine;
    backend::Handle<backend::HwSwapChain> mHwSwapChain;
    bool mFrameScheduledCallbackIsSet = false;
    void* mNativeWindow{};
    uint32_t mWidth{};
    uint32_t mHeight{};
    uint64_t mConfigFlags{};
    static uint64_t initFlags(FEngine& engine, uint64_t flags) noexcept;
};

FILAMENT_DOWNCAST(SwapChain)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SWAPCHAIN_H
