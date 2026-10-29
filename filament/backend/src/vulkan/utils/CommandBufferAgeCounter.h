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

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace filament::backend::fvkutils {

// A set of command buffer ages. Every age up to and including mLastCompletedAge is known to have
// completed and is never in the set. The remaining ages are kept in a bitset relative to
// mLastCompletedAge: bit `i` corresponds to age `mLastCompletedAge + i + 1`. Consequently, an age
// can only be inserted if it is within CAPACITY of mLastCompletedAge.
class AgeSet {
public:
    using ActiveBuffers = utils::bitset128;
    static constexpr size_t CAPACITY = sizeof(ActiveBuffers) * 8;

    uint64_t getLastCompletedAge() const noexcept {
        return mLastCompletedAge;
    }

    bool empty() const noexcept {
        return mActives.empty();
    }

    bool contains(uint64_t age) const noexcept {
        if (age <= mLastCompletedAge) {
            return false;
        }
        uint64_t const index = age - mLastCompletedAge - 1;
        return index < CAPACITY && mActives[size_t(index)];
    }

    void insert(uint64_t age) noexcept {
        assert_invariant(age > mLastCompletedAge && age - mLastCompletedAge <= CAPACITY);
        mActives.set(size_t(age - mLastCompletedAge - 1));
    }

    void erase(uint64_t age) noexcept {
        assert_invariant(age > mLastCompletedAge && age - mLastCompletedAge <= CAPACITY);
        mActives.unset(size_t(age - mLastCompletedAge - 1));
    }

    // Returns the oldest age in the set. The set must not be empty.
    uint64_t oldest() const noexcept {
        assert_invariant(!empty());
        return mLastCompletedAge + mActives.firstSetBit() + 1;
    }

    // Marks every age up to and including `age` as completed, which removes them from the set.
    // This is a no-op if `age` is not newer than mLastCompletedAge.
    void completeUpTo(uint64_t age) noexcept {
        if (age <= mLastCompletedAge) {
            return;
        }
        // Shifting by CAPACITY or more clears the bitset.
        mActives >>= size_t(std::min<uint64_t>(age - mLastCompletedAge, CAPACITY));
        mLastCompletedAge = age;
    }

    // Returns true if at least one age is in both sets.
    bool intersects(AgeSet const& other) const noexcept {
        // Ages up to the larger of the two completed ages cannot be in both sets, so align both
        // bitsets on it before comparing.
        uint64_t const base = std::max(mLastCompletedAge, other.mLastCompletedAge);
        AgeSet lhs = *this;
        AgeSet rhs = other;
        lhs.completeUpTo(base);
        rhs.completeUpTo(base);
        return (lhs.mActives & rhs.mActives).any();
    }

private:
    ActiveBuffers mActives;
    uint64_t mLastCompletedAge = 0;
};

// Hands out a monotonically increasing "age" to each command buffer when it starts recording, and
// tracks which of those ages are still in flight. A single counter is shared by the regular and
// the protected command buffer pools, so that an age uniquely identifies a command buffer across
// both pools.
//
// In-flight ages are kept in an AgeSet, so the *span* from the oldest in-flight age to the newest
// issued age must stay within MAX_ACTIVE. The number of in-flight command buffers is bounded by
// the pool capacities, but the span is not: an age only leaves the window once every older age has
// been released, and the two pools submit to different queues which may complete out of order
// relative to each other. Callers must therefore check hasCapacity() before count() and retire
// completed command buffers of both pools when it is false (see CommandBufferPool::getRecording()).
class CommandBufferAgeCounter {
public:
    // The maximum number of command buffers that can be in flight (recording or submitted) at the
    // same time: one regular pool and one protected pool.
    static constexpr size_t MAX_ACTIVE = 2 * FVK_MAX_COMMAND_BUFFERS;
    static_assert(AgeSet::CAPACITY >= MAX_ACTIVE);

    // Returns true if count() can hand out another age without exceeding MAX_ACTIVE.
    bool hasCapacity() const noexcept {
        return mAge - mActives.getLastCompletedAge() <= MAX_ACTIVE;
    }

    uint64_t count() noexcept {
        // This is a hard check rather than assert_invariant: exceeding the window would write
        // past the end of the active age bitset (and of every VulkanBuffer's age bitset).
        FILAMENT_CHECK_PRECONDITION(hasCapacity())
                << "CommandBufferAgeCounter: " << (mAge - mActives.getLastCompletedAge())
                << " ages in flight exceeds MAX_ACTIVE=" << MAX_ACTIVE;
        uint64_t const age = mAge++;
        mActives.insert(age);
        return age;
    }

    // Returns the most recent age such that it and every older age have completed.
    uint64_t getLastCompletedAge() const noexcept {
        return mActives.getLastCompletedAge();
    }

    // Returns the ages that have been handed out and not yet released, i.e. the command buffers
    // that are either recording or executing.
    AgeSet const& getActiveAges() const noexcept {
        return mActives;
    }

    // Records in `ages` (the ages that use some resource) that the command buffer of age `age`
    // uses it too. That command buffer must still be recording.
    void markInFlight(AgeSet& ages, uint64_t age) const noexcept {
        assert_invariant(mActives.contains(age));
        // Ages that are known to have completed can be dropped. This also keeps every recorded
        // age within the AgeSet window: `age` is still recording, so it is in this counter's
        // window, which is no larger than the AgeSet capacity.
        ages.completeUpTo(getLastCompletedAge());
        ages.insert(age);
    }

    // Returns true if any of `ages` is still recording or executing.
    bool isInFlight(AgeSet const& ages) const noexcept {
        return ages.intersects(mActives);
    }

    // This means that the command buffer associated with an age has completed.
    void release(uint64_t age) noexcept {
        assert_invariant(mActives.contains(age));
        mActives.erase(age);
        // Move the completed age forward to just before the oldest age still in flight, or to the
        // newest issued age if every issued age has completed.
        mActives.completeUpTo(mActives.empty() ? mAge - 1 : mActives.oldest() - 1);
    }

private:
    AgeSet mActives;
    // The next age to hand out. Age 0 is reserved to mean "no age".
    uint64_t mAge = 1;
};

} // namespace filament::backend::fvkutils

#endif // TNT_FILAMENT_BACKEND_VULKAN_UTILS_COMMANDBUFFERAGECOUNTER_H
