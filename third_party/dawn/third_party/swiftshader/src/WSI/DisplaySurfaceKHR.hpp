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

#ifndef SWIFTSHADER_DISPLAYSURFACEKHR_HPP
#define SWIFTSHADER_DISPLAYSURFACEKHR_HPP

#include "VkSurfaceKHR.hpp"
#include "Vulkan/VkObject.hpp"

#include <xf86drmMode.h>

namespace vk {

class DisplaySurfaceKHR : public SurfaceKHR, public ObjectBase<DisplaySurfaceKHR, VkSurfaceKHR>
{
public:
	static VkResult GetDisplayModeProperties(uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties);
	static VkResult GetDisplayPlaneCapabilities(VkDisplayPlaneCapabilitiesKHR *pCapabilities);
	static VkResult GetDisplayPlaneSupportedDisplays(uint32_t *pDisplayCount, VkDisplayKHR *pDisplays);
	static VkResult GetPhysicalDeviceDisplayPlaneProperties(uint32_t *pPropertyCount, VkDisplayPlanePropertiesKHR *pProperties);
	static VkResult GetPhysicalDeviceDisplayProperties(uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties);

	DisplaySurfaceKHR(const VkDisplaySurfaceCreateInfoKHR *pCreateInfo, void *mem);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const VkDisplaySurfaceCreateInfoKHR *pCreateInfo);

	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;

	virtual void attachImage(PresentImage *image) override;
	virtual void detachImage(PresentImage *image) override;
	VkResult present(PresentImage *image) override;

private:
	int fd;
	uint32_t connector_id;
	uint32_t encoder_id;
	uint32_t crtc_id;
	drmModeCrtc *crtc;
	drmModeModeInfo mode_info;
	uint32_t handle;
	uint32_t width;
	uint32_t height;
	uint32_t pitch;
	uint32_t size;
	uint32_t fb_id;
	uint8_t *fb_buffer;
};

}  // namespace vk
#endif  // SWIFTSHADER_DISPLAYSURFACEKHR_HPP
