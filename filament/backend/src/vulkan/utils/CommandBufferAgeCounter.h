/*
 * Copyright (C) 2026 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKAN_UTILS_COMMANDBUFFERAGECOUNTER_H
#define TNT_FILAMENT_BACKEND_VULKAN_UTILS_COMMANDBUFFERAGECOUNTER_H

#include "vulkan/VulkanConstants.h"

#include <utils/bitset.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <cstdint>

namespace filament::backend::fvkutils {

// Hands out a monotonically increasing "age" to each command buffer when it starts recording, and
// tracks which of those ages are still in flight. A single counter is shared by the regular and
// the protected command buffer pools, so that an age uniquely identifies a command buffer across
// both pools.
//
// In-flight ages are kept in a bitset relative to mLastCompletedAge, so the *span* from the oldest
// in-flight age to the newest issued age must stay within MAX_ACTIVE. The number of in-flight
// command buffers is bounded by the pool capacities, but the span is not: an age only leaves the
// window once every older age has been released, and the two pools submit to different queues
// which may complete out of order relative to each other. Callers must therefore check
// hasCapacity() before count() and retire completed command buffers of both pools when it is
// false (see CommandBufferPool::getRecording()).
class CommandBufferAgeCounter {
public:
    // The maximum number of command buffers that can be in flight (recording or submitted) at the
    // same time: one regular pool and one protected pool.
    static constexpr size_t MAX_ACTIVE = 2 * FVK_MAX_COMMAND_BUFFERS;

    // A bitmask with one bit per in-flight age. Bit `i` corresponds to age `base + i + 1`.
    using ActiveBuffers = utils::bitset128;
    static_assert(sizeof(ActiveBuffers) * 8 >= MAX_ACTIVE);

    // Returns true if count() can hand out another age without exceeding MAX_ACTIVE.
    bool hasCapacity() const noexcept {
        return mAge - mLastCompletedAge <= MAX_ACTIVE;
    }

    uint64_t count() noexcept {
        // This is a hard check rather than assert_invariant: exceeding the window would write
        // past the end of mActives (and of every VulkanBuffer's age bitset).
        FILAMENT_CHECK_PRECONDITION(hasCapacity())
                << "CommandBufferAgeCounter: " << (mAge - mLastCompletedAge)
                << " ages in flight exceeds MAX_ACTIVE=" << MAX_ACTIVE;
        auto age = mAge++;
        mActives.set(age - mLastCompletedAge - 1);
        return age;
    }

    uint64_t getBarrierAge() const noexcept {
        return mLastCompletedAge;
    }

    bool isDone(uint64_t age) const noexcept {
        // Note that an age of 0 (uninitialized age) is always considered "done".
        if (age <= mLastCompletedAge) {
            return true;
        }
        assert_invariant(age - mLastCompletedAge <= MAX_ACTIVE);
        return !mActives[age - mLastCompletedAge - 1];
    }

    // This means that the command buffer associated with an age has completed.
    void release(uint64_t age) noexcept {
        assert_invariant(
                age > mLastCompletedAge && age - mLastCompletedAge <= MAX_ACTIVE);

        mActives.unset(age - mLastCompletedAge - 1);

        if (mActives.empty()) {
            // All produced ages have completed.
            mLastCompletedAge = mAge - 1;
            return;
        }

        auto const activeShift = mActives.firstSetBit();
        mActives >>= activeShift;
        mLastCompletedAge += activeShift;
    }

private:
    uint64_t mLastCompletedAge = 0;
    uint64_t mAge = mLastCompletedAge + 1;

    ActiveBuffers mActives;
};

} // namespace filament::backend::fvkutils

#endif // TNT_FILAMENT_BACKEND_VULKAN_UTILS_COMMANDBUFFERAGECOUNTER_H
