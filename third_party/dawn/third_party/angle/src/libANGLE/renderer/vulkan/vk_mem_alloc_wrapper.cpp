//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vma_allocator_wrapper.cpp:
//    Hides VMA functions so we can use separate warning sets.
//

#include "vk_mem_alloc_wrapper.h"

#include <vk_mem_alloc.h>

namespace vma
{
#define VALIDATE_BLOCK_CREATE_FLAG_BITS(x)                                                 \
    static_assert(static_cast<uint32_t>(x) ==                                              \
                      static_cast<uint32_t>(VMA_VIRTUAL_BLOCK_CREATE_##x##_ALGORITHM_BIT), \
                  "VMA enum mismatch")
VALIDATE_BLOCK_CREATE_FLAG_BITS(LINEAR);

VkResult InitAllocator(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkInstance instance,
                       uint32_t apiVersion,
                       VkDeviceSize preferredLargeHeapBlockSize,
                       VmaAllocator *pAllocator)
{
    VmaVulkanFunctions funcs                  = {};
    funcs.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    funcs.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    funcs.vkAllocateMemory                    = vkAllocateMemory;
    funcs.vkFreeMemory                        = vkFreeMemory;
    funcs.vkMapMemory                         = vkMapMemory;
    funcs.vkUnmapMemory                       = vkUnmapMemory;
    funcs.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    funcs.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    funcs.vkBindBufferMemory                  = vkBindBufferMemory;
    funcs.vkBindImageMemory                   = vkBindImageMemory;
    funcs.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    funcs.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    funcs.vkCreateBuffer                      = vkCreateBuffer;
    funcs.vkDestroyBuffer                     = vkDestroyBuffer;
    funcs.vkCreateImage                       = vkCreateImage;
    funcs.vkDestroyImage                      = vkDestroyImage;
    funcs.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
    {
        funcs.vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2;
        funcs.vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2;
        funcs.vkBindBufferMemory2KHR                  = vkBindBufferMemory2;
        funcs.vkBindImageMemory2KHR                   = vkBindImageMemory2;
        funcs.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    }

    VmaAllocatorCreateInfo allocatorInfo      = {};
    allocatorInfo.physicalDevice              = physicalDevice;
    allocatorInfo.device                      = device;
    allocatorInfo.instance                    = instance;
    allocatorInfo.pVulkanFunctions            = &funcs;
    allocatorInfo.vulkanApiVersion            = apiVersion;
    allocatorInfo.preferredLargeHeapBlockSize = preferredLargeHeapBlockSize;

    return vmaCreateAllocator(&allocatorInfo, pAllocator);
}

void DestroyAllocator(VmaAllocator allocator)
{
    vmaDestroyAllocator(allocator);
}

void FreeMemory(VmaAllocator allocator, VmaAllocation allocation)
{
    vmaFreeMemory(allocator, allocation);
}

VkResult CreateBuffer(VmaAllocator allocator,
                      const VkBufferCreateInfo *pBufferCreateInfo,
                      VkMemoryPropertyFlags requiredFlags,
                      VkMemoryPropertyFlags preferredFlags,
                      bool persistentlyMapped,
                      uint32_t *pMemoryTypeIndexOut,
                      VkBuffer *pBuffer,
                      VmaAllocation *pAllocation)
{
    VkResult result;
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.flags       = (persistentlyMapped) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;
    VmaAllocationInfo allocationInfo = {};

    result = vmaCreateBuffer(allocator, pBufferCreateInfo, &allocationCreateInfo, pBuffer,
                             pAllocation, &allocationInfo);
    *pMemoryTypeIndexOut = allocationInfo.memoryType;

    return result;
}

VkResult AllocateAndBindMemoryForImage(VmaAllocator allocator,
                                       VkImage *pImage,
                                       VkMemoryPropertyFlags requiredFlags,
                                       VkMemoryPropertyFlags preferredFlags,
                                       uint32_t memoryTypeBits,
                                       bool allocateDedicatedMemory,
                                       VmaAllocation *pAllocationOut,
                                       uint32_t *pMemoryTypeIndexOut,
                                       VkDeviceSize *sizeOut)
{
    VkResult result;
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.memoryTypeBits          = memoryTypeBits;
    allocationCreateInfo.flags =
        allocateDedicatedMemory ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;
    VmaAllocationInfo allocationInfo = {};

    result = vmaAllocateMemoryForImage(allocator, *pImage, &allocationCreateInfo, pAllocationOut,
                                       &allocationInfo);
    if (result == VK_SUCCESS)
    {
        // If binding was unsuccessful, we should free the allocation.
        result = vmaBindImageMemory(allocator, *pAllocationOut, *pImage);
        if (result != VK_SUCCESS)
        {
            vmaFreeMemory(allocator, *pAllocationOut);
            *pAllocationOut = VK_NULL_HANDLE;
            return result;
        }

        *pMemoryTypeIndexOut = allocationInfo.memoryType;
        *sizeOut             = allocationInfo.size;
    }

    return result;
}

VkResult FindMemoryTypeIndexForBufferInfo(VmaAllocator allocator,
                                          const VkBufferCreateInfo *pBufferCreateInfo,
                                          VkMemoryPropertyFlags requiredFlags,
                                          VkMemoryPropertyFlags preferredFlags,
                                          bool persistentlyMappedBuffers,
                                          uint32_t *pMemoryTypeIndexOut)
{
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.flags = (persistentlyMappedBuffers) ? VMA_ALLOCATION_CREATE_MAPPED_BIT : 0;

    return vmaFindMemoryTypeIndexForBufferInfo(allocator, pBufferCreateInfo, &allocationCreateInfo,
                                               pMemoryTypeIndexOut);
}

VkResult FindMemoryTypeIndexForImageInfo(VmaAllocator allocator,
                                         const VkImageCreateInfo *pImageCreateInfo,
                                         VkMemoryPropertyFlags requiredFlags,
                                         VkMemoryPropertyFlags preferredFlags,
                                         bool allocateDedicatedMemory,
                                         uint32_t *pMemoryTypeIndexOut)
{
    VmaAllocationCreateInfo allocationCreateInfo = {};
    allocationCreateInfo.requiredFlags           = requiredFlags;
    allocationCreateInfo.preferredFlags          = preferredFlags;
    allocationCreateInfo.flags =
        allocateDedicatedMemory ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;

    return vmaFindMemoryTypeIndexForImageInfo(allocator, pImageCreateInfo, &allocationCreateInfo,
                                              pMemoryTypeIndexOut);
}

void GetMemoryTypeProperties(VmaAllocator allocator,
                             uint32_t memoryTypeIndex,
                             VkMemoryPropertyFlags *pFlags)
{
    vmaGetMemoryTypeProperties(allocator, memoryTypeIndex, pFlags);
}

VkResult MapMemory(VmaAllocator allocator, VmaAllocation allocation, void **ppData)
{
    return vmaMapMemory(allocator, allocation, ppData);
}

void UnmapMemory(VmaAllocator allocator, VmaAllocation allocation)
{
    return vmaUnmapMemory(allocator, allocation);
}

void FlushAllocation(VmaAllocator allocator,
                     VmaAllocation allocation,
                     VkDeviceSize offset,
                     VkDeviceSize size)
{
    vmaFlushAllocation(allocator, allocation, offset, size);
}

void InvalidateAllocation(VmaAllocator allocator,
                          VmaAllocation allocation,
                          VkDeviceSize offset,
                          VkDeviceSize size)
{
    vmaInvalidateAllocation(allocator, allocation, offset, size);
}

void BuildStatsString(VmaAllocator allocator, char **statsString, VkBool32 detailedMap)
{
    vmaBuildStatsString(allocator, statsString, detailedMap);
}

void FreeStatsString(VmaAllocator allocator, char *statsString)
{
    vmaFreeStatsString(allocator, statsString);
}

// VmaVirtualBlock implementation
VkResult CreateVirtualBlock(VkDeviceSize size,
                            VirtualBlockCreateFlags flags,
                            VmaVirtualBlock *pVirtualBlock)
{
    VmaVirtualBlockCreateInfo virtualBlockCreateInfo = {};
    virtualBlockCreateInfo.size                      = size;
    virtualBlockCreateInfo.flags                     = (VmaVirtualBlockCreateFlagBits)flags;
    return vmaCreateVirtualBlock(&virtualBlockCreateInfo, pVirtualBlock);
}

void DestroyVirtualBlock(VmaVirtualBlock virtualBlock)
{
    vmaDestroyVirtualBlock(virtualBlock);
}

VkResult VirtualAllocate(VmaVirtualBlock virtualBlock,
                         VkDeviceSize size,
                         VkDeviceSize alignment,
                         VmaVirtualAllocation *pAllocation,
                         VkDeviceSize *pOffset)
{
    VmaVirtualAllocationCreateInfo createInfo = {};
    createInfo.size                           = size;
    createInfo.alignment                      = alignment;
    createInfo.flags                          = 0;
    return vmaVirtualAllocate(virtualBlock, &createInfo, pAllocation, pOffset);
}

void VirtualFree(VmaVirtualBlock virtualBlock, VmaVirtualAllocation allocation, VkDeviceSize offset)
{
    vmaVirtualFree(virtualBlock, allocation);
}

VkBool32 IsVirtualBlockEmpty(VmaVirtualBlock virtualBlock)
{
    return vmaIsVirtualBlockEmpty(virtualBlock);
}

void GetVirtualAllocationInfo(VmaVirtualBlock virtualBlock,
                              VmaVirtualAllocation allocation,
                              VkDeviceSize offset,
                              VkDeviceSize *sizeOut,
                              void **pUserDataOut)
{
    VmaVirtualAllocationInfo virtualAllocInfo = {};
    vmaGetVirtualAllocationInfo(virtualBlock, allocation, &virtualAllocInfo);
    *sizeOut      = virtualAllocInfo.size;
    *pUserDataOut = virtualAllocInfo.pUserData;
}

void ClearVirtualBlock(VmaVirtualBlock virtualBlock)
{
    vmaClearVirtualBlock(virtualBlock);
}

void SetVirtualAllocationUserData(VmaVirtualBlock virtualBlock,
                                  VmaVirtualAllocation allocation,
                                  VkDeviceSize offset,
                                  void *pUserData)
{
    vmaSetVirtualAllocationUserData(virtualBlock, allocation, pUserData);
}

void CalculateVirtualBlockStats(VmaVirtualBlock virtualBlock, StatInfo *pStatInfo)
{
    vmaCalculateVirtualBlockStatistics(virtualBlock,
                                       reinterpret_cast<VmaDetailedStatistics *>(pStatInfo));
}

void BuildVirtualBlockStatsString(VmaVirtualBlock virtualBlock,
                                  char **ppStatsString,
                                  VkBool32 detailedMap)
{
    vmaBuildVirtualBlockStatsString(virtualBlock, ppStatsString, detailedMap);
}

void FreeVirtualBlockStatsString(VmaVirtualBlock virtualBlock, char *pStatsString)
{
    vmaFreeVirtualBlockStatsString(virtualBlock, pStatsString);
}
}  // namespace vma
