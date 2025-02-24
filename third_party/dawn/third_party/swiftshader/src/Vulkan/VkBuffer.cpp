// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "VkBuffer.hpp"

#include "VkConfig.hpp"
#include "VkDeviceMemory.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace vk {

Buffer::Buffer(const VkBufferCreateInfo *pCreateInfo, void *mem)
    : flags(pCreateInfo->flags)
    , size(pCreateInfo->size)
    , usage(pCreateInfo->usage)
    , sharingMode(pCreateInfo->sharingMode)
{
	if(pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT)
	{
		queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
		queueFamilyIndices = reinterpret_cast<uint32_t *>(mem);
		memcpy(queueFamilyIndices, pCreateInfo->pQueueFamilyIndices, sizeof(uint32_t) * queueFamilyIndexCount);
	}

	const auto *nextInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
	for(; nextInfo != nullptr; nextInfo = nextInfo->pNext)
	{
		if(nextInfo->sType == VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO)
		{
			const auto *externalInfo = reinterpret_cast<const VkExternalMemoryBufferCreateInfo *>(nextInfo);
			supportedExternalMemoryHandleTypes = externalInfo->handleTypes;
		}
		else if(nextInfo->sType == VK_STRUCTURE_TYPE_BUFFER_OPAQUE_CAPTURE_ADDRESS_CREATE_INFO)
		{
			const auto *opaqueCaptureAddressInfo = reinterpret_cast<const VkBufferOpaqueCaptureAddressCreateInfo *>(nextInfo);
			opaqueCaptureAddress = opaqueCaptureAddressInfo->opaqueCaptureAddress;
		}
	}
}

void Buffer::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::freeHostMemory(queueFamilyIndices, pAllocator);
}

size_t Buffer::ComputeRequiredAllocationSize(const VkBufferCreateInfo *pCreateInfo)
{
	return (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) ? sizeof(uint32_t) * pCreateInfo->queueFamilyIndexCount : 0;
}

const VkMemoryRequirements Buffer::GetMemoryRequirements(VkDeviceSize size, VkBufferUsageFlags usage)
{
	VkMemoryRequirements memoryRequirements = {};

	memoryRequirements.size = size;
	memoryRequirements.alignment = vk::MEMORY_REQUIREMENTS_OFFSET_ALIGNMENT;

	if(usage & (VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT))
	{
		memoryRequirements.alignment = std::max(memoryRequirements.alignment, vk::MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT);
	}

	if(usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
	{
		memoryRequirements.alignment = std::max(memoryRequirements.alignment, vk::MIN_STORAGE_BUFFER_OFFSET_ALIGNMENT);
	}

	if(usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
	{
		memoryRequirements.alignment = std::max(memoryRequirements.alignment, vk::MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
	}

	memoryRequirements.memoryTypeBits = vk::MEMORY_TYPE_GENERIC_BIT;

	return memoryRequirements;
}

const VkMemoryRequirements Buffer::getMemoryRequirements() const
{
	return GetMemoryRequirements(size, usage);
}

bool Buffer::canBindToMemory(DeviceMemory *pDeviceMemory) const
{
	return pDeviceMemory->checkExternalMemoryHandleType(supportedExternalMemoryHandleTypes);
}

void Buffer::bind(DeviceMemory *pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	memory = pDeviceMemory->getOffsetPointer(pMemoryOffset);
}

void Buffer::copyFrom(const void *srcMemory, VkDeviceSize pSize, VkDeviceSize pOffset)
{
	ASSERT((pSize + pOffset) <= size);

	memcpy(getOffsetPointer(pOffset), srcMemory, pSize);
}

void Buffer::copyTo(void *dstMemory, VkDeviceSize pSize, VkDeviceSize pOffset) const
{
	ASSERT((pSize + pOffset) <= size);

	memcpy(dstMemory, getOffsetPointer(pOffset), pSize);
}

void Buffer::copyTo(Buffer *dstBuffer, const VkBufferCopy2KHR &pRegion) const
{
	copyTo(dstBuffer->getOffsetPointer(pRegion.dstOffset), pRegion.size, pRegion.srcOffset);
}

void Buffer::fill(VkDeviceSize dstOffset, VkDeviceSize fillSize, uint32_t data)
{
	size_t bytes = (fillSize == VK_WHOLE_SIZE) ? (size - dstOffset) : fillSize;

	ASSERT((bytes + dstOffset) <= size);

	uint32_t *memToWrite = static_cast<uint32_t *>(getOffsetPointer(dstOffset));

	// Vulkan 1.1 spec: "If VK_WHOLE_SIZE is used and the remaining size of the buffer is
	//                   not a multiple of 4, then the nearest smaller multiple is used."
	for(; bytes >= 4; bytes -= 4, memToWrite++)
	{
		*memToWrite = data;
	}
}

void Buffer::update(VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
	ASSERT((dataSize + dstOffset) <= size);

	memcpy(getOffsetPointer(dstOffset), pData, dataSize);
}

void *Buffer::getOffsetPointer(VkDeviceSize offset) const
{
	return reinterpret_cast<uint8_t *>(memory) + offset;
}

uint64_t Buffer::getOpaqueCaptureAddress() const
{
	return (opaqueCaptureAddress != 0) ? opaqueCaptureAddress : static_cast<uint64_t>(reinterpret_cast<uintptr_t>(memory));
}

uint8_t *Buffer::end() const
{
	return reinterpret_cast<uint8_t *>(getOffsetPointer(size + 1));
}

}  // namespace vk
