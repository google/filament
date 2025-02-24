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

#ifndef SWIFTSHADER_XCBSURFACEKHR_HPP
#define SWIFTSHADER_XCBSURFACEKHR_HPP

#include "VkSurfaceKHR.hpp"
#include "Vulkan/VkObject.hpp"

#include <xcb/shm.h>
#include <xcb/xcb.h>
// XCB headers must be included before the Vulkan header.
#include <vulkan/vulkan_xcb.h>

#include <unordered_map>

namespace vk {

class XcbSurfaceKHR : public SurfaceKHR, public ObjectBase<XcbSurfaceKHR, VkSurfaceKHR>
{
public:
	static bool isSupported();
	XcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR *pCreateInfo, void *mem);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const VkXcbSurfaceCreateInfoKHR *pCreateInfo);

	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;

	virtual void *allocateImageMemory(PresentImage *image, const VkMemoryAllocateInfo &allocateInfo) override;
	virtual void releaseImageMemory(PresentImage *image) override;
	virtual void attachImage(PresentImage *image) override;
	virtual void detachImage(PresentImage *image) override;
	VkResult present(PresentImage *image) override;

private:
	xcb_connection_t *connection = nullptr;
	xcb_window_t window = XCB_NONE;
	bool mitSHM = false;
	xcb_gcontext_t gc = XCB_NONE;
	int windowDepth = 0;
	mutable bool surfaceLost = false;
	struct SHMPixmap
	{
		xcb_shm_seg_t shmseg = XCB_NONE;
		void *shmaddr = nullptr;
		xcb_pixmap_t pixmap = XCB_NONE;
	};
	std::unordered_map<PresentImage *, SHMPixmap> pixmaps;
	// std::unordered_map<PresentImage *, uint32_t> graphicsContexts;
};

}  // namespace vk
#endif  // SWIFTSHADER_XCBSURFACEKHR_HPP
