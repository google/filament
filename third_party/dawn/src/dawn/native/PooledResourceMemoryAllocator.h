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

#ifndef SRC_DAWN_NATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_
#define SRC_DAWN_NATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_

#include <deque>
#include <memory>

#include "dawn/common/SerialQueue.h"
#include "dawn/native/ResourceHeapAllocator.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class DeviceBase;

// |PooledResourceMemoryAllocator| allocates a fixed-size resource memory from a resource memory
// pool. Internally, it manages a list of heaps using LIFO (newest heaps are recycled first).
// The heap is in one of two states: AVAILABLE or not. Upon de-allocate, the heap is returned
// the pool and made AVAILABLE.
class PooledResourceMemoryAllocator : public ResourceHeapAllocator {
  public:
    explicit PooledResourceMemoryAllocator(ResourceHeapAllocator* heapAllocator);
    ~PooledResourceMemoryAllocator() override;

    ResultOrError<std::unique_ptr<ResourceHeapBase>> AllocateResourceHeap(uint64_t size) override;
    void DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> allocation) override;

    void FreeRecycledAllocations();

    // For testing purposes.
    uint64_t GetPoolSizeForTesting() const;

  private:
    raw_ptr<ResourceHeapAllocator> mHeapAllocator = nullptr;

    std::deque<std::unique_ptr<ResourceHeapBase>> mPool;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_POOLEDRESOURCEMEMORYALLOCATOR_H_
