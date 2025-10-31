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

#include "AndroidFrameCallback.h"

#include <utils/Panic.h>
#include <utils/debug.h>

namespace filament::backend {

AndroidFrameCallback::AndroidFrameCallback() = default;

AndroidFrameCallback::~AndroidFrameCallback() noexcept {
    assert_invariant(mLooper == nullptr);
}

void AndroidFrameCallback::init() {
    if (__builtin_available(android 33, *)) {
        FILAMENT_CHECK_PRECONDITION(!mLooperThread.joinable()) << "init() already called";

        // start the looper thread for our choreographer callbacks
        mLooperThread = std::thread([this] {
            // create the looper for this thread
            mLooper = ALooper_prepare(0);
            // acquire a reference, so we can use it from our main thread
            ALooper_acquire(mLooper);
            // start the choreographer callbacks
            if (__builtin_available(android 33, *)) {
                mChoreographer = AChoreographer_getInstance();
                // request our first callback for the next frame
                AChoreographer_postVsyncCallback(mChoreographer, &vsyncCallback, this);
            }
            // our main loop just sits there to handle events
            while (true) {
                int const result = ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
                if (result == ALOOPER_POLL_ERROR || mExitRequested.
                    load(std::memory_order_relaxed)) {
                    return; // exit the loop
                }
            }
        });
    }
}

void AndroidFrameCallback::terminate() {
    if (__builtin_available(android 33, *)) {
        // request exit
        mExitRequested.store(true, std::memory_order_relaxed);
        // wake the looper right away
        ALooper_wake(mLooper);
        // release our reference to the looper
        ALooper_release(mLooper);
        mLooper = nullptr;
        // and make sure to wait for the thread to terminate
        mLooperThread.join();
    }
}

bool AndroidFrameCallback::isSupported() noexcept {
    if (__builtin_available(android 33, *)) {
        return true;
    }
    return false;
}

AndroidFrameCallback::Timeline AndroidFrameCallback::getPreferredTimeline() const noexcept {
    std::lock_guard const l(mLock);
    return mPreferredTimeline;
}

void AndroidFrameCallback::vsyncCallback(const AChoreographerFrameCallbackData* callbackData) {
    if (__builtin_available(android 33, *)) {
        // request the next frame callback
        AChoreographer_postVsyncCallback(mChoreographer, &vsyncCallback, this);

        int64_t const frameTime = AChoreographerFrameCallbackData_getFrameTimeNanos(callbackData);

        size_t const preferredIndex =
                AChoreographerFrameCallbackData_getPreferredFrameTimelineIndex(callbackData);

        int64_t const expectedPresentTime =
                AChoreographerFrameCallbackData_getFrameTimelineExpectedPresentationTimeNanos(
                        callbackData, preferredIndex);

        int64_t const frameTimelineDeadline =
                AChoreographerFrameCallbackData_getFrameTimelineDeadlineNanos(
                        callbackData, preferredIndex);

        std::lock_guard const l(mLock);
        mPreferredTimeline = {
            .frameTime = frameTime,
            .expectedPresentTime = expectedPresentTime,
            .frameTimelineDeadline = frameTimelineDeadline
        };
    }
}

}
