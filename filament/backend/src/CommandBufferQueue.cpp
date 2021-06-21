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

#include <utils/Log.h>
#include <utils/Systrace.h>
#include <utils/Panic.h>
#include <utils/debug.h>

#include "private/backend/BackendUtils.h"
#include "private/backend/CommandStream.h"

using namespace utils;

namespace filament {
namespace backend {

CommandBufferQueue::CommandBufferQueue(size_t requiredSize, size_t bufferSize)
        : mRequiredSize((requiredSize + CircularBuffer::BLOCK_MASK) & ~CircularBuffer::BLOCK_MASK),
          mCircularBuffer(bufferSize),
          mFreeSpace(mCircularBuffer.size()) {
    assert_invariant(mCircularBuffer.size() > requiredSize);
}

CommandBufferQueue::~CommandBufferQueue() {
    assert_invariant(mCommandBuffersToExecute.empty());
}

void CommandBufferQueue::requestExit() {
    std::lock_guard<utils::Mutex> lock(mLock);
    mExitRequested = EXIT_REQUESTED;
    mCondition.notify_one();
}

bool CommandBufferQueue::isExitRequested() const {
    std::lock_guard<utils::Mutex> lock(mLock);
    ASSERT_PRECONDITION( mExitRequested == 0 || mExitRequested == EXIT_REQUESTED,
            "mExitRequested is corrupted (value = 0x%08x)!", mExitRequested);
    return (bool)mExitRequested;
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
    ASSERT_POSTCONDITION(used <= mFreeSpace,
            "Backend CommandStream overflow. Commands are corrupted and unrecoverable.\n"
            "Please increase FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB (currently %u MiB).\n"
            "Space used at this time: %u bytes",
            (unsigned)FILAMENT_MIN_COMMAND_BUFFERS_SIZE_IN_MB, (unsigned)used);

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

    mCondition.notify_one();
    if (UTILS_LIKELY(mFreeSpace < requiredSize)) {
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

    ASSERT_PRECONDITION( mExitRequested == 0 || mExitRequested == EXIT_REQUESTED,
            "mExitRequested is corrupted (value = 0x%08x)!", mExitRequested);

    return std::move(mCommandBuffersToExecute);
}

void CommandBufferQueue::releaseBuffer(CommandBufferQueue::Slice const& buffer) {
    std::lock_guard<utils::Mutex> lock(mLock);
    mFreeSpace += uintptr_t(buffer.end) - uintptr_t(buffer.begin);
    mCondition.notify_one();
}

} // namespace backend
} // namespace filament
