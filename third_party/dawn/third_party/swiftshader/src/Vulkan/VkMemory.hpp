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

#ifndef VK_MEMORY_HPP_
#define VK_MEMORY_HPP_

#include "Vulkan/VulkanPlatform.hpp"

namespace vk {

// TODO(b/192449828): Pass VkDeviceDeviceMemoryReportCreateInfoEXT into these functions to
// centralize device memory report callback usage.
void *allocateDeviceMemory(size_t bytes, size_t alignment);
void freeDeviceMemory(void *ptr);

// TODO(b/201798871): Fix host allocation callback usage. Uses of this symbolic constant indicate
// places where we should use an allocator instead of unaccounted memory allocations.
constexpr VkAllocationCallbacks *NULL_ALLOCATION_CALLBACKS = nullptr;

void *allocateHostMemory(size_t bytes, size_t alignment, const VkAllocationCallbacks *pAllocator,
                         VkSystemAllocationScope allocationScope);
void freeHostMemory(void *ptr, const VkAllocationCallbacks *pAllocator);

template<typename T>
T *allocateHostmemory(size_t bytes, const VkAllocationCallbacks *pAllocator)
{
	return static_cast<T *>(allocateHostMemory(bytes, alignof(T), pAllocator, T::GetAllocationScope()));
}

}  // namespace vk

#endif  // VK_MEMORY_HPP_
