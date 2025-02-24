// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_DEVICE_MEMORY_EXTERNAL_HOST_HPP_
#define VK_DEVICE_MEMORY_EXTERNAL_HOST_HPP_

#include "VkDeviceMemory.hpp"

// Host-allocated memory and host-mapped foreign memory
class ExternalMemoryHost : public vk::DeviceMemory, public vk::ObjectBase<ExternalMemoryHost, VkDeviceMemory>
{
public:
	struct AllocateInfo
	{
		bool supported = false;
		void *hostPointer = nullptr;

		AllocateInfo() = default;

		AllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo);
	};

	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = (VkExternalMemoryHandleTypeFlagBits)(VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT);

	static bool SupportsAllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo);

	explicit ExternalMemoryHost(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, vk::Device *pDevice);

	VkResult allocateBuffer() override;
	void freeBuffer() override;
	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override;

private:
	AllocateInfo allocateInfo;
};

#endif  // VK_DEVICE_MEMORY_EXTERNAL_HOST_HPP_
