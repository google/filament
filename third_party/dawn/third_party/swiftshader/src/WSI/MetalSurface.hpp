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

#ifndef SWIFTSHADER_METALSURFACE_HPP
#define SWIFTSHADER_METALSURFACE_HPP

#include "VkSurfaceKHR.hpp"
#include "Vulkan/VkObject.hpp"

#ifdef VK_USE_PLATFORM_MACOS_MVK
#	include <vulkan/vulkan_macos.h>
#endif
#ifdef VK_USE_PLATFORM_METAL_EXT
#	include <vulkan/vulkan_metal.h>
#endif

namespace vk {

class MetalLayer;

class MetalSurface : public SurfaceKHR
{
public:
	MetalSurface(const void *pCreateInfo, void *mem);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const void *pCreateInfo);

	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;

	virtual void attachImage(PresentImage *image) override {}
	virtual void detachImage(PresentImage *image) override {}
	VkResult present(PresentImage *image) override;

protected:
	MetalLayer *metalLayer = nullptr;
};

#ifdef VK_USE_PLATFORM_METAL_EXT
class MetalSurfaceEXT : public MetalSurface, public ObjectBase<MetalSurfaceEXT, VkSurfaceKHR>
{
public:
	MetalSurfaceEXT(const VkMetalSurfaceCreateInfoEXT *pCreateInfo, void *mem);
};
#endif

#ifdef VK_USE_PLATFORM_MACOS_MVK
class MacOSSurfaceMVK : public MetalSurface, public ObjectBase<MacOSSurfaceMVK, VkSurfaceKHR>
{
public:
	MacOSSurfaceMVK(const VkMacOSSurfaceCreateInfoMVK *pCreateInfo, void *mem);
};
#endif

}  // namespace vk
#endif  // SWIFTSHADER_METALSURFACE_HPP
