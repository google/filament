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

#ifndef VK_OBJECT_HPP_
#define VK_OBJECT_HPP_

#include "VkConfig.hpp"
#include "VkMemory.hpp"
#include "System/Debug.hpp"

#include <vulkan/vk_icd.h>
#undef None
#undef Bool

#include <new>

namespace vk {

template<typename T, typename VkT>
static inline T *VkTtoT(VkT vkObject)
{
	return static_cast<T *>(static_cast<void *>(vkObject));
}

template<typename T, typename VkT>
static inline VkT TtoVkT(T *object)
{
	return { static_cast<uint64_t>(reinterpret_cast<uintptr_t>(object)) };
}

template<typename T, typename VkT, typename CreateInfo, typename... ExtendedInfo>
static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
{
	*outObject = VK_NULL_HANDLE;

	size_t size = T::ComputeRequiredAllocationSize(pCreateInfo);
	void *memory = nullptr;
	if(size)
	{
		memory = vk::allocateHostMemory(size, vk::HOST_MEMORY_ALLOCATION_ALIGNMENT, pAllocator, T::GetAllocationScope());
		if(!memory)
		{
			return VK_ERROR_OUT_OF_HOST_MEMORY;
		}
	}

	void *objectMemory = vk::allocateHostMemory(sizeof(T), alignof(T), pAllocator, T::GetAllocationScope());
	if(!objectMemory)
	{
		vk::freeHostMemory(memory, pAllocator);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	auto object = new(objectMemory) T(pCreateInfo, memory, extendedInfo...);

	if(!object)
	{
		vk::freeHostMemory(memory, pAllocator);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	*outObject = *object;

	// Assert that potential v-table offsets from multiple inheritance aren't causing an offset on the handle
	ASSERT(*outObject == objectMemory);

	return VK_SUCCESS;
}

template<typename T, typename VkT>
class ObjectBase
{
public:
	using VkType = VkT;

	void destroy(const VkAllocationCallbacks *pAllocator) {}  // Method defined by objects to delete their content, if necessary

	template<typename CreateInfo, typename... ExtendedInfo>
	static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
	{
		return vk::Create<T, VkT, CreateInfo>(pAllocator, pCreateInfo, outObject, extendedInfo...);
	}

	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }
};

template<typename T, typename VkT>
class Object : public ObjectBase<T, VkT>
{
public:
	operator VkT()
	{
		// The static_cast<T*> is used to make sure the returned pointer points to the
		// beginning of the object, even if the derived class uses multiple inheritance
		return vk::TtoVkT<T, VkT>(static_cast<T *>(this));
	}

	static inline T *Cast(VkT vkObject)
	{
		return vk::VkTtoT<T, VkT>(vkObject);
	}
};

template<typename T, typename VkT>
class DispatchableObject
{
	VK_LOADER_DATA loaderData = { ICD_LOADER_MAGIC };

	T object;

public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return T::GetAllocationScope(); }

	template<typename... Args>
	DispatchableObject(Args... args)
	    : object(args...)
	{
	}

	~DispatchableObject() = delete;

	void destroy(const VkAllocationCallbacks *pAllocator)
	{
		object.destroy(pAllocator);
	}

	void operator delete(void *ptr, const VkAllocationCallbacks *pAllocator)
	{
		// Should never happen
		ASSERT(false);
	}

	template<typename CreateInfo, typename... ExtendedInfo>
	static VkResult Create(const VkAllocationCallbacks *pAllocator, const CreateInfo *pCreateInfo, VkT *outObject, ExtendedInfo... extendedInfo)
	{
		return vk::Create<DispatchableObject<T, VkT>, VkT, CreateInfo>(pAllocator, pCreateInfo, outObject, extendedInfo...);
	}

	template<typename CreateInfo>
	static size_t ComputeRequiredAllocationSize(const CreateInfo *pCreateInfo)
	{
		return T::ComputeRequiredAllocationSize(pCreateInfo);
	}

	static inline T *Cast(VkT vkObject)
	{
		return (vkObject == VK_NULL_HANDLE) ? nullptr : &(reinterpret_cast<DispatchableObject<T, VkT> *>(vkObject)->object);
	}

	operator VkT()
	{
		return reinterpret_cast<VkT>(this);
	}
};

template <typename T>
const T *GetExtendedStruct(const void *pNext, VkStructureType sType)
{
	const VkBaseInStructure *extendedStruct = reinterpret_cast<const VkBaseInStructure *>(pNext);
	while(extendedStruct)
	{
		if(extendedStruct->sType == sType)
		{
			return reinterpret_cast<const T *>(extendedStruct);
		}

		extendedStruct = extendedStruct->pNext;
	}

	return nullptr;
}

}  // namespace vk

#endif  // VK_OBJECT_HPP_
