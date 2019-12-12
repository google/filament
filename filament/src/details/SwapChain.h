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

#include "upcast.h"

#include "private/backend/DriverApi.h"

#include <filament/SwapChain.h>

#include <utils/compiler.h>

namespace filament {
namespace details {

class FEngine;

class FSwapChain : public SwapChain {
public:
    FSwapChain(FEngine& engine, void* nativeWindow, uint64_t flags);
    FSwapChain(FEngine& engine, uint32_t width, uint32_t height, uint64_t flags);
    void terminate(FEngine& engine) noexcept;

    void makeCurrent(backend::DriverApi& driverApi) noexcept {
        driverApi.makeCurrent(mSwapChain, mSwapChain);
    }

    void commit(backend::DriverApi& driverApi) noexcept {
        driverApi.commit(mSwapChain);
    }

    void* getNativeWindow() const noexcept {
        return mNativeWindow;
    }

    constexpr bool isTransparent() const noexcept {
        return (mConfigFlags & CONFIG_TRANSPARENT) != 0;
    }

    constexpr bool isReadable() const noexcept {
        return (mConfigFlags & CONFIG_READABLE) != 0;
    }

    backend::Handle<backend::HwSwapChain> getHwHandle() const noexcept {
      return mSwapChain;
    }

private:
    backend::Handle<backend::HwSwapChain> mSwapChain;
    void* mNativeWindow = nullptr;
    uint64_t mConfigFlags = 0;
};

FILAMENT_UPCAST(SwapChain)

} // namespace details
} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SWAPCHAIN_H
