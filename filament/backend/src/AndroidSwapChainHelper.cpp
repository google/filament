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

#include "AndroidSwapChainHelper.h"
#include "AndroidNativeWindow.h"

#include <android/native_window.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>

namespace filament::backend {

AndroidSwapChainHelper::AndroidSwapChainHelper() = default;

AndroidSwapChainHelper::~AndroidSwapChainHelper() noexcept = default;

bool AndroidSwapChainHelper::setPresentFrameId(
        ANativeWindow* anw, uint64_t frameId) const noexcept {
    uint64_t sysFrameId{};
    int const status = NativeWindow::getNextFrameId(anw, &sysFrameId);
    if (status == 0) {
        std::lock_guard const lock(mLock);
        auto const pos = mFrameIdToSystemFrameId.find(frameId);
        if (pos != mFrameIdToSystemFrameId.end() && pos->second != sysFrameId) {
            // we're trying to associate the same frame id to a different frame!
            return false;
        }
        // make space by destroying the oldest entry
        if (mFrameIdToSystemFrameId.size() >= MAX_HISTORY_SIZE) {
            mFrameIdToSystemFrameId.erase(mFrameIdToSystemFrameId.begin());
        }
        mFrameIdToSystemFrameId.insert(mFrameIdToSystemFrameId.end(), { frameId, sysFrameId });
        return true;
    }
    return false;
}

uint64_t AndroidSwapChainHelper::getFrameId(uint64_t frameId) const noexcept {
    std::lock_guard const lock(mLock);
    auto pos = mFrameIdToSystemFrameId.find(frameId);
    if (pos != mFrameIdToSystemFrameId.end()) {
        return pos->second;
    }
    return std::numeric_limits<uint64_t>::max();
}

}
