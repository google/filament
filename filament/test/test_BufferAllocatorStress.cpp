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

#include "../src/details/BufferAllocator.h"

#include <utils/Panic.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <vector>

using namespace filament;

namespace {

class BufferAllocatorStressTest : public ::testing::Test {
protected:
    // We use a total size of 1024 * 64 and a slot size (alignment) of 64.
    // This gives us 1024 total possible slots if aligned.
    static constexpr BufferAllocator::allocation_size_t SLOT_SIZE = 64;
    static constexpr BufferAllocator::allocation_size_t SLOT_COUNT = 4096;
    static constexpr BufferAllocator::allocation_size_t TOTAL_SIZE = SLOT_COUNT * SLOT_SIZE;

    BufferAllocatorStressTest() : mAllocator(TOTAL_SIZE, SLOT_SIZE) {
    }

    BufferAllocator mAllocator;
};

TEST_F(BufferAllocatorStressTest, StressTest) {
    // Many operations.
    constexpr int operationCount = 5000;

    struct ActiveAllocation {
        BufferAllocator::AllocationId id;
        BufferAllocator::allocation_size_t offset;
        BufferAllocator::allocation_size_t size;
    };

    // Run two passes:
    // - maxSlotCount = 64: stresses fragmentation and coalescing across many small allocations.
    // - maxSlotCount = SLOT_COUNT: stresses large allocations spanning up to the entire buffer.
    for (uint32_t const maxSlotCount : { 64u, SLOT_COUNT }) {
        SCOPED_TRACE(maxSlotCount);

        // 1. Prepare the deterministic random number distributions.
        std::mt19937 gen(20250101u + maxSlotCount);
        std::uniform_int_distribution<uint32_t> slotCountDistrib(1, maxSlotCount);
        // Random the slot offset we allocate within a slot.
        std::uniform_int_distribution<uint32_t> slotOffsetDistrib(0, SLOT_SIZE - 1);
        // Random the operation we perform: 0,1,2 -> allocate, 3 -> retire
        // We bias the operations to make more allocations than releases.
        std::uniform_int_distribution<int> operationDistrib(0, 3);

        // 2. Randomly allocate and retire while verifying non-overlapping invariants.
        std::vector<ActiveAllocation> active;
        size_t successfulAllocations = 0;
        size_t failedAllocations = 0;
        size_t successfulRetires = 0;
        for (int i = 0; i < operationCount; i++) {
            switch (operationDistrib(gen)) {
                case 0: // Allocate
                case 1: // Allocate
                case 2: // Allocate
                {
                    // Random the slot count + its offset.
                    const BufferAllocator::allocation_size_t requestedSize =
                            slotCountDistrib(gen) * SLOT_SIZE - slotOffsetDistrib(gen);

                    auto [id, offset] = mAllocator.allocate(requestedSize);
                    if (BufferAllocator::isValid(id)) {
                        BufferAllocator::allocation_size_t const alignedSize =
                                mAllocator.getAllocationSize(id);
                        EXPECT_EQ(offset % SLOT_SIZE, 0u);
                        EXPECT_GE(alignedSize, requestedSize);
                        EXPECT_LE(offset + alignedSize, TOTAL_SIZE);
                        for (auto const& existing : active) {
                            bool const disjoint = (offset + alignedSize <= existing.offset) ||
                                    (existing.offset + existing.size <= offset);
                            ASSERT_TRUE(disjoint);
                        }
                        active.push_back({ id, offset, alignedSize });
                        successfulAllocations++;
                    } else {
                        EXPECT_EQ(id, BufferAllocator::REALLOCATION_REQUIRED);
                        failedAllocations++;
                    }
                    break;
                }
                case 3: // Retire
                {
                    if (active.empty()) {
                        continue;
                    }

                    // Retire a random slot.
                    std::uniform_int_distribution<size_t> idDistrib(0, active.size() - 1);
                    size_t const indexToRetire = idDistrib(gen);

                    mAllocator.retire(active[indexToRetire].id);
                    successfulRetires++;

                    // Remove the retired allocation from the list.
                    std::swap(active[indexToRetire], active.back());
                    active.pop_back();
                    break;
                }
                default:
                    break;
            }
        }

        EXPECT_GT(successfulAllocations, 100u);
        EXPECT_GT(failedAllocations, 100u);
        EXPECT_GT(successfulRetires, 50u);

        // 3. Retire all remaining allocations.
        for (auto const& alloc : active) {
            mAllocator.retire(alloc.id);
        }
        active.clear();
        EXPECT_EQ(mAllocator.getAllocationCount(), 0u);

        // 4. The allocator should now be in a pristine state.
        //    A final allocation of the total size should succeed.
        auto [finalId, finalOffset] = mAllocator.allocate(TOTAL_SIZE);
        EXPECT_TRUE(BufferAllocator::isValid(finalId));
        EXPECT_EQ(finalOffset, 0u);
        mAllocator.retire(finalId);
        EXPECT_EQ(mAllocator.getAllocationCount(), 0u);
    }
}

} // anonymous namespace
