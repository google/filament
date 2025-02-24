// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "VkSwapchainKHR.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkFence.hpp"
#include "Vulkan/VkImage.hpp"
#include "Vulkan/VkSemaphore.hpp"

#include <algorithm>
#include <cstring>

namespace vk {

SwapchainKHR::SwapchainKHR(const VkSwapchainCreateInfoKHR *pCreateInfo, void *mem)
    : surface(vk::Cast(pCreateInfo->surface))
    , images(reinterpret_cast<PresentImage *>(mem))
    , imageCount(pCreateInfo->minImageCount)
    , retired(false)
{
	memset(reinterpret_cast<void *>(images), 0, imageCount * sizeof(PresentImage));
}

void SwapchainKHR::destroy(const VkAllocationCallbacks *pAllocator)
{
	for(uint32_t i = 0; i < imageCount; i++)
	{
		PresentImage &currentImage = images[i];
		if(currentImage.exists())
		{
			surface->detachImage(&currentImage);
			currentImage.release();
			surface->releaseImageMemory(&currentImage);
		}
	}

	if(!retired)
	{
		surface->disassociateSwapchain();
	}

	vk::freeHostMemory(images, pAllocator);
}

size_t SwapchainKHR::ComputeRequiredAllocationSize(const VkSwapchainCreateInfoKHR *pCreateInfo)
{
	return pCreateInfo->minImageCount * sizeof(PresentImage);
}

void SwapchainKHR::retire()
{
	if(!retired)
	{
		retired = true;
		surface->disassociateSwapchain();

		for(uint32_t i = 0; i < imageCount; i++)
		{
			PresentImage &currentImage = images[i];
			if(currentImage.isAvailable())
			{
				surface->detachImage(&currentImage);
				currentImage.release();
			}
		}
	}
}

void SwapchainKHR::resetImages()
{
	for(uint32_t i = 0; i < imageCount; i++)
	{
		images[i].release();
	}
}

VkResult SwapchainKHR::createImages(VkDevice device, const VkSwapchainCreateInfoKHR *pCreateInfo)
{
	resetImages();

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

	if(pCreateInfo->flags & VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT;
	}

	if(pCreateInfo->flags & VK_SWAPCHAIN_CREATE_PROTECTED_BIT_KHR)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_PROTECTED_BIT;
	}

	if(pCreateInfo->flags & VK_SWAPCHAIN_CREATE_MUTABLE_FORMAT_BIT_KHR)
	{
		imageInfo.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
	}

	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = pCreateInfo->imageFormat;
	imageInfo.extent.height = pCreateInfo->imageExtent.height;
	imageInfo.extent.width = pCreateInfo->imageExtent.width;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = pCreateInfo->imageArrayLayers;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = pCreateInfo->imageUsage;
	imageInfo.sharingMode = pCreateInfo->imageSharingMode;
	imageInfo.pQueueFamilyIndices = pCreateInfo->pQueueFamilyIndices;
	imageInfo.queueFamilyIndexCount = pCreateInfo->queueFamilyIndexCount;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_GENERAL;

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = 0;
	allocInfo.memoryTypeIndex = 0;

	VkResult status;
	for(uint32_t i = 0; i < imageCount; i++)
	{
		PresentImage &currentImage = images[i];

		status = currentImage.createImage(device, imageInfo);
		if(status != VK_SUCCESS)
		{
			return status;
		}

		allocInfo.allocationSize = currentImage.getImage()->getMemoryRequirements().size;
		void* memory = vk::Cast(pCreateInfo->surface)->allocateImageMemory(&currentImage, allocInfo);

		VkImportMemoryHostPointerInfoEXT importMemoryHostPointerInfo = {};
		if (memory)
		{
			importMemoryHostPointerInfo.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT;
			importMemoryHostPointerInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT;
			importMemoryHostPointerInfo.pHostPointer = memory;
			allocInfo.pNext = &importMemoryHostPointerInfo;
		}

		status = currentImage.allocateAndBindImageMemory(device, allocInfo);
		if(status != VK_SUCCESS)
		{
			vk::Cast(pCreateInfo->surface)->releaseImageMemory(&currentImage);
			return status;
		}

		surface->attachImage(&currentImage);
	}

	return VK_SUCCESS;
}

uint32_t SwapchainKHR::getImageCount() const
{
	return imageCount;
}

VkResult SwapchainKHR::getImages(uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages) const
{
	uint32_t i;
	for(i = 0; i < std::min(*pSwapchainImageCount, imageCount); i++)
	{
		pSwapchainImages[i] = images[i].asVkImage();
	}

	*pSwapchainImageCount = i;

	if(*pSwapchainImageCount < imageCount)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

VkResult SwapchainKHR::getNextImage(uint64_t timeout, BinarySemaphore *semaphore, Fence *fence, uint32_t *pImageIndex)
{
	for(uint32_t i = 0; i < imageCount; i++)
	{
		PresentImage &currentImage = images[i];
		if(currentImage.isAvailable())
		{
			currentImage.setStatus(DRAWING);
			*pImageIndex = i;

			if(semaphore)
			{
				semaphore->signal();
			}

			if(fence)
			{
				fence->complete();
			}

			return VK_SUCCESS;
		}
	}

	return (timeout > 0) ? VK_TIMEOUT : VK_NOT_READY;
}

VkResult SwapchainKHR::present(uint32_t index)
{
	auto &image = images[index];
	image.setStatus(PRESENTING);
	VkResult result = surface->present(&image);

	releaseImage(index);
	return result;
}

VkResult SwapchainKHR::releaseImages(uint32_t imageIndexCount, const uint32_t *pImageIndices)
{
	for(uint32_t i = 0; i < imageIndexCount; ++i)
	{
		releaseImage(pImageIndices[i]);
	}
	return VK_SUCCESS;
}

void SwapchainKHR::releaseImage(uint32_t index)
{
	auto &image = images[index];
	image.setStatus(AVAILABLE);

	if(retired)
	{
		surface->detachImage(&image);
		image.release();
		surface->releaseImageMemory(&image);
	}
}
}  // namespace vk
