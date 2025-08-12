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

#include "dawn/native/PooledResourceMemoryAllocator.h"

#include <utility>

#include "dawn/native/Device.h"

namespace dawn::native {

PooledResourceMemoryAllocator::PooledResourceMemoryAllocator(ResourceHeapAllocator* heapAllocator)
    : mHeapAllocator(heapAllocator) {}

PooledResourceMemoryAllocator::~PooledResourceMemoryAllocator() {
    DAWN_ASSERT(mPool.empty());
}

void PooledResourceMemoryAllocator::FreeRecycledAllocations() {
    for (auto& resourceHeap : mPool) {
        DAWN_ASSERT(resourceHeap != nullptr);
        mHeapAllocator->DeallocateResourceHeap(std::move(resourceHeap));
    }

    mPool.clear();
}

ResultOrError<std::unique_ptr<ResourceHeapBase>>
PooledResourceMemoryAllocator::AllocateResourceHeap(uint64_t size) {
    // Pooled memory is LIFO because memory can be evicted by LRU. However, this means
    // pooling is disabled in-frame when the memory is still pending. For high in-frame
    // memory users, FIFO might be preferable when memory consumption is a higher priority.
    std::unique_ptr<ResourceHeapBase> memory;
    if (!mPool.empty()) {
        memory = std::move(mPool.front());
        mPool.pop_front();
    }

    if (memory == nullptr) {
        DAWN_TRY_ASSIGN(memory, mHeapAllocator->AllocateResourceHeap(size));
    }

    return std::move(memory);
}

void PooledResourceMemoryAllocator::DeallocateResourceHeap(
    std::unique_ptr<ResourceHeapBase> allocation) {
    mPool.push_front(std::move(allocation));
}

uint64_t PooledResourceMemoryAllocator::GetPoolSizeForTesting() const {
    return mPool.size();
}
}  // namespace dawn::native
