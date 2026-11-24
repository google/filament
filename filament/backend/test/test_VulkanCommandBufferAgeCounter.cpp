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

#include "vulkan/utils/CommandBufferAgeCounter.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <vector>

namespace filament::backend::fvkutils {

TEST(AgeSetTest, InitialStateAndBoundaries) {
    AgeSet set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.getLastCompletedAge(), 0u);
    EXPECT_FALSE(set.contains(0));
    EXPECT_FALSE(set.contains(1));
    EXPECT_FALSE(set.contains(AgeSet::CAPACITY));
    EXPECT_FALSE(set.contains(AgeSet::CAPACITY + 1));
    EXPECT_FALSE(set.contains(std::numeric_limits<uint64_t>::max()));

    // Insert the lowest and highest valid ages in the initial window [1, CAPACITY].
    set.insert(1);
    set.insert(AgeSet::CAPACITY);
    EXPECT_FALSE(set.empty());
    EXPECT_TRUE(set.contains(1));
    EXPECT_FALSE(set.contains(2));
    EXPECT_TRUE(set.contains(AgeSet::CAPACITY));
    EXPECT_FALSE(set.contains(AgeSet::CAPACITY + 1));
    EXPECT_EQ(set.oldest(), 1u);

    // Duplicate insertion and erasing an absent age are both no-ops.
    set.insert(1);
    set.erase(2);
    EXPECT_TRUE(set.contains(1));
    EXPECT_TRUE(set.contains(AgeSet::CAPACITY));

    set.erase(1);
    EXPECT_FALSE(set.contains(1));
    EXPECT_FALSE(set.empty());
    EXPECT_EQ(set.oldest(), AgeSet::CAPACITY);

    set.erase(AgeSet::CAPACITY);
    EXPECT_TRUE(set.empty());
    EXPECT_FALSE(set.contains(AgeSet::CAPACITY));
}

TEST(AgeSetTest, WindowShiftAcrossWordBoundary) {
    AgeSet set;
    // Span both 64-bit halves of the bitset, including the 64/65 boundary and bit 127.
    uint64_t const initialAges[] = { 2, 4, 63, 64, 65, 66, 128 };
    for (uint64_t const age : initialAges) {
        set.insert(age);
    }

    // Completing up to 0 is a no-op.
    set.completeUpTo(0);
    EXPECT_EQ(set.getLastCompletedAge(), 0u);
    EXPECT_EQ(set.oldest(), 2u);
    for (uint64_t const age : initialAges) {
        EXPECT_TRUE(set.contains(age));
    }

    // Shift by 3: age 2 completes, age 4 shifts down to bit 0, and new window extends to 131.
    set.completeUpTo(3);
    EXPECT_EQ(set.getLastCompletedAge(), 3u);
    EXPECT_FALSE(set.contains(2));
    EXPECT_FALSE(set.contains(3));
    EXPECT_EQ(set.oldest(), 4u);
    EXPECT_TRUE(set.contains(63));
    EXPECT_TRUE(set.contains(64));
    EXPECT_TRUE(set.contains(65));
    EXPECT_TRUE(set.contains(66));
    EXPECT_TRUE(set.contains(128));
    EXPECT_FALSE(set.contains(3 + AgeSet::CAPACITY));

    // The shifted window now has room for 3 + CAPACITY.
    set.insert(3 + AgeSet::CAPACITY);
    EXPECT_TRUE(set.contains(3 + AgeSet::CAPACITY));

    // Completing to a stale or equal age must not move the window backward or drop active ages.
    set.completeUpTo(2);
    set.completeUpTo(3);
    EXPECT_EQ(set.getLastCompletedAge(), 3u);
    EXPECT_TRUE(set.contains(4));
    EXPECT_TRUE(set.contains(3 + AgeSet::CAPACITY));

    // Shift by 62 (across the 64-bit word boundary): 66 shifts into bit 0, 128 and 131 into word 0.
    set.completeUpTo(65);
    EXPECT_EQ(set.getLastCompletedAge(), 65u);
    EXPECT_FALSE(set.contains(4));
    EXPECT_FALSE(set.contains(63));
    EXPECT_FALSE(set.contains(64));
    EXPECT_FALSE(set.contains(65));
    EXPECT_EQ(set.oldest(), 66u);
    EXPECT_TRUE(set.contains(66));
    EXPECT_TRUE(set.contains(128));
    EXPECT_TRUE(set.contains(3 + AgeSet::CAPACITY));

    // Insert at the new upper bound (65 + CAPACITY) and advance right up to it.
    uint64_t const newMax = 65 + AgeSet::CAPACITY;
    set.insert(newMax);
    set.completeUpTo(newMax - 1);
    EXPECT_FALSE(set.empty());
    EXPECT_EQ(set.oldest(), newMax);
    EXPECT_TRUE(set.contains(newMax));

    set.completeUpTo(newMax);
    EXPECT_TRUE(set.empty());
    EXPECT_FALSE(set.contains(newMax));
}

TEST(AgeSetTest, WindowShiftClearsOnLargeAdvance) {
    AgeSet set;
    set.insert(1);
    set.insert(64);
    set.insert(AgeSet::CAPACITY);

    // Shifting by exactly CAPACITY clears the bitset.
    set.completeUpTo(AgeSet::CAPACITY);
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.getLastCompletedAge(), AgeSet::CAPACITY);
    EXPECT_FALSE(set.contains(1));
    EXPECT_FALSE(set.contains(64));
    EXPECT_FALSE(set.contains(AgeSet::CAPACITY));

    set.insert(AgeSet::CAPACITY + 1);
    set.insert(2 * AgeSet::CAPACITY);
    EXPECT_EQ(set.oldest(), AgeSet::CAPACITY + 1);

    // Shifting by more than CAPACITY also clears the bitset cleanly.
    set.completeUpTo(1000);
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.getLastCompletedAge(), 1000u);
    EXPECT_FALSE(set.contains(2 * AgeSet::CAPACITY));

    set.insert(1001);
    set.insert(1000 + AgeSet::CAPACITY);
    EXPECT_EQ(set.oldest(), 1001u);
    EXPECT_TRUE(set.contains(1000 + AgeSet::CAPACITY));

    // Stale advance after a large jump is a no-op.
    set.completeUpTo(500);
    EXPECT_EQ(set.getLastCompletedAge(), 1000u);
    EXPECT_TRUE(set.contains(1001));
}

TEST(AgeSetTest, IntersectsWithDifferentWindows) {
    AgeSet empty;
    AgeSet a;
    a.insert(3);
    a.insert(5);
    a.insert(70);
    a.completeUpTo(2);

    EXPECT_FALSE(empty.intersects(empty));
    EXPECT_FALSE(empty.intersects(a));
    EXPECT_FALSE(a.intersects(empty));
    EXPECT_TRUE(a.intersects(a));

    // Different completed ages (2 vs 5) that share age 70 across a >64-bit shift.
    AgeSet b;
    b.insert(6);
    b.insert(70);
    b.completeUpTo(5);
    EXPECT_TRUE(a.intersects(b));
    EXPECT_TRUE(b.intersects(a));

    // Ages 3 and 5 in `a` are <= `c`'s completed age (5), and 70 != 71, so they do not intersect.
    AgeSet c;
    c.insert(6);
    c.insert(71);
    c.completeUpTo(5);
    EXPECT_FALSE(a.intersects(c));
    EXPECT_FALSE(c.intersects(a));

    // intersects() must not mutate either operand.
    EXPECT_EQ(a.getLastCompletedAge(), 2u);
    EXPECT_TRUE(a.contains(3));
    EXPECT_TRUE(a.contains(5));
    EXPECT_TRUE(a.contains(70));

    // Windows separated by more than CAPACITY are disjoint.
    AgeSet distant;
    distant.completeUpTo(300);
    distant.insert(301);
    EXPECT_FALSE(a.intersects(distant));
    EXPECT_FALSE(distant.intersects(a));
    EXPECT_TRUE(a.contains(3));
}

TEST(CommandBufferAgeCounterTest, InitialStateAndInOrderRelease) {
    CommandBufferAgeCounter counter;
    EXPECT_TRUE(counter.hasCapacity());
    EXPECT_EQ(counter.getLastCompletedAge(), 0u);
    EXPECT_TRUE(counter.getActiveAges().empty());

    uint64_t const age1 = counter.count();
    uint64_t const age2 = counter.count();
    uint64_t const age3 = counter.count();
    EXPECT_EQ(age1, 1u);
    EXPECT_EQ(age2, 2u);
    EXPECT_EQ(age3, 3u);
    EXPECT_TRUE(counter.getActiveAges().contains(age1));
    EXPECT_TRUE(counter.getActiveAges().contains(age2));
    EXPECT_TRUE(counter.getActiveAges().contains(age3));

    counter.release(age1);
    EXPECT_EQ(counter.getLastCompletedAge(), 1u);
    EXPECT_FALSE(counter.getActiveAges().contains(age1));
    EXPECT_TRUE(counter.getActiveAges().contains(age2));

    counter.release(age2);
    EXPECT_EQ(counter.getLastCompletedAge(), 2u);

    counter.release(age3);
    EXPECT_EQ(counter.getLastCompletedAge(), 3u);
    EXPECT_TRUE(counter.getActiveAges().empty());
}

TEST(CommandBufferAgeCounterTest, OutOfOrderRelease) {
    CommandBufferAgeCounter counter;

    uint64_t const a1 = counter.count();
    uint64_t const a2 = counter.count();
    uint64_t const a3 = counter.count();

    // Releasing a newer age while an older one is still active must not advance lastCompleted.
    counter.release(a2);
    EXPECT_EQ(counter.getLastCompletedAge(), 0u);
    EXPECT_TRUE(counter.getActiveAges().contains(a1));
    EXPECT_FALSE(counter.getActiveAges().contains(a2));
    EXPECT_TRUE(counter.getActiveAges().contains(a3));

    // Releasing the oldest active age catches lastCompleted up past already-released age 2.
    counter.release(a1);
    EXPECT_EQ(counter.getLastCompletedAge(), 2u);
    EXPECT_EQ(counter.getActiveAges().oldest(), a3);

    counter.release(a3);
    EXPECT_EQ(counter.getLastCompletedAge(), 3u);
    EXPECT_TRUE(counter.getActiveAges().empty());

    // Simulate regular and protected pools interleaving and completing out of order.
    uint64_t const a4 = counter.count();
    uint64_t const a5 = counter.count(); // Held long-lived (e.g. protected queue)
    uint64_t const a6 = counter.count();
    uint64_t const a7 = counter.count();

    counter.release(a4);
    EXPECT_EQ(counter.getLastCompletedAge(), 4u);

    counter.release(a6);
    counter.release(a7);
    EXPECT_EQ(counter.getLastCompletedAge(), 4u);

    uint64_t const a8 = counter.count();
    uint64_t const a9 = counter.count();
    counter.release(a8);
    EXPECT_EQ(counter.getLastCompletedAge(), 4u);

    // Once a5 releases, lastCompleted leaps across {5, 6, 7, 8} up to just before a9.
    counter.release(a5);
    EXPECT_EQ(counter.getLastCompletedAge(), 8u);
    EXPECT_EQ(counter.getActiveAges().oldest(), a9);

    counter.release(a9);
    EXPECT_EQ(counter.getLastCompletedAge(), 9u);
    EXPECT_TRUE(counter.getActiveAges().empty());
}

TEST(CommandBufferAgeCounterTest, HasCapacityBoundaryAndStalledOldestAge) {
    CommandBufferAgeCounter counter;
    std::vector<uint64_t> issued;
    issued.reserve(CommandBufferAgeCounter::MAX_ACTIVE);

    for (size_t i = 0; i < CommandBufferAgeCounter::MAX_ACTIVE - 1; ++i) {
        EXPECT_TRUE(counter.hasCapacity());
        issued.push_back(counter.count());
    }

    // One slot left: hasCapacity() is still true right before the MAX_ACTIVE-th count().
    EXPECT_TRUE(counter.hasCapacity());
    issued.push_back(counter.count());
    EXPECT_EQ(issued.back(), CommandBufferAgeCounter::MAX_ACTIVE);

    // Window is now full.
    EXPECT_FALSE(counter.hasCapacity());
#if defined(GTEST_HAS_DEATH_TEST) && GTEST_HAS_DEATH_TEST
    // count() is noexcept, so its precondition panic terminates even when exceptions are enabled.
    EXPECT_DEATH(counter.count(), "exceeds MAX_ACTIVE");
#endif

    // Release every age except the oldest (issued[0]): the span remains blocked at MAX_ACTIVE,
    // so hasCapacity() must stay false even though only one age is still in flight.
    for (size_t i = issued.size() - 1; i >= 1; --i) {
        counter.release(issued[i]);
        EXPECT_EQ(counter.getLastCompletedAge(), 0u);
        EXPECT_FALSE(counter.hasCapacity());
    }

    // Releasing the oldest age frees the entire span at once.
    counter.release(issued[0]);
    EXPECT_EQ(counter.getLastCompletedAge(), CommandBufferAgeCounter::MAX_ACTIVE);
    EXPECT_TRUE(counter.getActiveAges().empty());
    EXPECT_TRUE(counter.hasCapacity());

    // Fill another full window across AgeSet::CAPACITY (128).
    std::vector<uint64_t> secondBatch;
    secondBatch.reserve(CommandBufferAgeCounter::MAX_ACTIVE);
    for (size_t i = 0; i < CommandBufferAgeCounter::MAX_ACTIVE; ++i) {
        ASSERT_TRUE(counter.hasCapacity());
        secondBatch.push_back(counter.count());
    }
    EXPECT_FALSE(counter.hasCapacity());

    // Releasing just the oldest age in the new window frees exactly one slot.
    counter.release(secondBatch.front());
    EXPECT_EQ(counter.getLastCompletedAge(), secondBatch.front());
    EXPECT_TRUE(counter.hasCapacity());
    uint64_t const next = counter.count();
    EXPECT_EQ(next, secondBatch.back() + 1);
    EXPECT_FALSE(counter.hasCapacity());
}

TEST(CommandBufferAgeCounterTest, MarkInFlightAndIsInFlightPrunesCompletedAges) {
    CommandBufferAgeCounter counter;
    AgeSet resourceAges;
    EXPECT_FALSE(counter.isInFlight(resourceAges));

    uint64_t const a1 = counter.count();
    uint64_t const a2 = counter.count();
    counter.markInFlight(resourceAges, a1);
    counter.markInFlight(resourceAges, a2);
    EXPECT_TRUE(counter.isInFlight(resourceAges));

    // Releasing one of two ages leaves the resource in flight.
    counter.release(a1);
    EXPECT_TRUE(counter.isInFlight(resourceAges));

    // An unrelated active command buffer does not keep the resource in flight once a2 completes.
    uint64_t const unrelated = counter.count();
    counter.release(a2);
    EXPECT_FALSE(counter.isInFlight(resourceAges));
    counter.release(unrelated);

    // Advance the counter well beyond AgeSet::CAPACITY while resourceAges still holds old bits.
    constexpr size_t EXTRA_CYCLES = AgeSet::CAPACITY + 40;
    for (size_t i = 0; i < EXTRA_CYCLES; ++i) {
        uint64_t const age = counter.count();
        counter.release(age);
    }
    EXPECT_FALSE(counter.isInFlight(resourceAges));

    // Re-marking the resource after > AgeSet::CAPACITY ages must prune the completed ages first
    // instead of overflowing the resource's AgeSet window.
    uint64_t const lateAge = counter.count();
    EXPECT_GT(lateAge, AgeSet::CAPACITY);
    counter.markInFlight(resourceAges, lateAge);
    EXPECT_TRUE(counter.isInFlight(resourceAges));

    counter.release(lateAge);
    EXPECT_FALSE(counter.isInFlight(resourceAges));
}

} // namespace filament::backend::fvkutils
