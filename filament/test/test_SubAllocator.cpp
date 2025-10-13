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

#include "../src/details/SubAllocator.h"
#include "utils/Panic.h"

#include <utility>
#include <vector>

using namespace filament;

namespace {

class SubAllocatorTest : public ::testing::Test {
protected:
    // We use a total size of 1024 and a slot size (alignment) of 64.
    // This gives us 1024 / 64 = 16 total possible slots if aligned.
    static constexpr SubAllocator::allocation_size_t TOTAL_SIZE = 1024;
    static constexpr SubAllocator::allocation_size_t SLOT_SIZE = 64;

    SubAllocatorTest() : mAllocator(TOTAL_SIZE, SLOT_SIZE) {}

    SubAllocator mAllocator;
};

TEST_F(SubAllocatorTest, ConstructorFailure) {
    // The constructor requires slotSize to be a power of two.
    constexpr SubAllocator::allocation_size_t NON_POT_SLOT_SIZE = 60;
    EXPECT_DEATH(SubAllocator(TOTAL_SIZE, NON_POT_SLOT_SIZE), "failed assertion");
}

TEST_F(SubAllocatorTest, InitialState) {
    EXPECT_EQ(mAllocator.getTotalSize(), TOTAL_SIZE);
    // Initially, there should be one large free block of the total size.
    // Let's try to allocate the whole thing.
    auto [id, offset] = mAllocator.allocate(TOTAL_SIZE);
    EXPECT_EQ(id, 1); // The first ID should be 1.
    EXPECT_EQ(offset, 0);
}

TEST_F(SubAllocatorTest, SimpleAllocation) {
    // Allocate 100 bytes, which should be aligned up to 128 (2 * 64).
    auto [id, offset] = mAllocator.allocate(100);
    EXPECT_EQ(id, 1);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(mAllocator.getAllocationOffset(id), offset);

    // Try to allocate again. The next allocation should start after the first one.
    auto [id2, offset2] = mAllocator.allocate(50); // Aligns to 64
    EXPECT_EQ(id2, 3); // ID is (128 / 64) + 1 = 3
    EXPECT_EQ(offset2, 128);
    EXPECT_EQ(mAllocator.getAllocationOffset(id2), offset2);
}

TEST_F(SubAllocatorTest, AllocateZeroSize) {
    // Allocating zero bytes should return an unallocated ID and not affect the state.
    auto [id, offset] = mAllocator.allocate(0);
    EXPECT_EQ(id, SubAllocator::UNALLOCATED);
    EXPECT_EQ(offset, 0);

    // The allocator should still be in its initial state, able to allocate the full size.
    auto [fullId, fullOffset] = mAllocator.allocate(TOTAL_SIZE);
    EXPECT_EQ(fullId, 1);
    EXPECT_EQ(fullOffset, 0);
}

TEST_F(SubAllocatorTest, AllocateAll) {
    auto [id1, offset1] = mAllocator.allocate(512);
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(offset1, 0);

    auto [id2, offset2] = mAllocator.allocate(512);
    EXPECT_EQ(id2, 9); // ID is (512 / 64) + 1 = 9
    EXPECT_EQ(offset2, 512);

    // The buffer is now full. The next allocation should fail.
    auto [id3, offset3] = mAllocator.allocate(1);
    EXPECT_EQ(id3, SubAllocator::REALLOCATION_REQUIRED);
}

TEST_F(SubAllocatorTest, AllocationFailure) {
    auto [id1, offset1] = mAllocator.allocate(TOTAL_SIZE - 1); // Allocate most of the buffer.
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(offset1, 0);

    // Try to allocate more than the remaining space.
    auto [id2, offset2] = mAllocator.allocate(100);
    EXPECT_EQ(id2, SubAllocator::REALLOCATION_REQUIRED);
    EXPECT_EQ(offset2, 0);
}

TEST_F(SubAllocatorTest, AllocationLifecycle) {
    // 1. Allocate
    auto [id, offset] = mAllocator.allocate(128);
    EXPECT_EQ(id, 1);

    // 2. Retire from CPU side
    mAllocator.retire(id);

    // 3. The slot is not free yet because the GPU might still be using it.
    // Let's simulate the GPU acquiring and releasing it.
    mAllocator.acquireGpu(id);
    mAllocator.releaseGpu(id);

    // Now the slot should be considered free, but it's not merged yet.
    // Let's try to allocate something else to see where it goes.
    auto [id2, offset2] = mAllocator.allocate(200); // Aligns to 256
    EXPECT_EQ(id2, 3); // It should use the space after the first retired slot.
    EXPECT_EQ(offset2, 128);

    // Now, let's release the free slots.
    mAllocator.releaseFreeSlots();

    // The first slot (128 bytes) should now be available again.
    // Let's try to allocate something that fits in it.
    auto [id3, offset3] = mAllocator.allocate(100); // Aligns to 128
    EXPECT_EQ(id3, 1); // It should reuse the first slot.
    EXPECT_EQ(offset3, 0);
}

TEST_F(SubAllocatorTest, RetireThenReleaseGpu) {
    // 1. Allocate a block and a dummy block next to it.
    auto [id1, offset1] = mAllocator.allocate(128);
    auto [id2, offset2] = mAllocator.allocate(128);
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 3);
    EXPECT_EQ(offset1, 0);
    EXPECT_EQ(offset2, 128);

    // 2. Retire the first block (CPU is done), then acquire it for the GPU.
    mAllocator.retire(id1);
    mAllocator.acquireGpu(id1); // gpuUseCount = 1

    // 3. The slot is now free from CPU but locked by GPU. releaseFreeSlots should do nothing to it.
    mAllocator.releaseFreeSlots();

    // 4. Try to allocate the same space. It should fail, and the allocation should go
    //    to the next available free space.
    auto [id3, offset3] = mAllocator.allocate(64);
    EXPECT_EQ(id3, 5);
    EXPECT_EQ(offset3, 256); // It should be allocated after id2.

    // 5. Now, release the GPU lock.
    mAllocator.releaseGpu(id1); // gpuUseCount = 0

    // 6. Call releaseFreeSlots again. This time it should be freed.
    mAllocator.releaseFreeSlots();

    // 7. The original slot should now be available for allocation.
    auto [id4, offset4] = mAllocator.allocate(64);
    EXPECT_EQ(id4, 1);
    EXPECT_EQ(offset4, 0); // Success! It reuses the first slot.
}

TEST_F(SubAllocatorTest, MultipleGpuAcquires) {
    // 1. Allocate a block.
    auto [id, offset] = mAllocator.allocate(128);
    EXPECT_EQ(id, 1);
    EXPECT_EQ(offset, 0);

    // 2. Retire from CPU, then acquire multiple times for GPU (e.g., used in 3 command buffers).
    mAllocator.retire(id);
    mAllocator.acquireGpu(id); // gpuUseCount = 1
    mAllocator.acquireGpu(id); // gpuUseCount = 2
    mAllocator.acquireGpu(id); // gpuUseCount = 3

    // 3. Release GPU lock once. The slot should still be locked.
    mAllocator.releaseGpu(id); // gpuUseCount = 2
    mAllocator.releaseFreeSlots();
    auto [failId1, failOffset1] = mAllocator.allocate(64);
    EXPECT_NE(failOffset1, 0); // Should not be able to allocate at offset 0.
    EXPECT_NE(failId1, 1);

    // 4. Release GPU lock again. The slot should still be locked.
    mAllocator.releaseGpu(id); // gpuUseCount = 1
    mAllocator.releaseFreeSlots();
    auto [failId2, failOffset2] = mAllocator.allocate(64);
    EXPECT_NE(failOffset2, 0); // Still cannot allocate at offset 0.
    EXPECT_NE(failId2, 1);

    // 5. Final release. The lock count is now 0.
    mAllocator.releaseGpu(id); // gpuUseCount = 0

    // 6. Now it should be freed and available.
    mAllocator.releaseFreeSlots();
    auto [successId, successOffset] = mAllocator.allocate(64);
    EXPECT_EQ(successOffset, 0);
    EXPECT_EQ(successId, 1);
}

TEST_F(SubAllocatorTest, GpuPanicOnUnderflow) {
    // 1. Allocate a block and acquire it.
    auto [id, _] = mAllocator.allocate(128);
    mAllocator.acquireGpu(id);

    // 2. Release it once, which is fine.
    mAllocator.releaseGpu(id);

    // 3. Releasing it again when the count is 0 should trigger a failed assertion.
    EXPECT_DEATH(mAllocator.releaseGpu(id), "failed assertion");
}

TEST_F(SubAllocatorTest, MergeFreeSlots) {
    // Allocate three blocks
    auto [id1, offset1] = mAllocator.allocate(128); // Slot 0-127
    auto [id2, offset2] = mAllocator.allocate(128); // Slot 128-255
    auto [id3, offset3] = mAllocator.allocate(128); // Slot 256-383

    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 3);
    EXPECT_EQ(id3, 5);

    EXPECT_EQ(offset1, 0);
    EXPECT_EQ(offset2, 128);
    EXPECT_EQ(offset3, 256);

    // Retire the first and third blocks
    mAllocator.retire(id1);
    mAllocator.retire(id3);

    // At this point, we have: [Free, Allocated, Free, Free (remaining)]
    // releaseFreeSlots should not merge slot 1 and 3 because they are not adjacent.
    // It should merge slot 3 and slot 4 instead.
    mAllocator.releaseFreeSlots();

    // At this point, we have: [Free, Allocated, Free (remaining)]
    // Let's verify by trying to allocate 200. It should go into the merged slot.
    auto [id4, offset4] = mAllocator.allocate(200); // Aligns to 256
    EXPECT_EQ(id4, 5);
    EXPECT_EQ(offset4, 256);

    // Now, retire the middle block
    mAllocator.retire(id2);

    // Now we have: [Free, Free, Allocated, Free (remaining)]
    // The first two blocks are now adjacent and free.
    mAllocator.releaseFreeSlots();

    // After merging, the first 256 bytes should be one large free block.
    // Let's try to allocate something that requires this merged space.
    auto [id5, offset5] = mAllocator.allocate(200); // Aligns to 256
    EXPECT_EQ(id5, 1);
    EXPECT_EQ(offset5, 0);
}

TEST_F(SubAllocatorTest, MergeAllSlots) {
    // Allocate the entire buffer in small chunks.
    constexpr SubAllocator::allocation_size_t CHUNK_SIZE = 128;
    constexpr uint32_t NUM_CHUNKS = TOTAL_SIZE / CHUNK_SIZE;
    std::vector<SubAllocator::AllocationId> ids;
    for (uint32_t i = 0; i < NUM_CHUNKS; ++i) {
        auto [id, offset] = mAllocator.allocate(CHUNK_SIZE);
        ASSERT_NE(id, SubAllocator::REALLOCATION_REQUIRED);
        ids.push_back(id);
    }

    // The buffer should be full.
    auto [failId, failOffset] = mAllocator.allocate(1);
    EXPECT_EQ(failId, SubAllocator::REALLOCATION_REQUIRED);
    EXPECT_EQ(failOffset, 0);

    // Retire all chunks.
    for (auto id : ids) {
        mAllocator.retire(id);
    }

    // Release and merge.
    mAllocator.releaseFreeSlots();

    // Now, the allocator should be back to its initial state with one large free block.
    // We should be able to allocate the entire buffer again.
    auto [fullId, fullOffset] = mAllocator.allocate(TOTAL_SIZE);
    EXPECT_EQ(fullId, 1);
    EXPECT_EQ(fullOffset, 0);
}

TEST_F(SubAllocatorTest, NoMergePossible) {
    // Allocate blocks in an alternating pattern.
    auto [id1, offset1] = mAllocator.allocate(128);
    auto [id2, offset2] = mAllocator.allocate(128);
    auto [id3, offset3] = mAllocator.allocate(128);
    auto [id4, offset4] = mAllocator.allocate(128);
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 3);
    EXPECT_EQ(id3, 5);
    EXPECT_EQ(id4, 7);
    EXPECT_EQ(offset1, 0);
    EXPECT_EQ(offset2, 128);
    EXPECT_EQ(offset3, 256);
    EXPECT_EQ(offset4, 384);

    // Retire the first and third blocks, leaving allocated blocks in between.
    mAllocator.retire(id1);
    mAllocator.retire(id3);

    // At this point, we have: [Free, Allocated, Free, Allocated, Free (remaining)]
    mAllocator.releaseFreeSlots();

    // Now, let's try to re-allocate the two 128-byte slots.
    auto [id5, offset5] = mAllocator.allocate(128);
    auto [id6, offset6] = mAllocator.allocate(128);
    EXPECT_EQ(offset5, 0);
    EXPECT_EQ(offset6, 256);
}

TEST_F(SubAllocatorTest, Reset) {
    auto [id1, offset1] = mAllocator.allocate(100);
    auto [id2, offset2] = mAllocator.allocate(200);
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 3);
    EXPECT_EQ(offset1, 0);
    EXPECT_EQ(offset2, 128);

    // Reset the allocator to a new size.
    mAllocator.reset(2048);
    EXPECT_EQ(mAllocator.getTotalSize(), 2048);

    // After reset, all previous allocations should be gone,
    // and we should be able to allocate the entire new size.
    auto [id3, offset3] = mAllocator.allocate(2048);
    EXPECT_EQ(id3, 1);
    EXPECT_EQ(offset3, 0);
}

TEST_F(SubAllocatorTest, ResetWithInvalidSize) {
    // Reset to a size which is not a power of two.
    EXPECT_DEATH(mAllocator.reset(123), "failed assertion");
}


TEST_F(SubAllocatorTest, ResetWithGpuLock) {
    // 1. Allocate a block and acquire a GPU lock on it.
    auto [id1, offset1] = mAllocator.allocate(128);
    EXPECT_EQ(id1, 1);
    mAllocator.acquireGpu(id1); // gpuUseCount = 1

    // 2. Call reset. This should disregard the GPU lock and clear everything.
    constexpr SubAllocator::allocation_size_t NEW_TOTAL_SIZE = 4096;
    mAllocator.reset(NEW_TOTAL_SIZE);

    // 3. Verify the allocator is in a pristine state with the new size.
    EXPECT_EQ(mAllocator.getTotalSize(), NEW_TOTAL_SIZE);

    // 4. The strongest verification is to allocate the entire new size, which should
    //    succeed, proving that the old GPU-locked block is gone.
    auto [id2, offset2] = mAllocator.allocate(NEW_TOTAL_SIZE);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(offset2, 0);
}

TEST_F(SubAllocatorTest, InvalidOperations) {
    // These operations on invalid IDs should not crash and should be handled gracefully.
    EXPECT_DEATH(mAllocator.retire(SubAllocator::UNALLOCATED), "failed assertion");
    EXPECT_DEATH(mAllocator.retire(999), "failed assertion"); // Non-existent ID

    EXPECT_DEATH(mAllocator.acquireGpu(SubAllocator::UNALLOCATED), "failed assertion");
    EXPECT_DEATH(mAllocator.acquireGpu(999), "failed assertion");

    EXPECT_DEATH(mAllocator.releaseGpu(SubAllocator::UNALLOCATED), "failed assertion");
    EXPECT_DEATH(mAllocator.releaseGpu(999), "failed assertion");

    // Check that an invalid offset query panics in debug/testing builds.
    EXPECT_DEATH(mAllocator.getAllocationOffset(SubAllocator::UNALLOCATED), "failed assertion");
    EXPECT_DEATH(mAllocator.getAllocationOffset(SubAllocator::REALLOCATION_REQUIRED),
            "failed assertion");
}

TEST_F(SubAllocatorTest, ComplexScenario) {
    std::vector<SubAllocator::AllocationId> ids;
    // 1. Allocate 4 blocks of 256 bytes
    for (int i = 0; i < 4; ++i) {
        auto [id, offset] = mAllocator.allocate(256);
        EXPECT_EQ(offset, i * 256);
        ids.push_back(id);
    }

    // Buffer should be full
    auto [failId1, failOffset1] = mAllocator.allocate(1);
    EXPECT_EQ(failId1, SubAllocator::REALLOCATION_REQUIRED);

    // 2. Retire the 2nd and 3rd blocks (ids[1] and ids[2])
    mAllocator.retire(ids[1]);
    mAllocator.retire(ids[2]);

    // 3. Release free slots. This should merge the two retired blocks.
    mAllocator.releaseFreeSlots();

    // We now have a free block of 512 bytes at offset 256.
    // Let's allocate 512 bytes. It should fit perfectly.
    auto [id, offset] = mAllocator.allocate(512);
    EXPECT_EQ(id, 5); // (256 / 64) + 1 = 5
    EXPECT_EQ(offset, 256);

    // 4. The buffer should be full again.
    auto [failId2,failOffset2] = mAllocator.allocate(1);
    EXPECT_EQ(failId2, SubAllocator::REALLOCATION_REQUIRED);
    EXPECT_EQ(failOffset2,0);
}

} // anonymous namespace