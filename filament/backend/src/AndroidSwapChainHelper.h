/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_ANDROIDSWAPCHAINHELPER_H
#define TNT_FILAMENT_BACKEND_ANDROIDSWAPCHAINHELPER_H

#include <utils/Mutex.h>
#include <utils/MonotonicRingMap.h>

#include <android/native_window.h>

#include <map>

#include <cstddef>
#include <cstdint>

namespace filament::backend {

struct AndroidSwapChainHelper {
    AndroidSwapChainHelper();
    ~AndroidSwapChainHelper() noexcept;

    // methods below are thread-safe
    bool setPresentFrameId(ANativeWindow* anw, uint64_t frameId) const noexcept;
    uint64_t getFrameId(uint64_t frameId) const noexcept;

private:
    static constexpr size_t MAX_HISTORY_SIZE = 32;
    mutable utils::Mutex mLock; // very low-contention lock
    mutable utils::MonotonicRingMap<MAX_HISTORY_SIZE, uint64_t, uint64_t> mFrameIdToSystemFrameId{};
};

}

#endif //TNT_FILAMENT_BACKEND_ANDROIDSWAPCHAINHELPER_H
