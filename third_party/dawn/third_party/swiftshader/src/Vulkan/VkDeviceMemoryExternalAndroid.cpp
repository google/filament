// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "VkDeviceMemoryExternalAndroid.hpp"

#include "VkDestroy.hpp"
#include "VkFormat.hpp"
#include "VkObject.hpp"
#include "VkPhysicalDevice.hpp"
#include "VkStringify.hpp"
#include "System/Debug.hpp"

namespace {

uint32_t GetAHBFormatFromVkFormat(VkFormat format)
{
	switch(format)
	{
	case VK_FORMAT_D16_UNORM:
		return AHARDWAREBUFFER_FORMAT_D16_UNORM;
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat VK_FORMAT_X8_D24_UNORM_PACK32");
		return AHARDWAREBUFFER_FORMAT_D24_UNORM;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat VK_FORMAT_D24_UNORM_S8_UINT");
		return AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT;
	case VK_FORMAT_D32_SFLOAT:
		return AHARDWAREBUFFER_FORMAT_D32_FLOAT;
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		return AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		return AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		return AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
	case VK_FORMAT_R8_UNORM:
		return AHARDWAREBUFFER_FORMAT_R8_UNORM;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
	case VK_FORMAT_R8G8B8_UNORM:
		return AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
	case VK_FORMAT_S8_UINT:
		return AHARDWAREBUFFER_FORMAT_S8_UINT;
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		return AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		return AHARDWAREBUFFER_FORMAT_YCbCr_P010;
	default:
		UNSUPPORTED("AHardwareBufferExternalMemory::VkFormat %d", int(format));
		return 0;
	}
}

uint64_t GetAHBLockUsageFromVkImageUsageFlags(VkImageUsageFlags flags)
{
	uint64_t usage = 0;

	if(flags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ||
	   flags & VK_IMAGE_USAGE_SAMPLED_BIT ||
	   flags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		usage |= AHARDWAREBUFFER_USAGE_CPU_READ_MASK;
	}

	if(flags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ||
	   flags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		usage |= AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK;
	}

	return usage;
}

uint64_t GetAHBLockUsageFromVkBufferUsageFlags(VkBufferUsageFlags flags)
{
	uint64_t usage = 0;

	if(flags & VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
	{
		usage |= AHARDWAREBUFFER_USAGE_CPU_READ_MASK;
	}

	if(flags & VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	{
		usage |= AHARDWAREBUFFER_USAGE_CPU_WRITE_MASK;
	}

	return usage;
}

uint64_t GetAHBUsageFromVkImageFlags(VkImageCreateFlags createFlags, VkImageUsageFlags usageFlags)
{
	uint64_t ahbUsage = 0;

	if(usageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN;
	}
	if(usageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN;
	}
	if(usageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_COLOR_OUTPUT | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
	}

	if(createFlags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_GPU_CUBE_MAP;
	}
	if(createFlags & VK_IMAGE_CREATE_PROTECTED_BIT)
	{
		ahbUsage |= AHARDWAREBUFFER_USAGE_PROTECTED_CONTENT;
	}

	// No usage bits set - set at least one GPU usage
	if(ahbUsage == 0)
	{
		ahbUsage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN |
		           AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
	}

	return ahbUsage;
}

uint64_t GetAHBUsageFromVkBufferFlags(VkBufferCreateFlags /*createFlags*/, VkBufferUsageFlags /*usageFlags*/)
{
	uint64_t ahbUsage = 0;

	// TODO(b/141698760): needs fleshing out.
	ahbUsage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;

	return ahbUsage;
}

VkFormatFeatureFlags GetVkFormatFeaturesFromAHBFormat(uint32_t ahbFormat)
{
	VkFormatFeatureFlags features = 0;

	VkFormat format = AHardwareBufferExternalMemory::GetVkFormatFromAHBFormat(ahbFormat);
	VkFormatProperties formatProperties;
	vk::PhysicalDevice::GetFormatProperties(vk::Format(format), &formatProperties);

	formatProperties.optimalTilingFeatures |= VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT;

	// TODO: b/167896057
	//   The correct formatFeatureFlags depends on consumer and format
	//   So this solution is incomplete without more information
	features |= formatProperties.linearTilingFeatures |
	            formatProperties.optimalTilingFeatures |
	            formatProperties.bufferFeatures;

	return features;
}

}  // namespace

AHardwareBufferExternalMemory::AllocateInfo::AllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
{
	if(extendedAllocationInfo.importAndroidHardwareBufferInfo)
	{
		importAhb = true;
		ahb = extendedAllocationInfo.importAndroidHardwareBufferInfo->buffer;
	}

	if(extendedAllocationInfo.exportMemoryAllocateInfo)
	{
		if(extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes == VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
		{
			exportAhb = true;
		}
		else
		{
			UNSUPPORTED("VkExportMemoryAllocateInfo::handleTypes %d", int(extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes));
		}
	}

	if(extendedAllocationInfo.dedicatedAllocateInfo)
	{
		dedicatedImageHandle = vk::Cast(extendedAllocationInfo.dedicatedAllocateInfo->image);
		dedicatedBufferHandle = vk::Cast(extendedAllocationInfo.dedicatedAllocateInfo->buffer);
	}
}

AHardwareBufferExternalMemory::AHardwareBufferExternalMemory(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, vk::Device *pDevice)
    : vk::DeviceMemory(pCreateInfo, extendedAllocationInfo, pDevice)
    , allocateInfo(extendedAllocationInfo)
{
}

AHardwareBufferExternalMemory::~AHardwareBufferExternalMemory()
{
	freeBuffer();
}

// vkAllocateMemory
VkResult AHardwareBufferExternalMemory::allocateBuffer()
{
	if(allocateInfo.importAhb)
	{
		return importAndroidHardwareBuffer(allocateInfo.ahb, &buffer);
	}
	else
	{
		ASSERT(allocateInfo.exportAhb);
		return allocateAndroidHardwareBuffer(allocationSize, &buffer);
	}
}

void AHardwareBufferExternalMemory::freeBuffer()
{
	if(ahb != nullptr)
	{
		unlockAndroidHardwareBuffer();

		AHardwareBuffer_release(ahb);
		ahb = nullptr;
	}
}

VkResult AHardwareBufferExternalMemory::importAndroidHardwareBuffer(AHardwareBuffer *buffer, void **pBuffer)
{
	ahb = buffer;

	AHardwareBuffer_acquire(ahb);
	AHardwareBuffer_describe(ahb, &ahbDesc);

	return lockAndroidHardwareBuffer(pBuffer);
}

VkResult AHardwareBufferExternalMemory::allocateAndroidHardwareBuffer(size_t size, void **pBuffer)
{
	if(allocateInfo.dedicatedImageHandle)
	{
		vk::Image *image = allocateInfo.dedicatedImageHandle;
		ASSERT(image->getArrayLayers() == 1);

		VkExtent3D extent = image->getExtent();

		ahbDesc.width = extent.width;
		ahbDesc.height = extent.height;
		ahbDesc.layers = image->getArrayLayers();
		ahbDesc.format = GetAHBFormatFromVkFormat(image->getFormat());
		ahbDesc.usage = GetAHBUsageFromVkImageFlags(image->getFlags(), image->getUsage());
	}
	else if(allocateInfo.dedicatedBufferHandle)
	{
		vk::Buffer *buffer = allocateInfo.dedicatedBufferHandle;

		ahbDesc.width = static_cast<uint32_t>(buffer->getSize());
		ahbDesc.height = 1;
		ahbDesc.layers = 1;
		ahbDesc.format = AHARDWAREBUFFER_FORMAT_BLOB;
		ahbDesc.usage = GetAHBUsageFromVkBufferFlags(buffer->getFlags(), buffer->getUsage());
	}
	else
	{
		// Android Hardware Buffer Buffer Resources: "Android hardware buffers with a format of
		// AHARDWAREBUFFER_FORMAT_BLOB and usage that includes AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER can
		// be used as the backing store for VkBuffer objects. Such Android hardware buffers have a size
		// in bytes specified by their width; height and layers are both 1."
		ahbDesc.width = static_cast<uint32_t>(size);
		ahbDesc.height = 1;
		ahbDesc.layers = 1;
		ahbDesc.format = AHARDWAREBUFFER_FORMAT_BLOB;
		ahbDesc.usage = AHARDWAREBUFFER_USAGE_GPU_DATA_BUFFER | AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
	}

	int ret = AHardwareBuffer_allocate(&ahbDesc, &ahb);
	if(ret != 0)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	AHardwareBuffer_describe(ahb, &ahbDesc);

	return lockAndroidHardwareBuffer(pBuffer);
}

VkResult AHardwareBufferExternalMemory::lockAndroidHardwareBuffer(void **pBuffer)
{
	uint64_t usage = 0;
	if(allocateInfo.dedicatedImageHandle)
	{
		usage = GetAHBLockUsageFromVkImageUsageFlags(allocateInfo.dedicatedImageHandle->getUsage());
	}
	else if(allocateInfo.dedicatedBufferHandle)
	{
		usage = GetAHBLockUsageFromVkBufferUsageFlags(allocateInfo.dedicatedBufferHandle->getUsage());
	}
	else
	{
		usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
	}

	// Empty fence, lock immedietly.
	int32_t fence = -1;

	// Empty rect, lock entire buffer.
	ARect *rect = nullptr;

	int ret = AHardwareBuffer_lockPlanes(ahb, usage, fence, rect, &ahbPlanes);
	if(ret != 0)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	*pBuffer = ahbPlanes.planes[0].data;

	return VK_SUCCESS;
}

VkResult AHardwareBufferExternalMemory::unlockAndroidHardwareBuffer()
{
	int ret = AHardwareBuffer_unlock(ahb, /*fence=*/nullptr);
	if(ret != 0)
	{
		return VK_ERROR_UNKNOWN;
	}

	return VK_SUCCESS;
}

VkResult AHardwareBufferExternalMemory::exportAndroidHardwareBuffer(AHardwareBuffer **pAhb) const
{
	if(getFlagBit() != VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	// Each call to vkGetMemoryAndroidHardwareBufferANDROID *must* return an Android hardware buffer with a new reference
	// acquired in addition to the reference held by the VkDeviceMemory. To avoid leaking resources, the application *must*
	// release the reference by calling AHardwareBuffer_release when it is no longer needed.
	AHardwareBuffer_acquire(ahb);
	*pAhb = ahb;
	return VK_SUCCESS;
}

VkFormat AHardwareBufferExternalMemory::GetVkFormatFromAHBFormat(uint32_t ahbFormat)
{
	switch(ahbFormat)
	{
	case AHARDWAREBUFFER_FORMAT_BLOB:
		return VK_FORMAT_UNDEFINED;
	case AHARDWAREBUFFER_FORMAT_D16_UNORM:
		return VK_FORMAT_D16_UNORM;
	case AHARDWAREBUFFER_FORMAT_D24_UNORM:
		UNSUPPORTED("AHardwareBufferExternalMemory::AndroidHardwareBuffer_Format AHARDWAREBUFFER_FORMAT_D24_UNORM");
		return VK_FORMAT_X8_D24_UNORM_PACK32;
	case AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT:
		UNSUPPORTED("AHardwareBufferExternalMemory::AndroidHardwareBuffer_Format AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT");
		return VK_FORMAT_X8_D24_UNORM_PACK32;
	case AHARDWAREBUFFER_FORMAT_D32_FLOAT:
		return VK_FORMAT_D32_SFLOAT;
	case AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT:
		return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
		return VK_FORMAT_R5G6B5_UNORM_PACK16;
	case AHARDWAREBUFFER_FORMAT_R8_UNORM:
		return VK_FORMAT_R8_UNORM;
	case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
		return VK_FORMAT_R8G8B8_UNORM;
	case AHARDWAREBUFFER_FORMAT_S8_UINT:
		return VK_FORMAT_S8_UINT;
	case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
	case AHARDWAREBUFFER_FORMAT_YV12:
		return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
	case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
		return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
	default:
		UNSUPPORTED("AHardwareBufferExternalMemory::AHardwareBuffer_Format %d", int(ahbFormat));
		return VK_FORMAT_UNDEFINED;
	}
}

VkResult AHardwareBufferExternalMemory::GetAndroidHardwareBufferFormatProperties(const AHardwareBuffer_Desc &ahbDesc, VkAndroidHardwareBufferFormatPropertiesANDROID *pFormat)
{
	pFormat->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
	pFormat->pNext = nullptr;
	pFormat->format = GetVkFormatFromAHBFormat(ahbDesc.format);
	pFormat->externalFormat = ahbDesc.format;
	pFormat->formatFeatures = GetVkFormatFeaturesFromAHBFormat(ahbDesc.format);
	pFormat->samplerYcbcrConversionComponents = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	pFormat->suggestedYcbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
	pFormat->suggestedYcbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
	pFormat->suggestedXChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
	pFormat->suggestedYChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;

	// YUV formats are not listed in the AHardwareBuffer Format Equivalence table in the Vulkan spec.
	// Clients must use VkExternalFormatANDROID.
	if(pFormat->format == VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM ||
	   pFormat->format == VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16)
	{
		pFormat->format = VK_FORMAT_UNDEFINED;
	}

	return VK_SUCCESS;
}

VkResult AHardwareBufferExternalMemory::GetAndroidHardwareBufferProperties(VkDevice &device, const AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
	VkResult result = VK_SUCCESS;

	AHardwareBuffer_Desc ahbDesc;
	AHardwareBuffer_describe(buffer, &ahbDesc);

	if(pProperties->pNext != nullptr)
	{
		result = GetAndroidHardwareBufferFormatProperties(ahbDesc, (VkAndroidHardwareBufferFormatPropertiesANDROID *)pProperties->pNext);
		if(result != VK_SUCCESS)
		{
			return result;
		}
	}

	const VkPhysicalDeviceMemoryProperties phyDeviceMemProps = vk::PhysicalDevice::GetMemoryProperties();
	pProperties->memoryTypeBits = phyDeviceMemProps.memoryTypes[0].propertyFlags;

	if(ahbDesc.format == AHARDWAREBUFFER_FORMAT_BLOB)
	{
		pProperties->allocationSize = ahbDesc.width;
	}
	else
	{
		VkImageCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.format = GetVkFormatFromAHBFormat(ahbDesc.format);
		info.extent.width = ahbDesc.width;
		info.extent.height = ahbDesc.height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.samples = VK_SAMPLE_COUNT_1_BIT;
		info.tiling = VK_IMAGE_TILING_OPTIMAL;
		info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		VkImage Image;

		result = vk::Image::Create(vk::NULL_ALLOCATION_CALLBACKS, &info, &Image, vk::Cast(device));
		if(result != VK_SUCCESS)
		{
			return result;
		}

		pProperties->allocationSize = vk::Cast(Image)->getMemoryRequirements().size;
		vk::destroy(Image, vk::NULL_ALLOCATION_CALLBACKS);
	}

	return result;
}

int AHardwareBufferExternalMemory::externalImageRowPitchBytes(VkImageAspectFlagBits aspect) const
{
	ASSERT(allocateInfo.dedicatedImageHandle != nullptr);

	switch(ahbDesc.format)
	{
	case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
	case AHARDWAREBUFFER_FORMAT_YV12:
	case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
		switch(aspect)
		{
		case VK_IMAGE_ASPECT_PLANE_0_BIT:
			return static_cast<int>(ahbPlanes.planes[0].rowStride);
		case VK_IMAGE_ASPECT_PLANE_1_BIT:
			return static_cast<int>(ahbPlanes.planes[1].rowStride);
		case VK_IMAGE_ASPECT_PLANE_2_BIT:
			return static_cast<int>(ahbPlanes.planes[2].rowStride);
		default:
			UNSUPPORTED("Unsupported aspect %d for AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420", int(aspect));
			return 0;
		}
		break;
	default:
		break;
	}
	return static_cast<int>(ahbPlanes.planes[0].rowStride);
}

// TODO(b/208505033): Treat each image plane data pointer as a separate address instead of an offset.
VkDeviceSize AHardwareBufferExternalMemory::externalImageMemoryOffset(VkImageAspectFlagBits aspect) const
{
	ASSERT(allocateInfo.dedicatedImageHandle != nullptr);

	switch(ahbDesc.format)
	{
	case AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420:
	case AHARDWAREBUFFER_FORMAT_YV12:
	case AHARDWAREBUFFER_FORMAT_YCbCr_P010:
		switch(aspect)
		{
		case VK_IMAGE_ASPECT_PLANE_0_BIT:
			return 0;
		case VK_IMAGE_ASPECT_PLANE_1_BIT:
			return reinterpret_cast<const char *>(ahbPlanes.planes[1].data) -
			       reinterpret_cast<const char *>(ahbPlanes.planes[0].data);
		case VK_IMAGE_ASPECT_PLANE_2_BIT:
			return reinterpret_cast<const char *>(ahbPlanes.planes[2].data) -
			       reinterpret_cast<const char *>(ahbPlanes.planes[0].data);
		default:
			UNSUPPORTED("Unsupported aspect %d for AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420", int(aspect));
			return 0;
		}
		break;
	default:
		break;
	}
	return 0;
}

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
uint64_t AHardwareBufferExternalMemory::getMemoryObjectId() const
{
	uint64_t id = 0;
	int ret = AHardwareBuffer_getId(ahb, &id);
	ASSERT(ret == 0);
	return id;
}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT
