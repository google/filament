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

#include "WaylandSurfaceKHR.hpp"

#include "libWaylandClient.hpp"
#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

namespace vk {

bool WaylandSurfaceKHR::isSupported()
{
	return libWaylandClient.isPresent();
}

static void wl_registry_handle_global(void *data, struct wl_registry *registry, unsigned int name, const char *interface, unsigned int version)
{
	struct wl_shm **pshm = (struct wl_shm **)data;
	if(!strcmp(interface, "wl_shm"))
	{
		*pshm = static_cast<struct wl_shm *>(libWaylandClient->wl_registry_bind(registry, name, libWaylandClient->wl_shm_interface, 1));
	}
}

static void wl_registry_handle_global_remove(void *data, struct wl_registry *registry, unsigned int name)
{
}

static const struct wl_registry_listener wl_registry_listener = { wl_registry_handle_global, wl_registry_handle_global_remove };

WaylandSurfaceKHR::WaylandSurfaceKHR(const VkWaylandSurfaceCreateInfoKHR *pCreateInfo, void *mem)
    : display(pCreateInfo->display)
    , surface(pCreateInfo->surface)
{
	struct wl_registry *registry = libWaylandClient->wl_display_get_registry(display);
	libWaylandClient->wl_registry_add_listener(registry, &wl_registry_listener, &shm);
	libWaylandClient->wl_display_dispatch(display);
}

void WaylandSurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
}

size_t WaylandSurfaceKHR::ComputeRequiredAllocationSize(const VkWaylandSurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

VkResult WaylandSurfaceKHR::getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const
{
	pSurfaceCapabilities->currentExtent = { 0xFFFFFFFF, 0xFFFFFFFF };
	pSurfaceCapabilities->minImageExtent = { 1, 1 };
	pSurfaceCapabilities->maxImageExtent = { 0xFFFFFFFF, 0xFFFFFFFF };

	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);
	return VK_SUCCESS;
}

void WaylandSurfaceKHR::attachImage(PresentImage *image)
{
	WaylandImage *wlImage = new WaylandImage;
	char path[] = "/tmp/XXXXXX";
	int fd = mkstemp(path);
	const VkExtent3D &extent = image->getImage()->getExtent();
	int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	assert(ftruncate(fd, extent.height * stride) == 0);
	struct wl_shm_pool *pool = libWaylandClient->wl_shm_create_pool(shm, fd, extent.height * stride);
	wlImage->buffer = libWaylandClient->wl_shm_pool_create_buffer(pool, 0, extent.width, extent.height, stride, WL_SHM_FORMAT_XRGB8888);
	wlImage->data = static_cast<uint8_t *>(mmap(NULL, extent.height * stride, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
	libWaylandClient->wl_shm_pool_destroy(pool);
	close(fd);
	imageMap[image] = wlImage;
}

void WaylandSurfaceKHR::detachImage(PresentImage *image)
{
	auto it = imageMap.find(image);
	if(it != imageMap.end())
	{
		WaylandImage *wlImage = it->second;
		const VkExtent3D &extent = image->getImage()->getExtent();
		int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
		munmap(wlImage->data, extent.height * stride);
		libWaylandClient->wl_buffer_destroy(wlImage->buffer);
		delete wlImage;
		imageMap.erase(it);
	}
}

VkResult WaylandSurfaceKHR::present(PresentImage *image)
{
	auto it = imageMap.find(image);
	if(it != imageMap.end())
	{
		WaylandImage *wlImage = it->second;
		const VkExtent3D &extent = image->getImage()->getExtent();
		int bufferRowPitch = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
		image->getImage()->copyTo(reinterpret_cast<uint8_t *>(wlImage->data), bufferRowPitch);
		libWaylandClient->wl_surface_attach(surface, wlImage->buffer, 0, 0);
		libWaylandClient->wl_surface_damage(surface, 0, 0, extent.width, extent.height);
		libWaylandClient->wl_surface_commit(surface);
		libWaylandClient->wl_display_roundtrip(display);
		libWaylandClient->wl_display_sync(display);
	}

	return VK_SUCCESS;
}

}  // namespace vk
