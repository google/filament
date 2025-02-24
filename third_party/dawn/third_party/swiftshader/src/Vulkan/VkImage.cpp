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

#include "VkImage.hpp"

#include "VkBuffer.hpp"
#include "VkDevice.hpp"
#include "VkDeviceMemory.hpp"
#include "VkImageView.hpp"
#include "VkStringify.hpp"
#include "VkStructConversion.hpp"
#include "Device/ASTC_Decoder.hpp"
#include "Device/BC_Decoder.hpp"
#include "Device/Blitter.hpp"
#include "Device/ETC_Decoder.hpp"

#ifdef __ANDROID__
#	include <vndk/hardware_buffer.h>

#	include "VkDeviceMemoryExternalAndroid.hpp"
#endif

#include <cstring>

namespace {

ETC_Decoder::InputType GetInputType(const vk::Format &format)
{
	switch(format)
	{
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		return ETC_Decoder::ETC_R_UNSIGNED;
	case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		return ETC_Decoder::ETC_R_SIGNED;
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		return ETC_Decoder::ETC_RG_UNSIGNED;
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
		return ETC_Decoder::ETC_RG_SIGNED;
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		return ETC_Decoder::ETC_RGB;
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		return ETC_Decoder::ETC_RGB_PUNCHTHROUGH_ALPHA;
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		return ETC_Decoder::ETC_RGBA;
	default:
		UNSUPPORTED("format: %d", int(format));
		return ETC_Decoder::ETC_RGBA;
	}
}

int GetBCn(const vk::Format &format)
{
	switch(format)
	{
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		return 1;
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
		return 2;
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
		return 3;
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
		return 4;
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
		return 5;
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		return 6;
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
		return 7;
	default:
		UNSUPPORTED("format: %d", int(format));
		return 0;
	}
}

// Returns true for BC1 if we have an RGB format, false for RGBA
// Returns true for BC4, BC5, BC6H if we have an unsigned format, false for signed
// Ignored by BC2, BC3, and BC7
bool GetNoAlphaOrUnsigned(const vk::Format &format)
{
	switch(format)
	{
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		return true;
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
		return false;
	default:
		UNSUPPORTED("format: %d", int(format));
		return false;
	}
}

VkFormat GetImageFormat(const VkImageCreateInfo *pCreateInfo)
{
	const auto *nextInfo = reinterpret_cast<const VkBaseInStructure *>(pCreateInfo->pNext);
	while(nextInfo)
	{
		// Casting to an int since some structures, such as VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID and
		// VK_STRUCTURE_TYPE_SWAPCHAIN_IMAGE_CREATE_INFO_ANDROID, are not enumerated in the official Vulkan headers.
		switch((int)(nextInfo->sType))
		{
#ifdef __ANDROID__
		case VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID:
			{
				const VkExternalFormatANDROID *externalFormatAndroid = reinterpret_cast<const VkExternalFormatANDROID *>(nextInfo);

				// VkExternalFormatANDROID: "If externalFormat is zero, the effect is as if the VkExternalFormatANDROID structure was not present."
				if(externalFormatAndroid->externalFormat == 0)
				{
					break;
				}

				const VkFormat correspondingVkFormat = AHardwareBufferExternalMemory::GetVkFormatFromAHBFormat(externalFormatAndroid->externalFormat);
				ASSERT(pCreateInfo->format == VK_FORMAT_UNDEFINED || pCreateInfo->format == correspondingVkFormat);
				return correspondingVkFormat;
			}
			break;
		case VK_STRUCTURE_TYPE_NATIVE_BUFFER_ANDROID:
			break;
		case VK_STRUCTURE_TYPE_SWAPCHAIN_IMAGE_CREATE_INFO_ANDROID:
			break;
#endif
		// We support these extensions, but they don't affect the image format.
		case VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO:
		case VK_STRUCTURE_TYPE_IMAGE_SWAPCHAIN_CREATE_INFO_KHR:
		case VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO:
		case VK_STRUCTURE_TYPE_IMAGE_STENCIL_USAGE_CREATE_INFO:
			break;
		case VK_STRUCTURE_TYPE_IMAGE_DRM_FORMAT_MODIFIER_LIST_CREATE_INFO_EXT:
			{
				// Explicitly ignored, since VK_EXT_image_drm_format_modifier is not supported
			}
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:
			// dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("pCreateInfo->pNext->sType = %s", vk::Stringify(nextInfo->sType).c_str());
			break;
		}

		nextInfo = nextInfo->pNext;
	}

	return pCreateInfo->format;
}

}  // anonymous namespace

namespace vk {

Image::Image(const VkImageCreateInfo *pCreateInfo, void *mem, Device *device)
    : device(device)
    , flags(pCreateInfo->flags)
    , imageType(pCreateInfo->imageType)
    , format(GetImageFormat(pCreateInfo))
    , extent(pCreateInfo->extent)
    , mipLevels(pCreateInfo->mipLevels)
    , arrayLayers(pCreateInfo->arrayLayers)
    , samples(pCreateInfo->samples)
    , tiling(pCreateInfo->tiling)
    , usage(pCreateInfo->usage)
{
	if(format.isCompressed())
	{
		VkImageCreateInfo compressedImageCreateInfo = *pCreateInfo;
		compressedImageCreateInfo.format = format.getDecompressedFormat();
		decompressedImage = new(mem) Image(&compressedImageCreateInfo, nullptr, device);
	}

	const auto *externalInfo = GetExtendedStruct<VkExternalMemoryImageCreateInfo>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO);
	if(externalInfo)
	{
		supportedExternalMemoryHandleTypes = externalInfo->handleTypes;
	}
}

void Image::destroy(const VkAllocationCallbacks *pAllocator)
{
	if(decompressedImage)
	{
		vk::freeHostMemory(decompressedImage, pAllocator);
	}
}

size_t Image::ComputeRequiredAllocationSize(const VkImageCreateInfo *pCreateInfo)
{
	return Format(pCreateInfo->format).isCompressed() ? sizeof(Image) : 0;
}

const VkMemoryRequirements Image::getMemoryRequirements() const
{
	VkMemoryRequirements memoryRequirements;
	memoryRequirements.alignment = vk::MEMORY_REQUIREMENTS_OFFSET_ALIGNMENT;
	memoryRequirements.memoryTypeBits = vk::MEMORY_TYPE_GENERIC_BIT;
	memoryRequirements.size = getStorageSize(format.getAspects()) +
	                          (decompressedImage ? decompressedImage->getStorageSize(decompressedImage->format.getAspects()) : 0);
	return memoryRequirements;
}

void Image::getMemoryRequirements(VkMemoryRequirements2 *pMemoryRequirements) const
{
	VkBaseOutStructure *extensionRequirements = reinterpret_cast<VkBaseOutStructure *>(pMemoryRequirements->pNext);
	while(extensionRequirements)
	{
		switch(extensionRequirements->sType)
		{
		case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS:
			{
				auto *requirements = reinterpret_cast<VkMemoryDedicatedRequirements *>(extensionRequirements);
				device->getRequirements(requirements);
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
				if(getSupportedExternalMemoryHandleTypes() == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
				{
					requirements->prefersDedicatedAllocation = VK_TRUE;
					requirements->requiresDedicatedAllocation = VK_TRUE;
				}
#endif
			}
			break;
		default:
			UNSUPPORTED("pMemoryRequirements->pNext sType = %s", vk::Stringify(extensionRequirements->sType).c_str());
			break;
		}

		extensionRequirements = extensionRequirements->pNext;
	}

	pMemoryRequirements->memoryRequirements = getMemoryRequirements();
}

size_t Image::getSizeInBytes(const VkImageSubresourceRange &subresourceRange) const
{
	size_t size = 0;
	uint32_t lastLayer = getLastLayerIndex(subresourceRange);
	uint32_t lastMipLevel = getLastMipLevel(subresourceRange);
	uint32_t layerCount = lastLayer - subresourceRange.baseArrayLayer + 1;
	uint32_t mipLevelCount = lastMipLevel - subresourceRange.baseMipLevel + 1;

	auto aspect = static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask);

	if(layerCount > 1)
	{
		if(mipLevelCount < mipLevels)  // Compute size for all layers except the last one, then add relevant mip level sizes only for last layer
		{
			size = (layerCount - 1) * getLayerSize(aspect);
			for(uint32_t mipLevel = subresourceRange.baseMipLevel; mipLevel <= lastMipLevel; ++mipLevel)
			{
				size += getMultiSampledLevelSize(aspect, mipLevel);
			}
		}
		else  // All mip levels used, compute full layer sizes
		{
			size = layerCount * getLayerSize(aspect);
		}
	}
	else  // Single layer, add all mip levels in the subresource range
	{
		for(uint32_t mipLevel = subresourceRange.baseMipLevel; mipLevel <= lastMipLevel; ++mipLevel)
		{
			size += getMultiSampledLevelSize(aspect, mipLevel);
		}
	}

	return size;
}

bool Image::canBindToMemory(DeviceMemory *pDeviceMemory) const
{
	return pDeviceMemory->checkExternalMemoryHandleType(supportedExternalMemoryHandleTypes);
}

void Image::bind(DeviceMemory *pDeviceMemory, VkDeviceSize pMemoryOffset)
{
	deviceMemory = pDeviceMemory;
	memoryOffset = pMemoryOffset;
	if(decompressedImage)
	{
		decompressedImage->deviceMemory = deviceMemory;
		decompressedImage->memoryOffset = memoryOffset + getStorageSize(format.getAspects());
	}
}

#ifdef __ANDROID__
VkResult Image::prepareForExternalUseANDROID() const
{
	VkExtent3D extent = getMipLevelExtent(VK_IMAGE_ASPECT_COLOR_BIT, 0);

	AHardwareBuffer_Desc ahbDesc = {};
	ahbDesc.width = extent.width;
	ahbDesc.height = extent.height;
	ahbDesc.layers = 1;
	ahbDesc.format = static_cast<uint32_t>(backingMemory.nativeBufferInfo.format);
	ahbDesc.usage = static_cast<uint64_t>(backingMemory.nativeBufferInfo.usage);
	ahbDesc.stride = static_cast<uint32_t>(backingMemory.nativeBufferInfo.stride);

	AHardwareBuffer *ahb = nullptr;
	if(AHardwareBuffer_createFromHandle(&ahbDesc, backingMemory.nativeBufferInfo.handle, AHARDWAREBUFFER_CREATE_FROM_HANDLE_METHOD_CLONE, &ahb) != 0)
	{
		return VK_ERROR_OUT_OF_DATE_KHR;
	}
	if(!ahb)
	{
		return VK_ERROR_OUT_OF_DATE_KHR;
	}

	ARect ahbRect = {};
	ahbRect.left = 0;
	ahbRect.top = 0;
	ahbRect.right = static_cast<int32_t>(extent.width);
	ahbRect.bottom = static_cast<int32_t>(extent.height);

	AHardwareBuffer_Planes ahbPlanes = {};
	if(AHardwareBuffer_lockPlanes(ahb, AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN, /*fence=*/-1, &ahbRect, &ahbPlanes) != 0)
	{
		return VK_ERROR_OUT_OF_DATE_KHR;
	}

	int imageRowBytes = rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	int bufferRowBytes = backingMemory.nativeBufferInfo.stride * getFormat().bytes();
	ASSERT(imageRowBytes <= bufferRowBytes);

	uint8_t *srcBuffer = static_cast<uint8_t *>(deviceMemory->getOffsetPointer(0));
	uint8_t *dstBuffer = static_cast<uint8_t *>(ahbPlanes.planes[0].data);
	for(uint32_t i = 0; i < extent.height; i++)
	{
		memcpy(dstBuffer + (i * bufferRowBytes), srcBuffer + (i * imageRowBytes), imageRowBytes);
	}

	AHardwareBuffer_unlock(ahb, /*fence=*/nullptr);
	AHardwareBuffer_release(ahb);

	return VK_SUCCESS;
}

VkDeviceMemory Image::getExternalMemory() const
{
	return backingMemory.externalMemory ? *deviceMemory : VkDeviceMemory{ VK_NULL_HANDLE };
}
#endif

void Image::getSubresourceLayout(const VkImageSubresource *pSubresource, VkSubresourceLayout *pLayout) const
{
	// By spec, aspectMask has a single bit set.
	if(!((pSubresource->aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	     (pSubresource->aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
	     (pSubresource->aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) ||
	     (pSubresource->aspectMask == VK_IMAGE_ASPECT_PLANE_0_BIT) ||
	     (pSubresource->aspectMask == VK_IMAGE_ASPECT_PLANE_1_BIT) ||
	     (pSubresource->aspectMask == VK_IMAGE_ASPECT_PLANE_2_BIT)))
	{
		UNSUPPORTED("aspectMask %X", pSubresource->aspectMask);
	}

	auto aspect = static_cast<VkImageAspectFlagBits>(pSubresource->aspectMask);
	pLayout->offset = getSubresourceOffset(aspect, pSubresource->mipLevel, pSubresource->arrayLayer);
	pLayout->size = getMultiSampledLevelSize(aspect, pSubresource->mipLevel);
	pLayout->rowPitch = rowPitchBytes(aspect, pSubresource->mipLevel);
	pLayout->depthPitch = slicePitchBytes(aspect, pSubresource->mipLevel);
	pLayout->arrayPitch = getLayerSize(aspect);
}

void Image::copyTo(Image *dstImage, const VkImageCopy2KHR &region) const
{
	static constexpr VkImageAspectFlags CombinedDepthStencilAspects =
	    VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if((region.srcSubresource.aspectMask == CombinedDepthStencilAspects) &&
	   (region.dstSubresource.aspectMask == CombinedDepthStencilAspects))
	{
		// Depth and stencil can be specified together, copy each separately
		VkImageCopy2KHR singleAspectRegion = region;
		singleAspectRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		singleAspectRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		copySingleAspectTo(dstImage, singleAspectRegion);
		singleAspectRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		singleAspectRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		copySingleAspectTo(dstImage, singleAspectRegion);
		return;
	}

	copySingleAspectTo(dstImage, region);
}

void Image::copySingleAspectTo(Image *dstImage, const VkImageCopy2KHR &region) const
{
	// Image copy does not perform any conversion, it simply copies memory from
	// an image to another image that has the same number of bytes per pixel.

	if(!((region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	     (region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
	     (region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) ||
	     (region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_0_BIT) ||
	     (region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_1_BIT) ||
	     (region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_2_BIT)))
	{
		UNSUPPORTED("srcSubresource.aspectMask %X", region.srcSubresource.aspectMask);
	}

	if(!((region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	     (region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) ||
	     (region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) ||
	     (region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_0_BIT) ||
	     (region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_1_BIT) ||
	     (region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_PLANE_2_BIT)))
	{
		UNSUPPORTED("dstSubresource.aspectMask %X", region.dstSubresource.aspectMask);
	}

	VkImageAspectFlagBits srcAspect = static_cast<VkImageAspectFlagBits>(region.srcSubresource.aspectMask);
	VkImageAspectFlagBits dstAspect = static_cast<VkImageAspectFlagBits>(region.dstSubresource.aspectMask);

	Format srcFormat = getFormat(srcAspect);
	Format dstFormat = dstImage->getFormat(dstAspect);
	int bytesPerBlock = srcFormat.bytesPerBlock();
	ASSERT(bytesPerBlock == dstFormat.bytesPerBlock());
	ASSERT(samples == dstImage->samples);

	VkExtent3D srcExtent = getMipLevelExtent(srcAspect, region.srcSubresource.mipLevel);
	VkExtent3D dstExtent = dstImage->getMipLevelExtent(dstAspect, region.dstSubresource.mipLevel);
	VkExtent3D copyExtent = imageExtentInBlocks(region.extent, srcAspect);

	VkImageType srcImageType = imageType;
	VkImageType dstImageType = dstImage->getImageType();
	bool one3D = (srcImageType == VK_IMAGE_TYPE_3D) != (dstImageType == VK_IMAGE_TYPE_3D);
	bool both3D = (srcImageType == VK_IMAGE_TYPE_3D) && (dstImageType == VK_IMAGE_TYPE_3D);

	// Texel layout pitches, using the VkSubresourceLayout nomenclature.
	int srcRowPitch = rowPitchBytes(srcAspect, region.srcSubresource.mipLevel);
	int srcDepthPitch = slicePitchBytes(srcAspect, region.srcSubresource.mipLevel);
	int dstRowPitch = dstImage->rowPitchBytes(dstAspect, region.dstSubresource.mipLevel);
	int dstDepthPitch = dstImage->slicePitchBytes(dstAspect, region.dstSubresource.mipLevel);
	VkDeviceSize srcArrayPitch = getLayerSize(srcAspect);
	VkDeviceSize dstArrayPitch = dstImage->getLayerSize(dstAspect);

	// These are the pitches used when iterating over the layers that are being copied by the
	// vkCmdCopyImage command. They can differ from the above array piches because the spec states that:
	// "If one image is VK_IMAGE_TYPE_3D and the other image is VK_IMAGE_TYPE_2D with multiple
	//  layers, then each slice is copied to or from a different layer."
	VkDeviceSize srcLayerPitch = (srcImageType == VK_IMAGE_TYPE_3D) ? srcDepthPitch : srcArrayPitch;
	VkDeviceSize dstLayerPitch = (dstImageType == VK_IMAGE_TYPE_3D) ? dstDepthPitch : dstArrayPitch;

	// If one image is 3D, extent.depth must match the layer count. If both images are 2D,
	// depth is 1 but the source and destination subresource layer count must match.
	uint32_t layerCount = one3D ? copyExtent.depth : region.srcSubresource.layerCount;

	// Copies between 2D and 3D images are treated as layers, so only use depth as the slice count when
	// both images are 3D.
	// Multisample images are currently implemented similar to 3D images by storing one sample per slice.
	// TODO(b/160600347): Store samples consecutively.
	uint32_t sliceCount = both3D ? copyExtent.depth : samples;

	bool isSingleSlice = (sliceCount == 1);
	bool isSingleRow = (copyExtent.height == 1) && isSingleSlice;
	// In order to copy multiple rows using a single memcpy call, we
	// have to make sure that we need to copy the entire row and that
	// both source and destination rows have the same size in bytes
	bool isEntireRow = (region.extent.width == srcExtent.width) &&
	                   (region.extent.width == dstExtent.width) &&
	                   // For non-compressed formats, blockWidth is 1. For compressed
	                   // formats, rowPitchBytes returns the number of bytes for a row of
	                   // blocks, so we have to divide by the block height, which means:
	                   // srcRowPitchBytes / srcBlockWidth == dstRowPitchBytes / dstBlockWidth
	                   // And, to avoid potential non exact integer division, for example if a
	                   // block has 16 bytes and represents 5 rows, we change the equation to:
	                   // srcRowPitchBytes * dstBlockWidth == dstRowPitchBytes * srcBlockWidth
	                   ((srcRowPitch * dstFormat.blockWidth()) ==
	                    (dstRowPitch * srcFormat.blockWidth()));
	// In order to copy multiple slices using a single memcpy call, we
	// have to make sure that we need to copy the entire slice and that
	// both source and destination slices have the same size in bytes
	bool isEntireSlice = isEntireRow &&
	                     (copyExtent.height == srcExtent.height) &&
	                     (copyExtent.height == dstExtent.height) &&
	                     (srcDepthPitch == dstDepthPitch);

	const uint8_t *srcLayer = static_cast<const uint8_t *>(getTexelPointer(region.srcOffset, ImageSubresource(region.srcSubresource)));
	uint8_t *dstLayer = static_cast<uint8_t *>(dstImage->getTexelPointer(region.dstOffset, ImageSubresource(region.dstSubresource)));

	for(uint32_t layer = 0; layer < layerCount; layer++)
	{
		if(isSingleRow)  // Copy one row
		{
			size_t copySize = copyExtent.width * bytesPerBlock;
			ASSERT((srcLayer + copySize) < end());
			ASSERT((dstLayer + copySize) < dstImage->end());
			memcpy(dstLayer, srcLayer, copySize);
		}
		else if(isEntireRow && isSingleSlice)  // Copy one slice
		{
			size_t copySize = copyExtent.height * srcRowPitch;
			ASSERT((srcLayer + copySize) < end());
			ASSERT((dstLayer + copySize) < dstImage->end());
			memcpy(dstLayer, srcLayer, copySize);
		}
		else if(isEntireSlice)  // Copy multiple slices
		{
			size_t copySize = sliceCount * srcDepthPitch;
			ASSERT((srcLayer + copySize) < end());
			ASSERT((dstLayer + copySize) < dstImage->end());
			memcpy(dstLayer, srcLayer, copySize);
		}
		else if(isEntireRow)  // Copy slice by slice
		{
			size_t sliceSize = copyExtent.height * srcRowPitch;
			const uint8_t *srcSlice = srcLayer;
			uint8_t *dstSlice = dstLayer;

			for(uint32_t z = 0; z < sliceCount; z++)
			{
				ASSERT((srcSlice + sliceSize) < end());
				ASSERT((dstSlice + sliceSize) < dstImage->end());

				memcpy(dstSlice, srcSlice, sliceSize);

				dstSlice += dstDepthPitch;
				srcSlice += srcDepthPitch;
			}
		}
		else  // Copy row by row
		{
			size_t rowSize = copyExtent.width * bytesPerBlock;
			const uint8_t *srcSlice = srcLayer;
			uint8_t *dstSlice = dstLayer;

			for(uint32_t z = 0; z < sliceCount; z++)
			{
				const uint8_t *srcRow = srcSlice;
				uint8_t *dstRow = dstSlice;

				for(uint32_t y = 0; y < copyExtent.height; y++)
				{
					ASSERT((srcRow + rowSize) < end());
					ASSERT((dstRow + rowSize) < dstImage->end());

					memcpy(dstRow, srcRow, rowSize);

					srcRow += srcRowPitch;
					dstRow += dstRowPitch;
				}

				srcSlice += srcDepthPitch;
				dstSlice += dstDepthPitch;
			}
		}

		srcLayer += srcLayerPitch;
		dstLayer += dstLayerPitch;
	}

	dstImage->contentsChanged(ImageSubresourceRange(region.dstSubresource));
}

void Image::copy(const void *srcCopyMemory,
                 void *dstCopyMemory,
                 uint32_t rowLength,
                 uint32_t imageHeight,
                 const VkImageSubresourceLayers &imageSubresource,
                 const VkOffset3D &imageCopyOffset,
                 const VkExtent3D &imageCopyExtent)
{
	// Decide on whether copying from buffer/memory or to buffer/memory
	ASSERT((srcCopyMemory == nullptr) != (dstCopyMemory == nullptr));
	const bool memoryIsSource = srcCopyMemory != nullptr;

	switch(imageSubresource.aspectMask)
	{
	case VK_IMAGE_ASPECT_COLOR_BIT:
	case VK_IMAGE_ASPECT_DEPTH_BIT:
	case VK_IMAGE_ASPECT_STENCIL_BIT:
	case VK_IMAGE_ASPECT_PLANE_0_BIT:
	case VK_IMAGE_ASPECT_PLANE_1_BIT:
	case VK_IMAGE_ASPECT_PLANE_2_BIT:
		break;
	default:
		UNSUPPORTED("aspectMask %x", int(imageSubresource.aspectMask));
		break;
	}

	auto aspect = static_cast<VkImageAspectFlagBits>(imageSubresource.aspectMask);
	Format copyFormat = getFormat(aspect);

	VkExtent3D imageExtent = imageExtentInBlocks(imageCopyExtent, aspect);

	if(imageExtent.width == 0 || imageExtent.height == 0 || imageExtent.depth == 0)
	{
		return;
	}

	VkExtent2D extent = bufferExtentInBlocks(Extent2D(imageExtent), rowLength, imageHeight, imageSubresource, imageCopyOffset);
	int bytesPerBlock = copyFormat.bytesPerBlock();
	int memoryRowPitchBytes = extent.width * bytesPerBlock;
	int memorySlicePitchBytes = extent.height * memoryRowPitchBytes;
	ASSERT(samples == 1);

	uint8_t *imageMemory = static_cast<uint8_t *>(getTexelPointer(imageCopyOffset, ImageSubresource(imageSubresource)));
	const uint8_t *srcMemory = memoryIsSource ? static_cast<const uint8_t *>(srcCopyMemory) : imageMemory;
	uint8_t *dstMemory = memoryIsSource ? imageMemory : static_cast<uint8_t *>(dstCopyMemory);
	int imageRowPitchBytes = rowPitchBytes(aspect, imageSubresource.mipLevel);
	int imageSlicePitchBytes = slicePitchBytes(aspect, imageSubresource.mipLevel);

	int srcSlicePitchBytes = memoryIsSource ? memorySlicePitchBytes : imageSlicePitchBytes;
	int dstSlicePitchBytes = memoryIsSource ? imageSlicePitchBytes : memorySlicePitchBytes;
	int srcRowPitchBytes = memoryIsSource ? memoryRowPitchBytes : imageRowPitchBytes;
	int dstRowPitchBytes = memoryIsSource ? imageRowPitchBytes : memoryRowPitchBytes;

	VkDeviceSize copySize = imageExtent.width * bytesPerBlock;

	VkDeviceSize imageLayerSize = getLayerSize(aspect);
	VkDeviceSize srcLayerSize = memoryIsSource ? memorySlicePitchBytes : imageLayerSize;
	VkDeviceSize dstLayerSize = memoryIsSource ? imageLayerSize : memorySlicePitchBytes;

	for(uint32_t i = 0; i < imageSubresource.layerCount; i++)
	{
		const uint8_t *srcLayerMemory = srcMemory;
		uint8_t *dstLayerMemory = dstMemory;
		for(uint32_t z = 0; z < imageExtent.depth; z++)
		{
			const uint8_t *srcSliceMemory = srcLayerMemory;
			uint8_t *dstSliceMemory = dstLayerMemory;
			for(uint32_t y = 0; y < imageExtent.height; y++)
			{
				ASSERT(((memoryIsSource ? dstSliceMemory : srcSliceMemory) + copySize) < end());
				memcpy(dstSliceMemory, srcSliceMemory, copySize);
				srcSliceMemory += srcRowPitchBytes;
				dstSliceMemory += dstRowPitchBytes;
			}
			srcLayerMemory += srcSlicePitchBytes;
			dstLayerMemory += dstSlicePitchBytes;
		}

		srcMemory += srcLayerSize;
		dstMemory += dstLayerSize;
	}

	if(memoryIsSource)
	{
		contentsChanged(ImageSubresourceRange(imageSubresource));
	}
}

void Image::copyTo(Buffer *dstBuffer, const VkBufferImageCopy2KHR &region)
{
	copy(nullptr, dstBuffer->getOffsetPointer(region.bufferOffset), region.bufferRowLength, region.bufferImageHeight, region.imageSubresource, region.imageOffset, region.imageExtent);
}

void Image::copyFrom(Buffer *srcBuffer, const VkBufferImageCopy2KHR &region)
{
	copy(srcBuffer->getOffsetPointer(region.bufferOffset), nullptr, region.bufferRowLength, region.bufferImageHeight, region.imageSubresource, region.imageOffset, region.imageExtent);
}

void Image::copyToMemory(const VkImageToMemoryCopyEXT &region)
{
	copy(nullptr, region.pHostPointer, region.memoryRowLength, region.memoryImageHeight, region.imageSubresource, region.imageOffset, region.imageExtent);
}

void Image::copyFromMemory(const VkMemoryToImageCopyEXT &region)
{
	copy(region.pHostPointer, nullptr, region.memoryRowLength, region.memoryImageHeight, region.imageSubresource, region.imageOffset, region.imageExtent);
}

void *Image::getTexelPointer(const VkOffset3D &offset, const VkImageSubresource &subresource) const
{
	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(subresource.aspectMask);
	return deviceMemory->getOffsetPointer(getMemoryOffset(aspect) +
	                                      texelOffsetBytesInStorage(offset, subresource) +
	                                      getSubresourceOffset(aspect, subresource.mipLevel, subresource.arrayLayer));
}

VkExtent3D Image::imageExtentInBlocks(const VkExtent3D &extent, VkImageAspectFlagBits aspect) const
{
	VkExtent3D adjustedExtent = extent;
	Format usedFormat = getFormat(aspect);
	if(usedFormat.isCompressed())
	{
		// When using a compressed format, we use the block as the base unit, instead of the texel
		int blockWidth = usedFormat.blockWidth();
		int blockHeight = usedFormat.blockHeight();

		// Mip level allocations will round up to the next block for compressed texture
		adjustedExtent.width = ((adjustedExtent.width + blockWidth - 1) / blockWidth);
		adjustedExtent.height = ((adjustedExtent.height + blockHeight - 1) / blockHeight);
	}
	return adjustedExtent;
}

VkOffset3D Image::imageOffsetInBlocks(const VkOffset3D &offset, VkImageAspectFlagBits aspect) const
{
	VkOffset3D adjustedOffset = offset;
	Format usedFormat = getFormat(aspect);
	if(usedFormat.isCompressed())
	{
		// When using a compressed format, we use the block as the base unit, instead of the texel
		int blockWidth = usedFormat.blockWidth();
		int blockHeight = usedFormat.blockHeight();

		ASSERT(((offset.x % blockWidth) == 0) && ((offset.y % blockHeight) == 0));  // We can't offset within a block

		adjustedOffset.x /= blockWidth;
		adjustedOffset.y /= blockHeight;
	}
	return adjustedOffset;
}

VkExtent2D Image::bufferExtentInBlocks(const VkExtent2D &extent, uint32_t rowLength, uint32_t imageHeight, const VkImageSubresourceLayers &imageSubresource, const VkOffset3D &imageOffset) const
{
	VkExtent2D adjustedExtent = extent;
	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(imageSubresource.aspectMask);
	Format usedFormat = getFormat(aspect);

	if(rowLength != 0)
	{
		adjustedExtent.width = rowLength;

		if(usedFormat.isCompressed())
		{
			int blockWidth = usedFormat.blockWidth();
			ASSERT((adjustedExtent.width % blockWidth == 0) || (adjustedExtent.width + imageOffset.x == extent.width));
			adjustedExtent.width = (rowLength + blockWidth - 1) / blockWidth;
		}
	}

	if(imageHeight != 0)
	{
		adjustedExtent.height = imageHeight;

		if(usedFormat.isCompressed())
		{
			int blockHeight = usedFormat.blockHeight();
			ASSERT((adjustedExtent.height % blockHeight == 0) || (adjustedExtent.height + imageOffset.y == extent.height));
			adjustedExtent.height = (imageHeight + blockHeight - 1) / blockHeight;
		}
	}

	return adjustedExtent;
}

int Image::borderSize() const
{
	// We won't add a border to compressed cube textures, we'll add it when we decompress the texture
	return (isCubeCompatible() && !format.isCompressed()) ? 1 : 0;
}

VkDeviceSize Image::texelOffsetBytesInStorage(const VkOffset3D &offset, const VkImageSubresource &subresource) const
{
	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(subresource.aspectMask);
	VkOffset3D adjustedOffset = imageOffsetInBlocks(offset, aspect);
	int border = borderSize();
	return adjustedOffset.z * slicePitchBytes(aspect, subresource.mipLevel) +
	       (adjustedOffset.y + border) * rowPitchBytes(aspect, subresource.mipLevel) +
	       (adjustedOffset.x + border) * getFormat(aspect).bytesPerBlock();
}

VkExtent3D Image::getMipLevelExtent(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	VkExtent3D mipLevelExtent;
	mipLevelExtent.width = extent.width >> mipLevel;
	mipLevelExtent.height = extent.height >> mipLevel;
	mipLevelExtent.depth = extent.depth >> mipLevel;

	if(mipLevelExtent.width == 0) { mipLevelExtent.width = 1; }
	if(mipLevelExtent.height == 0) { mipLevelExtent.height = 1; }
	if(mipLevelExtent.depth == 0) { mipLevelExtent.depth = 1; }

	switch(aspect)
	{
	case VK_IMAGE_ASPECT_COLOR_BIT:
	case VK_IMAGE_ASPECT_DEPTH_BIT:
	case VK_IMAGE_ASPECT_STENCIL_BIT:
	case VK_IMAGE_ASPECT_PLANE_0_BIT:  // Vulkan 1.1 Table 31. Plane Format Compatibility Table: plane 0 of all defined formats is full resolution.
		break;
	case VK_IMAGE_ASPECT_PLANE_1_BIT:
	case VK_IMAGE_ASPECT_PLANE_2_BIT:
		switch(format)
		{
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
			ASSERT(mipLevelExtent.width % 2 == 0 && mipLevelExtent.height % 2 == 0);  // Vulkan 1.1: "Images in this format must be defined with a width and height that is a multiple of two."
			// Vulkan 1.1 Table 31. Plane Format Compatibility Table:
			// Half-resolution U and V planes.
			mipLevelExtent.width /= 2;
			mipLevelExtent.height /= 2;
			break;
		default:
			UNSUPPORTED("format %d", int(format));
		}
		break;
	default:
		UNSUPPORTED("aspect %x", int(aspect));
	}

	return mipLevelExtent;
}

size_t Image::rowPitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	if(deviceMemory && deviceMemory->hasExternalImagePlanes())
	{
		return deviceMemory->externalImageRowPitchBytes(aspect);
	}

	// Depth and Stencil pitch should be computed separately
	ASSERT((aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) !=
	       (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));

	VkExtent3D mipLevelExtent = getMipLevelExtent(aspect, mipLevel);
	Format usedFormat = getFormat(aspect);
	if(usedFormat.isCompressed())
	{
		VkExtent3D extentInBlocks = imageExtentInBlocks(mipLevelExtent, aspect);
		return extentInBlocks.width * usedFormat.bytesPerBlock();
	}

	return usedFormat.pitchB(mipLevelExtent.width, borderSize());
}

size_t Image::slicePitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	// Depth and Stencil slice should be computed separately
	ASSERT((aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) !=
	       (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));

	VkExtent3D mipLevelExtent = getMipLevelExtent(aspect, mipLevel);
	Format usedFormat = getFormat(aspect);
	if(usedFormat.isCompressed())
	{
		VkExtent3D extentInBlocks = imageExtentInBlocks(mipLevelExtent, aspect);
		return extentInBlocks.height * extentInBlocks.width * usedFormat.bytesPerBlock();
	}

	return usedFormat.sliceB(mipLevelExtent.width, mipLevelExtent.height, borderSize());
}

Format Image::getFormat(VkImageAspectFlagBits aspect) const
{
	return format.getAspectFormat(aspect);
}

bool Image::isCubeCompatible() const
{
	bool cubeCompatible = (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT);
	ASSERT(!cubeCompatible || (imageType == VK_IMAGE_TYPE_2D));  // VUID-VkImageCreateInfo-flags-00949
	ASSERT(!cubeCompatible || (arrayLayers >= 6));               // VUID-VkImageCreateInfo-imageType-00954

	return cubeCompatible;
}

uint8_t *Image::end() const
{
	return reinterpret_cast<uint8_t *>(deviceMemory->getOffsetPointer(deviceMemory->getCommittedMemoryInBytes() + 1));
}

VkDeviceSize Image::getMemoryOffset(VkImageAspectFlagBits aspect) const
{
	if(deviceMemory && deviceMemory->hasExternalImagePlanes())
	{
		return deviceMemory->externalImageMemoryOffset(aspect);
	}

	return memoryOffset;
}

VkDeviceSize Image::getAspectOffset(VkImageAspectFlagBits aspect) const
{
	switch(format)
	{
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		if(aspect == VK_IMAGE_ASPECT_STENCIL_BIT)
		{
			// Offset by depth buffer to get to stencil buffer
			return getStorageSize(VK_IMAGE_ASPECT_DEPTH_BIT);
		}
		break;

	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		if(aspect == VK_IMAGE_ASPECT_PLANE_2_BIT)
		{
			return getStorageSize(VK_IMAGE_ASPECT_PLANE_1_BIT) + getStorageSize(VK_IMAGE_ASPECT_PLANE_0_BIT);
		}
		// Fall through to 2PLANE case:
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		if(aspect == VK_IMAGE_ASPECT_PLANE_1_BIT)
		{
			return getStorageSize(VK_IMAGE_ASPECT_PLANE_0_BIT);
		}
		else
		{
			ASSERT(aspect == VK_IMAGE_ASPECT_PLANE_0_BIT);

			return 0;
		}
		break;

	default:
		break;
	}

	return 0;
}

VkDeviceSize Image::getSubresourceOffset(VkImageAspectFlagBits aspect, uint32_t mipLevel, uint32_t layer) const
{
	// "If the image is disjoint, then the offset is relative to the base address of the plane.
	//  If the image is non-disjoint, then the offset is relative to the base address of the image."
	// Multi-plane external images are essentially disjoint.
	bool disjoint = (flags & VK_IMAGE_CREATE_DISJOINT_BIT) || (deviceMemory && deviceMemory->hasExternalImagePlanes());
	VkDeviceSize offset = !disjoint ? getAspectOffset(aspect) : 0;

	for(uint32_t i = 0; i < mipLevel; i++)
	{
		offset += getMultiSampledLevelSize(aspect, i);
	}

	return offset + layer * getLayerOffset(aspect, mipLevel);
}

VkDeviceSize Image::getMipLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	return slicePitchBytes(aspect, mipLevel) * getMipLevelExtent(aspect, mipLevel).depth;
}

VkDeviceSize Image::getMultiSampledLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	return getMipLevelSize(aspect, mipLevel) * samples;
}

bool Image::is3DSlice() const
{
	return ((imageType == VK_IMAGE_TYPE_3D) && (flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT));
}

VkDeviceSize Image::getLayerOffset(VkImageAspectFlagBits aspect, uint32_t mipLevel) const
{
	if(is3DSlice())
	{
		// When the VkImageSubresourceRange structure is used to select a subset of the slices of a 3D
		// image's mip level in order to create a 2D or 2D array image view of a 3D image created with
		// VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT, baseArrayLayer and layerCount specify the first
		// slice index and the number of slices to include in the created image view.
		ASSERT(samples == VK_SAMPLE_COUNT_1_BIT);

		// Offset to the proper slice of the 3D image's mip level
		return slicePitchBytes(aspect, mipLevel);
	}

	return getLayerSize(aspect);
}

VkDeviceSize Image::getLayerSize(VkImageAspectFlagBits aspect) const
{
	VkDeviceSize layerSize = 0;

	for(uint32_t mipLevel = 0; mipLevel < mipLevels; ++mipLevel)
	{
		layerSize += getMultiSampledLevelSize(aspect, mipLevel);
	}

	return layerSize;
}

VkDeviceSize Image::getStorageSize(VkImageAspectFlags aspectMask) const
{
	if((aspectMask & ~(VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT |
	                   VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT)) != 0)
	{
		UNSUPPORTED("aspectMask %x", int(aspectMask));
	}

	VkDeviceSize storageSize = 0;

	if(aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_COLOR_BIT);
	if(aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_DEPTH_BIT);
	if(aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_STENCIL_BIT);
	if(aspectMask & VK_IMAGE_ASPECT_PLANE_0_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_PLANE_0_BIT);
	if(aspectMask & VK_IMAGE_ASPECT_PLANE_1_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_PLANE_1_BIT);
	if(aspectMask & VK_IMAGE_ASPECT_PLANE_2_BIT) storageSize += getLayerSize(VK_IMAGE_ASPECT_PLANE_2_BIT);

	return arrayLayers * storageSize;
}

const Image *Image::getSampledImage(const vk::Format &imageViewFormat) const
{
	bool isImageViewCompressed = imageViewFormat.isCompressed();
	if(decompressedImage && !isImageViewCompressed)
	{
		ASSERT(flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT);
		ASSERT(format.bytesPerBlock() == imageViewFormat.bytesPerBlock());
	}
	// If the ImageView's format is compressed, then we do need to decompress the image so that
	// it may be sampled properly by texture sampling functions, which don't support compressed
	// textures. If the ImageView's format is NOT compressed, then we reinterpret cast the
	// compressed image into the ImageView's format, so we must return the compressed image as is.
	return (decompressedImage && isImageViewCompressed) ? decompressedImage : this;
}

void Image::blitTo(Image *dstImage, const VkImageBlit2KHR &region, VkFilter filter) const
{
	prepareForSampling(ImageSubresourceRange(region.srcSubresource));
	device->getBlitter()->blit(decompressedImage ? decompressedImage : this, dstImage, region, filter);
}

void Image::copyTo(uint8_t *dst, unsigned int dstPitch) const
{
	device->getBlitter()->copy(this, dst, dstPitch);
}

void Image::resolveTo(Image *dstImage, const VkImageResolve2KHR &region) const
{
	device->getBlitter()->resolve(this, dstImage, region);
}

void Image::resolveDepthStencilTo(const ImageView *src, ImageView *dst, VkResolveModeFlagBits depthResolveMode, VkResolveModeFlagBits stencilResolveMode) const
{
	device->getBlitter()->resolveDepthStencil(src, dst, depthResolveMode, stencilResolveMode);
}

uint32_t Image::getLastLayerIndex(const VkImageSubresourceRange &subresourceRange) const
{
	return ((subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS) ? arrayLayers : (subresourceRange.baseArrayLayer + subresourceRange.layerCount)) - 1;
}

uint32_t Image::getLastMipLevel(const VkImageSubresourceRange &subresourceRange) const
{
	return ((subresourceRange.levelCount == VK_REMAINING_MIP_LEVELS) ? mipLevels : (subresourceRange.baseMipLevel + subresourceRange.levelCount)) - 1;
}

void Image::clear(const void *pixelData, VkFormat pixelFormat, const vk::Format &viewFormat, const VkImageSubresourceRange &subresourceRange, const VkRect2D *renderArea)
{
	device->getBlitter()->clear(pixelData, pixelFormat, this, viewFormat, subresourceRange, renderArea);
}

void Image::clear(const VkClearColorValue &color, const VkImageSubresourceRange &subresourceRange)
{
	ASSERT(subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);

	clear(color.float32, format.getClearFormat(), format, subresourceRange, nullptr);
}

void Image::clear(const VkClearDepthStencilValue &color, const VkImageSubresourceRange &subresourceRange)
{
	ASSERT((subresourceRange.aspectMask & ~(VK_IMAGE_ASPECT_DEPTH_BIT |
	                                        VK_IMAGE_ASPECT_STENCIL_BIT)) == 0);

	if(subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
	{
		VkImageSubresourceRange depthSubresourceRange = subresourceRange;
		depthSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		clear(&color.depth, VK_FORMAT_D32_SFLOAT, format, depthSubresourceRange, nullptr);
	}

	if(subresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
	{
		VkImageSubresourceRange stencilSubresourceRange = subresourceRange;
		stencilSubresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		clear(&color.stencil, VK_FORMAT_S8_UINT, format, stencilSubresourceRange, nullptr);
	}
}

void Image::clear(const VkClearValue &clearValue, const vk::Format &viewFormat, const VkRect2D &renderArea, const VkImageSubresourceRange &subresourceRange)
{
	ASSERT((subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT) ||
	       (subresourceRange.aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT |
	                                       VK_IMAGE_ASPECT_STENCIL_BIT)));

	if(subresourceRange.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT)
	{
		clear(clearValue.color.float32, viewFormat.getClearFormat(), viewFormat, subresourceRange, &renderArea);
	}
	else
	{
		if(subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
		{
			VkImageSubresourceRange depthSubresourceRange = subresourceRange;
			depthSubresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			clear(&clearValue.depthStencil.depth, VK_FORMAT_D32_SFLOAT, viewFormat, depthSubresourceRange, &renderArea);
		}

		if(subresourceRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
		{
			VkImageSubresourceRange stencilSubresourceRange = subresourceRange;
			stencilSubresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
			clear(&clearValue.depthStencil.stencil, VK_FORMAT_S8_UINT, viewFormat, stencilSubresourceRange, &renderArea);
		}
	}
}

bool Image::requiresPreprocessing() const
{
	return isCubeCompatible() || decompressedImage;
}

void Image::contentsChanged(const VkImageSubresourceRange &subresourceRange, ContentsChangedContext contentsChangedContext)
{
	// If this function is called after (possibly) writing to this image from a shader,
	// this must have the VK_IMAGE_USAGE_STORAGE_BIT set for the write operation to be
	// valid. Otherwise, we can't have legally written to this image, so we know we can
	// skip updating dirtyResources.
	if((contentsChangedContext == USING_STORAGE) && !(usage & VK_IMAGE_USAGE_STORAGE_BIT))
	{
		return;
	}

	// If this isn't a cube or a compressed image, we'll never need dirtyResources,
	// so we can skip updating dirtyResources
	if(!requiresPreprocessing())
	{
		return;
	}

	uint32_t lastLayer = getLastLayerIndex(subresourceRange);
	uint32_t lastMipLevel = getLastMipLevel(subresourceRange);

	VkImageSubresource subresource = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer
	};

	marl::lock lock(mutex);
	for(subresource.arrayLayer = subresourceRange.baseArrayLayer;
	    subresource.arrayLayer <= lastLayer;
	    subresource.arrayLayer++)
	{
		for(subresource.mipLevel = subresourceRange.baseMipLevel;
		    subresource.mipLevel <= lastMipLevel;
		    subresource.mipLevel++)
		{
			dirtySubresources.insert(subresource);
		}
	}
}

void Image::prepareForSampling(const VkImageSubresourceRange &subresourceRange) const
{
	// If this isn't a cube or a compressed image, there's nothing to do
	if(!requiresPreprocessing())
	{
		return;
	}

	uint32_t lastLayer = getLastLayerIndex(subresourceRange);
	uint32_t lastMipLevel = getLastMipLevel(subresourceRange);

	VkImageSubresource subresource = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer
	};

	marl::lock lock(mutex);

	if(dirtySubresources.empty())
	{
		return;
	}

	// First, decompress all relevant dirty subregions
	if(decompressedImage)
	{
		for(subresource.mipLevel = subresourceRange.baseMipLevel;
		    subresource.mipLevel <= lastMipLevel;
		    subresource.mipLevel++)
		{
			for(subresource.arrayLayer = subresourceRange.baseArrayLayer;
			    subresource.arrayLayer <= lastLayer;
			    subresource.arrayLayer++)
			{
				auto it = dirtySubresources.find(subresource);
				if(it != dirtySubresources.end())
				{
					decompress(subresource);
				}
			}
		}
	}

	// Second, update cubemap borders
	if(isCubeCompatible())
	{
		for(subresource.mipLevel = subresourceRange.baseMipLevel;
		    subresource.mipLevel <= lastMipLevel;
		    subresource.mipLevel++)
		{
			for(subresource.arrayLayer = subresourceRange.baseArrayLayer;
			    subresource.arrayLayer <= lastLayer;
			    subresource.arrayLayer++)
			{
				auto it = dirtySubresources.find(subresource);
				if(it != dirtySubresources.end())
				{
					// Since cube faces affect each other's borders, we update all 6 layers.

					subresource.arrayLayer -= subresource.arrayLayer % 6;  // Round down to a multiple of 6.

					if(subresource.arrayLayer + 5 <= lastLayer)
					{
						device->getBlitter()->updateBorders(decompressedImage ? decompressedImage : this, subresource);
					}

					subresource.arrayLayer += 5;  // Together with the loop increment, advances to the next cube.
				}
			}
		}
	}

	// Finally, mark all updated subregions clean
	for(subresource.mipLevel = subresourceRange.baseMipLevel;
	    subresource.mipLevel <= lastMipLevel;
	    subresource.mipLevel++)
	{
		for(subresource.arrayLayer = subresourceRange.baseArrayLayer;
		    subresource.arrayLayer <= lastLayer;
		    subresource.arrayLayer++)
		{
			auto it = dirtySubresources.find(subresource);
			if(it != dirtySubresources.end())
			{
				dirtySubresources.erase(it);
			}
		}
	}
}

void Image::decompress(const VkImageSubresource &subresource) const
{
	switch(format)
	{
	case VK_FORMAT_EAC_R11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11_SNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
	case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
	case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		decodeETC2(subresource);
		break;
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
		decodeBC(subresource);
		break;
	case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
	case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
	case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
	case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
	case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
	case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
	case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
	case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
		decodeASTC(subresource);
		break;
	default:
		UNSUPPORTED("Compressed format %d", (VkFormat)format);
		break;
	}
}

void Image::decodeETC2(const VkImageSubresource &subresource) const
{
	ASSERT(decompressedImage);

	ETC_Decoder::InputType inputType = GetInputType(format);

	int bytes = decompressedImage->format.bytes();
	bool fakeAlpha = (format == VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK) || (format == VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
	size_t sizeToWrite = 0;

	VkExtent3D mipLevelExtent = getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresource.aspectMask), subresource.mipLevel);

	int pitchB = decompressedImage->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, subresource.mipLevel);

	if(fakeAlpha)
	{
		// To avoid overflow in case of cube textures, which are offset in memory to account for the border,
		// compute the size from the first pixel to the last pixel, excluding any padding or border before
		// the first pixel or after the last pixel.
		sizeToWrite = ((mipLevelExtent.height - 1) * pitchB) + (mipLevelExtent.width * bytes);
	}

	for(int32_t depth = 0; depth < static_cast<int32_t>(mipLevelExtent.depth); depth++)
	{
		uint8_t *source = static_cast<uint8_t *>(getTexelPointer({ 0, 0, depth }, subresource));
		uint8_t *dest = static_cast<uint8_t *>(decompressedImage->getTexelPointer({ 0, 0, depth }, subresource));

		if(fakeAlpha)
		{
			ASSERT((dest + sizeToWrite) < decompressedImage->end());
			memset(dest, 0xFF, sizeToWrite);
		}

		ETC_Decoder::Decode(source, dest, mipLevelExtent.width, mipLevelExtent.height,
		                    pitchB, bytes, inputType);
	}
}

void Image::decodeBC(const VkImageSubresource &subresource) const
{
	ASSERT(decompressedImage);

	int n = GetBCn(format);
	int noAlphaU = GetNoAlphaOrUnsigned(format);

	int bytes = decompressedImage->format.bytes();

	VkExtent3D mipLevelExtent = getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresource.aspectMask), subresource.mipLevel);

	int pitchB = decompressedImage->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, subresource.mipLevel);

	for(int32_t depth = 0; depth < static_cast<int32_t>(mipLevelExtent.depth); depth++)
	{
		uint8_t *source = static_cast<uint8_t *>(getTexelPointer({ 0, 0, depth }, subresource));
		uint8_t *dest = static_cast<uint8_t *>(decompressedImage->getTexelPointer({ 0, 0, depth }, subresource));

		BC_Decoder::Decode(source, dest, mipLevelExtent.width, mipLevelExtent.height,
		                   pitchB, bytes, n, noAlphaU);
	}
}

void Image::decodeASTC(const VkImageSubresource &subresource) const
{
	ASSERT(decompressedImage);

	int xBlockSize = format.blockWidth();
	int yBlockSize = format.blockHeight();
	int zBlockSize = 1;
	bool isUnsigned = format.isUnsignedComponent(0);

	int bytes = decompressedImage->format.bytes();

	VkExtent3D mipLevelExtent = getMipLevelExtent(static_cast<VkImageAspectFlagBits>(subresource.aspectMask), subresource.mipLevel);

	int xblocks = (mipLevelExtent.width + xBlockSize - 1) / xBlockSize;
	int yblocks = (mipLevelExtent.height + yBlockSize - 1) / yBlockSize;
	int zblocks = (zBlockSize > 1) ? (mipLevelExtent.depth + zBlockSize - 1) / zBlockSize : 1;

	if(xblocks <= 0 || yblocks <= 0 || zblocks <= 0)
	{
		return;
	}

	int pitchB = decompressedImage->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, subresource.mipLevel);
	int sliceB = decompressedImage->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, subresource.mipLevel);

	for(int32_t depth = 0; depth < static_cast<int32_t>(mipLevelExtent.depth); depth++)
	{
		uint8_t *source = static_cast<uint8_t *>(getTexelPointer({ 0, 0, depth }, subresource));
		uint8_t *dest = static_cast<uint8_t *>(decompressedImage->getTexelPointer({ 0, 0, depth }, subresource));

		ASTC_Decoder::Decode(source, dest, mipLevelExtent.width, mipLevelExtent.height, mipLevelExtent.depth, bytes, pitchB, sliceB,
		                     xBlockSize, yBlockSize, zBlockSize, xblocks, yblocks, zblocks, isUnsigned);
	}
}

}  // namespace vk
