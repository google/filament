// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VULKAN_PLATFORM
#define VULKAN_PLATFORM

#include <cstddef>
#include <cstdint>
#include <type_traits>

template<typename T>
class VkNonDispatchableHandle
{
public:
	operator void *() const
	{
		static_assert(sizeof(VkNonDispatchableHandle) == sizeof(uint64_t), "Size is not 64 bits!");

		// VkNonDispatchableHandle must be POD to ensure it gets passed by value the same way as a uint64_t,
		// which is the upstream header's handle type when compiled for 32-bit architectures. On 64-bit architectures,
		// the upstream header's handle type is a pointer type.
		static_assert(std::is_trivial<VkNonDispatchableHandle<T>>::value, "VkNonDispatchableHandle<T> is not trivial!");
		static_assert(std::is_standard_layout<VkNonDispatchableHandle<T>>::value, "VkNonDispatchableHandle<T> is not standard layout!");

		return reinterpret_cast<void *>(static_cast<uintptr_t>(handle));
	}

	void operator=(uint64_t h)
	{
		handle = h;
	}

	uint64_t handle;
};

#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)        \
	typedef struct object##_T *object##Ptr;              \
	typedef VkNonDispatchableHandle<object##Ptr> object; \
	template class VkNonDispatchableHandle<object##Ptr>;

#include <vulkan/vulkan_core.h>

#endif  // VULKAN_PLATFORM
