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

#ifndef TNT_FILAMENT_BACKEND_ANDROIDFRAMECALLBACK_H
#define TNT_FILAMENT_BACKEND_ANDROIDFRAMECALLBACK_H

#include <android/choreographer.h>
#include <android/looper.h>

#include <utils/CountDownLatch.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>

namespace filament::backend {

/*
 * AndroidFrameCallback uses a dedicated thread running a Looper and uses
 * Choreographer to receive the FrameCallbackData.
 *
 */
class AndroidFrameCallback {
public:
    struct Timeline {
        using timepoint_ns = int64_t;
        static constexpr timepoint_ns INVALID = -1;
        timepoint_ns frameTime{ INVALID };
        timepoint_ns expectedPresentTime{ INVALID };
        timepoint_ns frameTimelineDeadline{ INVALID };
    };

    AndroidFrameCallback();

    ~AndroidFrameCallback() noexcept;

    AndroidFrameCallback(AndroidFrameCallback const&) = delete;
    AndroidFrameCallback& operator=(AndroidFrameCallback const&) = delete;

    void init();
    void terminate();

    static  bool isSupported() noexcept;

    Timeline getPreferredTimeline() const noexcept;

private:
    // looper
    std::thread mLooperThread;
    mutable utils::CountDownLatch mInitBarrier{ 1 };
    ALooper* mLooper = nullptr;
    std::atomic_bool mExitRequested{ false };

    // choreographer
    AChoreographer* mChoreographer{};

    void vsyncCallback(const AChoreographerFrameCallbackData* callbackData);
    static void vsyncCallback(const AChoreographerFrameCallbackData* callbackData, void* data) {
        static_cast<AndroidFrameCallback*>(data)->vsyncCallback(callbackData);
    }

    mutable std::mutex mLock;
    Timeline mPreferredTimeline{};
};

} // namespace filament::backend

#endif //TNT_FILAMENT_BACKEND_ANDROIDFRAMECALLBACK_H
