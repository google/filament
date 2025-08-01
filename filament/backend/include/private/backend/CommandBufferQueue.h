/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_COMMANDBUFFERQUEUE_H
#define TNT_FILAMENT_BACKEND_PRIVATE_COMMANDBUFFERQUEUE_H

#include "private/backend/CircularBuffer.h"

#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <vector>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

/*
 * A producer-consumer command queue that uses a CircularBuffer as main storage
 */
class CommandBufferQueue {
public:
    struct Range {
        void* begin;
        void* end;
    };

    // requiredSize: guaranteed available space after flush()
    // bufferSize: size of the circular buffer, must be multiple of CircularBuffer::getBlockSize()
    // requiredSize must be smaller or equal to bufferSize
    // The implementation will enforce these constraints by rounding bufferSize up if necessary
    // and adjusting requiredSize to be at least that.
    CommandBufferQueue(size_t requiredSize, size_t bufferSize, bool paused);
    ~CommandBufferQueue();

    CircularBuffer& getCircularBuffer() noexcept { return mCircularBuffer; }
    CircularBuffer const& getCircularBuffer() const noexcept { return mCircularBuffer; }

    size_t getCapacity() const noexcept { return mRequiredSize; }

    size_t getHighWatermark() const noexcept { return mHighWatermark; }

    // wait for commands to be available and returns an array containing these commands
    std::vector<Range> waitForCommands() const;

    // return the memory used by this command buffer to the circular buffer
    // WARNING: releaseBuffer() must be called in sequence of the Slices returned by
    // waitForCommands()
    void releaseBuffer(Range const& buffer);

    // all commands buffers (Slices) written to this point are returned by waitForCommand(). This
    // call blocks until the CircularBuffer has at least mRequiredSize bytes available.
    void flush();

    // returns from waitForCommands() immediately.
    void requestExit();

    // suspend or unsuspend the queue.
    bool isPaused() const noexcept;
    void setPaused(bool paused);

    bool isExitRequested() const;

private:
    const size_t mRequiredSize;

    CircularBuffer mCircularBuffer;

    // space available in the circular buffer

    mutable utils::Mutex mLock;
    mutable utils::Condition mCondition;
    mutable std::vector<Range> mCommandBuffersToExecute;
    size_t mFreeSpace = 0;
    size_t mHighWatermark = 0;
    uint32_t mExitRequested = 0;
    bool mPaused = false;

    static constexpr uint32_t EXIT_REQUESTED = 0x31415926;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_COMMANDBUFFERQUEUE_H
