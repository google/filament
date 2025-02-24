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

#ifndef VK_IMAGE_VIEW_HPP_
#define VK_IMAGE_VIEW_HPP_

#include "VkFormat.hpp"
#include "VkImage.hpp"
#include "VkObject.hpp"

#include "System/Debug.hpp"

#include <atomic>

namespace vk {

class SamplerYcbcrConversion;

// Uniquely identifies state used by sampling routine generation.
// Integer ID space shared by image views and buffer views.
union Identifier
{
	// Image view identifier
	Identifier(const VkImageViewCreateInfo *pCreateInfo);

	// Buffer view identifier
	Identifier(VkFormat format);

	// Copy constructor from existing identifier
	Identifier(uint32_t fromId)
	    : id(fromId)
	{}

	operator uint32_t() const
	{
		static_assert(sizeof(Identifier) == sizeof(uint32_t), "Identifier must be 32-bit");
		return id;
	}

	struct State
	{
		VkImageViewType imageViewType;
		VkFormat format;
		VkComponentMapping mapping;
		uint8_t minLod;
		uint8_t maxLod;
		bool singleMipLevel;
	};
	State getState() const;

private:
	void pack(const State &data);

	// Identifier is a union of this struct and the integer below.
	static_assert(sw::MIPMAP_LEVELS <= 15);
	struct
	{
		uint32_t imageViewType : 3;
		uint32_t format : 8;
		uint32_t r : 3;
		uint32_t g : 3;
		uint32_t b : 3;
		uint32_t a : 3;
		uint32_t minLod : 4;
		uint32_t maxLod : 4;
		uint32_t singleMipLevel : 1;
	};

	uint32_t id = 0;
};

class ImageView : public Object<ImageView, VkImageView>
{
public:
	// Image usage:
	// RAW: Use the base image as is
	// SAMPLING: Image used for texture sampling
	enum Usage
	{
		RAW,
		SAMPLING
	};

	ImageView(const VkImageViewCreateInfo *pCreateInfo, void *mem, const vk::SamplerYcbcrConversion *ycbcrConversion);
	void destroy(const VkAllocationCallbacks *pAllocator);

	static size_t ComputeRequiredAllocationSize(const VkImageViewCreateInfo *pCreateInfo);

	void clear(const VkClearValue &clearValues, VkImageAspectFlags aspectMask, const VkRect2D &renderArea, uint32_t layerMask);
	void clear(const VkClearValue &clearValue, VkImageAspectFlags aspectMask, const VkClearRect &renderArea, uint32_t layerMask);
	void resolve(ImageView *resolveAttachment, uint32_t layerMask);
	void resolveDepthStencil(ImageView *resolveAttachment, VkResolveModeFlagBits depthResolveMode, VkResolveModeFlagBits stencilResolveMode);

	VkImageViewType getType() const { return viewType; }
	Format getFormat(Usage usage = RAW) const;
	Format getFormat(VkImageAspectFlagBits aspect) const { return image->getFormat(aspect); }
	uint32_t rowPitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage = RAW) const;
	uint32_t slicePitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage = RAW) const;
	uint32_t getMipLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage = RAW) const;
	uint32_t layerPitchBytes(VkImageAspectFlagBits aspect, Usage usage = RAW) const;
	VkExtent2D getMipLevelExtent(uint32_t mipLevel) const;
	VkExtent2D getMipLevelExtent(uint32_t mipLevel, VkImageAspectFlagBits aspect) const;
	uint32_t getDepthOrLayerCount(uint32_t mipLevel) const;

	int getSampleCount() const
	{
		switch(image->getSampleCount())
		{
		case VK_SAMPLE_COUNT_1_BIT: return 1;
		case VK_SAMPLE_COUNT_4_BIT: return 4;
		default:
			UNSUPPORTED("Sample count %d", image->getSampleCount());
			return 1;
		}
	}

	void *getOffsetPointer(const VkOffset3D &offset, VkImageAspectFlagBits aspect, uint32_t mipLevel, uint32_t layer, Usage usage = RAW) const;
	bool hasDepthAspect() const { return (subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0; }
	bool hasStencilAspect() const { return (subresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0; }

	void contentsChanged(Image::ContentsChangedContext context) { image->contentsChanged(subresourceRange, context); }

	void prepareForSampling() { image->prepareForSampling(subresourceRange); }

	const VkComponentMapping &getComponentMapping() const { return components; }
	const VkImageSubresourceRange &getSubresourceRange() const { return subresourceRange; }
	size_t getSizeInBytes() const { return image->getSizeInBytes(subresourceRange); }

private:
	bool imageTypesMatch(VkImageType imageType) const;
	const Image *getImage(Usage usage) const;
	void clear(const VkClearValue &clearValues, VkImageAspectFlags aspectMask, const VkRect2D &renderArea);
	void clear(const VkClearValue &clearValue, VkImageAspectFlags aspectMask, const VkClearRect &renderArea);
	void clearWithLayerMask(const VkClearValue &clearValue, VkImageAspectFlags aspectMask, const VkRect2D &renderArea, uint32_t layerMask);
	void resolve(ImageView *resolveAttachment);
	void resolveSingleLayer(ImageView *resolveAttachment, int layer);
	void resolveWithLayerMask(ImageView *resolveAttachment, uint32_t layerMask);

	Image *const image = nullptr;
	const VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
	const Format format = VK_FORMAT_UNDEFINED;
	const VkComponentMapping components = {};
	const VkImageSubresourceRange subresourceRange = {};

	const vk::SamplerYcbcrConversion *ycbcrConversion = nullptr;

public:
	const Identifier id;
};

VkComponentMapping ResolveIdentityMapping(VkComponentMapping mapping);
VkComponentMapping ResolveComponentMapping(VkComponentMapping mapping, vk::Format format);
VkImageSubresourceRange ResolveRemainingLevelsLayers(VkImageSubresourceRange range, const vk::Image *image);

static inline ImageView *Cast(VkImageView object)
{
	return ImageView::Cast(object);
}

}  // namespace vk

#endif  // VK_IMAGE_VIEW_HPP_
