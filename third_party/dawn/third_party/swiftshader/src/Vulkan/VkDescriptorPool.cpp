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

#include "VkDescriptorPool.hpp"

#include "VkDescriptorSet.hpp"
#include "VkDescriptorSetLayout.hpp"

#include <algorithm>
#include <memory>

namespace {

inline uint8_t *asMemory(VkDescriptorSet descriptorSet)
{
	return reinterpret_cast<uint8_t *>(vk::Cast(descriptorSet));
}

}  // anonymous namespace

namespace vk {

DescriptorPool::DescriptorPool(const VkDescriptorPoolCreateInfo *pCreateInfo, void *mem)
    : pool(static_cast<uint8_t *>(mem))
    , poolSize(ComputeRequiredAllocationSize(pCreateInfo))
{
}

void DescriptorPool::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::freeHostMemory(pool, pAllocator);
}

size_t DescriptorPool::ComputeRequiredAllocationSize(const VkDescriptorPoolCreateInfo *pCreateInfo)
{
	size_t size = pCreateInfo->maxSets * sw::align(sizeof(DescriptorSetHeader), 16);

	for(uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++)
	{
		uint32_t descriptorSize = DescriptorSetLayout::GetDescriptorSize(pCreateInfo->pPoolSizes[i].type);
		if(pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
		{
			// If type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK then descriptorCount
			// is the number of bytes to allocate for descriptors of this type.
			size += sw::align(pCreateInfo->pPoolSizes[i].descriptorCount, 16);
		}
		else
		{
			size += pCreateInfo->pPoolSizes[i].descriptorCount * sw::align(descriptorSize, 16);
		}
	}

	return size;
}

VkResult DescriptorPool::allocateSets(uint32_t descriptorSetCount, const VkDescriptorSetLayout *pSetLayouts, VkDescriptorSet *pDescriptorSets, const VkDescriptorSetVariableDescriptorCountAllocateInfo *variableDescriptorCountAllocateInfo)
{
	const uint32_t *variableDescriptorCounts =
	    (variableDescriptorCountAllocateInfo && (variableDescriptorCountAllocateInfo->descriptorSetCount == descriptorSetCount)) ? variableDescriptorCountAllocateInfo->pDescriptorCounts : nullptr;

	// FIXME (b/119409619): use an allocator here so we can control all memory allocations
	std::unique_ptr<size_t[]> layoutSizes(new size_t[descriptorSetCount]);
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		pDescriptorSets[i] = VK_NULL_HANDLE;
		layoutSizes[i] = vk::Cast(pSetLayouts[i])->getDescriptorSetAllocationSize(variableDescriptorCounts ? variableDescriptorCounts[i] : 0);
	}

	VkResult result = allocateSets(&(layoutSizes[0]), descriptorSetCount, pDescriptorSets);
	if(result == VK_SUCCESS)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			vk::Cast(pSetLayouts[i])->initialize(vk::Cast(pDescriptorSets[i]), variableDescriptorCounts ? variableDescriptorCounts[i] : 0);
		}
	}
	return result;
}

uint8_t *DescriptorPool::findAvailableMemory(size_t size)
{
	if(nodes.empty())
	{
		return pool;
	}

	// First, look for space at the end of the pool
	const auto itLast = nodes.rbegin();
	ptrdiff_t itemStart = itLast->set - pool;
	ptrdiff_t nextItemStart = itemStart + itLast->size;
	size_t freeSpace = poolSize - nextItemStart;
	if(freeSpace >= size)
	{
		return pool + nextItemStart;
	}

	// Second, look for space at the beginning of the pool
	const auto itBegin = nodes.begin();
	freeSpace = itBegin->set - pool;
	if(freeSpace >= size)
	{
		return pool;
	}

	// Finally, look between existing pool items
	const auto itEnd = nodes.end();
	auto nextIt = itBegin;
	++nextIt;
	for(auto it = itBegin; nextIt != itEnd; ++it, ++nextIt)
	{
		uint8_t *freeSpaceStart = it->set + it->size;
		freeSpace = nextIt->set - freeSpaceStart;
		if(freeSpace >= size)
		{
			return freeSpaceStart;
		}
	}

	return nullptr;
}

VkResult DescriptorPool::allocateSets(size_t *sizes, uint32_t numAllocs, VkDescriptorSet *pDescriptorSets)
{
	size_t totalSize = 0;
	for(uint32_t i = 0; i < numAllocs; i++)
	{
		totalSize += sizes[i];
	}

	if(totalSize > poolSize)
	{
		return VK_ERROR_OUT_OF_POOL_MEMORY;
	}

	// Attempt to allocate single chunk of memory
	{
		uint8_t *memory = findAvailableMemory(totalSize);
		if(memory)
		{
			for(uint32_t i = 0; i < numAllocs; i++)
			{
				pDescriptorSets[i] = *(new(memory) DescriptorSet());
				nodes.insert(Node(memory, sizes[i]));
				memory += sizes[i];
			}

			return VK_SUCCESS;
		}
	}

	// Attempt to allocate each descriptor set separately
	for(uint32_t i = 0; i < numAllocs; i++)
	{
		uint8_t *memory = findAvailableMemory(sizes[i]);
		if(memory)
		{
			pDescriptorSets[i] = *(new(memory) DescriptorSet());
		}
		else
		{
			// vkAllocateDescriptorSets can be used to create multiple descriptor sets. If the
			// creation of any of those descriptor sets fails, then the implementation must
			// destroy all successfully created descriptor set objects from this command, set
			// all entries of the pDescriptorSets array to VK_NULL_HANDLE and return the error.
			for(uint32_t j = 0; j < i; j++)
			{
				freeSet(pDescriptorSets[j]);
				pDescriptorSets[j] = VK_NULL_HANDLE;
			}
			return (computeTotalFreeSize() > totalSize) ? VK_ERROR_FRAGMENTED_POOL : VK_ERROR_OUT_OF_POOL_MEMORY;
		}
		nodes.insert(Node(memory, sizes[i]));
	}

	return VK_SUCCESS;
}

void DescriptorPool::freeSets(uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets)
{
	for(uint32_t i = 0; i < descriptorSetCount; i++)
	{
		freeSet(pDescriptorSets[i]);
	}
}

void DescriptorPool::freeSet(const VkDescriptorSet descriptorSet)
{
	const auto itEnd = nodes.end();
	auto it = std::find(nodes.begin(), itEnd, asMemory(descriptorSet));
	if(it != itEnd)
	{
		nodes.erase(it);
	}
}

VkResult DescriptorPool::reset()
{
	nodes.clear();

	return VK_SUCCESS;
}

size_t DescriptorPool::computeTotalFreeSize() const
{
	size_t totalFreeSize = 0;

	// Compute space at the end of the pool
	const auto itLast = nodes.rbegin();
	totalFreeSize += poolSize - ((itLast->set - pool) + itLast->size);

	// Compute space at the beginning of the pool
	const auto itBegin = nodes.begin();
	totalFreeSize += itBegin->set - pool;

	// Finally, look between existing pool items
	const auto itEnd = nodes.end();
	auto nextIt = itBegin;
	++nextIt;
	for(auto it = itBegin; nextIt != itEnd; ++it, ++nextIt)
	{
		totalFreeSize += (nextIt->set - it->set) - it->size;
	}

	return totalFreeSize;
}

}  // namespace vk