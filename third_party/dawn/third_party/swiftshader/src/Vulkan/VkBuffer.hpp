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

#ifndef VK_BUFFER_HPP_
#define VK_BUFFER_HPP_

#include "VkObject.hpp"

namespace vk {

class DeviceMemory;

class Buffer : public Object<Buffer, VkBuffer>
{
public:
	Buffer(const VkBufferCreateInfo *pCreateInfo, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkBufferCreateInfo *pCreateInfo);
	static const VkMemoryRequirements GetMemoryRequirements(VkDeviceSize size, VkBufferUsageFlags usage);

	const VkMemoryRequirements getMemoryRequirements() const;
	void bind(DeviceMemory *pDeviceMemory, VkDeviceSize pMemoryOffset);
	void copyFrom(const void *srcMemory, VkDeviceSize size, VkDeviceSize offset);
	void copyTo(void *dstMemory, VkDeviceSize size, VkDeviceSize offset) const;
	void copyTo(Buffer *dstBuffer, const VkBufferCopy2KHR &pRegion) const;
	void fill(VkDeviceSize dstOffset, VkDeviceSize fillSize, uint32_t data);
	void update(VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
	void *getOffsetPointer(VkDeviceSize offset) const;
	uint64_t getOpaqueCaptureAddress() const;
	inline VkDeviceSize getSize() const { return size; }
	uint8_t *end() const;
	bool canBindToMemory(DeviceMemory *pDeviceMemory) const;

	VkBufferUsageFlags getUsage() const { return usage; }
	VkBufferCreateFlags getFlags() const { return flags; }

private:
	void *memory = nullptr;
	VkBufferCreateFlags flags = 0;
	VkDeviceSize size = 0;
	VkBufferUsageFlags usage = 0;
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	uint32_t queueFamilyIndexCount = 0;
	uint32_t *queueFamilyIndices = nullptr;
	uint64_t opaqueCaptureAddress = 0;

	VkExternalMemoryHandleTypeFlags supportedExternalMemoryHandleTypes = (VkExternalMemoryHandleTypeFlags)0;
};

static inline Buffer *Cast(VkBuffer object)
{
	return Buffer::Cast(object);
}

}  // namespace vk

#endif  // VK_BUFFER_HPP_
