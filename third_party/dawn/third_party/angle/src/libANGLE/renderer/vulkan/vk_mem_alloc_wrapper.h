//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vma_allocator_wrapper.h:
//    Hides VMA functions so we can use separate warning sets.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_
#define LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_

#include "common/vulkan/vk_headers.h"

VK_DEFINE_HANDLE(VmaAllocator)
VK_DEFINE_HANDLE(VmaAllocation)
VK_DEFINE_HANDLE(VmaPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VmaVirtualAllocation)
VK_DEFINE_HANDLE(VmaVirtualBlock)

namespace vma
{
typedef VkFlags VirtualBlockCreateFlags;
typedef enum VirtualBlockCreateFlagBits
{
    GENERAL = 0x00000000,
    LINEAR  = 0x00000001,
} VirtualBlockCreateFlagBits;

typedef struct StatInfo
{
    struct BasicInfo
    {
        // Number of VkDeviceMemory Vulkan memory blocks allocated.
        uint32_t blockCount;
        // Number of VmaAllocation allocation objects allocated.
        uint32_t allocationCount;
        VkDeviceSize blockBytes;
        VkDeviceSize allocationBytes;
    } basicInfo;
    /// Number of free ranges of memory between allocations.
    uint32_t unusedRangeCount;
    /// Smallest allocation size. `VK_WHOLE_SIZE` if there are 0 allocations.
    VkDeviceSize allocationSizeMin;
    /// Largest allocation size. 0 if there are 0 allocations.
    VkDeviceSize allocationSizeMax;
    /// Smallest empty range size. `VK_WHOLE_SIZE` if there are 0 empty ranges.
    VkDeviceSize unusedRangeSizeMin;
    /// Largest empty range size. 0 if there are 0 empty ranges.
    VkDeviceSize unusedRangeSizeMax;
} StatInfo;

VkResult InitAllocator(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkInstance instance,
                       uint32_t apiVersion,
                       VkDeviceSize preferredLargeHeapBlockSize,
                       VmaAllocator *pAllocator);

void DestroyAllocator(VmaAllocator allocator);

void FreeMemory(VmaAllocator allocator, VmaAllocation allocation);

VkResult CreateBuffer(VmaAllocator allocator,
                      const VkBufferCreateInfo *pBufferCreateInfo,
                      VkMemoryPropertyFlags requiredFlags,
                      VkMemoryPropertyFlags preferredFlags,
                      bool persistentlyMapped,
                      uint32_t *pMemoryTypeIndexOut,
                      VkBuffer *pBuffer,
                      VmaAllocation *pAllocation);

VkResult AllocateAndBindMemoryForImage(VmaAllocator allocator,
                                       VkImage *pImage,
                                       VkMemoryPropertyFlags requiredFlags,
                                       VkMemoryPropertyFlags preferredFlags,
                                       uint32_t memoryTypeBits,
                                       bool allocateDedicatedMemory,
                                       VmaAllocation *pAllocationOut,
                                       uint32_t *pMemoryTypeIndexOut,
                                       VkDeviceSize *sizeOut);

VkResult FindMemoryTypeIndexForBufferInfo(VmaAllocator allocator,
                                          const VkBufferCreateInfo *pBufferCreateInfo,
                                          VkMemoryPropertyFlags requiredFlags,
                                          VkMemoryPropertyFlags preferredFlags,
                                          bool persistentlyMappedBuffers,
                                          uint32_t *pMemoryTypeIndexOut);

VkResult FindMemoryTypeIndexForImageInfo(VmaAllocator allocator,
                                         const VkImageCreateInfo *pImageCreateInfo,
                                         VkMemoryPropertyFlags requiredFlags,
                                         VkMemoryPropertyFlags preferredFlags,
                                         bool allocateDedicatedMemory,
                                         uint32_t *pMemoryTypeIndexOut);

void GetMemoryTypeProperties(VmaAllocator allocator,
                             uint32_t memoryTypeIndex,
                             VkMemoryPropertyFlags *pFlags);

VkResult MapMemory(VmaAllocator allocator, VmaAllocation allocation, void **ppData);

void UnmapMemory(VmaAllocator allocator, VmaAllocation allocation);

void FlushAllocation(VmaAllocator allocator,
                     VmaAllocation allocation,
                     VkDeviceSize offset,
                     VkDeviceSize size);

void InvalidateAllocation(VmaAllocator allocator,
                          VmaAllocation allocation,
                          VkDeviceSize offset,
                          VkDeviceSize size);

void BuildStatsString(VmaAllocator allocator, char **statsString, VkBool32 detailedMap);
void FreeStatsString(VmaAllocator allocator, char *statsString);

// VMA virtual block
VkResult CreateVirtualBlock(VkDeviceSize size,
                            VirtualBlockCreateFlags flags,
                            VmaVirtualBlock *pVirtualBlock);
void DestroyVirtualBlock(VmaVirtualBlock virtualBlock);
VkResult VirtualAllocate(VmaVirtualBlock virtualBlock,
                         VkDeviceSize size,
                         VkDeviceSize alignment,
                         VmaVirtualAllocation *pAllocation,
                         VkDeviceSize *pOffset);
void VirtualFree(VmaVirtualBlock virtualBlock,
                 VmaVirtualAllocation allocation,
                 VkDeviceSize offset);
VkBool32 IsVirtualBlockEmpty(VmaVirtualBlock virtualBlock);
void GetVirtualAllocationInfo(VmaVirtualBlock virtualBlock,
                              VmaVirtualAllocation allocation,
                              VkDeviceSize offset,
                              VkDeviceSize *sizeOut,
                              void **pUserDataOut);
void ClearVirtualBlock(VmaVirtualBlock virtualBlock);
void SetVirtualAllocationUserData(VmaVirtualBlock virtualBlock,
                                  VkDeviceSize offset,
                                  void *pUserData);
void CalculateVirtualBlockStats(VmaVirtualBlock virtualBlock, StatInfo *pStatInfo);
void BuildVirtualBlockStatsString(VmaVirtualBlock virtualBlock,
                                  char **ppStatsString,
                                  VkBool32 detailedMap);
void FreeVirtualBlockStatsString(VmaVirtualBlock virtualBlock, char *pStatsString);
}  // namespace vma

#endif  // LIBANGLE_RENDERER_VULKAN_VK_MEM_ALLOC_WRAPPER_H_
