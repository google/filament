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

#include "VkMemory.hpp"

#include "VkConfig.hpp"
#include "System/Debug.hpp"
#include "System/Memory.hpp"

namespace vk {

void *allocateDeviceMemory(size_t bytes, size_t alignment)
{
	ASSERT(bytes <= vk::MAX_MEMORY_ALLOCATION_SIZE);

#if defined(SWIFTSHADER_ZERO_INITIALIZE_DEVICE_MEMORY)
	return sw::allocateZeroOrPoison(bytes, alignment);
#else
	return sw::allocate(bytes, alignment);
#endif
}

void freeDeviceMemory(void *ptr)
{
	sw::freeMemory(ptr);
}

void *allocateHostMemory(size_t bytes, size_t alignment, const VkAllocationCallbacks *pAllocator, VkSystemAllocationScope allocationScope)
{
	ASSERT(bytes <= vk::MAX_MEMORY_ALLOCATION_SIZE);

	if(pAllocator)
	{
		return pAllocator->pfnAllocation(pAllocator->pUserData, bytes, alignment, allocationScope);
	}
	else
	{
		// TODO(b/140991626): Use allocate() instead of allocateZeroOrPoison() to improve startup performance.
		return sw::allocateZeroOrPoison(bytes, alignment);
	}
}

void freeHostMemory(void *ptr, const VkAllocationCallbacks *pAllocator)
{
	if(pAllocator)
	{
		pAllocator->pfnFree(pAllocator->pUserData, ptr);
	}
	else
	{
		sw::freeMemory(ptr);
	}
}

}  // namespace vk
