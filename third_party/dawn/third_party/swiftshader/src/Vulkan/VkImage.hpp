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

#ifndef VK_IMAGE_HPP_
#define VK_IMAGE_HPP_

#include "VkFormat.hpp"
#include "VkObject.hpp"

#include "marl/mutex.h"

#ifdef __ANDROID__
#	include <vulkan/vk_android_native_buffer.h>  // For VkSwapchainImageUsageFlagsANDROID and buffer_handle_t
#endif

#include <unordered_set>

namespace vk {

class Buffer;
class Device;
class DeviceMemory;
class ImageView;

#ifdef __ANDROID__
struct BackingMemory
{
	bool externalMemory = false;
	VkSwapchainImageUsageFlagsANDROID androidUsage = 0;
	VkNativeBufferANDROID nativeBufferInfo = {};
};
#endif

class Image : public Object<Image, VkImage>
{
public:
	Image(const VkImageCreateInfo *pCreateInfo, void *mem, Device *device);
	void destroy(const VkAllocationCallbacks *pAllocator);

#ifdef __ANDROID__
	VkResult prepareForExternalUseANDROID() const;
#endif

	static size_t ComputeRequiredAllocationSize(const VkImageCreateInfo *pCreateInfo);

	const VkMemoryRequirements getMemoryRequirements() const;
	void getMemoryRequirements(VkMemoryRequirements2 *pMemoryRequirements) const;
	size_t getSizeInBytes(const VkImageSubresourceRange &subresourceRange) const;
	void getSubresourceLayout(const VkImageSubresource *pSubresource, VkSubresourceLayout *pLayout) const;
	void bind(DeviceMemory *pDeviceMemory, VkDeviceSize pMemoryOffset);
	void copyTo(Image *dstImage, const VkImageCopy2KHR &region) const;
	void copyTo(Buffer *dstBuffer, const VkBufferImageCopy2KHR &region);
	void copyFrom(Buffer *srcBuffer, const VkBufferImageCopy2KHR &region);

	// VK_EXT_host_image_copy variants of copy
	void copyToMemory(const VkImageToMemoryCopyEXT &region);
	void copyFromMemory(const VkMemoryToImageCopyEXT &region);

	void blitTo(Image *dstImage, const VkImageBlit2KHR &region, VkFilter filter) const;
	void copyTo(uint8_t *dst, unsigned int dstPitch) const;
	void resolveTo(Image *dstImage, const VkImageResolve2KHR &region) const;
	void resolveDepthStencilTo(const ImageView *src, ImageView *dst, VkResolveModeFlagBits depthResolveMode, VkResolveModeFlagBits stencilResolveMode) const;
	void clear(const VkClearValue &clearValue, const vk::Format &viewFormat, const VkRect2D &renderArea, const VkImageSubresourceRange &subresourceRange);
	void clear(const VkClearColorValue &color, const VkImageSubresourceRange &subresourceRange);
	void clear(const VkClearDepthStencilValue &color, const VkImageSubresourceRange &subresourceRange);

	// Get the last layer and mipmap level, handling VK_REMAINING_ARRAY_LAYERS and
	// VK_REMAINING_MIP_LEVELS, respectively. Note VkImageSubresourceLayers does not
	// allow these symbolic values, so only VkImageSubresourceRange is accepted.
	uint32_t getLastLayerIndex(const VkImageSubresourceRange &subresourceRange) const;
	uint32_t getLastMipLevel(const VkImageSubresourceRange &subresourceRange) const;

	VkImageType getImageType() const { return imageType; }
	const Format &getFormat() const { return format; }
	Format getFormat(VkImageAspectFlagBits aspect) const;
	uint32_t getArrayLayers() const { return arrayLayers; }
	uint32_t getMipLevels() const { return mipLevels; }
	VkImageUsageFlags getUsage() const { return usage; }
	VkImageCreateFlags getFlags() const { return flags; }
	VkSampleCountFlagBits getSampleCount() const { return samples; }
	const VkExtent3D &getExtent() const { return extent; }
	VkExtent3D getMipLevelExtent(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	size_t rowPitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	size_t slicePitchBytes(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	void *getTexelPointer(const VkOffset3D &offset, const VkImageSubresource &subresource) const;
	bool isCubeCompatible() const;
	bool is3DSlice() const;
	uint8_t *end() const;
	VkDeviceSize getLayerSize(VkImageAspectFlagBits aspect) const;
	VkDeviceSize getMipLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	bool canBindToMemory(DeviceMemory *pDeviceMemory) const;

	void prepareForSampling(const VkImageSubresourceRange &subresourceRange) const;
	enum ContentsChangedContext
	{
		DIRECT_MEMORY_ACCESS = 0,
		USING_STORAGE = 1
	};
	void contentsChanged(const VkImageSubresourceRange &subresourceRange, ContentsChangedContext contentsChangedContext = DIRECT_MEMORY_ACCESS);
	const Image *getSampledImage(const vk::Format &imageViewFormat) const;

#ifdef __ANDROID__
	void setBackingMemory(BackingMemory &bm)
	{
		backingMemory = bm;
	}
	bool hasExternalMemory() const { return backingMemory.externalMemory; }
	VkDeviceMemory getExternalMemory() const;
	VkExternalMemoryHandleTypeFlags getSupportedExternalMemoryHandleTypes() const { return supportedExternalMemoryHandleTypes; }
#endif

	DeviceMemory *deviceMemory = nullptr;

private:
	void copy(const void *srcCopyMemory,
		void *dstCopyMemory,
		uint32_t rowLength,
		uint32_t imageHeight,
		const VkImageSubresourceLayers    &imageSubresource,
		const VkOffset3D                  &imageCopyOffset,
		const VkExtent3D                  &imageCopyExtent);
	void copySingleAspectTo(Image *dstImage, const VkImageCopy2KHR &region) const;
	VkDeviceSize getStorageSize(VkImageAspectFlags flags) const;
	VkDeviceSize getMultiSampledLevelSize(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	VkDeviceSize getLayerOffset(VkImageAspectFlagBits aspect, uint32_t mipLevel) const;
	VkDeviceSize getMemoryOffset(VkImageAspectFlagBits aspect) const;
	VkDeviceSize getAspectOffset(VkImageAspectFlagBits aspect) const;
	VkDeviceSize getSubresourceOffset(VkImageAspectFlagBits aspect, uint32_t mipLevel, uint32_t layer) const;
	VkDeviceSize texelOffsetBytesInStorage(const VkOffset3D &offset, const VkImageSubresource &subresource) const;
	VkExtent3D imageExtentInBlocks(const VkExtent3D &extent, VkImageAspectFlagBits aspect) const;
	VkOffset3D imageOffsetInBlocks(const VkOffset3D &offset, VkImageAspectFlagBits aspect) const;
	VkExtent2D bufferExtentInBlocks(const VkExtent2D &extent, uint32_t rowLength, uint32_t imageHeight, const VkImageSubresourceLayers &imageSubresource, const VkOffset3D &imageOffset) const;
	void clear(const void *pixelData, VkFormat pixelFormat, const vk::Format &viewFormat, const VkImageSubresourceRange &subresourceRange, const VkRect2D *renderArea);
	int borderSize() const;

	bool requiresPreprocessing() const;
	void decompress(const VkImageSubresource &subresource) const;
	void decodeETC2(const VkImageSubresource &subresource) const;
	void decodeBC(const VkImageSubresource &subresource) const;
	void decodeASTC(const VkImageSubresource &subresource) const;

	const Device *const device = nullptr;
	VkDeviceSize memoryOffset = 0;
	VkImageCreateFlags flags = 0;
	VkImageType imageType = VK_IMAGE_TYPE_2D;
	Format format;
	VkExtent3D extent = { 0, 0, 0 };
	uint32_t mipLevels = 0;
	uint32_t arrayLayers = 0;
	VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkImageUsageFlags usage = (VkImageUsageFlags)0;
	Image *decompressedImage = nullptr;
#ifdef __ANDROID__
	BackingMemory backingMemory = {};
#endif

	VkExternalMemoryHandleTypeFlags supportedExternalMemoryHandleTypes = (VkExternalMemoryHandleTypeFlags)0;

	// VkImageSubresource wrapper for use in unordered_set
	class Subresource
	{
	public:
		Subresource()
		    : subresource{ (VkImageAspectFlags)0, 0, 0 }
		{}
		Subresource(const VkImageSubresource &subres)
		    : subresource(subres)
		{}
		inline operator VkImageSubresource() const { return subresource; }

		bool operator==(const Subresource &other) const
		{
			return (subresource.aspectMask == other.subresource.aspectMask) &&
			       (subresource.mipLevel == other.subresource.mipLevel) &&
			       (subresource.arrayLayer == other.subresource.arrayLayer);
		};

		size_t operator()(const Subresource &other) const
		{
			return static_cast<size_t>(other.subresource.aspectMask) ^
			       static_cast<size_t>(other.subresource.mipLevel) ^
			       static_cast<size_t>(other.subresource.arrayLayer);
		};

	private:
		VkImageSubresource subresource;
	};

	mutable marl::mutex mutex;
	mutable std::unordered_set<Subresource, Subresource> dirtySubresources GUARDED_BY(mutex);
};

static inline Image *Cast(VkImage object)
{
	return Image::Cast(object);
}

}  // namespace vk

inline bool operator==(const VkExtent3D &lhs, const VkExtent3D &rhs)
{
	return lhs.width == rhs.width &&
	       lhs.height == rhs.height &&
	       lhs.depth == rhs.depth;
}

inline bool operator!=(const VkExtent3D &lhs, const VkExtent3D &rhs)
{
	return !(lhs == rhs);
}

inline bool operator==(const VkOffset3D &lhs, const VkOffset3D &rhs)
{
	return lhs.x == rhs.x &&
	       lhs.y == rhs.y &&
	       lhs.z == rhs.z;
}

inline bool operator!=(const VkOffset3D &lhs, const VkOffset3D &rhs)
{
	return !(lhs == rhs);
}

#endif  // VK_IMAGE_HPP_
