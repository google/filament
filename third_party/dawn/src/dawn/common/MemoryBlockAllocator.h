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

#ifndef SRC_DAWN_COMMON_MEMORYBLOCKALLOCATOR_H_
#define SRC_DAWN_COMMON_MEMORYBLOCKALLOCATOR_H_

#include <cstdint>
#include <utility>
#include <vector>

#include "src/dawn/common/LinkedList.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/utils/heap_array.h"
#include "src/utils/non_copyable.h"
#include "src/utils/non_movable.h"

namespace dawn {

// MemoryBlockAllocator is a pool allocator that recycles fixed-size memory blocks
// (`HeapArray<std::byte>`).
//
// All pooled blocks are uniformly block size (defaults to 16 KB). Requests larger than the block
// size bypass the pool entirely: allocated and freed directly, unpooled.
//
// Block retention is tracked via an internal tick serial: every call to Tick() increments
// mTickSerial by 1, and blocks whose age exceeds the keep alive window are evicted.
//
// Recycled blocks are stored as an intrusive LinkedList<FreeBlock>, protected by MutexProtected.
// Each FreeBlock node is placement-new'd directly inside the (otherwise idle) memory block it
// describes, so recycling a block requires no separate allocation. Appending to the tail
// (`Append()`) and popping from the tail provides LIFO / MRU allocation order so that returned
// blocks remain warm in CPU cache. Since new nodes are always appended at the tail, the oldest
// blocks remain at the head for early-terminating eviction in Tick().
class MemoryBlockAllocator : public dawn::NonCopyable {
  public:
    // Sized slightly below 16 KiB (16384 bytes) so that heap allocator bookkeeping
    // metadata (such as malloc/PartitionAlloc chunk headers) doesn't cause the total
    // allocation to exceed the 16 KiB power-of-two size bin and round up to the next bin.
    static constexpr size_t kDefaultBlockSize = 16352;

    explicit MemoryBlockAllocator(size_t blockSize = kDefaultBlockSize,
                                  uint64_t keepAliveWindow = 60);
    ~MemoryBlockAllocator();

    HeapArray<std::byte> Allocate(size_t minimumSize);
    void Return(HeapArray<std::byte>&& block);
    void Return(std::vector<HeapArray<std::byte>>&& blocks);
    void Tick();
    void TrimMemory();
    size_t GetRecycledBlockCountForTesting();
    size_t GetBlockSize() const;

  private:
    // A FreeBlock is placement-new'd into the HeapArray it describes, so that recycling a block
    // requires no separate node allocation.
    class FreeBlock : public LinkNode<FreeBlock>, public dawn::NonMovable {
      public:
        static FreeBlock* FromHeapArray(uint64_t serial, HeapArray<std::byte> block);
        HeapArray<std::byte> UnlinkAndAcquireHeapArray();

        uint64_t GetLastKnownSerial() const { return mLastKnownSerial; }

      private:
        FreeBlock(uint64_t serial, HeapArray<std::byte> block);
        ~FreeBlock() = default;

        uint64_t mLastKnownSerial;
        // We store the HeapArray inside FreeBlock to simplify manipulating and reclaiming the
        // memory allocation.
        HeapArray<std::byte> mSelfBlock;
    };

    const size_t mBlockSize;
    const uint64_t mKeepAliveWindow;
    MutexProtected<LinkedList<FreeBlock>> mFreeList;
    uint64_t mTickSerial = 0;
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_MEMORYBLOCKALLOCATOR_H_
