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

#include "Win32SurfaceKHR.hpp"

#include "System/Debug.hpp"
#include "Vulkan/VkDeviceMemory.hpp"

#include <string.h>

namespace {
VkResult getWindowSize(HWND hwnd, VkExtent2D &windowSize)
{
	RECT clientRect = {};
	if(!IsWindow(hwnd) || !GetClientRect(hwnd, &clientRect))
	{
		windowSize = { 0, 0 };
		return VK_ERROR_SURFACE_LOST_KHR;
	}

	windowSize = { static_cast<uint32_t>(clientRect.right - clientRect.left),
		           static_cast<uint32_t>(clientRect.bottom - clientRect.top) };

	return VK_SUCCESS;
}
}  // namespace

namespace vk {

Win32SurfaceKHR::Win32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR *pCreateInfo, void *mem)
    : hwnd(pCreateInfo->hwnd)
{
	ASSERT(IsWindow(hwnd) == TRUE);
	windowContext = GetDC(hwnd);
}

void Win32SurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	ReleaseDC(hwnd, windowContext);
}

size_t Win32SurfaceKHR::ComputeRequiredAllocationSize(const VkWin32SurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

VkResult Win32SurfaceKHR::getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const
{
	VkExtent2D extent;
	VkResult result = getWindowSize(hwnd, extent);
	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;

	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);
	return VK_SUCCESS;
}

void Win32SurfaceKHR::attachImage(PresentImage *image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

void Win32SurfaceKHR::detachImage(PresentImage *image)
{
	// Nothing to do here, the current implementation based on GDI blits on
	// present instead of associating the image with the surface.
}

VkResult Win32SurfaceKHR::present(PresentImage *image)
{
	VkExtent2D windowExtent = {};
	VkResult result = getWindowSize(hwnd, windowExtent);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	const Image *vkImage = image->getImage();
	const VkExtent3D &extent = vkImage->getExtent();
	if(windowExtent.width != extent.width || windowExtent.height != extent.height)
	{
		return VK_ERROR_OUT_OF_DATE_KHR;
	}

	int stride = vkImage->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	int bytesPerPixel = static_cast<int>(vkImage->getFormat(VK_IMAGE_ASPECT_COLOR_BIT).bytes());

	BITMAPINFO bitmapInfo = {};
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biBitCount = bytesPerPixel * 8;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biHeight = -static_cast<LONG>(extent.height);  // Negative for top-down DIB, origin in upper-left corner
	bitmapInfo.bmiHeader.biWidth = stride / bytesPerPixel;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	void *bits = image->getImage()->getTexelPointer({ 0, 0, 0 }, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 });
	StretchDIBits(windowContext, 0, 0, extent.width, extent.height, 0, 0, extent.width, extent.height, bits, &bitmapInfo, DIB_RGB_COLORS, SRCCOPY);

	return VK_SUCCESS;
}

}  // namespace vk
