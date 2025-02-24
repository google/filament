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

#ifndef SWIFTSHADER_WIN32SURFACEKHR_HPP
#define SWIFTSHADER_WIN32SURFACEKHR_HPP

#include "VkSurfaceKHR.hpp"
#include "Vulkan/VkImage.hpp"
#include "Vulkan/VkObject.hpp"

#if !defined(WIN32_LEAN_AND_MEAN)
#	define WIN32_LEAN_AND_MEAN
#endif  // !defined(WIN32_LEAN_AND_MEAN)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>

namespace vk {

class Win32SurfaceKHR : public SurfaceKHR, public ObjectBase<Win32SurfaceKHR, VkSurfaceKHR>
{
public:
	Win32SurfaceKHR(const VkWin32SurfaceCreateInfoKHR *pCreateInfo, void *mem);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const VkWin32SurfaceCreateInfoKHR *pCreateInfo);

	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;

	virtual void attachImage(PresentImage *image) override;
	virtual void detachImage(PresentImage *image) override;
	VkResult present(PresentImage *image) override;

private:
	const HWND hwnd;

	HDC windowContext = {};
	HDC bitmapContext = {};
};

}  // namespace vk
#endif  // SWIFTSHADER_WIN32SURFACEKHR_HPP
