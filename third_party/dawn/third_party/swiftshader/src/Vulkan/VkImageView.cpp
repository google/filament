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

#include "VkImageView.hpp"

#include "VkImage.hpp"
#include "VkStructConversion.hpp"
#include "System/Math.hpp"
#include "System/Types.hpp"

#include <climits>

namespace vk {
namespace {

Format GetImageViewFormat(const VkImageViewCreateInfo *pCreateInfo)
{
	// VkImageViewCreateInfo: "If image has an external format, format must be VK_FORMAT_UNDEFINED"
	// In that case, obtain the format from the underlying image.
	if(pCreateInfo->format != VK_FORMAT_UNDEFINED)
	{
		return Format(pCreateInfo->format);
	}

	return vk::Cast(pCreateInfo->image)->getFormat();
}

}  // anonymous namespace

VkComponentMapping ResolveIdentityMapping(VkComponentMapping mapping)
{
	return {
		(mapping.r == VK_COMPONENT_SWIZZLE_IDENTITY) ? VK_COMPONENT_SWIZZLE_R : mapping.r,
		(mapping.g == VK_COMPONENT_SWIZZLE_IDENTITY) ? VK_COMPONENT_SWIZZLE_G : mapping.g,
		(mapping.b == VK_COMPONENT_SWIZZLE_IDENTITY) ? VK_COMPONENT_SWIZZLE_B : mapping.b,
		(mapping.a == VK_COMPONENT_SWIZZLE_IDENTITY) ? VK_COMPONENT_SWIZZLE_A : mapping.a,
	};
}

VkComponentMapping ResolveComponentMapping(VkComponentMapping mapping, vk::Format format)
{
	mapping = vk::ResolveIdentityMapping(mapping);

	// Replace non-present components with zero/one swizzles so that the sampler
	// will give us correct interactions between channel replacement and texel replacement,
	// where we've had to invent new channels behind the app's back (eg transparent decompression
	// of ETC2 RGB -> BGRA8)
	VkComponentSwizzle table[] = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_ZERO,
		VK_COMPONENT_SWIZZLE_ONE,
		VK_COMPONENT_SWIZZLE_R,
		format.componentCount() < 2 ? VK_COMPONENT_SWIZZLE_ZERO : VK_COMPONENT_SWIZZLE_G,
		format.componentCount() < 3 ? VK_COMPONENT_SWIZZLE_ZERO : VK_COMPONENT_SWIZZLE_B,
		format.componentCount() < 4 ? VK_COMPONENT_SWIZZLE_ONE : VK_COMPONENT_SWIZZLE_A,
	};

	return { table[mapping.r], table[mapping.g], table[mapping.b], table[mapping.a] };
}

VkImageSubresourceRange ResolveRemainingLevelsLayers(VkImageSubresourceRange range, const vk::Image *image)
{
	return {
		range.aspectMask,
		range.baseMipLevel,
		(range.levelCount == VK_REMAINING_MIP_LEVELS) ? (image->getMipLevels() - range.baseMipLevel) : range.levelCount,
		range.baseArrayLayer,
		(range.layerCount == VK_REMAINING_ARRAY_LAYERS) ? (image->getArrayLayers() - range.baseArrayLayer) : range.layerCount,
	};
}

Identifier::Identifier(const VkImageViewCreateInfo *pCreateInfo)
{
	const Image *image = vk::Cast(pCreateInfo->image);

	VkImageSubresourceRange subresource = ResolveRemainingLevelsLayers(pCreateInfo->subresourceRange, image);
	vk::Format viewFormat = GetImageViewFormat(pCreateInfo).getAspectFormat(subresource.aspectMask);
	const Image *sampledImage = image->getSampledImage(viewFormat);

	vk::Format samplingFormat = (image == sampledImage) ? viewFormat : sampledImage->getFormat().getAspectFormat(subresource.aspectMask);
	pack({ pCreateInfo->viewType, samplingFormat, ResolveComponentMapping(pCreateInfo->components, viewFormat),
	       static_cast<uint8_t>(subresource.baseMipLevel),
	       static_cast<uint8_t>(subresource.baseMipLevel + subresource.levelCount), subresource.levelCount <= 1u });
}

Identifier::Identifier(VkFormat bufferFormat)
{
	constexpr VkComponentMapping identityMapping = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	pack({ VK_IMAGE_VIEW_TYPE_1D, bufferFormat, ResolveComponentMapping(identityMapping, bufferFormat), 0, 1, true });
}

void Identifier::pack(const State &state)
{
	imageViewType = static_cast<uint32_t>(state.imageViewType);
	format = Format::mapTo8bit(state.format);
	r = static_cast<uint32_t>(state.mapping.r);
	g = static_cast<uint32_t>(state.mapping.g);
	b = static_cast<uint32_t>(state.mapping.b);
	a = static_cast<uint32_t>(state.mapping.a);
	minLod = state.minLod;
	maxLod = state.maxLod;
	singleMipLevel = state.singleMipLevel;
}

Identifier::State Identifier::getState() const
{
	return { static_cast<VkImageViewType>(imageViewType),
		     Format::mapFrom8bit(static_cast<uint8_t>(format)),
		     { static_cast<VkComponentSwizzle>(r),
		       static_cast<VkComponentSwizzle>(g),
		       static_cast<VkComponentSwizzle>(b),
		       static_cast<VkComponentSwizzle>(a) },
		     static_cast<uint8_t>(minLod),
		     static_cast<uint8_t>(maxLod),
		     static_cast<bool>(singleMipLevel) };
}

ImageView::ImageView(const VkImageViewCreateInfo *pCreateInfo, void *mem, const vk::SamplerYcbcrConversion *ycbcrConversion)
    : image(vk::Cast(pCreateInfo->image))
    , viewType(pCreateInfo->viewType)
    , format(GetImageViewFormat(pCreateInfo))
    , components(ResolveComponentMapping(pCreateInfo->components, format))
    , subresourceRange(ResolveRemainingLevelsLayers(pCreateInfo->subresourceRange, image))
    , ycbcrConversion(ycbcrConversion)
    , id(pCreateInfo)
{
}

size_t ImageView::ComputeRequiredAllocationSize(const VkImageViewCreateInfo *pCreateInfo)
{
	return 0;
}

void ImageView::destroy(const VkAllocationCallbacks *pAllocator)
{
}

// Vulkan 1.2 Table 8. Image and image view parameter compatibility requirements
bool ImageView::imageTypesMatch(VkImageType imageType) const
{
	uint32_t imageArrayLayers = image->getArrayLayers();

	switch(viewType)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
		return (imageType == VK_IMAGE_TYPE_1D) &&
		       (subresourceRange.layerCount == 1);
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		return imageType == VK_IMAGE_TYPE_1D;
	case VK_IMAGE_VIEW_TYPE_2D:
		return ((imageType == VK_IMAGE_TYPE_2D) ||
		        ((imageType == VK_IMAGE_TYPE_3D) &&
		         (imageArrayLayers == 1))) &&
		       (subresourceRange.layerCount == 1);
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		return (imageType == VK_IMAGE_TYPE_2D) ||
		       ((imageType == VK_IMAGE_TYPE_3D) &&
		        (imageArrayLayers == 1));
	case VK_IMAGE_VIEW_TYPE_CUBE:
		return image->isCubeCompatible() &&
		       (imageArrayLayers >= subresourceRange.layerCount) &&
		       (subresourceRange.layerCount == 6);
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
		return image->isCubeCompatible() &&
		       (imageArrayLayers >= subresourceRange.layerCount) &&
		       (subresourceRange.layerCount >= 6);
	case VK_IMAGE_VIEW_TYPE_3D:
		return (imageType == VK_IMAGE_TYPE_3D) &&
		       (imageArrayLayers == 1) &&
		       (subresourceRange.layerCount == 1);
	default:
		UNREACHABLE("Unexpected viewType %d", (int)viewType);
	}

	return false;
}

void ImageView::clear(const VkClearValue &clearValue, const VkImageAspectFlags aspectMask, const VkRect2D &renderArea)
{
	// Note: clearing ignores swizzling, so components is ignored.

	ASSERT(imageTypesMatch(image->getImageType()));
	ASSERT(format.isCompatible(image->getFormat()));

	VkImageSubresourceRange sr = subresourceRange;
	sr.aspectMask = aspectMask;
	image->clear(clearValue, format, renderArea, sr);
}

void ImageView::clear(const VkClearValue &clearValue, const VkImageAspectFlags aspectMask, const VkClearRect &renderArea)
{
	// Note: clearing ignores swizzling, so components is ignored.

	ASSERT(imageTypesMatch(image->getImageType()));
	ASSERT(format.isCompatible(image->getFormat()));

	VkImageSubresourceRange sr;
	sr.aspectMask = aspectMask;
	sr.baseMipLevel = subresourceRange.baseMipLevel;
	sr.levelCount = subresourceRange.levelCount;
	sr.baseArrayLayer = renderArea.baseArrayLayer + subresourceRange.baseArrayLayer;
	sr.layerCount = renderArea.layerCount;

	image->clear(clearValue, format, renderArea.rect, sr);
}

void ImageView::clear(const VkClearValue &clearValue, const VkImageAspectFlags aspectMask, const VkRect2D &renderArea, uint32_t layerMask)
{
	if(layerMask == 0)
	{
		clear(clearValue, aspectMask, renderArea);
	}
	else
	{
		clearWithLayerMask(clearValue, aspectMask, renderArea, layerMask);
	}
}

void ImageView::clear(const VkClearValue &clearValue, const VkImageAspectFlags aspectMask, const VkClearRect &renderArea, uint32_t layerMask)
{
	if(layerMask == 0)
	{
		clear(clearValue, aspectMask, renderArea);
	}
	else
	{
		clearWithLayerMask(clearValue, aspectMask, renderArea.rect, layerMask);
	}
}

void ImageView::clearWithLayerMask(const VkClearValue &clearValue, VkImageAspectFlags aspectMask, const VkRect2D &renderArea, uint32_t layerMask)
{
	while(layerMask)
	{
		uint32_t layer = sw::log2i(layerMask);
		layerMask &= ~(1 << layer);
		VkClearRect r = { renderArea, layer, 1 };
		r.baseArrayLayer = layer;
		clear(clearValue, aspectMask, r);
	}
}

void ImageView::resolveSingleLayer(ImageView *resolveAttachment, int layer)
{
	if((subresourceRange.levelCount != 1) || (resolveAttachment->subresourceRange.levelCount != 1))
	{
		UNIMPLEMENTED("b/148242443: levelCount != 1");  // FIXME(b/148242443)
	}

	VkImageResolve2KHR region;
	region.sType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR;
	region.pNext = nullptr;
	region.srcSubresource = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer + layer,
		1
	};
	region.srcOffset = { 0, 0, 0 };
	region.dstSubresource = {
		resolveAttachment->subresourceRange.aspectMask,
		resolveAttachment->subresourceRange.baseMipLevel,
		resolveAttachment->subresourceRange.baseArrayLayer + layer,
		1
	};
	region.dstOffset = { 0, 0, 0 };
	region.extent = image->getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask),
	                                         subresourceRange.baseMipLevel);

	image->resolveTo(resolveAttachment->image, region);
}

void ImageView::resolve(ImageView *resolveAttachment)
{
	if((subresourceRange.levelCount != 1) || (resolveAttachment->subresourceRange.levelCount != 1))
	{
		UNIMPLEMENTED("b/148242443: levelCount != 1");  // FIXME(b/148242443)
	}

	VkImageResolve2KHR region;
	region.sType = VK_STRUCTURE_TYPE_IMAGE_RESOLVE_2_KHR;
	region.pNext = nullptr;
	region.srcSubresource = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer,
		subresourceRange.layerCount
	};
	region.srcOffset = { 0, 0, 0 };
	region.dstSubresource = {
		resolveAttachment->subresourceRange.aspectMask,
		resolveAttachment->subresourceRange.baseMipLevel,
		resolveAttachment->subresourceRange.baseArrayLayer,
		resolveAttachment->subresourceRange.layerCount
	};
	region.dstOffset = { 0, 0, 0 };
	region.extent = image->getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask),
	                                         subresourceRange.baseMipLevel);

	image->resolveTo(resolveAttachment->image, region);
}

void ImageView::resolve(ImageView *resolveAttachment, uint32_t layerMask)
{
	if(layerMask == 0)
	{
		resolve(resolveAttachment);
	}
	else
	{
		resolveWithLayerMask(resolveAttachment, layerMask);
	}
}

void ImageView::resolveWithLayerMask(ImageView *resolveAttachment, uint32_t layerMask)
{
	while(layerMask)
	{
		int layer = sw::log2i(layerMask);
		layerMask &= ~(1 << layer);
		resolveSingleLayer(resolveAttachment, layer);
	}
}

void ImageView::resolveDepthStencil(ImageView *resolveAttachment, VkResolveModeFlagBits depthResolveMode, VkResolveModeFlagBits stencilResolveMode)
{
	ASSERT(subresourceRange.levelCount == 1 && resolveAttachment->subresourceRange.levelCount == 1);
	if((subresourceRange.layerCount != 1) || (resolveAttachment->subresourceRange.layerCount != 1))
	{
		UNIMPLEMENTED("b/148242443: layerCount != 1");  // FIXME(b/148242443)
	}

	image->resolveDepthStencilTo(this, resolveAttachment, depthResolveMode, stencilResolveMode);
}

const Image *ImageView::getImage(Usage usage) const
{
	switch(usage)
	{
	case RAW:
		return image;
	case SAMPLING:
		return image->getSampledImage(format);
	default:
		UNREACHABLE("usage %d", int(usage));
		return nullptr;
	}
}

Format ImageView::getFormat(Usage usage) const
{
	Format imageFormat = ((usage == RAW) || (getImage(usage) == image)) ? format : getImage(usage)->getFormat();
	return imageFormat.getAspectFormat(subresourceRange.aspectMask);
}

uint32_t ImageView::rowPitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage) const
{
	return sw::assert_cast<uint32_t>(getImage(usage)->rowPitchBytes(aspect, subresourceRange.baseMipLevel + mipLevel));
}

uint32_t ImageView::slicePitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage) const
{
	return sw::assert_cast<uint32_t>(getImage(usage)->slicePitchBytes(aspect, subresourceRange.baseMipLevel + mipLevel));
}

uint32_t ImageView::getMipLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel, Usage usage) const
{
	return sw::assert_cast<uint32_t>(getImage(usage)->getMipLevelSize(aspect, subresourceRange.baseMipLevel + mipLevel));
}

uint32_t ImageView::layerPitchBytes(VkImageAspectFlagBits aspect, Usage usage) const
{
	return sw::assert_cast<uint32_t>(getImage(usage)->getLayerSize(aspect));
}

VkExtent2D ImageView::getMipLevelExtent(uint32_t mipLevel) const
{
	return Extent2D(image->getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask),
	                                         subresourceRange.baseMipLevel + mipLevel));
}

VkExtent2D ImageView::getMipLevelExtent(uint32_t mipLevel, VkImageAspectFlagBits aspect) const
{
	return Extent2D(image->getMipLevelExtent(aspect, subresourceRange.baseMipLevel + mipLevel));
}

uint32_t ImageView::getDepthOrLayerCount(uint32_t mipLevel) const
{
	VkExtent3D extent = image->getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask),
	                                             subresourceRange.baseMipLevel + mipLevel);
	uint32_t layers = subresourceRange.layerCount;
	uint32_t depthOrLayers = layers > 1 ? layers : extent.depth;

	// For cube images the number of whole cubes is returned
	if(viewType == VK_IMAGE_VIEW_TYPE_CUBE ||
	   viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
	{
		depthOrLayers /= 6;
	}

	return depthOrLayers;
}

void *ImageView::getOffsetPointer(const VkOffset3D &offset, VkImageAspectFlagBits aspect, uint32_t mipLevel, uint32_t layer, Usage usage) const
{
	ASSERT(mipLevel < subresourceRange.levelCount);

	VkImageSubresource imageSubresource = {
		static_cast<VkImageAspectFlags>(aspect),
		subresourceRange.baseMipLevel + mipLevel,
		subresourceRange.baseArrayLayer + layer,
	};

	return getImage(usage)->getTexelPointer(offset, imageSubresource);
}

}  // namespace vk
