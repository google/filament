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

#ifndef TNT_FILAMENT_DRIVER_COMMANDBUFFERQUEUE_H
#define TNT_FILAMENT_DRIVER_COMMANDBUFFERQUEUE_H

#include "private/backend/CircularBuffer.h"

#include <utils/compiler.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>

#include <vector>

namespace filament {
namespace backend {

/*
 * A producer-consumer command queue that uses a CircularBuffer as main storage
 */
class CommandBufferQueue {
    struct Slice {
        void* begin;
        void* end;
    };

    const size_t mRequiredSize;

    CircularBuffer mCircularBuffer;

    // space available in the circular buffer

    mutable utils::Mutex mLock;
    mutable utils::Condition mCondition;
    mutable std::vector<Slice> mCommandBuffersToExecute;
    size_t mFreeSpace = 0;
    size_t mHighWatermark = 0;
    bool mExitRequested = false;

public:
    // requiredSize: guaranteed available space after flush()
    CommandBufferQueue(size_t requiredSize, size_t bufferSize);
    ~CommandBufferQueue();

    CircularBuffer& getCircularBuffer() { return mCircularBuffer; }

    size_t getHigWatermark() const noexcept { return mHighWatermark; }

    // wait for commands to be available and returns an array containing these commands
    std::vector<Slice> waitForCommands() const;

    // return the memory used by this command buffer to the circular buffer
    // WARNING: releaseBuffer() must be called in sequence of the Slices returned by
    // waitForCommands()
    void releaseBuffer(Slice const& buffer);

    // all commands buffers (Slices) written to this point are returned by waitForCommand(). This
    // call blocks until the CircularBuffer has at least mRequiredSize bytes available.
    void flush() noexcept;

    // returns from waitForCommands() immediately.
    void requestExit();

    bool isExitRequested() const;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_COMMANDBUFFERQUEUE_H
