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

#include <utils/compiler.h>
#include <utils/Logger.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>

namespace filament::backend {

AndroidSwapChainHelper::AndroidSwapChainHelper() = default;

AndroidSwapChainHelper::~AndroidSwapChainHelper() noexcept = default;

bool AndroidSwapChainHelper::setPresentFrameId(
        ANativeWindow* anw, uint64_t const frameId) const noexcept {
    uint64_t sysFrameId{};
    int const status = NativeWindow::getNextFrameId(anw, &sysFrameId);
    if (status == 0) {
        std::lock_guard const lock(mLock);
        // frameIds must be strictly monotonic, if that's not the case (i.e. the new frameId is
        // less or equal to the last one in the map), we have to clear the map, because the
        // map's find() assume sorted keys.
        // This case can happen if two different filament::Renderer are used with the same
        // ANativeWindow (the Renderer would have different frameIds). This is expected to
        // be a rare case.
        if (UTILS_UNLIKELY(!mFrameIdToSystemFrameId.empty() &&
                frameId <= mFrameIdToSystemFrameId.back().first)) {
            // this log is expected to happen very rarely
            DLOG(INFO) << "clearing frame history anw=" << anw
                    << ", frameId=" << frameId
                    << ", previous=" << mFrameIdToSystemFrameId.back().first
                    << ", sysFrameId=" << sysFrameId;
            // clear the frame history
            mFrameIdToSystemFrameId.clear();
        }

        // oldest entry is removed
        mFrameIdToSystemFrameId.insert(frameId, sysFrameId);
        return true;
    }
    return false;
}

uint64_t AndroidSwapChainHelper::getFrameId(uint64_t const frameId) const noexcept {
    std::lock_guard const lock(mLock);
    if (auto const* const pos = mFrameIdToSystemFrameId.find(frameId)) {
        return *pos;
    }
    return std::numeric_limits<uint64_t>::max();
}

}
