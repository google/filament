//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryTracking.h:
//    Defines the classes used for memory tracking in ANGLE.
//

#ifndef LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_
#define LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_

#include <array>
#include <atomic>

#include "common/SimpleMutex.h"
#include "common/angleutils.h"
#include "common/backtrace_utils.h"
#include "common/hash_containers.h"
#include "common/vulkan/vk_headers.h"

namespace rx
{
namespace vk
{
class Renderer;

// Used to designate memory allocation type for tracking purposes.
enum class MemoryAllocationType
{
    Unspecified                              = 0,
    ImageExternal                            = 1,
    OffscreenSurfaceAttachmentImage          = 2,
    SwapchainMSAAImage                       = 3,
    SwapchainDepthStencilImage               = 4,
    StagingImage                             = 5,
    ImplicitMultisampledRenderToTextureImage = 6,
    TextureImage                             = 7,
    FontImage                                = 8,
    RenderBufferStorageImage                 = 9,
    Buffer                                   = 10,
    BufferExternal                           = 11,

    InvalidEnum = 12,
    EnumCount   = InvalidEnum,
};

constexpr const char *kMemoryAllocationTypeMessage[] = {
    "Unspecified",
    "ImageExternal",
    "OffscreenSurfaceAttachmentImage",
    "SwapchainMSAAImage",
    "SwapchainDepthStencilImage",
    "StagingImage",
    "ImplicitMultisampledRenderToTextureImage",
    "TextureImage",
    "FontImage",
    "RenderBufferStorageImage",
    "Buffer",
    "BufferExternal",
    "Invalid",
};
constexpr const uint32_t kMemoryAllocationTypeCount =
    static_cast<uint32_t>(MemoryAllocationType::EnumCount);

// Used to select the severity for memory allocation logs.
enum class MemoryLogSeverity
{
    INFO,
    WARN,
};

// Used to store memory allocation information for tracking purposes.
struct MemoryAllocationInfo
{
    MemoryAllocationInfo() = default;
    uint64_t id;
    MemoryAllocationType allocType;
    uint32_t memoryHeapIndex;
    void *handle;
    VkDeviceSize size;
};

class MemoryAllocInfoMapKey
{
  public:
    MemoryAllocInfoMapKey() : handle(nullptr) {}
    MemoryAllocInfoMapKey(void *handle) : handle(handle) {}

    bool operator==(const MemoryAllocInfoMapKey &rhs) const
    {
        return reinterpret_cast<uint64_t>(handle) == reinterpret_cast<uint64_t>(rhs.handle);
    }

    size_t hash() const;

  private:
    void *handle;
};

// Process GPU memory reports
class MemoryReport final : angle::NonCopyable
{
  public:
    MemoryReport();
    void processCallback(const VkDeviceMemoryReportCallbackDataEXT &callbackData, bool logCallback);
    void logMemoryReportStats() const;

  private:
    struct MemorySizes
    {
        VkDeviceSize allocatedMemory;
        VkDeviceSize allocatedMemoryMax;
        VkDeviceSize importedMemory;
        VkDeviceSize importedMemoryMax;
    };
    mutable angle::SimpleMutex mMemoryReportMutex;
    VkDeviceSize mCurrentTotalAllocatedMemory;
    VkDeviceSize mMaxTotalAllocatedMemory;
    angle::HashMap<VkObjectType, MemorySizes> mSizesPerType;
    VkDeviceSize mCurrentTotalImportedMemory;
    VkDeviceSize mMaxTotalImportedMemory;
    angle::HashMap<uint64_t, int> mUniqueIDCounts;
};
}  // namespace vk
}  // namespace rx

// Introduce std::hash for MemoryAllocInfoMapKey.
namespace std
{
template <>
struct hash<rx::vk::MemoryAllocInfoMapKey>
{
    size_t operator()(const rx::vk::MemoryAllocInfoMapKey &key) const { return key.hash(); }
};
}  // namespace std

namespace rx
{

// Memory tracker for allocations and deallocations, which is used in vk::Renderer.
class MemoryAllocationTracker : angle::NonCopyable
{
  public:
    MemoryAllocationTracker(vk::Renderer *renderer);
    void initMemoryTrackers();
    void onDeviceInit();
    void onDestroy();

    // Memory statistics are logged when handling a context error.
    void logMemoryStatsOnError();

    // Collect information regarding memory allocations and deallocations.
    void onMemoryAllocImpl(vk::MemoryAllocationType allocType,
                           VkDeviceSize size,
                           uint32_t memoryTypeIndex,
                           void *handle);
    void onMemoryDeallocImpl(vk::MemoryAllocationType allocType,
                             VkDeviceSize size,
                             uint32_t memoryTypeIndex,
                             void *handle);

    // Memory allocation statistics functions.
    VkDeviceSize getActiveMemoryAllocationsSize(uint32_t allocTypeIndex) const;
    VkDeviceSize getActiveHeapMemoryAllocationsSize(uint32_t allocTypeIndex,
                                                    uint32_t heapIndex) const;

    uint64_t getActiveMemoryAllocationsCount(uint32_t allocTypeIndex) const;
    uint64_t getActiveHeapMemoryAllocationsCount(uint32_t allocTypeIndex, uint32_t heapIndex) const;

    // Compare the expected flags with the flags of the allocated memory.
    void compareExpectedFlagsWithAllocatedFlags(VkMemoryPropertyFlags requiredFlags,
                                                VkMemoryPropertyFlags preferredFlags,
                                                VkMemoryPropertyFlags allocatedFlags,
                                                void *handle);

    // Issue warning in case of exceeding maximum allowed allocation size.
    void onExceedingMaxMemoryAllocationSize(VkDeviceSize size);

    // Pending memory allocation information is used for logging in case of an unsuccessful
    // allocation. It is cleared in onMemoryAlloc().
    VkDeviceSize getPendingMemoryAllocationSize() const;
    vk::MemoryAllocationType getPendingMemoryAllocationType() const;
    uint32_t getPendingMemoryTypeIndex() const;

    void resetPendingMemoryAlloc();
    void setPendingMemoryAlloc(vk::MemoryAllocationType allocType,
                               VkDeviceSize size,
                               uint32_t memoryTypeIndex);

  private:
    // Pointer to parent renderer object.
    vk::Renderer *const mRenderer;

    // For tracking the overall memory allocation sizes and counts per memory allocation type.
    std::array<std::atomic<VkDeviceSize>, vk::kMemoryAllocationTypeCount>
        mActiveMemoryAllocationsSize;
    std::array<std::atomic<uint64_t>, vk::kMemoryAllocationTypeCount> mActiveMemoryAllocationsCount;

    // Memory allocation data per memory heap.
    using PerHeapMemoryAllocationSizeArray =
        std::array<std::atomic<VkDeviceSize>, VK_MAX_MEMORY_HEAPS>;
    using PerHeapMemoryAllocationCountArray =
        std::array<std::atomic<uint64_t>, VK_MAX_MEMORY_HEAPS>;

    std::array<PerHeapMemoryAllocationSizeArray, vk::kMemoryAllocationTypeCount>
        mActivePerHeapMemoryAllocationsSize;
    std::array<PerHeapMemoryAllocationCountArray, vk::kMemoryAllocationTypeCount>
        mActivePerHeapMemoryAllocationsCount;

    // Pending memory allocation information is used for logging in case of an allocation error.
    // It includes the size and type of the last attempted allocation, which are cleared after
    // the allocation is successful.
    std::atomic<VkDeviceSize> mPendingMemoryAllocationSize;
    std::atomic<vk::MemoryAllocationType> mPendingMemoryAllocationType;
    std::atomic<uint32_t> mPendingMemoryTypeIndex;

    // Mutex is used to update the data when debug layers are enabled.
    angle::SimpleMutex mMemoryAllocationMutex;

    // Additional information regarding memory allocation with debug layers enabled, including
    // allocation ID and a record of all active allocations.
    uint64_t mMemoryAllocationID;
    using MemoryAllocInfoMap = angle::HashMap<vk::MemoryAllocInfoMapKey, vk::MemoryAllocationInfo>;
    std::unordered_map<angle::BacktraceInfo, MemoryAllocInfoMap> mMemoryAllocationRecord;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_MEMORYTRACKING_H_
