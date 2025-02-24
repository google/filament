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

#ifndef SWIFTSHADER_WAYLANDSURFACEKHR_HPP
#define SWIFTSHADER_WAYLANDSURFACEKHR_HPP

#include "VkSurfaceKHR.hpp"
#include "Vulkan/VkObject.hpp"

#include <vulkan/vulkan_wayland.h>
#include <wayland-client.h>

#include <unordered_map>

namespace vk {

struct WaylandImage
{
	struct wl_buffer *buffer;
	uint8_t *data;
};

class WaylandSurfaceKHR : public SurfaceKHR, public ObjectBase<WaylandSurfaceKHR, VkSurfaceKHR>
{
public:
	static bool isSupported();
	WaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR *pCreateInfo, void *mem);

	void destroySurface(const VkAllocationCallbacks *pAllocator) override;

	static size_t ComputeRequiredAllocationSize(const VkWaylandSurfaceCreateInfoKHR *pCreateInfo);

	VkResult getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const override;

	virtual void attachImage(PresentImage *image) override;
	virtual void detachImage(PresentImage *image) override;
	VkResult present(PresentImage *image) override;

private:
	struct wl_display *display;
	struct wl_surface *surface;
	struct wl_shm *shm;
	std::unordered_map<PresentImage *, WaylandImage *> imageMap;
};

}  // namespace vk
#endif  // SWIFTSHADER_WAYLANDSURFACEKHR_HPP
