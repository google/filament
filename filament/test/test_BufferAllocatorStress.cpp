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

#include <gtest/gtest.h>

#include "../src/details/BufferAllocator.h"
#include "utils/Panic.h"

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

    // 1. Prepare the random number distributions.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> slotCountDistrib(1, SLOT_COUNT);
    // Random the slot offset we allocate within a slot.
    std::uniform_int_distribution<> slotOffsetDistrib(0, SLOT_SIZE - 1);
    // Random the operation we perform: 0,1,2 -> allocate, 3 -> retire
    // We bias the operations to make more allocations than releases.
    std::uniform_int_distribution operationDistrib(0, 3);

    // 2. Randomly do some actions and expect to have no crash
    std::vector<BufferAllocator::AllocationId> ids;
    for (int i = 0; i < operationCount; i++) {
        switch (operationDistrib(gen)) {
            case 0: // Allocate
            case 1: // Allocate
            case 2: // Allocate
            {
                // Random the slot count + its offset.
                const BufferAllocator::allocation_size_t size =
                        slotCountDistrib(gen) * SLOT_SIZE - slotOffsetDistrib(gen);

                if (auto [id, _] = mAllocator.allocate(size);
                    id != BufferAllocator::REALLOCATION_REQUIRED && id !=
                    BufferAllocator::UNALLOCATED) {
                    ids.push_back(id);
                }
                break;
            }
            case 3: // Retire
            {
                if (ids.empty()) {
                    continue;
                }

                // Retire a random slot.
                std::uniform_int_distribution<uint32_t> idDistrib(0, ids.size() - 1);
                uint32_t indexToRetire = idDistrib(gen);

                BufferAllocator::AllocationId idToRetire = ids[indexToRetire];
                mAllocator.retire(idToRetire);

                // Remove the retired id from the list.
                std::swap(ids[indexToRetire], ids.back());
                ids.pop_back();
                break;
            }
            default:
                break;
        }
    }

    // 3. Retire all remaining allocations.
    for (const auto& id: ids) {
        mAllocator.retire(id);
    }
    ids.clear();

    // 5. The allocator should now be in a pristine state.
    //    A final allocation of the total size should succeed.
    auto [finalId, finalOffset] = mAllocator.allocate(TOTAL_SIZE);
    EXPECT_TRUE(BufferAllocator::isValid(finalId));
    EXPECT_EQ(finalOffset, 0);
}

} // anonymous namespace
