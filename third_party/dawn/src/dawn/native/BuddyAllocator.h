// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_BUDDYALLOCATOR_H_
#define SRC_DAWN_NATIVE_BUDDYALLOCATOR_H_

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

// Buddy allocator uses the buddy memory allocation technique to satisfy an allocation request.
// Memory is split into halves until just large enough to fit to the request. This
// requires the allocation size to be a power-of-two value. The allocator "allocates" a block by
// returning the starting offset whose size is guaranteed to be greater than or equal to the
// allocation size. To deallocate, the same offset is used to find the corresponding block.
//
// Internally, it manages a free list to track free blocks in a full binary tree.
// Every index in the free list corresponds to a level in the tree. That level also determines
// the size of the block to be used to satisfy the request. The first level (index=0) represents
// the root whose size is also called the max block size.
//
class BuddyAllocator {
  public:
    explicit BuddyAllocator(uint64_t maxSize);
    ~BuddyAllocator();

    // Required methods.
    uint64_t Allocate(uint64_t allocationSize, uint64_t alignment = 1);
    void Deallocate(uint64_t offset);

    // For testing purposes only.
    uint64_t ComputeTotalNumOfFreeBlocksForTesting() const;

    static constexpr uint64_t kInvalidOffset = std::numeric_limits<uint64_t>::max();

  private:
    uint32_t ComputeLevelFromBlockSize(uint64_t blockSize) const;
    uint64_t GetNextFreeAlignedBlock(size_t allocationBlockLevel, uint64_t alignment) const;

    enum class BlockState { Free, Split, Allocated };

    struct BuddyBlock {
        BuddyBlock(uint64_t size, uint64_t offset)
            : mOffset(offset), mSize(size), mState(BlockState::Free) {
            free.pPrev = nullptr;
            free.pNext = nullptr;
        }

        uint64_t mOffset;
        uint64_t mSize;

        // Pointer to this block's buddy, iff parent is split.
        // Used to quickly merge buddy blocks upon de-allocate.
        raw_ptr<BuddyBlock> pBuddy = nullptr;
        raw_ptr<BuddyBlock> pParent = nullptr;

        // Track whether this block has been split or not.
        BlockState mState;

        struct FreeLinks {
            BuddyBlock* pPrev;
            BuddyBlock* pNext;
        };

        struct SplitLink {
            BuddyBlock* pLeft;
        };

        union {
            // Used upon allocation.
            // Avoids searching for the next free block.
            FreeLinks free;

            // Used upon de-allocation.
            // Had this block split upon allocation, it and it's buddy is to be deleted.
            SplitLink split;
        };
    };

    void InsertFreeBlock(BuddyBlock* block, size_t level);
    void RemoveFreeBlock(BuddyBlock* block, size_t level);
    void DeleteBlock(BuddyBlock* block);

    uint64_t ComputeNumOfFreeBlocks(BuddyBlock* block) const;

    // Keep track the head and tail (for faster insertion/removal).
    struct BlockList {
        raw_ptr<BuddyBlock> head = nullptr;  // First free block in level.
        // TODO(crbug.com/dawn/827): Track the tail.
    };

    raw_ptr<BuddyBlock> mRoot = nullptr;  // Used to deallocate non-free blocks.

    uint64_t mMaxBlockSize = 0;

    // List of linked-lists of free blocks where the index is a level that
    // corresponds to a power-of-two sized block.
    std::vector<BlockList> mFreeLists;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUDDYALLOCATOR_H_
