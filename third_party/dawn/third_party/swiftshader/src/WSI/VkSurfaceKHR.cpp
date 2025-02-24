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

#include "VkSurfaceKHR.hpp"

#include "Vulkan/VkDestroy.hpp"
#include "Vulkan/VkStringify.hpp"

#include <algorithm>

namespace {

static const VkSurfaceFormatKHR surfaceFormats[] = {
	{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
	{ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
};

static const VkPresentModeKHR presentModes[] = {
	// FIXME(b/124265819): Make present modes behave correctly. Currently we use XPutImage
	// with no synchronization, which behaves more like VK_PRESENT_MODE_IMMEDIATE_KHR. We
	// should convert to using the Present extension, which allows us to request presentation
	// at particular msc values. Will need a similar solution on Windows - possibly interact
	// with DXGI directly.
	VK_PRESENT_MODE_FIFO_KHR,
	VK_PRESENT_MODE_MAILBOX_KHR,
};

}  // namespace

namespace vk {

VkResult PresentImage::createImage(VkDevice device, const VkImageCreateInfo &createInfo)
{
	VkImage image;
	VkResult status = vkCreateImage(device, &createInfo, nullptr, &image);
	if(status != VK_SUCCESS)
	{
		return status;
	}

	this->image = Cast(image);

	return status;
}

VkResult PresentImage::allocateAndBindImageMemory(VkDevice device, const VkMemoryAllocateInfo &allocateInfo)
{
	ASSERT(image);

	VkDeviceMemory deviceMemory;
	VkResult status = vkAllocateMemory(device, &allocateInfo, nullptr, &deviceMemory);
	if(status != VK_SUCCESS)
	{
		release();
		return status;
	}

	imageMemory = Cast(deviceMemory);
	vkBindImageMemory(device, *image, deviceMemory, 0);
	imageStatus = AVAILABLE;

	return VK_SUCCESS;
}

void PresentImage::release()
{
	if(imageMemory)
	{
		vk::destroy(static_cast<VkDeviceMemory>(*imageMemory), nullptr);
		imageMemory = nullptr;
	}

	if(image)
	{
		vk::destroy(static_cast<VkImage>(*image), nullptr);
		image = nullptr;
	}

	imageStatus = NONEXISTENT;
}

VkImage PresentImage::asVkImage() const
{
	return image ? static_cast<VkImage>(*image) : VkImage({ VK_NULL_HANDLE });
}

uint32_t SurfaceKHR::GetSurfaceFormatsCount(const void *pSurfaceInfoPNext)
{
	return static_cast<uint32_t>(sizeof(surfaceFormats) / sizeof(surfaceFormats[0]));
}

VkResult SurfaceKHR::GetSurfaceFormats(const void *pSurfaceInfoPNext, uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats)
{
	uint32_t count = GetSurfaceFormatsCount(pSurfaceInfoPNext);

	uint32_t i;
	for(i = 0; i < std::min(*pSurfaceFormatCount, count); i++)
	{
		pSurfaceFormats[i].surfaceFormat = surfaceFormats[i];
	}

	*pSurfaceFormatCount = i;

	if(*pSurfaceFormatCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

uint32_t SurfaceKHR::GetPresentModeCount()
{
	return static_cast<uint32_t>(sizeof(presentModes) / sizeof(presentModes[0]));
}

VkResult SurfaceKHR::GetPresentModes(uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes)
{
	uint32_t count = GetPresentModeCount();

	uint32_t i;
	for(i = 0; i < std::min(*pPresentModeCount, count); i++)
	{
		pPresentModes[i] = presentModes[i];
	}

	*pPresentModeCount = i;

	if(*pPresentModeCount < count)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

void SurfaceKHR::associateSwapchain(SwapchainKHR *swapchain)
{
	associatedSwapchain = swapchain;
}

void SurfaceKHR::disassociateSwapchain()
{
	associatedSwapchain = nullptr;
}

bool SurfaceKHR::hasAssociatedSwapchain()
{
	return (associatedSwapchain != nullptr);
}

VkResult SurfaceKHR::getPresentRectangles(uint32_t *pRectCount, VkRect2D *pRects) const
{
	if(!pRects)
	{
		*pRectCount = 1;
		return VK_SUCCESS;
	}

	if(*pRectCount < 1)
	{
		return VK_INCOMPLETE;
	}

	VkSurfaceCapabilitiesKHR capabilities;
	getSurfaceCapabilities(nullptr, &capabilities, nullptr);

	pRects[0].offset = { 0, 0 };
	pRects[0].extent = capabilities.currentExtent;
	*pRectCount = 1;

	return VK_SUCCESS;
}

void SurfaceKHR::GetSurfacelessCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext)
{
	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);

	// When the surface is VK_NULL_HANDLE (with VK_GOOGLE_surfaceless_query), the following
	// cannot be calculated and must have assigned values as below.
	pSurfaceCapabilities->minImageCount = 0xFFFFFFFF;
	pSurfaceCapabilities->maxImageCount = 0xFFFFFFFF;
	pSurfaceCapabilities->currentExtent = { 0xFFFFFFFF, 0xFFFFFFFF };
	pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR;

	// The following values depend on the surface as well, initialize them with something
	// reasonable despite VK_GOOGLE_surfaceless_query not mentioning that they are not correct
	// when surfaceless.  This was missed when developing VK_GOOGLE_surfaceless_query because
	// Android always sets the following min/max extents regardless of the surface.
	pSurfaceCapabilities->minImageExtent = { 1, 1 };
	pSurfaceCapabilities->maxImageExtent = { 4096, 4096 };
}

void SurfaceKHR::SetCommonSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext)
{
	pSurfaceCapabilities->minImageCount = 1;
	pSurfaceCapabilities->maxImageCount = 0;

	pSurfaceCapabilities->maxImageArrayLayers = 1;

	pSurfaceCapabilities->supportedTransforms = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	pSurfaceCapabilities->currentTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	pSurfaceCapabilities->supportedCompositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	pSurfaceCapabilities->supportedUsageFlags =
	    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
	    VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
	    VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
	    VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	    VK_IMAGE_USAGE_SAMPLED_BIT |
	    VK_IMAGE_USAGE_STORAGE_BIT;

	auto *extInfo = reinterpret_cast<VkBaseOutStructure *>(pSurfaceCapabilitiesPNext);
	while(extInfo)
	{
		switch(extInfo->sType)
		{
		case VK_STRUCTURE_TYPE_SURFACE_PRESENT_SCALING_CAPABILITIES_EXT:
			{
				// Supported scaling is per present mode, but currently that's identical for all present modes.
				ASSERT(vk::GetExtendedStruct<VkSurfacePresentModeEXT>(pSurfaceInfoPNext, VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT) != nullptr);
				VkSurfacePresentScalingCapabilitiesEXT *presentScalingCapabilities = reinterpret_cast<VkSurfacePresentScalingCapabilitiesEXT *>(extInfo);
				presentScalingCapabilities->supportedPresentScaling = 0;
				presentScalingCapabilities->supportedPresentGravityX = 0;
				presentScalingCapabilities->supportedPresentGravityY = 0;
				presentScalingCapabilities->minScaledImageExtent = pSurfaceCapabilities->minImageExtent;
				presentScalingCapabilities->maxScaledImageExtent = pSurfaceCapabilities->maxImageExtent;
				break;
			}
		case VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_COMPATIBILITY_EXT:
			{
				VkSurfacePresentModeCompatibilityEXT *presentModeCompatibility = reinterpret_cast<VkSurfacePresentModeCompatibilityEXT *>(extInfo);
				const auto *presentMode = vk::GetExtendedStruct<VkSurfacePresentModeEXT>(pSurfaceInfoPNext, VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT);
				ASSERT(presentMode != nullptr);

				// Present mode is ignored, so FIFO and MAILBOX are compatible.
				if(presentModeCompatibility->pPresentModes == nullptr)
				{
					presentModeCompatibility->presentModeCount = 2;
				}
				else if(presentModeCompatibility->presentModeCount == 1)
				{
					presentModeCompatibility->pPresentModes[0] = presentMode->presentMode;
					presentModeCompatibility->presentModeCount = 1;
				}
				else if(presentModeCompatibility->presentModeCount > 1)
				{
					presentModeCompatibility->pPresentModes[0] = presentModes[0];
					presentModeCompatibility->pPresentModes[1] = presentModes[1];
					presentModeCompatibility->presentModeCount = 2;
				}
				break;
			}
		default:
			UNSUPPORTED("pSurfaceCapabilities->pNext sType = %s", vk::Stringify(extInfo->sType).c_str());
			break;
		}
		extInfo = extInfo->pNext;
	}
}

}  // namespace vk
