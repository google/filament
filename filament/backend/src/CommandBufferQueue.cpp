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

#include "private/backend/CommandBufferQueue.h"

#include <assert.h>

#include <utils/Log.h>
#include <utils/Systrace.h>

#include "private/backend/CommandStream.h"

using namespace utils;

namespace filament {
namespace backend {

CommandBufferQueue::CommandBufferQueue(size_t requiredSize, size_t bufferSize)
        : mRequiredSize((requiredSize + CircularBuffer::BLOCK_MASK) & ~CircularBuffer::BLOCK_MASK),
          mCircularBuffer(bufferSize),
          mFreeSpace(mCircularBuffer.size()) {
    assert(mCircularBuffer.size() > requiredSize);
}

CommandBufferQueue::~CommandBufferQueue() {
    assert(mCommandBuffersToExecute.empty());
}

void CommandBufferQueue::requestExit() {
    std::unique_lock<utils::Mutex> lock(mLock);
    mExitRequested = true;
    mCondition.notify_one();
}

bool CommandBufferQueue::isExitRequested() const {
    std::unique_lock<utils::Mutex> lock(mLock);
    return mExitRequested;
}


void CommandBufferQueue::flush() noexcept {
    SYSTRACE_CALL();

    CircularBuffer& circularBuffer = mCircularBuffer;
    if (circularBuffer.empty()) {
        return;
    }

    // add the terminating command
    // always guaranteed to have enough space for the NoopCommand
    new(circularBuffer.allocate(sizeof(NoopCommand))) NoopCommand(nullptr);

    // end of this slice
    void* const head = circularBuffer.getHead();

    // beginning of this slice
    void* const tail = circularBuffer.getTail();

    // size of this slice
    uint32_t used = uint32_t(intptr_t(head) - intptr_t(tail));

    circularBuffer.circularize();

    std::unique_lock<utils::Mutex> lock(mLock);
    mCommandBuffersToExecute.push_back({ tail, head });

    // circular buffer is too small, we corrupted the stream
    assert(used <= mFreeSpace);

    // wait until there is enough space in the buffer
    mFreeSpace -= used;
    const size_t requiredSize = mRequiredSize;

#ifndef NDEBUG
    size_t totalUsed = circularBuffer.size() - mFreeSpace;
    mHighWatermark = std::max(mHighWatermark, totalUsed);
    if (UTILS_UNLIKELY(totalUsed > requiredSize)) {
        slog.d << "CommandStream used too much space: " << totalUsed
            << ", out of " << requiredSize << " (will block)" << io::endl;
    }
#endif

    if (UTILS_LIKELY(mFreeSpace >= requiredSize)) {
        // ideally (and usually) we don't have to wait, this is the common case, so special case
        // the unlock-before-notify, optimization.
        lock.unlock();
        mCondition.notify_one();
    } else {
        // unfortunately, there is not enough space left, we'll have to wait.
        mCondition.notify_one(); // too bad there isn't a notify-and-wait
        SYSTRACE_NAME("waiting: CircularBuffer::flush()");
        mCondition.wait(lock, [this, requiredSize]() -> bool {
            return mFreeSpace >= requiredSize;
        });
    }
}

std::vector<CommandBufferQueue::Slice> CommandBufferQueue::waitForCommands() const {
    if (!UTILS_HAS_THREADING) {
        return std::move(mCommandBuffersToExecute);
    }
    std::unique_lock<utils::Mutex> lock(mLock);
    while (mCommandBuffersToExecute.empty() && !mExitRequested) {
        mCondition.wait(lock);
    }
    return std::move(mCommandBuffersToExecute);
}

void CommandBufferQueue::releaseBuffer(CommandBufferQueue::Slice const& buffer) {
    std::unique_lock<utils::Mutex> lock(mLock);
    mFreeSpace += uintptr_t(buffer.end) - uintptr_t(buffer.begin);
    lock.unlock();
    mCondition.notify_one();
}

} // namespace backend
} // namespace filament
