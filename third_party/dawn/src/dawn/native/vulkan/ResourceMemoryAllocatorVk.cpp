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

#include "dawn/native/vulkan/ResourceMemoryAllocatorVk.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Math.h"
#include "dawn/native/BuddyMemoryAllocator.h"
#include "dawn/native/Queue.h"
#include "dawn/native/ResourceHeapAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/ResourceHeapVk.h"
#include "dawn/native/vulkan/VulkanError.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::vulkan {

namespace {

// On Vulkan the memory type of the mappable buffers with extended usages must have all below memory
// property flags.
constexpr VkMemoryPropertyFlags kMapExtendedUsageMemoryPropertyFlags =
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

VkDeviceSize GetMaxSuballocationSize(VkDeviceSize heapBlockSize) {
    // Have each bucket of the buddy system allocate at least some resource of the maximum
    // size
    // TODO(crbug.com/dawn/849): This is a hardcoded heuristic to choose when to suballocate but it
    // should ideally depend on the size of the memory heaps and other factors.
    return heapBlockSize / 2;
}

bool IsMemoryKindMappable(MemoryKind memoryKind) {
    return memoryKind & (MemoryKind::ReadMappable | MemoryKind::WriteMappable);
}

bool HasMemoryTypeWithFlags(const VulkanDeviceInfo& deviceInfo, VkMemoryPropertyFlags flags) {
    for (auto& memoryType : deviceInfo.memoryTypes) {
        if ((memoryType.propertyFlags & flags) == flags) {
            return true;
        }
    }
    return false;
}

}  // anonymous namespace

bool SupportsBufferMapExtendedUsages(const VulkanDeviceInfo& deviceInfo) {
    return HasMemoryTypeWithFlags(deviceInfo, kMapExtendedUsageMemoryPropertyFlags);
}

// SingleTypeAllocator is a combination of a BuddyMemoryAllocator and its client and can
// service suballocation requests, but for a single Vulkan memory type.
class ResourceMemoryAllocator::SingleTypeAllocator : public ResourceHeapAllocator {
  public:
    SingleTypeAllocator(Device* device,
                        size_t memoryTypeIndex,
                        bool isLazyMemoryType,
                        VkDeviceSize maxHeapSize,
                        VkDeviceSize heapBlockSize,
                        ResourceMemoryAllocator* memoryAllocator)
        : mDevice(device),
          mResourceMemoryAllocator(memoryAllocator),
          mMemoryTypeIndex(memoryTypeIndex),
          mIsLazyMemoryType(isLazyMemoryType),
          mMaxHeapSize(maxHeapSize),
          mPooledMemoryAllocator(this),
          mBuddySystem(
              // Round down to a power of 2 that's <= mMemoryHeapSize. This will always
              // be a multiple of heapBlockSize because heapBlockSize is a power of 2.
              uint64_t(1) << Log2(mMaxHeapSize),
              // Take the min in the very unlikely case the memory heap is tiny.
              std::min(uint64_t(1) << Log2(mMaxHeapSize), heapBlockSize),
              &mPooledMemoryAllocator) {
        DAWN_ASSERT(IsPowerOfTwo(heapBlockSize));
    }
    ~SingleTypeAllocator() override = default;

    bool IsLazyMemoryType() const { return mIsLazyMemoryType; }

    // Frees any heaps that are unused and waiting to be recycled by the pool allocator.
    void FreeRecycledMemory() { mPooledMemoryAllocator.FreeRecycledAllocations(); }

    ResultOrError<ResourceMemoryAllocation> AllocateMemory(uint64_t size, uint64_t alignment) {
        return mBuddySystem.Allocate(size, alignment, mIsLazyMemoryType);
    }

    void DeallocateMemory(const ResourceMemoryAllocation& allocation) {
        mBuddySystem.Deallocate(allocation);
    }

    // Implementation of the MemoryAllocator interface to be a client of BuddyMemoryAllocator
    ResultOrError<std::unique_ptr<ResourceHeapBase>> AllocateResourceHeap(uint64_t size) override {
        if (size > mMaxHeapSize) {
            return DAWN_OUT_OF_MEMORY_ERROR("Allocation size too large");
        }

        VkMemoryAllocateInfo allocateInfo;
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.pNext = nullptr;
        allocateInfo.allocationSize = size;
        allocateInfo.memoryTypeIndex = mMemoryTypeIndex;

        VkDeviceMemory allocatedMemory = VK_NULL_HANDLE;

        // First check OOM that we want to surface to the application.
        DAWN_TRY(
            CheckVkOOMThenSuccess(mDevice->fn.AllocateMemory(mDevice->GetVkDevice(), &allocateInfo,
                                                             nullptr, &*allocatedMemory),
                                  "vkAllocateMemory"));

        DAWN_ASSERT(allocatedMemory != VK_NULL_HANDLE);
        mResourceMemoryAllocator->RecordHeapAllocation(size, mIsLazyMemoryType);
        return {std::make_unique<ResourceHeap>(allocatedMemory, mMemoryTypeIndex, size)};
    }

    void DeallocateResourceHeap(std::unique_ptr<ResourceHeapBase> allocation) override {
        mResourceMemoryAllocator->DeallocateResourceHeap(ToBackend(allocation.get()),
                                                         mIsLazyMemoryType);
    }

  private:
    raw_ptr<Device> mDevice;
    raw_ptr<ResourceMemoryAllocator> mResourceMemoryAllocator;
    size_t mMemoryTypeIndex;
    const bool mIsLazyMemoryType;
    VkDeviceSize mMaxHeapSize;
    PooledResourceMemoryAllocator mPooledMemoryAllocator;
    BuddyMemoryAllocator mBuddySystem;
};

void ResourceMemoryAllocator::AllocationSizeTracker::Increment(VkDeviceSize incrementSize) {
    mTotalSize += incrementSize;
}

void ResourceMemoryAllocator::AllocationSizeTracker::Decrement(ExecutionSerial currentSerial,
                                                               VkDeviceSize decrementSize) {
    DAWN_ASSERT(mTotalSize >= decrementSize);
    mMemoryToDecrement[currentSerial] += decrementSize;
}

void ResourceMemoryAllocator::AllocationSizeTracker::Tick(ExecutionSerial completedSerial) {
    auto it = mMemoryToDecrement.begin();
    while (it != mMemoryToDecrement.end() && it->first <= completedSerial) {
        // Update tracking for allocation/used memory that will be deallocated.
        DAWN_ASSERT(mTotalSize >= it->second);
        mTotalSize -= it->second;
        it++;
    }
    // Erase the map serials up to the completed serial.
    mMemoryToDecrement.erase(mMemoryToDecrement.begin(), it);
}

VkDeviceSize ResourceMemoryAllocator::GetHeapBlockSize(const DawnDeviceAllocatorControl* control) {
    static constexpr VkDeviceSize kDefaultHeapBlockSize = 8ull * 1024ull * 1024ull;  // 8MiB
    VkDeviceSize heapBlockSize = kDefaultHeapBlockSize;
    if (control && control->allocatorHeapBlockSize > 0) {
        heapBlockSize = control->allocatorHeapBlockSize;
    }
    DAWN_ASSERT(IsPowerOfTwo(heapBlockSize));
    return heapBlockSize;
}

// Implementation of ResourceMemoryAllocator
ResourceMemoryAllocator::ResourceMemoryAllocator(Device* device, VkDeviceSize heapBlockSize)
    : mDevice(device), mMaxSizeForSuballocation(GetMaxSuballocationSize(heapBlockSize)) {
    const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();

    // Check if mappable host coherent and host cached buffers will work.
    mUseHostCachedForMappable = HasMemoryTypeWithFlags(
        info, kMapExtendedUsageMemoryPropertyFlags | VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    mAllocatorsPerType.reserve(info.memoryTypes.size());
    for (size_t i = 0; i < info.memoryTypes.size(); i++) {
        const auto& memoryType = info.memoryTypes[i];
        bool isLazyMemoryType =
            (memoryType.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0u;
        mAllocatorsPerType.emplace_back(std::make_unique<SingleTypeAllocator>(
            mDevice, i, isLazyMemoryType, info.memoryHeaps[memoryType.heapIndex].size,
            heapBlockSize, this));
    }
}

ResourceMemoryAllocator::~ResourceMemoryAllocator() = default;

ResultOrError<ResourceMemoryAllocation> ResourceMemoryAllocator::Allocate(
    const VkMemoryRequirements& requirements,
    MemoryKind kind,
    bool forceDisableSubAllocation) {
    // The Vulkan spec guarantees at least one memory type is valid.
    int memoryType = FindBestTypeIndex(requirements, kind);
    bool isLazyMemoryType = mAllocatorsPerType[memoryType]->IsLazyMemoryType();
    DAWN_ASSERT(memoryType >= 0);

    VkDeviceSize size = requirements.size;

    // Sub-allocate non-mappable resources because at the moment the mapped pointer
    // is part of the resource and not the heap, which doesn't match the Vulkan model.
    // TODO(crbug.com/dawn/849): allow sub-allocating mappable resources, maybe.
    if (!forceDisableSubAllocation && requirements.size < mMaxSizeForSuballocation &&
        !IsMemoryKindMappable(kind) &&
        !mDevice->IsToggleEnabled(Toggle::DisableResourceSuballocation)) {
        // When sub-allocating, Vulkan requires that we respect bufferImageGranularity. Some
        // hardware puts information on the memory's page table entry and allocating a linear
        // resource in the same page as a non-linear (aka opaque) resource can cause issues.
        // Probably because some texture compression flags are stored on the page table entry,
        // and allocating a linear resource removes these flags.
        //
        // Anyway, just to be safe we ask that all sub-allocated resources are allocated with at
        // least this alignment. TODO(crbug.com/dawn/849): this is suboptimal because multiple
        // linear (resp. opaque) resources can coexist in the same page. In particular Nvidia
        // GPUs often use a granularity of 64k which will lead to a lot of wasted spec. Revisit
        // with a more efficient algorithm later.
        const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();
        uint64_t alignment =
            std::max(requirements.alignment, info.properties.limits.bufferImageGranularity);

        if ((info.memoryTypes[memoryType].propertyFlags &
             (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            // Host accesses to non-coherent memory are bounded by nonCoherentAtomSize. We may map
            // host visible "non-mappable" memory when taking the fast path during buffer uploads.
            alignment = std::max(alignment, info.properties.limits.nonCoherentAtomSize);
        }

        ResourceMemoryAllocation subAllocation;
        DAWN_TRY_ASSIGN(subAllocation, mAllocatorsPerType[memoryType]->AllocateMemory(
                                           requirements.size, alignment));
        if (subAllocation.GetInfo().mMethod != AllocationMethod::kInvalid) {
            mUsedMemory.Increment(requirements.size);
            mLazyUsedMemory.Increment(isLazyMemoryType ? requirements.size : 0);
            return subAllocation;
        }
    }

    // If sub-allocation failed, allocate memory just for it.
    std::unique_ptr<ResourceHeapBase> resourceHeap;
    DAWN_TRY_ASSIGN(resourceHeap, mAllocatorsPerType[memoryType]->AllocateResourceHeap(size));

    void* mappedPointer = nullptr;
    if (IsMemoryKindMappable(kind)) {
        DAWN_TRY_WITH_CLEANUP(
            CheckVkSuccess(mDevice->fn.MapMemory(mDevice->GetVkDevice(),
                                                 ToBackend(resourceHeap.get())->GetMemory(), 0,
                                                 size, 0, &mappedPointer),
                           "vkMapMemory"),
            { mAllocatorsPerType[memoryType]->DeallocateResourceHeap(std::move(resourceHeap)); });
    }

    mUsedMemory.Increment(size);
    mLazyUsedMemory.Increment(isLazyMemoryType ? size : 0);

    AllocationInfo info;
    info.mMethod = AllocationMethod::kDirect;
    info.mRequestedSize = size;
    info.mIsLazyAllocated = isLazyMemoryType;
    return ResourceMemoryAllocation(info, /*offset*/ 0, resourceHeap.release(),
                                    static_cast<uint8_t*>(mappedPointer));
}

void ResourceMemoryAllocator::Deallocate(ResourceMemoryAllocation* allocation) {
    AllocationInfo info = allocation->GetInfo();
    switch (info.mMethod) {
        // Some memory allocation can never be initialized, for example when wrapping
        // swapchain VkImages with a Texture.
        case AllocationMethod::kInvalid:
            break;

        // For direct allocation we can put the memory for deletion immediately and the fence
        // deleter will make sure the resources are freed before the memory.
        case AllocationMethod::kDirect: {
            ResourceHeap* heap = ToBackend(allocation->GetResourceHeap());
            auto currentDeletionSerial = mDevice->GetFencedDeleter()->GetCurrentDeletionSerial();
            // Track the direct allocation that will be deallocated used memory sizes.
            mUsedMemory.Decrement(currentDeletionSerial, info.mRequestedSize);
            if (info.mIsLazyAllocated) {
                mLazyUsedMemory.Decrement(currentDeletionSerial, info.mRequestedSize);
            }
            allocation->Invalidate();
            DeallocateResourceHeap(heap, info.mIsLazyAllocated);
            delete heap;
            break;
        }

        // Suballocations aren't freed immediately, otherwise another resource allocation could
        // happen just after that aliases the old one and would require a barrier.
        // TODO(crbug.com/dawn/851): Maybe we can produce the correct barriers to reduce the
        // latency to reclaim memory.
        case AllocationMethod::kSubAllocated: {
            ExecutionSerial deletionSerial =
                mDevice->GetFencedDeleter()->GetCurrentDeletionSerial();
            mSubAllocationsToDelete.Enqueue(*allocation, deletionSerial);
            // Track suballocation that will be deallocated for used memory sizes.
            mUsedMemory.Decrement(deletionSerial, info.mRequestedSize);
            if (info.mIsLazyAllocated) {
                mLazyUsedMemory.Decrement(deletionSerial, info.mRequestedSize);
            }
            break;
        }

        default:
            DAWN_UNREACHABLE();
            break;
    }

    // Invalidate the underlying resource heap in case the client accidentally
    // calls DeallocateMemory again using the same allocation.
    allocation->Invalidate();
}

ExecutionSerial ResourceMemoryAllocator::GetLastPendingDeletionSerial() {
    ExecutionSerial lastSerial = kBeginningOfGPUTime;
    auto GetLastSubmitted = [&lastSerial](auto& queue) {
        if (!queue.Empty()) {
            lastSerial = std::max(lastSerial, queue.LastSerial());
        }
    };
    GetLastSubmitted(mSubAllocationsToDelete);
    return lastSerial;
}

void ResourceMemoryAllocator::RecordHeapAllocation(VkDeviceSize size, bool isLazyMemoryType) {
    mAllocatedMemory.Increment(size);
    mLazyAllocatedMemory.Increment(isLazyMemoryType ? size : 0);
}

void ResourceMemoryAllocator::DeallocateResourceHeap(ResourceHeap* heap, bool isLazyMemoryType) {
    VkDeviceSize heapSize = heap->GetSize();
    MutexProtected<FencedDeleter>& fencedDeleter = mDevice->GetFencedDeleter();
    auto currentDeletionSerial = fencedDeleter->GetCurrentDeletionSerial();

    // Track heap that will be deallocated for allocated memory sizes.
    mAllocatedMemory.Decrement(currentDeletionSerial, heapSize);
    if (isLazyMemoryType) {
        mLazyAllocatedMemory.Decrement(currentDeletionSerial, heapSize);
    }
    fencedDeleter->DeleteWhenUnused(heap->GetMemory());
}

void ResourceMemoryAllocator::Tick(ExecutionSerial completedSerial) {
    for (const ResourceMemoryAllocation& allocation :
         mSubAllocationsToDelete.IterateUpTo(completedSerial)) {
        DAWN_ASSERT(allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated);
        size_t memoryType = ToBackend(allocation.GetResourceHeap())->GetMemoryType();
        mAllocatorsPerType[memoryType]->DeallocateMemory(allocation);
    }
    mSubAllocationsToDelete.ClearUpTo(completedSerial);

    // Update the allocation sizes after completed serials.
    mAllocatedMemory.Tick(completedSerial);
    mUsedMemory.Tick(completedSerial);
    mLazyAllocatedMemory.Tick(completedSerial);
    mLazyUsedMemory.Tick(completedSerial);
}

int ResourceMemoryAllocator::FindBestTypeIndex(VkMemoryRequirements requirements, MemoryKind kind) {
    const VulkanDeviceInfo& info = mDevice->GetDeviceInfo();
    bool mappable = IsMemoryKindMappable(kind);
    VkMemoryPropertyFlags vkRequiredFlags = GetRequiredMemoryPropertyFlags(kind);

    // Find a suitable memory type for this allocation
    int bestType = -1;
    for (size_t i = 0; i < info.memoryTypes.size(); ++i) {
        // Resource must support this memory type
        if ((requirements.memoryTypeBits & (1 << i)) == 0) {
            continue;
        }

        // Memory type must have all the required memory properties.
        if ((info.memoryTypes[i].propertyFlags & vkRequiredFlags) != vkRequiredFlags) {
            continue;
        }

        // Found the first candidate memory type
        if (bestType == -1) {
            bestType = static_cast<int>(i);
            continue;
        }

        // For non-mappable resources that can be lazily allocated, favor lazy
        // allocation (note: this is a more important property than that of
        // device local memory and hence is checked first).
        bool currentLazilyAllocated =
            (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0u;
        bool bestLazilyAllocated = (info.memoryTypes[bestType].propertyFlags &
                                    VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0u;
        if ((kind == MemoryKind::LazilyAllocated) &&
            (currentLazilyAllocated != bestLazilyAllocated)) {
            if (currentLazilyAllocated) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // For non-mappable, non-lazily-allocated resources, favor device local
        // memory.
        bool currentDeviceLocal =
            (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0u;
        bool bestDeviceLocal =
            (info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0u;
        if (!mappable && (currentDeviceLocal != bestDeviceLocal)) {
            if (currentDeviceLocal) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // Cached memory is optimal for read-only access from CPU as host memory accesses to
        // uncached memory are slower than to cached memory.
        bool currentHostCached =
            (info.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0u;
        bool bestHostCached =
            (info.memoryTypes[bestType].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0u;
        if ((kind & MemoryKind::ReadMappable) && currentHostCached != bestHostCached) {
            if (currentHostCached) {
                bestType = static_cast<int>(i);
            }
            continue;
        }

        // All things equal favor the memory in the biggest heap
        VkDeviceSize bestTypeHeapSize = info.memoryHeaps[info.memoryTypes[bestType].heapIndex].size;
        VkDeviceSize candidateHeapSize = info.memoryHeaps[info.memoryTypes[i].heapIndex].size;
        if (candidateHeapSize > bestTypeHeapSize) {
            bestType = static_cast<int>(i);
            continue;
        }
    }

    return bestType;
}

void ResourceMemoryAllocator::FreeRecycledMemory() {
    for (auto& alloc : mAllocatorsPerType) {
        alloc->FreeRecycledMemory();
    }
}

uint64_t ResourceMemoryAllocator::GetTotalUsedMemory() const {
    return mUsedMemory.Size();
}

uint64_t ResourceMemoryAllocator::GetTotalAllocatedMemory() const {
    return mAllocatedMemory.Size();
}

uint64_t ResourceMemoryAllocator::GetTotalLazyAllocatedMemory() const {
    return mLazyAllocatedMemory.Size();
}

uint64_t ResourceMemoryAllocator::GetTotalLazyUsedMemory() const {
    return mLazyUsedMemory.Size();
}

VkMemoryPropertyFlags ResourceMemoryAllocator::GetRequiredMemoryPropertyFlags(
    MemoryKind memoryKind) const {
    VkMemoryPropertyFlags vkFlags = 0;

    // Mappable resource must be host visible and host coherent.
    if (IsMemoryKindMappable(memoryKind)) {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        vkFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        // For device local mappable buffers prefer to use host coherent plus host cached memory
        // when it's available for better host access performance.
        DAWN_ASSERT(!(memoryKind & MemoryKind::HostCached));
        if (memoryKind & MemoryKind::DeviceLocal && mUseHostCachedForMappable) {
            vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
    }

    // DEVICE_LOCAL_BIT must be set when MemoryKind::DeviceLocal is required.
    if (memoryKind & MemoryKind::DeviceLocal) {
        vkFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    }

    // HOST_CACHED_BIT must be set when MemoryKind::HostCached is required.
    if (memoryKind & MemoryKind::HostCached) {
        vkFlags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    }

    return vkFlags;
}

}  // namespace dawn::native::vulkan
