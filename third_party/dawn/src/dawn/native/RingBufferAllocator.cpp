// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/RingBufferAllocator.h"

#include <utility>

#include "dawn/common/Math.h"

// Note: Current RingBufferAllocator implementation uses two indices (start and end) to implement a
// circular queue. However, this approach defines a full queue when one element is still unused.
//
// For example, [E,E,E,E] would be equivelent to [U,U,U,U].
//                 ^                                ^
//                S=E=1                            S=E=1
//
// The latter case is eliminated by counting used bytes >= capacity. This definition prevents
// (the last) byte and requires an extra variable to count used bytes. Alternatively, we could use
// only two indices that keep increasing (unbounded) but can be still indexed using bit masks.
// However, this 1) requires the size to always be a power-of-two and 2) remove tests that check
// used bytes.
namespace dawn::native {

RingBufferAllocator::RingBufferAllocator() = default;

RingBufferAllocator::RingBufferAllocator(uint64_t maxSize) : mMaxBlockSize(maxSize) {}

RingBufferAllocator::RingBufferAllocator(const RingBufferAllocator&) = default;

RingBufferAllocator::~RingBufferAllocator() = default;

RingBufferAllocator& RingBufferAllocator::operator=(const RingBufferAllocator&) = default;

void RingBufferAllocator::Deallocate(ExecutionSerial lastCompletedSerial) {
    // Reclaim memory from previously recorded blocks.
    for (Request& request : mInflightRequests.IterateUpTo(lastCompletedSerial)) {
        mUsedStartOffset = request.endOffset;
        mUsedSize -= request.size;
    }

    // Dequeue previously recorded requests.
    mInflightRequests.ClearUpTo(lastCompletedSerial);
}

uint64_t RingBufferAllocator::GetSize() const {
    return mMaxBlockSize;
}

uint64_t RingBufferAllocator::GetUsedSize() const {
    return mUsedSize;
}

bool RingBufferAllocator::Empty() const {
    return mInflightRequests.Empty();
}

// Sub-allocate the ring-buffer by requesting a chunk of the specified size.
// This is a serial-based resource scheme, the life-span of resources (and the allocations) get
// tracked by GPU progress via serials. Memory can be reused by determining if the GPU has
// completed up to a given serial. Each sub-allocation request is tracked in the serial offset
// queue, which identifies an existing (or new) frames-worth of resources. Internally, the
// ring-buffer maintains offsets of 3 "memory" states: Free, Reclaimed, and Used. This is done
// in FIFO order as older frames would free resources before newer ones.
uint64_t RingBufferAllocator::Allocate(uint64_t allocationSize,
                                       ExecutionSerial serial,
                                       uint64_t offsetAlignment) {
    // Check if the buffer is full by comparing the used size.
    // If the buffer is not split where waste occurs (e.g. cannot fit new sub-alloc in front), a
    // subsequent sub-alloc could fail where the used size was previously adjusted to include
    // the wasted.
    if (mUsedSize >= mMaxBlockSize) {
        return kInvalidOffset;
    }

    // Ensure adding allocationSize does not overflow.
    const uint64_t remainingSize = (mMaxBlockSize - mUsedSize);
    if (allocationSize > remainingSize) {
        return kInvalidOffset;
    }

    uint64_t startOffset = kInvalidOffset;
    uint64_t currentRequestSize = 0u;

    // Compute an alignment offset for the buffer if allocating at the end.
    const uint64_t alignmentOffset = Align(mUsedEndOffset, offsetAlignment) - mUsedEndOffset;
    const uint64_t alignedUsedEndOffset = mUsedEndOffset + alignmentOffset;

    // Check if the buffer is NOT split (i.e sub-alloc on ends)
    if (mUsedStartOffset <= mUsedEndOffset) {
        // Order is important (try to sub-alloc at end first).
        // This is due to FIFO order where sub-allocs are inserted from left-to-right (when not
        // wrapped).
        if (alignedUsedEndOffset + allocationSize <= mMaxBlockSize) {
            startOffset = alignedUsedEndOffset;
            mUsedSize += allocationSize + alignmentOffset;
            currentRequestSize = allocationSize + alignmentOffset;
        } else if (allocationSize <= mUsedStartOffset) {  // Try to sub-alloc at front.
            // Count the space at the end so that a subsequent
            // sub-alloc cannot fail when the buffer is full.
            const uint64_t requestSize = (mMaxBlockSize - mUsedEndOffset) + allocationSize;

            startOffset = 0;
            mUsedSize += requestSize;
            currentRequestSize = requestSize;
        }
    } else if (alignedUsedEndOffset + allocationSize <= mUsedStartOffset) {
        // Otherwise, buffer is split where sub-alloc must be in-between.
        startOffset = alignedUsedEndOffset;
        mUsedSize += allocationSize + alignmentOffset;
        currentRequestSize = allocationSize + alignmentOffset;
    }

    if (startOffset != kInvalidOffset) {
        mUsedEndOffset = startOffset + allocationSize;

        Request request;
        request.endOffset = mUsedEndOffset;
        request.size = currentRequestSize;

        mInflightRequests.Enqueue(std::move(request), serial);
    }

    return startOffset;
}
}  // namespace dawn::native
