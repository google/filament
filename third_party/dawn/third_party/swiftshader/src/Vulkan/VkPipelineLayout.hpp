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

#ifndef VK_PIPELINE_LAYOUT_HPP_
#define VK_PIPELINE_LAYOUT_HPP_

#include "VkConfig.hpp"
#include "VkDescriptorSetLayout.hpp"

namespace vk {

class PipelineLayout : public Object<PipelineLayout, VkPipelineLayout>
{
public:
	PipelineLayout(const VkPipelineLayoutCreateInfo *pCreateInfo, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator);
	bool release(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkPipelineLayoutCreateInfo *pCreateInfo);

	size_t getDescriptorSetCount() const;
	uint32_t getBindingCount(uint32_t setNumber) const;

	// Returns the index into the pipeline's dynamic offsets array for
	// the given descriptor set and binding number.
	uint32_t getDynamicOffsetIndex(uint32_t setNumber, uint32_t bindingNumber) const;
	uint32_t getDescriptorCount(uint32_t setNumber, uint32_t bindingNumber) const;
	uint32_t getBindingOffset(uint32_t setNumber, uint32_t bindingNumber) const;
	VkDescriptorType getDescriptorType(uint32_t setNumber, uint32_t bindingNumber) const;
	uint32_t getDescriptorSize(uint32_t setNumber, uint32_t bindingNumber) const;
	bool isDescriptorDynamic(uint32_t setNumber, uint32_t bindingNumber) const;

	const uint32_t identifier;

	uint32_t incRefCount();
	uint32_t decRefCount();

private:
	struct Binding
	{
		VkDescriptorType descriptorType;
		uint32_t offset;  // Offset in bytes in the descriptor set data.
		uint32_t dynamicOffsetIndex;
		uint32_t descriptorCount;
	};

	struct DescriptorSet
	{
		Binding *bindings;
		uint32_t bindingCount;
	};

	DescriptorSet descriptorSets[MAX_BOUND_DESCRIPTOR_SETS];

	const uint32_t descriptorSetCount = 0;
	const uint32_t pushConstantRangeCount = 0;
	VkPushConstantRange *pushConstantRanges = nullptr;

	std::atomic<uint32_t> refCount{ 0 };
};

static inline PipelineLayout *Cast(VkPipelineLayout object)
{
	return PipelineLayout::Cast(object);
}

}  // namespace vk

#endif  // VK_PIPELINE_LAYOUT_HPP_
