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

#ifndef SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_
#define SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_

#include <memory>
#include <vector>

#include "dawn/native/BuddyAllocator.h"
#include "dawn/native/Error.h"
#include "dawn/native/ResourceMemoryAllocation.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native {

class ResourceHeapAllocator;

// BuddyMemoryAllocator uses the buddy allocator to sub-allocate blocks of device
// memory created by MemoryAllocator clients. It creates a very large buddy system
// where backing device memory blocks equal a specified level in the system.
//
// Upon sub-allocating, the offset gets mapped to device memory by computing the corresponding
// memory index and should the memory not exist, it is created. If two sub-allocations share the
// same memory index, the memory refcount is incremented to ensure de-allocating one doesn't
// release the other prematurely.
//
// The MemoryAllocator should return ResourceHeaps that are all compatible with each other.
// It should also outlive all the resources that are in the buddy allocator.
class BuddyMemoryAllocator {
  public:
    BuddyMemoryAllocator(uint64_t maxSystemSize,
                         uint64_t memoryBlockSize,
                         ResourceHeapAllocator* heapAllocator);
    ~BuddyMemoryAllocator();

    ResultOrError<ResourceMemoryAllocation> Allocate(uint64_t allocationSize,
                                                     uint64_t alignment,
                                                     bool isLazyMemoryType);
    void Deallocate(const ResourceMemoryAllocation& allocation);

    uint64_t GetMemoryBlockSize() const;

    // For testing purposes.
    uint64_t ComputeTotalNumOfHeapsForTesting() const;

  private:
    uint64_t GetMemoryIndex(uint64_t offset) const;

    uint64_t mMemoryBlockSize = 0;

    BuddyAllocator mBuddyBlockAllocator;
    raw_ptr<ResourceHeapAllocator> mHeapAllocator;

    struct TrackedSubAllocations {
        size_t refcount = 0;
        std::unique_ptr<ResourceHeapBase> mMemoryAllocation;
    };

    std::vector<TrackedSubAllocations> mTrackedSubAllocations;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_BUDDYMEMORYALLOCATOR_H_
