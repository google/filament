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

#include "VkDescriptorSetLayout.hpp"

#include "VkBuffer.hpp"
#include "VkBufferView.hpp"
#include "VkDescriptorSet.hpp"
#include "VkImageView.hpp"
#include "VkSampler.hpp"

#include "Reactor/Reactor.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace vk {

static bool UsesImmutableSamplers(const VkDescriptorSetLayoutBinding &binding)
{
	return (((binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
	         (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)) &&
	        (binding.pImmutableSamplers != nullptr));
}

DescriptorSetLayout::DescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo *pCreateInfo, void *mem)
    : flags(pCreateInfo->flags)
    , bindings(reinterpret_cast<Binding *>(mem))
{
	// The highest binding number determines the size of the direct-indexed array.
	bindingsArraySize = 0;
	for(uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
	{
		bindingsArraySize = std::max(bindingsArraySize, pCreateInfo->pBindings[i].binding + 1);
	}

	uint8_t *immutableSamplersStorage = static_cast<uint8_t *>(mem) + bindingsArraySize * sizeof(Binding);

	// pCreateInfo->pBindings[] can have gaps in the binding numbers, so first initialize the entire bindings array.
	// "Bindings that are not specified have a descriptorCount and stageFlags of zero, and the value of descriptorType is undefined."
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		bindings[i].descriptorCount = 0;
		bindings[i].immutableSamplers = nullptr;
	}

	for(uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
	{
		const VkDescriptorSetLayoutBinding &srcBinding = pCreateInfo->pBindings[i];
		vk::DescriptorSetLayout::Binding &dstBinding = bindings[srcBinding.binding];

		dstBinding.descriptorType = srcBinding.descriptorType;
		dstBinding.descriptorCount = srcBinding.descriptorCount;

		if(UsesImmutableSamplers(srcBinding))
		{
			size_t immutableSamplersSize = dstBinding.descriptorCount * sizeof(VkSampler);
			dstBinding.immutableSamplers = reinterpret_cast<const vk::Sampler **>(immutableSamplersStorage);
			immutableSamplersStorage += immutableSamplersSize;

			for(uint32_t i = 0; i < dstBinding.descriptorCount; i++)
			{
				dstBinding.immutableSamplers[i] = vk::Cast(srcBinding.pImmutableSamplers[i]);
			}
		}
	}

	uint32_t offset = 0;
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		bindings[i].offset = offset;
		offset += bindings[i].descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}

	ASSERT_MSG(offset == getDescriptorSetDataSize(0), "offset: %d, size: %d", int(offset), int(getDescriptorSetDataSize(0)));
}

void DescriptorSetLayout::destroy(const VkAllocationCallbacks *pAllocator)
{
	vk::freeHostMemory(bindings, pAllocator);  // This allocation also contains pImmutableSamplers
}

size_t DescriptorSetLayout::ComputeRequiredAllocationSize(const VkDescriptorSetLayoutCreateInfo *pCreateInfo)
{
	uint32_t bindingsArraySize = 0;
	uint32_t immutableSamplerCount = 0;
	for(uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
	{
		bindingsArraySize = std::max(bindingsArraySize, pCreateInfo->pBindings[i].binding + 1);

		if(UsesImmutableSamplers(pCreateInfo->pBindings[i]))
		{
			immutableSamplerCount += pCreateInfo->pBindings[i].descriptorCount;
		}
	}

	return bindingsArraySize * sizeof(Binding) +
	       immutableSamplerCount * sizeof(VkSampler);
}

uint32_t DescriptorSetLayout::GetDescriptorSize(VkDescriptorType type)
{
	switch(type)
	{
	case VK_DESCRIPTOR_TYPE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return static_cast<uint32_t>(sizeof(SampledImageDescriptor));
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return static_cast<uint32_t>(sizeof(StorageImageDescriptor));
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return static_cast<uint32_t>(sizeof(BufferDescriptor));
	case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
		return 1;
	default:
		UNSUPPORTED("Unsupported Descriptor Type: %d", int(type));
		return 0;
	}
}

bool DescriptorSetLayout::IsDescriptorDynamic(VkDescriptorType type)
{
	return type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	       type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}

size_t DescriptorSetLayout::getDescriptorSetAllocationSize(uint32_t variableDescriptorCount) const
{
	// vk::DescriptorSet has a header with a pointer to the layout.
	return sw::align<alignof(DescriptorSet)>(sizeof(DescriptorSetHeader) + getDescriptorSetDataSize(variableDescriptorCount));
}

size_t DescriptorSetLayout::getDescriptorSetDataSize(uint32_t variableDescriptorCount) const
{
	size_t size = 0;
	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		uint32_t descriptorCount = bindings[i].descriptorCount;

		if((i == (bindingsArraySize - 1)) && (variableDescriptorCount > 0))
		{
			descriptorCount = variableDescriptorCount;
		}

		size += descriptorCount * GetDescriptorSize(bindings[i].descriptorType);
	}

	return size;
}

void DescriptorSetLayout::initialize(DescriptorSet *descriptorSet, uint32_t variableDescriptorCount)
{
	ASSERT(descriptorSet->header.layout == nullptr);

	// Set a pointer to this descriptor set layout in the descriptor set's header.
	descriptorSet->header.layout = this;
	uint8_t *data = descriptorSet->getDataAddress();  // Descriptor payload

	for(uint32_t i = 0; i < bindingsArraySize; i++)
	{
		size_t descriptorSize = GetDescriptorSize(bindings[i].descriptorType);

		uint32_t descriptorCount = bindings[i].descriptorCount;

		if((i == (bindingsArraySize - 1)) && (variableDescriptorCount > 0))
		{
			descriptorCount = variableDescriptorCount;
		}

		if(bindings[i].immutableSamplers)
		{
			for(uint32_t j = 0; j < descriptorCount; j++)
			{
				SampledImageDescriptor *imageSamplerDescriptor = reinterpret_cast<SampledImageDescriptor *>(data);
				imageSamplerDescriptor->samplerId = bindings[i].immutableSamplers[j]->id;
				imageSamplerDescriptor->memoryOwner = nullptr;
				data += descriptorSize;
			}
		}
		else
		{
			switch(bindings[i].descriptorType)
			{
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				for(uint32_t j = 0; j < descriptorCount; j++)
				{
					SampledImageDescriptor *imageSamplerDescriptor = reinterpret_cast<SampledImageDescriptor *>(data);
					imageSamplerDescriptor->memoryOwner = nullptr;
					data += descriptorSize;
				}
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				for(uint32_t j = 0; j < descriptorCount; j++)
				{
					StorageImageDescriptor *storageImage = reinterpret_cast<StorageImageDescriptor *>(data);
					storageImage->memoryOwner = nullptr;
					data += descriptorSize;
				}
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
				data += descriptorCount * descriptorSize;
				break;
			case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
				data += descriptorCount;
				break;
			default:
				UNSUPPORTED("Unsupported Descriptor Type: %d", int(bindings[i].descriptorType));
			}
		}
	}
}

uint32_t DescriptorSetLayout::getBindingOffset(uint32_t bindingNumber) const
{
	ASSERT(bindingNumber < bindingsArraySize);
	return bindings[bindingNumber].offset;
}

uint32_t DescriptorSetLayout::getDescriptorCount(uint32_t bindingNumber) const
{
	ASSERT(bindingNumber < bindingsArraySize);
	return bindings[bindingNumber].descriptorCount;
}

uint32_t DescriptorSetLayout::getDynamicDescriptorCount() const
{
	uint32_t count = 0;
	for(size_t i = 0; i < bindingsArraySize; i++)
	{
		if(IsDescriptorDynamic(bindings[i].descriptorType))
		{
			count += bindings[i].descriptorCount;
		}
	}

	return count;
}

uint32_t DescriptorSetLayout::getDynamicOffsetIndex(uint32_t bindingNumber) const
{
	ASSERT(bindingNumber < bindingsArraySize);
	ASSERT(IsDescriptorDynamic(bindings[bindingNumber].descriptorType));

	uint32_t index = 0;
	for(uint32_t i = 0; i < bindingNumber; i++)
	{
		if(IsDescriptorDynamic(bindings[i].descriptorType))
		{
			index += bindings[i].descriptorCount;
		}
	}

	return index;
}

VkDescriptorType DescriptorSetLayout::getDescriptorType(uint32_t bindingNumber) const
{
	ASSERT(bindingNumber < bindingsArraySize);
	return bindings[bindingNumber].descriptorType;
}

uint8_t *DescriptorSetLayout::getDescriptorPointer(DescriptorSet *descriptorSet, uint32_t bindingNumber, uint32_t arrayElement, uint32_t count, size_t *typeSize) const
{
	ASSERT(bindingNumber < bindingsArraySize);
	*typeSize = GetDescriptorSize(bindings[bindingNumber].descriptorType);
	size_t byteOffset = bindings[bindingNumber].offset + (*typeSize * arrayElement);
	ASSERT(((*typeSize * count) + byteOffset) <= getDescriptorSetDataSize(0));  // Make sure the operation will not go out of bounds

	return descriptorSet->getDataAddress() + byteOffset;
}

static void WriteTextureLevelInfo(sw::Texture *texture, uint32_t level, uint32_t width, uint32_t height, uint32_t depth, uint32_t pitchP, uint32_t sliceP, uint32_t samplePitchP, uint32_t sampleMax)
{
	if(level == 0)
	{
		texture->widthWidthHeightHeight[0] = static_cast<float>(width);
		texture->widthWidthHeightHeight[1] = static_cast<float>(width);
		texture->widthWidthHeightHeight[2] = static_cast<float>(height);
		texture->widthWidthHeightHeight[3] = static_cast<float>(height);

		texture->width = sw::float4(static_cast<float>(width));
		texture->height = sw::float4(static_cast<float>(height));
		texture->depth = sw::float4(static_cast<float>(depth));
	}

	sw::Mipmap &mipmap = texture->mipmap[level];

	uint16_t halfTexelU = 0x8000 / width;
	uint16_t halfTexelV = 0x8000 / height;
	uint16_t halfTexelW = 0x8000 / depth;

	mipmap.uHalf = sw::ushort4(halfTexelU);
	mipmap.vHalf = sw::ushort4(halfTexelV);
	mipmap.wHalf = sw::ushort4(halfTexelW);

	mipmap.width = sw::uint4(width);
	mipmap.height = sw::uint4(height);
	mipmap.depth = sw::uint4(depth);

	mipmap.onePitchP[0] = 1;
	mipmap.onePitchP[1] = sw::assert_cast<short>(pitchP);
	mipmap.onePitchP[2] = 1;
	mipmap.onePitchP[3] = sw::assert_cast<short>(pitchP);

	mipmap.pitchP = sw::uint4(pitchP);
	mipmap.sliceP = sw::uint4(sliceP);
	mipmap.samplePitchP = sw::uint4(samplePitchP);
	mipmap.sampleMax = sw::uint4(sampleMax);
}

void DescriptorSetLayout::WriteDescriptorSet(Device *device, DescriptorSet *dstSet, const VkDescriptorUpdateTemplateEntry &entry, const char *src)
{
	DescriptorSetLayout *dstLayout = dstSet->header.layout;
	const DescriptorSetLayout::Binding &binding = dstLayout->bindings[entry.dstBinding];
	ASSERT(dstLayout);
	ASSERT(binding.descriptorType == entry.descriptorType);

	size_t typeSize = 0;
	uint8_t *memToWrite = dstLayout->getDescriptorPointer(dstSet, entry.dstBinding, entry.dstArrayElement, entry.descriptorCount, &typeSize);

	ASSERT(reinterpret_cast<intptr_t>(memToWrite) % 16 == 0);  // Each descriptor must be 16-byte aligned.

	if(entry.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLER)
	{
		SampledImageDescriptor *sampledImage = reinterpret_cast<SampledImageDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkDescriptorImageInfo *update = reinterpret_cast<const VkDescriptorImageInfo *>(src + entry.offset + entry.stride * i);

			// "All consecutive bindings updated via a single VkWriteDescriptorSet structure, except those with a
			//  descriptorCount of zero, must all either use immutable samplers or must all not use immutable samplers."
			if(!binding.immutableSamplers)
			{
				sampledImage[i].samplerId = vk::Cast(update->sampler)->id;
			}
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER)
	{
		SampledImageDescriptor *sampledImage = reinterpret_cast<SampledImageDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkBufferView *update = reinterpret_cast<const VkBufferView *>(src + entry.offset + entry.stride * i);
			const vk::BufferView *bufferView = vk::Cast(*update);

			sampledImage[i].imageViewId = bufferView->id;

			uint32_t numElements = bufferView->getElementCount();
			sampledImage[i].width = numElements;
			sampledImage[i].height = 1;
			sampledImage[i].depth = 1;
			sampledImage[i].mipLevels = 1;
			sampledImage[i].sampleCount = 1;
			sampledImage[i].texture.widthWidthHeightHeight = sw::float4(static_cast<float>(numElements), static_cast<float>(numElements), 1, 1);
			sampledImage[i].texture.width = sw::float4(static_cast<float>(numElements));
			sampledImage[i].texture.height = sw::float4(1);
			sampledImage[i].texture.depth = sw::float4(1);

			sw::Mipmap &mipmap = sampledImage[i].texture.mipmap[0];
			mipmap.buffer = bufferView->getPointer();
			mipmap.width[0] = mipmap.width[1] = mipmap.width[2] = mipmap.width[3] = numElements;
			mipmap.height[0] = mipmap.height[1] = mipmap.height[2] = mipmap.height[3] = 1;
			mipmap.depth[0] = mipmap.depth[1] = mipmap.depth[2] = mipmap.depth[3] = 1;
			mipmap.pitchP.x = mipmap.pitchP.y = mipmap.pitchP.z = mipmap.pitchP.w = numElements;
			mipmap.sliceP.x = mipmap.sliceP.y = mipmap.sliceP.z = mipmap.sliceP.w = 0;
			mipmap.onePitchP[0] = mipmap.onePitchP[2] = 1;
			mipmap.onePitchP[1] = mipmap.onePitchP[3] = 0;
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
	        entry.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
	{
		SampledImageDescriptor *sampledImage = reinterpret_cast<SampledImageDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkDescriptorImageInfo *update = reinterpret_cast<const VkDescriptorImageInfo *>(src + entry.offset + entry.stride * i);

			vk::ImageView *imageView = vk::Cast(update->imageView);
			Format format = imageView->getFormat(ImageView::SAMPLING);

			sw::Texture *texture = &sampledImage[i].texture;

			if(entry.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				// "All consecutive bindings updated via a single VkWriteDescriptorSet structure, except those with a
				//  descriptorCount of zero, must all either use immutable samplers or must all not use immutable samplers."
				if(!binding.immutableSamplers)
				{
					sampledImage[i].samplerId = vk::Cast(update->sampler)->id;
				}
			}

			const auto &extent = imageView->getMipLevelExtent(0);

			sampledImage[i].imageViewId = imageView->id;
			sampledImage[i].width = extent.width;
			sampledImage[i].height = extent.height;
			sampledImage[i].depth = imageView->getDepthOrLayerCount(0);
			sampledImage[i].mipLevels = imageView->getSubresourceRange().levelCount;
			sampledImage[i].sampleCount = imageView->getSampleCount();
			sampledImage[i].memoryOwner = imageView;

			auto &subresourceRange = imageView->getSubresourceRange();

			if(format.isYcbcrFormat())
			{
				ASSERT(subresourceRange.levelCount == 1);

				// YCbCr images can only have one level, so we can store parameters for the
				// different planes in the descriptor's mipmap levels instead.

				const int level = 0;
				VkOffset3D offset = { 0, 0, 0 };
				texture->mipmap[0].buffer = imageView->getOffsetPointer(offset, VK_IMAGE_ASPECT_PLANE_0_BIT, level, 0, ImageView::SAMPLING);
				texture->mipmap[1].buffer = imageView->getOffsetPointer(offset, VK_IMAGE_ASPECT_PLANE_1_BIT, level, 0, ImageView::SAMPLING);
				if(format.getAspects() & VK_IMAGE_ASPECT_PLANE_2_BIT)
				{
					texture->mipmap[2].buffer = imageView->getOffsetPointer(offset, VK_IMAGE_ASPECT_PLANE_2_BIT, level, 0, ImageView::SAMPLING);
				}

				VkExtent2D extent = imageView->getMipLevelExtent(0);

				uint32_t width = extent.width;
				uint32_t height = extent.height;
				uint32_t pitchP0 = imageView->rowPitchBytes(VK_IMAGE_ASPECT_PLANE_0_BIT, level, ImageView::SAMPLING) /
				                   imageView->getFormat(VK_IMAGE_ASPECT_PLANE_0_BIT).bytes();

				// Write plane 0 parameters to mipmap level 0.
				WriteTextureLevelInfo(texture, 0, width, height, 1, pitchP0, 0, 0, 0);

				// Plane 2, if present, has equal parameters to plane 1, so we use mipmap level 1 for both.
				uint32_t pitchP1 = imageView->rowPitchBytes(VK_IMAGE_ASPECT_PLANE_1_BIT, level, ImageView::SAMPLING) /
				                   imageView->getFormat(VK_IMAGE_ASPECT_PLANE_1_BIT).bytes();

				WriteTextureLevelInfo(texture, 1, width / 2, height / 2, 1, pitchP1, 0, 0, 0);
			}
			else
			{
				for(int mipmapLevel = 0; mipmapLevel < sw::MIPMAP_LEVELS; mipmapLevel++)
				{
					int level = sw::clamp(mipmapLevel, 0, (int)subresourceRange.levelCount - 1);  // Level within the image view

					VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(imageView->getSubresourceRange().aspectMask);
					sw::Mipmap &mipmap = texture->mipmap[mipmapLevel];

					if((imageView->getType() == VK_IMAGE_VIEW_TYPE_CUBE) ||
					   (imageView->getType() == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY))
					{
						// Obtain the pointer to the corner of the level including the border, for seamless sampling.
						// This is taken into account in the sampling routine, which can't handle negative texel coordinates.
						VkOffset3D offset = { -1, -1, 0 };
						mipmap.buffer = imageView->getOffsetPointer(offset, aspect, level, 0, ImageView::SAMPLING);
					}
					else
					{
						VkOffset3D offset = { 0, 0, 0 };
						mipmap.buffer = imageView->getOffsetPointer(offset, aspect, level, 0, ImageView::SAMPLING);
					}

					VkExtent2D extent = imageView->getMipLevelExtent(level);

					uint32_t width = extent.width;
					uint32_t height = extent.height;
					uint32_t layerCount = imageView->getSubresourceRange().layerCount;
					uint32_t depth = imageView->getDepthOrLayerCount(level);
					uint32_t bytes = format.bytes();
					uint32_t pitchP = imageView->rowPitchBytes(aspect, level, ImageView::SAMPLING) / bytes;
					uint32_t sliceP = (layerCount > 1 ? imageView->layerPitchBytes(aspect, ImageView::SAMPLING) : imageView->slicePitchBytes(aspect, level, ImageView::SAMPLING)) / bytes;
					uint32_t samplePitchP = imageView->getMipLevelSize(aspect, level, ImageView::SAMPLING) / bytes;
					uint32_t sampleMax = imageView->getSampleCount() - 1;

					WriteTextureLevelInfo(texture, mipmapLevel, width, height, depth, pitchP, sliceP, samplePitchP, sampleMax);
				}
			}
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
	        entry.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
	{
		StorageImageDescriptor *storageImage = reinterpret_cast<StorageImageDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkDescriptorImageInfo *update = reinterpret_cast<const VkDescriptorImageInfo *>(src + entry.offset + entry.stride * i);
			vk::ImageView *imageView = vk::Cast(update->imageView);
			const auto &extent = imageView->getMipLevelExtent(0);
			uint32_t layerCount = imageView->getSubresourceRange().layerCount;

			storageImage[i].imageViewId = imageView->id;
			storageImage[i].ptr = imageView->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_COLOR_BIT, 0, 0);
			storageImage[i].width = extent.width;
			storageImage[i].height = extent.height;
			storageImage[i].depth = imageView->getDepthOrLayerCount(0);
			storageImage[i].rowPitchBytes = imageView->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
			storageImage[i].samplePitchBytes = imageView->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
			storageImage[i].slicePitchBytes = layerCount > 1
			                                      ? imageView->layerPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT)
			                                      : imageView->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
			storageImage[i].sampleCount = imageView->getSampleCount();
			storageImage[i].sizeInBytes = static_cast<int>(imageView->getSizeInBytes());
			storageImage[i].memoryOwner = imageView;

			if(imageView->getFormat().isStencil())
			{
				storageImage[i].stencilPtr = imageView->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0);
				storageImage[i].stencilRowPitchBytes = imageView->rowPitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
				storageImage[i].stencilSamplePitchBytes = imageView->slicePitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
				storageImage[i].stencilSlicePitchBytes = (imageView->getSubresourceRange().layerCount > 1)
				                                             ? imageView->layerPitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT)
				                                             : imageView->slicePitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);
			}
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER)
	{
		StorageImageDescriptor *storageImage = reinterpret_cast<StorageImageDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkBufferView *update = reinterpret_cast<const VkBufferView *>(src + entry.offset + entry.stride * i);
			const vk::BufferView *bufferView = vk::Cast(*update);

			storageImage[i].imageViewId = bufferView->id;
			storageImage[i].ptr = bufferView->getPointer();
			storageImage[i].width = bufferView->getElementCount();
			storageImage[i].height = 1;
			storageImage[i].depth = 1;
			storageImage[i].rowPitchBytes = 0;
			storageImage[i].slicePitchBytes = 0;
			storageImage[i].samplePitchBytes = 0;
			storageImage[i].sampleCount = 1;
			storageImage[i].sizeInBytes = bufferView->getRangeInBytes();
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	        entry.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
	        entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	        entry.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
	{
		BufferDescriptor *bufferDescriptor = reinterpret_cast<BufferDescriptor *>(memToWrite);

		for(uint32_t i = 0; i < entry.descriptorCount; i++)
		{
			const VkDescriptorBufferInfo *update = reinterpret_cast<const VkDescriptorBufferInfo *>(src + entry.offset + entry.stride * i);
			const vk::Buffer *buffer = vk::Cast(update->buffer);
			bufferDescriptor[i].ptr = buffer->getOffsetPointer(update->offset);
			bufferDescriptor[i].sizeInBytes = static_cast<int>((update->range == VK_WHOLE_SIZE) ? buffer->getSize() - update->offset : update->range);

			// TODO(b/195684837): The spec states that "vertexBufferRangeSize is the byte size of the memory
			// range bound to the vertex buffer binding", while the code below uses the full size of the buffer.
			bufferDescriptor[i].robustnessSize = static_cast<int>(buffer->getSize() - update->offset);
		}
	}
	else if(entry.descriptorType == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK)
	{
		memcpy(memToWrite, src + entry.offset, entry.descriptorCount);
	}
}

void DescriptorSetLayout::WriteDescriptorSet(Device *device, const VkWriteDescriptorSet &writeDescriptorSet)
{
	DescriptorSet *dstSet = vk::Cast(writeDescriptorSet.dstSet);
	VkDescriptorUpdateTemplateEntry e;
	e.descriptorType = writeDescriptorSet.descriptorType;
	e.dstBinding = writeDescriptorSet.dstBinding;
	e.dstArrayElement = writeDescriptorSet.dstArrayElement;
	e.descriptorCount = writeDescriptorSet.descriptorCount;
	e.offset = 0;
	const void *ptr = nullptr;

	switch(writeDescriptorSet.descriptorType)
	{
	case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		ptr = writeDescriptorSet.pTexelBufferView;
		e.stride = sizeof(VkBufferView);
		break;

	case VK_DESCRIPTOR_TYPE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
	case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
	case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
	case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		ptr = writeDescriptorSet.pImageInfo;
		e.stride = sizeof(VkDescriptorImageInfo);
		break;

	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
	case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
	case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		ptr = writeDescriptorSet.pBufferInfo;
		e.stride = sizeof(VkDescriptorBufferInfo);
		break;

	case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
		{
			const auto *inlineBlock = GetExtendedStruct<VkWriteDescriptorSetInlineUniformBlock>(writeDescriptorSet.pNext, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK);
			ASSERT(inlineBlock);

			// "The descriptorCount of VkDescriptorSetLayoutBinding thus provides the total
			//  number of bytes a particular binding with an inline uniform block descriptor
			//  type can hold, while the srcArrayElement, dstArrayElement, and descriptorCount
			//  members of VkWriteDescriptorSet, VkCopyDescriptorSet, and
			//  VkDescriptorUpdateTemplateEntry (where applicable) specify the byte offset and
			//  number of bytes to write/copy to the binding's backing store. Additionally,
			//  the stride member of VkDescriptorUpdateTemplateEntry is ignored for inline
			//  uniform blocks and a default value of one is used, meaning that the data to
			//  update inline uniform block bindings with must be contiguous in memory."
			ptr = inlineBlock->pData;
			e.stride = 1;
		}
		break;

	default:
		UNSUPPORTED("descriptor type %u", writeDescriptorSet.descriptorType);
	}

	WriteDescriptorSet(device, dstSet, e, reinterpret_cast<const char *>(ptr));
}

void DescriptorSetLayout::CopyDescriptorSet(const VkCopyDescriptorSet &descriptorCopies)
{
	DescriptorSet *srcSet = vk::Cast(descriptorCopies.srcSet);
	DescriptorSetLayout *srcLayout = srcSet->header.layout;
	ASSERT(srcLayout);

	DescriptorSet *dstSet = vk::Cast(descriptorCopies.dstSet);
	DescriptorSetLayout *dstLayout = dstSet->header.layout;
	ASSERT(dstLayout);

	size_t srcTypeSize = 0;
	uint8_t *memToRead = srcLayout->getDescriptorPointer(srcSet, descriptorCopies.srcBinding, descriptorCopies.srcArrayElement, descriptorCopies.descriptorCount, &srcTypeSize);

	size_t dstTypeSize = 0;
	uint8_t *memToWrite = dstLayout->getDescriptorPointer(dstSet, descriptorCopies.dstBinding, descriptorCopies.dstArrayElement, descriptorCopies.descriptorCount, &dstTypeSize);

	ASSERT(srcTypeSize == dstTypeSize);
	size_t writeSize = dstTypeSize * descriptorCopies.descriptorCount;
	memcpy(memToWrite, memToRead, writeSize);
}

}  // namespace vk
