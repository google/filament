/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_FRAMESKIPPER_H
#define TNT_FILAMENT_DETAILS_FRAMESKIPPER_H

#include <backend/Handle.h>
#include <private/backend/DriverApi.h>

#include <array>

#include <stddef.h>
#include <stdint.h>

namespace filament {

/*
 * FrameSkipper is used to determine if the current frame needs to be skipped so that we don't
 * outrun the GPU.
 */
class FrameSkipper {
    /*
     * The maximum frame latency acceptable on ANDROID is 2 because higher latencies will be
     * throttled anyway is BufferQueueProducer::dequeueBuffer(), because ANDROID is generally
     * triple-buffered no more; that case is actually pretty bad because the GL thread can block
     * anywhere (usually inside the first draw command that touches the swapchain).
     *
     * A frame latency of 1 has the benefit of reducing render latency,
     * but the drawback of preventing CPU / GPU overlap.
     *
     * Generally a frame latency of 2 is the best compromise.
     */
    static constexpr size_t MAX_FRAME_LATENCY = 2;
public:
    /*
     * The latency parameter defines how many unfinished frames we want to accept before we start
     * dropping frames. This affects frame latency.
     *
     * A latency of 1 means that the GPU must be finished with the previous frame so that
     * we don't drop the current frame. While this provides the best latency this doesn't allow
     * much overlap between the main thread, the back thread and the GPU.
     *
     * A latency of 2 (default) allows full overlap between the CPU And GPU, but the main and driver
     * thread can't fully overlap.
     *
     * A latency 3 allows the main thread, driver thread and GPU to overlap, each being able to
     * use up to 16ms (or whatever the refresh rate is).
     */
    explicit FrameSkipper(size_t latency = 2) noexcept;
    ~FrameSkipper() noexcept;

    void terminate(backend::DriverApi& driver) noexcept;

    // Returns false if we need to skip this frame, because the GPU is running behind the CPU;
    // In that case, don't call render endFrame()
    // Returns true if rendering can proceed. Always call endFrame() when done.
    bool beginFrame(backend::DriverApi& driver) noexcept;

    void endFrame(backend::DriverApi& driver) noexcept;

private:
    using Container = std::array<backend::Handle<backend::HwFence>, MAX_FRAME_LATENCY>;
    mutable Container mDelayedFences{};
    uint8_t const mLast;
};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_FRAMESKIPPER_H
