// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/common/SlabAllocator.h"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <new>
#include "dawn/common/AlignedAlloc.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"

namespace dawn {

// IndexLinkNode

SlabAllocatorImpl::IndexLinkNode::IndexLinkNode(Index index, Index nextIndex)
    : index(index), nextIndex(nextIndex) {}

// Slab

SlabAllocatorImpl::Slab::Slab() = default;
SlabAllocatorImpl::Slab::Slab(char allocation[], IndexLinkNode* head)
    : allocation(allocation), freeList(head) {}

SlabAllocatorImpl::Slab::Slab(Slab&& rhs) = default;

// SentinelSlab

SlabAllocatorImpl::SentinelSlab::SentinelSlab() = default;
SlabAllocatorImpl::SentinelSlab::SentinelSlab(SentinelSlab&& rhs) = default;

SlabAllocatorImpl::SentinelSlab::~SentinelSlab() {
    // Delete the full linked list.
    while (next) {
        Slab* slab = next;
        slab->Splice();
        DAWN_ASSERT(slab->blocksInUse == 0);
        char* allocation = slab->allocation;
        slab->~Slab();  // Placement delete.
        AlignedFree(allocation);
    }
}

// SlabAllocatorImpl

SlabAllocatorImpl::Index SlabAllocatorImpl::kInvalidIndex =
    std::numeric_limits<SlabAllocatorImpl::Index>::max();

SlabAllocatorImpl::SlabAllocatorImpl(Index blocksPerSlab,
                                     uint32_t objectSize,
                                     uint32_t objectAlignment)
    : mAllocationAlignment(std::max(u32_alignof<Slab>, objectAlignment)),
      mSlabBlocksOffset(Align(u32_sizeof<Slab>, objectAlignment)),
      mIndexLinkNodeOffset(Align(objectSize, alignof(IndexLinkNode))),
      mBlockStride(Align(mIndexLinkNodeOffset + u32_sizeof<IndexLinkNode>, objectAlignment)),
      mBlocksPerSlab(blocksPerSlab),
      mTotalAllocationSize(static_cast<size_t>(mSlabBlocksOffset) + mBlocksPerSlab * mBlockStride) {
    DAWN_ASSERT(IsPowerOfTwo(mAllocationAlignment));
}

SlabAllocatorImpl::SlabAllocatorImpl(SlabAllocatorImpl&& rhs)
    : mAllocationAlignment(rhs.mAllocationAlignment),
      mSlabBlocksOffset(rhs.mSlabBlocksOffset),
      mIndexLinkNodeOffset(rhs.mIndexLinkNodeOffset),
      mBlockStride(rhs.mBlockStride),
      mBlocksPerSlab(rhs.mBlocksPerSlab),
      mTotalAllocationSize(rhs.mTotalAllocationSize),
      mAvailableSlabs(std::move(rhs.mAvailableSlabs)),
      mFullSlabs(std::move(rhs.mFullSlabs)),
      mRecycledSlabs(std::move(rhs.mRecycledSlabs)) {}

SlabAllocatorImpl::~SlabAllocatorImpl() = default;

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::OffsetFrom(
    IndexLinkNode* node,
    std::make_signed_t<Index> offset) const {
    return reinterpret_cast<IndexLinkNode*>(reinterpret_cast<char*>(node) +
                                            static_cast<intptr_t>(mBlockStride) * offset);
}

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::NodeFromObject(void* object) const {
    return reinterpret_cast<SlabAllocatorImpl::IndexLinkNode*>(static_cast<char*>(object) +
                                                               mIndexLinkNodeOffset);
}

void* SlabAllocatorImpl::ObjectFromNode(IndexLinkNode* node) const {
    return static_cast<void*>(reinterpret_cast<char*>(node) - mIndexLinkNodeOffset);
}

bool SlabAllocatorImpl::IsNodeInSlab(Slab* slab, IndexLinkNode* node) const {
    char* firstObjectPtr = reinterpret_cast<char*>(slab) + mSlabBlocksOffset;
    IndexLinkNode* firstNode = NodeFromObject(firstObjectPtr);
    IndexLinkNode* lastNode = OffsetFrom(firstNode, mBlocksPerSlab - 1);
    return node >= firstNode && node <= lastNode && node->index < mBlocksPerSlab;
}

void SlabAllocatorImpl::PushFront(Slab* slab, IndexLinkNode* node) const {
    DAWN_ASSERT(IsNodeInSlab(slab, node));

    IndexLinkNode* head = slab->freeList;
    if (head == nullptr) {
        node->nextIndex = kInvalidIndex;
    } else {
        DAWN_ASSERT(IsNodeInSlab(slab, head));
        node->nextIndex = head->index;
    }
    slab->freeList = node;

    DAWN_ASSERT(slab->blocksInUse != 0);
    slab->blocksInUse--;
}

SlabAllocatorImpl::IndexLinkNode* SlabAllocatorImpl::PopFront(Slab* slab) const {
    DAWN_ASSERT(slab->freeList != nullptr);

    IndexLinkNode* head = slab->freeList;
    if (head->nextIndex == kInvalidIndex) {
        slab->freeList = nullptr;
    } else {
        DAWN_ASSERT(IsNodeInSlab(slab, head));
        slab->freeList = OffsetFrom(head, head->nextIndex - head->index);
        DAWN_ASSERT(IsNodeInSlab(slab, slab->freeList));
    }

    DAWN_ASSERT(slab->blocksInUse < mBlocksPerSlab);
    slab->blocksInUse++;
    return head;
}

void SlabAllocatorImpl::SentinelSlab::Prepend(SlabAllocatorImpl::Slab* slab) {
    if (next != nullptr) {
        next->prev = slab;
    }
    slab->prev = this;
    slab->next = next;
    next = slab;
}

void SlabAllocatorImpl::Slab::Splice() {
    DAWN_ASSERT(prev != nullptr);
    prev->next = next;
    if (next != nullptr) {
        next->prev = prev;
    }
    prev = nullptr;
    next = nullptr;
}

void* SlabAllocatorImpl::Allocate() {
    if (mAvailableSlabs.next == nullptr) {
        GetNewSlab();
    }

    Slab* slab = mAvailableSlabs.next;
    IndexLinkNode* node = PopFront(slab);
    DAWN_ASSERT(node != nullptr);

    // Move full slabs to a separate list, so allocate can always return quickly.
    if (slab->blocksInUse == mBlocksPerSlab) {
        slab->Splice();
        mFullSlabs.Prepend(slab);
    }

    return ObjectFromNode(node);
}

void SlabAllocatorImpl::Deallocate(void* ptr) {
    IndexLinkNode* node = NodeFromObject(ptr);

    DAWN_ASSERT(node->index < mBlocksPerSlab);
    void* firstAllocation = ObjectFromNode(OffsetFrom(node, -node->index));
    Slab* slab = reinterpret_cast<Slab*>(static_cast<char*>(firstAllocation) - mSlabBlocksOffset);
    DAWN_ASSERT(slab != nullptr);

    bool slabWasFull = slab->blocksInUse == mBlocksPerSlab;
    DAWN_ASSERT(slab->blocksInUse != 0);
    PushFront(slab, node);

    if (slabWasFull) {
        // Slab is in the full list. Move it to the recycled list.
        DAWN_ASSERT(slab->freeList != nullptr);
        slab->Splice();
        mRecycledSlabs.Prepend(slab);
    }

    // TODO(crbug.com/dawn/825): Occasionally prune slabs if |blocksInUse == 0|.
    // Doing so eagerly hurts performance.
}

void SlabAllocatorImpl::GetNewSlab() {
    // Should only be called when there are no available slabs.
    DAWN_ASSERT(mAvailableSlabs.next == nullptr);

    if (mRecycledSlabs.next != nullptr) {
        // If the recycled list is non-empty, swap their contents.
        std::swap(mAvailableSlabs.next, mRecycledSlabs.next);

        // We swapped the next pointers, so the prev pointer is wrong.
        // Update it here.
        mAvailableSlabs.next->prev = &mAvailableSlabs;
        DAWN_ASSERT(mRecycledSlabs.next == nullptr);
        return;
    }

    char* alignedPtr = static_cast<char*>(AlignedAlloc(mTotalAllocationSize, mAllocationAlignment));

    char* dataStart = alignedPtr + mSlabBlocksOffset;

    IndexLinkNode* node = NodeFromObject(dataStart);
    for (uint32_t i = 0; i < mBlocksPerSlab; ++i) {
        new (OffsetFrom(node, i)) IndexLinkNode(i, i + 1);
    }

    IndexLinkNode* lastNode = OffsetFrom(node, mBlocksPerSlab - 1);
    lastNode->nextIndex = kInvalidIndex;

    mAvailableSlabs.Prepend(new (alignedPtr) Slab(alignedPtr, node));
}

}  // namespace dawn
