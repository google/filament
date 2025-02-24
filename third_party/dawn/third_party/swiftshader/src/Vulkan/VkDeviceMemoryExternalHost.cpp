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

#include "VkDeviceMemoryExternalHost.hpp"

ExternalMemoryHost::AllocateInfo::AllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
{
	if(extendedAllocationInfo.importMemoryHostPointerInfo)
	{
		if((extendedAllocationInfo.importMemoryHostPointerInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT) &&
		   (extendedAllocationInfo.importMemoryHostPointerInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT))
		{
			UNSUPPORTED("extendedAllocationInfo.importMemoryHostPointerInfo->handleType, %d",
			            int(extendedAllocationInfo.importMemoryHostPointerInfo->handleType));
		}
		hostPointer = extendedAllocationInfo.importMemoryHostPointerInfo->pHostPointer;
		supported = true;
	}
}

bool ExternalMemoryHost::SupportsAllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
{
	AllocateInfo info(extendedAllocationInfo);
	return info.supported;
}

ExternalMemoryHost::ExternalMemoryHost(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, vk::Device *pDevice)
    : vk::DeviceMemory(pCreateInfo, extendedAllocationInfo, pDevice)
    , allocateInfo(extendedAllocationInfo)
{}

VkResult ExternalMemoryHost::allocateBuffer()
{
	if(allocateInfo.supported)
	{
		buffer = allocateInfo.hostPointer;
		return VK_SUCCESS;
	}
	return VK_ERROR_INVALID_EXTERNAL_HANDLE;
}

void ExternalMemoryHost::freeBuffer()
{}

VkExternalMemoryHandleTypeFlagBits ExternalMemoryHost::getFlagBit() const
{
	return typeFlagBit;
}