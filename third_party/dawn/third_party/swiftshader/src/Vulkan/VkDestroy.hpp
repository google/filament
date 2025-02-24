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
#include "VkBufferView.hpp"
#include "VkCommandBuffer.hpp"
#include "VkCommandPool.hpp"
#include "VkDebugUtilsMessenger.hpp"
#include "VkDevice.hpp"
#include "VkDeviceMemory.hpp"
#include "VkEvent.hpp"
#include "VkFence.hpp"
#include "VkFramebuffer.hpp"
#include "VkImage.hpp"
#include "VkImageView.hpp"
#include "VkInstance.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkPipeline.hpp"
#include "VkPipelineCache.hpp"
#include "VkPipelineLayout.hpp"
#include "VkPrivateData.hpp"
#include "VkQueryPool.hpp"
#include "VkQueue.hpp"
#include "VkRenderPass.hpp"
#include "VkSampler.hpp"
#include "VkSemaphore.hpp"
#include "VkShaderModule.hpp"
#include "WSI/VkSurfaceKHR.hpp"
#include "WSI/VkSwapchainKHR.hpp"

#include <type_traits>

namespace vk {

// Because Vulkan uses optional allocation callbacks, we use them in a custom
// placement new operator in the VkObjectBase class for simplicity.
// Unfortunately, since we use a placement new to allocate VkObjectBase derived
// classes objects, the corresponding deletion operator is a placement delete,
// which does nothing. In order to properly dispose of these objects' memory,
// we use this function, which calls the T:destroy() function then the T
// destructor prior to releasing the object (by default,
// VkObjectBase::destroy does nothing).
template<typename VkT>
inline void destroy(VkT vkObject, const VkAllocationCallbacks *pAllocator)
{
	auto object = Cast(vkObject);
	if(object)
	{
		using T = typename std::remove_pointer<decltype(object)>::type;
		object->destroy(pAllocator);
		object->~T();
		// object may not point to the same pointer as vkObject, for dispatchable objects,
		// for example, so make sure to deallocate based on the vkObject pointer, which
		// should always point to the beginning of the allocated memory
		vk::freeHostMemory(vkObject, pAllocator);
	}
}

template<typename VkT>
inline void release(VkT vkObject, const VkAllocationCallbacks *pAllocator)
{
	auto object = Cast(vkObject);
	if(object)
	{
		using T = typename std::remove_pointer<decltype(object)>::type;
		if(object->release(pAllocator))
		{
			object->~T();
			// object may not point to the same pointer as vkObject, for dispatchable objects,
			// for example, so make sure to deallocate based on the vkObject pointer, which
			// should always point to the beginning of the allocated memory
			vk::freeHostMemory(vkObject, pAllocator);
		}
	}
}

}  // namespace vk
