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

#ifndef VK_DESCRIPTOR_SET_LAYOUT_HPP_
#define VK_DESCRIPTOR_SET_LAYOUT_HPP_

#include "VkObject.hpp"

#include "Device/Sampler.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkSampler.hpp"

#include <cstdint>

namespace vk {

class DescriptorSet;
class Device;

// TODO(b/129523279): Move to the Device or Pipeline layer.
struct alignas(16) ImageDescriptor
{
	uint32_t imageViewId;
};

struct alignas(16) SampledImageDescriptor : ImageDescriptor
{
	~SampledImageDescriptor() = delete;

	uint32_t samplerId;

	alignas(16) sw::Texture texture;
	int width;  // Of base mip-level.
	int height;
	int depth;  // Layer/cube count for arrayed images
	int mipLevels;
	int sampleCount;

	ImageView *memoryOwner;  // Pointer to the view which owns the memory used by the descriptor set
};

struct alignas(16) StorageImageDescriptor : ImageDescriptor
{
	~StorageImageDescriptor() = delete;

	void *ptr;
	int width;
	int height;
	int depth;  // Layer/cube count for arrayed images
	int rowPitchBytes;
	int slicePitchBytes;  // Layer pitch in case of array image
	int samplePitchBytes;
	int sampleCount;
	int sizeInBytes;

	void *stencilPtr;
	int stencilRowPitchBytes;
	int stencilSlicePitchBytes;  // Layer pitch in case of array image
	int stencilSamplePitchBytes;

	ImageView *memoryOwner;  // Pointer to the view which owns the memory used by the descriptor set
};

struct alignas(16) BufferDescriptor
{
	~BufferDescriptor() = delete;

	void *ptr;
	int sizeInBytes;     // intended size of the bound region -- slides along with dynamic offsets
	int robustnessSize;  // total accessible size from static offset -- does not move with dynamic offset
};

class DescriptorSetLayout : public Object<DescriptorSetLayout, VkDescriptorSetLayout>
{
	struct Binding
	{
		VkDescriptorType descriptorType;
		uint32_t descriptorCount;
		const vk::Sampler **immutableSamplers;

		uint32_t offset;  // Offset in bytes in the descriptor set data.
	};

public:
	DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, void *mem);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkDescriptorSetLayoutCreateInfo *pCreateInfo);

	static uint32_t GetDescriptorSize(VkDescriptorType type);
	static bool IsDescriptorDynamic(VkDescriptorType type);

	static void WriteDescriptorSet(Device *device, const VkWriteDescriptorSet &descriptorWrites);
	static void CopyDescriptorSet(const VkCopyDescriptorSet &descriptorCopies);

	static void WriteDescriptorSet(Device *device, DescriptorSet *dstSet, const VkDescriptorUpdateTemplateEntry &entry, const char *src);

	void initialize(DescriptorSet *descriptorSet, uint32_t variableDescriptorCount);

	// Returns the total size of the descriptor set in bytes.
	size_t getDescriptorSetAllocationSize(uint32_t variableDescriptorCount) const;

	// Returns the byte offset from the base address of the descriptor set for
	// the given binding number.
	uint32_t getBindingOffset(uint32_t bindingNumber) const;

	// Returns the number of descriptors for the given binding number.
	uint32_t getDescriptorCount(uint32_t bindingNumber) const;

	// Returns the number of descriptors across all bindings that are dynamic.
	uint32_t getDynamicDescriptorCount() const;

	// Returns the relative index into the pipeline's dynamic offsets array for
	// the given binding number. This index should be added to the base index
	// returned by PipelineLayout::getDynamicOffsetBase() to produce the
	// starting index for dynamic descriptors.
	uint32_t getDynamicOffsetIndex(uint32_t bindingNumber) const;

	// Returns the descriptor type for the given binding number.
	VkDescriptorType getDescriptorType(uint32_t bindingNumber) const;

	// Returns the number of entries in the direct-indexed array of bindings.
	// It equals the highest binding number + 1.
	uint32_t getBindingsArraySize() const { return bindingsArraySize; }

private:
	uint8_t *getDescriptorPointer(DescriptorSet *descriptorSet, uint32_t bindingNumber, uint32_t arrayElement, uint32_t count, size_t *typeSize) const;
	size_t getDescriptorSetDataSize(uint32_t variableDescriptorCount) const;
	static bool isDynamic(VkDescriptorType type);

	const VkDescriptorSetLayoutCreateFlags flags;
	uint32_t bindingsArraySize = 0;
	Binding *const bindings;  // Direct-indexed array of bindings.
};

static inline DescriptorSetLayout *Cast(VkDescriptorSetLayout object)
{
	return DescriptorSetLayout::Cast(object);
}

}  // namespace vk

#endif  // VK_DESCRIPTOR_SET_LAYOUT_HPP_
