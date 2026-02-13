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

#ifndef SRC_DAWN_NATIVE_RESOURCEMEMORYALLOCATION_H_
#define SRC_DAWN_NATIVE_RESOURCEMEMORYALLOCATION_H_

#include <cstdint>

namespace dawn::native {

class ResourceHeapBase;

// Allocation method determines how memory was sub-divided.
// Used by the device to get the allocator that was responsible for the allocation.
enum class AllocationMethod {
    // Memory not sub-divided.
    kDirect,

    // Memory sub-divided using one or more blocks of various sizes.
    kSubAllocated,

    // Memory was allocated outside of Dawn.
    kExternal,

    // Memory not allocated or freed.
    kInvalid
};

// Metadata that describes how the allocation was allocated.
struct AllocationInfo {
    // AllocationInfo contains a separate offset to not confuse block vs memory offsets.
    // The block offset is within the entire allocator memory range and only required by the
    // buddy sub-allocator to get the corresponding memory. Unlike the block offset, the
    // allocation offset is always local to the memory.
    uint64_t mBlockOffset = 0;

    AllocationMethod mMethod = AllocationMethod::kInvalid;

    // Represents the requested memory allocation size (without padding) by the allocator.
    uint64_t mRequestedSize = 0;

    // Tracks whether the memory allocation contains VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT.
    bool mIsLazyAllocated = false;
};

// Handle into a resource heap pool.
class ResourceMemoryAllocation {
  public:
    ResourceMemoryAllocation();
    ResourceMemoryAllocation(const AllocationInfo& info,
                             uint64_t offset,
                             ResourceHeapBase* resourceHeap,
                             uint8_t* mappedPointer = nullptr);
    virtual ~ResourceMemoryAllocation() = default;

    ResourceMemoryAllocation(const ResourceMemoryAllocation&) = default;
    ResourceMemoryAllocation& operator=(const ResourceMemoryAllocation&) = default;

    ResourceHeapBase* GetResourceHeap() const;
    uint64_t GetOffset() const;
    uint8_t* GetMappedPointer() const;
    AllocationInfo GetInfo() const;

    virtual void Invalidate();

  private:
    AllocationInfo mInfo;
    uint64_t mOffset;
    ResourceHeapBase* mResourceHeap;
    uint8_t* mMappedPointer;
};
}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RESOURCEMEMORYALLOCATION_H_
