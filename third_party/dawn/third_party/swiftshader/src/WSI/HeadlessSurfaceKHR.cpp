// Copyright 2021 The SwiftShader Authors. All Rights Reserved.
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

#include "HeadlessSurfaceKHR.hpp"

namespace vk {

HeadlessSurfaceKHR::HeadlessSurfaceKHR(const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo, void *mem)
{
}

size_t HeadlessSurfaceKHR::ComputeRequiredAllocationSize(const VkHeadlessSurfaceCreateInfoEXT *pCreateInfo)
{
	return 0;
}

void HeadlessSurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
}

VkResult HeadlessSurfaceKHR::getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const
{
	pSurfaceCapabilities->currentExtent = { 1280, 720 };
	pSurfaceCapabilities->minImageExtent = { 0, 0 };
	pSurfaceCapabilities->maxImageExtent = { 3840, 2160 };

	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);
	return VK_SUCCESS;
}

void HeadlessSurfaceKHR::attachImage(PresentImage *image)
{
}

void HeadlessSurfaceKHR::detachImage(PresentImage *image)
{
}

VkResult HeadlessSurfaceKHR::present(PresentImage *image)
{
	return VK_SUCCESS;
}

}  // namespace vk
