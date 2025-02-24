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

#include "VkCommandPool.hpp"

#include "VkCommandBuffer.hpp"
#include "VkDestroy.hpp"

#include <algorithm>
#include <new>

namespace vk {

CommandPool::CommandPool(const VkCommandPoolCreateInfo *pCreateInfo, void *mem)
{
}

void CommandPool::destroy(const VkAllocationCallbacks *pAllocator)
{
	// Free command Buffers allocated in allocateCommandBuffers
	for(auto commandBuffer : commandBuffers)
	{
		vk::destroy(commandBuffer, NULL_ALLOCATION_CALLBACKS);
	}
}

size_t CommandPool::ComputeRequiredAllocationSize(const VkCommandPoolCreateInfo *pCreateInfo)
{
	return 0;
}

VkResult CommandPool::allocateCommandBuffers(Device *device, VkCommandBufferLevel level, uint32_t commandBufferCount, VkCommandBuffer *pCommandBuffers)
{
	for(uint32_t i = 0; i < commandBufferCount; i++)
	{
		// TODO(b/119409619): Allocate command buffers from the pool memory.
		void *memory = vk::allocateHostMemory(sizeof(DispatchableCommandBuffer), vk::HOST_MEMORY_ALLOCATION_ALIGNMENT,
		                                      NULL_ALLOCATION_CALLBACKS, DispatchableCommandBuffer::GetAllocationScope());
		ASSERT(memory);
		DispatchableCommandBuffer *commandBuffer = new(memory) DispatchableCommandBuffer(device, level);
		if(commandBuffer)
		{
			pCommandBuffers[i] = *commandBuffer;
		}
		else
		{
			for(uint32_t j = 0; j < i; j++)
			{
				vk::destroy(pCommandBuffers[j], NULL_ALLOCATION_CALLBACKS);
			}

			for(uint32_t j = 0; j < commandBufferCount; j++)
			{
				pCommandBuffers[j] = VK_NULL_HANDLE;
			}

			return VK_ERROR_OUT_OF_DEVICE_MEMORY;
		}
	}

	commandBuffers.insert(pCommandBuffers, pCommandBuffers + commandBufferCount);

	return VK_SUCCESS;
}

void CommandPool::freeCommandBuffers(uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
	for(uint32_t i = 0; i < commandBufferCount; i++)
	{
		commandBuffers.erase(pCommandBuffers[i]);
		vk::destroy(pCommandBuffers[i], NULL_ALLOCATION_CALLBACKS);
	}
}

VkResult CommandPool::reset(VkCommandPoolResetFlags flags)
{
	// According the Vulkan 1.1 spec:
	// "All command buffers that have been allocated from
	//  the command pool are put in the initial state."
	for(auto commandBuffer : commandBuffers)
	{
		vk::Cast(commandBuffer)->reset(flags);
	}

	return VK_SUCCESS;
}

void CommandPool::trim(VkCommandPoolTrimFlags flags)
{
	// TODO (b/119827933): Optimize memory usage here
}

}  // namespace vk
