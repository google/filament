// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/native/DeviceGuard.h"

#include <algorithm>
#include <utility>

#include "src/dawn/native/Device.h"
#include "src/dawn/platform/metrics/HistogramMacros.h"
#include "src/utils/assert.h"

namespace dawn::native {

DeviceMutex::DeviceMutex(dawn::platform::Platform* platform) : mPlatform(platform) {
    DAWN_ASSERT(mPlatform != nullptr);
}

DeviceMutex::~DeviceMutex() {
    DAWN_ASSERT(mRecursionStackDepth == 0);
}

void DeviceMutex::Lock() {
    // We can't use `mRecursionStackDepth` here to figure out if this is a recursive acquire since
    // `mRecursionStackDepth` is protected by the lock itself which we haven't acquired yet.
    const std::thread::id currentThread = std::this_thread::get_id();
    const bool isRecursive = mOwningThread == currentThread;
    double startTime = 0.0;
    if (!isRecursive) {
        startTime = mPlatform->MonotonicallyIncreasingTime();
    }

    RecursiveMutex::Lock();

    if (mRecursionStackDepth == 0) {
        DAWN_ASSERT(!isRecursive);
        mDefer.emplace();
        mOwningThread = currentThread;

        double endTime = mPlatform->MonotonicallyIncreasingTime();
        double acquireTime = endTime - startTime;

        mAcquireTimeSum += acquireTime;
        mAcquireTimeMax = std::max(mAcquireTimeMax, acquireTime);
        mAcquireCount++;

        static constexpr uint64_t kAcquireTimeMetricSampleCount = 100;
        if (mAcquireCount >= kAcquireTimeMetricSampleCount) {
            double avgTimeUs = (mAcquireTimeSum / static_cast<double>(mAcquireCount)) * 1'000'000.0;
            double maxTimeUs = mAcquireTimeMax * 1'000'000.0;

            DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(mPlatform, "DeviceLockAcquireTimeAvgUs",
                                                    static_cast<int>(avgTimeUs), 1, 1'000'000, 50);
            DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(mPlatform, "DeviceLockAcquireTimeMaxUs",
                                                    static_cast<int>(maxTimeUs), 1, 1'000'000, 50);

            mAcquireTimeSum = 0.0;
            mAcquireTimeMax = 0.0;
            mAcquireCount = 0;
        }
    }
    mRecursionStackDepth++;
}

void DeviceMutex::Unlock() {
    // Optional Defer here is used to destroy the Defer object after releasing the lock.
    std::optional<class Defer> defer;

    mRecursionStackDepth--;
    if (mRecursionStackDepth == 0) {
        mOwningThread = std::thread::id();
        defer.swap(mDefer);
    }
    RecursiveMutex::Unlock();
}

namespace detail {

DeviceGuardBase::DeviceGuardBase(DeviceMutex* mutex) : mMutex(mutex) {}

}  // namespace detail

DeviceGuard::DeviceGuard(DeviceBase* device, DeviceMutex* mutex)
    : detail::DeviceGuardBase(mutex), GuardBase(device, device->mMutex) {}

DeviceGuard::DeviceGuard(DeviceGuard&& other)
    : detail::DeviceGuardBase(std::move(other)), GuardBase(std::move(other)) {}

}  // namespace dawn::native
