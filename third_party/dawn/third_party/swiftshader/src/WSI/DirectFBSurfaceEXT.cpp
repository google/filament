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

#include "DirectFBSurfaceEXT.hpp"

#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

namespace vk {

DirectFBSurfaceEXT::DirectFBSurfaceEXT(const VkDirectFBSurfaceCreateInfoEXT *pCreateInfo, void *mem)
    : dfb(pCreateInfo->dfb)
    , surface(pCreateInfo->surface)
{
}

void DirectFBSurfaceEXT::destroySurface(const VkAllocationCallbacks *pAllocator)
{
}

size_t DirectFBSurfaceEXT::ComputeRequiredAllocationSize(const VkDirectFBSurfaceCreateInfoEXT *pCreateInfo)
{
	return 0;
}

VkResult DirectFBSurfaceEXT::getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const
{
	int width, height;
	surface->GetSize(surface, &width, &height);
	VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;

	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);
	return VK_SUCCESS;
}

void DirectFBSurfaceEXT::attachImage(PresentImage *image)
{
	DFBSurfaceDescription desc;
	desc.flags = static_cast<DFBSurfaceDescriptionFlags>(DSDESC_PIXELFORMAT | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PREALLOCATED);
	desc.pixelformat = DSPF_RGB32;
	const VkExtent3D &extent = image->getImage()->getExtent();
	desc.width = extent.width;
	desc.height = extent.height;
	desc.preallocated[0].data = image->getImageMemory()->getOffsetPointer(0);
	desc.preallocated[0].pitch = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	IDirectFBSurface *dfbImage;
	dfb->CreateSurface(dfb, &desc, &dfbImage);
	imageMap[image] = dfbImage;
}

void DirectFBSurfaceEXT::detachImage(PresentImage *image)
{
	auto it = imageMap.find(image);
	if(it != imageMap.end())
	{
		IDirectFBSurface *dfbImage = it->second;
		dfbImage->Release(dfbImage);
		imageMap.erase(it);
	}
}

VkResult DirectFBSurfaceEXT::present(PresentImage *image)
{
	auto it = imageMap.find(image);
	if(it != imageMap.end())
	{
		IDirectFBSurface *dfbImage = it->second;
		surface->Blit(surface, dfbImage, NULL, 0, 0);
		surface->Flip(surface, NULL, DSFLIP_WAITFORSYNC);
	}

	return VK_SUCCESS;
}

}  // namespace vk
