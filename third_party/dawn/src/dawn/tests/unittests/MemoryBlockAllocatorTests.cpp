// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/dawn/common/MemoryBlockAllocator.h"

namespace dawn {
namespace {

constexpr size_t kBlockSize = 16384;

}  // anonymous namespace

// A deallocated block is handed back out by a later request for kBlockSize,
// instead of a fresh block being allocated.
TEST(MemoryBlockAllocator, AllocateReturnReusesBlock) {
    MemoryBlockAllocator pool(kBlockSize);

    HeapArray<std::byte> block = pool.Allocate(kBlockSize);
    std::byte* originalPtr = block.data();

    std::vector<HeapArray<std::byte>> blocks;
    blocks.push_back(std::move(block));
    pool.Return(std::move(blocks));
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 1u);

    HeapArray<std::byte> reused = pool.Allocate(kBlockSize);
    EXPECT_EQ(reused.data(), originalPtr);
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);

    std::vector<HeapArray<std::byte>> cleanUp;
    cleanUp.push_back(std::move(reused));
    pool.Return(std::move(cleanUp));
}

// Blocks not reused within the keep alive window are evicted when Tick() increments
// mTickSerial past their eviction threshold; blocks within the window survive.
TEST(MemoryBlockAllocator, TickEvictsColdBlocks) {
    MemoryBlockAllocator pool(kBlockSize, 2);

    std::vector<HeapArray<std::byte>> blocks;
    blocks.push_back(pool.Allocate(kBlockSize));
    pool.Return(std::move(blocks));

    pool.Tick();
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 1u);

    pool.Tick();
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);
}

// TrimMemory() drops every pooled block unconditionally.
TEST(MemoryBlockAllocator, TrimMemoryDropsEverythingRegardlessOfSerial) {
    MemoryBlockAllocator pool(kBlockSize);

    std::vector<HeapArray<std::byte>> blocks;
    blocks.push_back(pool.Allocate(kBlockSize));
    pool.Return(std::move(blocks));

    pool.TrimMemory();
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);
}

// Requests larger than block size bypass the pool.
TEST(MemoryBlockAllocator, OversizedBypassesPool) {
    MemoryBlockAllocator pool(kBlockSize);
    HeapArray<std::byte> block = pool.Allocate(kBlockSize + 1);
    EXPECT_EQ(block.size(), kBlockSize + 1);

    std::vector<HeapArray<std::byte>> blocks;
    blocks.push_back(std::move(block));
    pool.Return(std::move(blocks));
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);
}

// Verify that Return pushes to the back of the free list, and Allocate
// pops from the back, resulting in exact LIFO (MRU) reuse order.
TEST(MemoryBlockAllocator, LIFOOrderAndReuse) {
    MemoryBlockAllocator pool(kBlockSize);

    HeapArray<std::byte> blockA = pool.Allocate(kBlockSize);
    HeapArray<std::byte> blockB = pool.Allocate(kBlockSize);
    HeapArray<std::byte> blockC = pool.Allocate(kBlockSize);

    std::byte* ptrA = blockA.data();
    std::byte* ptrB = blockB.data();
    std::byte* ptrC = blockC.data();

    // Return sequentially: A, then B, then C.
    // Back of deque after return should be C (with B then A before it).
    std::vector<HeapArray<std::byte>> batch1;
    batch1.push_back(std::move(blockA));
    std::vector<HeapArray<std::byte>> batch2;
    batch2.push_back(std::move(blockB));
    std::vector<HeapArray<std::byte>> batch3;
    batch3.push_back(std::move(blockC));
    pool.Return(std::move(batch1));
    pool.Return(std::move(batch2));
    pool.Return(std::move(batch3));
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 3u);

    // Allocate sequentially: should pop C, then B, then A.
    HeapArray<std::byte> pop1 = pool.Allocate(kBlockSize);
    HeapArray<std::byte> pop2 = pool.Allocate(kBlockSize);
    HeapArray<std::byte> pop3 = pool.Allocate(kBlockSize);

    EXPECT_EQ(pop1.data(), ptrC);
    EXPECT_EQ(pop2.data(), ptrB);
    EXPECT_EQ(pop3.data(), ptrA);
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);

    std::vector<HeapArray<std::byte>> cleanUp;
    cleanUp.push_back(std::move(pop1));
    cleanUp.push_back(std::move(pop2));
    cleanUp.push_back(std::move(pop3));
    pool.Return(std::move(cleanUp));
}

// Verify that many blocks can be allocated, returned in batch, and reallocated cleanly.
TEST(MemoryBlockAllocator, AllocateAndReturnManyBlocks) {
    MemoryBlockAllocator pool(kBlockSize);
    constexpr size_t kNumBlocks = 100;

    std::vector<HeapArray<std::byte>> blocks;
    blocks.reserve(kNumBlocks);
    std::vector<std::byte*> origPtrs;
    origPtrs.reserve(kNumBlocks);

    for (size_t i = 0; i < kNumBlocks; ++i) {
        HeapArray<std::byte> block = pool.Allocate(kBlockSize);
        origPtrs.push_back(block.data());
        blocks.push_back(std::move(block));
    }

    pool.Return(std::move(blocks));
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), kNumBlocks);

    std::vector<HeapArray<std::byte>> reallocated;
    reallocated.reserve(kNumBlocks);
    for (size_t i = 0; i < kNumBlocks; ++i) {
        HeapArray<std::byte> block = pool.Allocate(kBlockSize);
        // Since list is LIFO, the i-th pop corresponds to the (kNumBlocks - 1 - i)-th push.
        EXPECT_EQ(block.data(), origPtrs[kNumBlocks - 1 - i]);
        reallocated.push_back(std::move(block));
    }
    EXPECT_EQ(pool.GetRecycledBlockCountForTesting(), 0u);

    pool.Return(std::move(reallocated));
}

}  // namespace dawn
