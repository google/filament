// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "XcbSurfaceKHR.hpp"

#include "libXCB.hpp"
#include "Vulkan/VkDeviceMemory.hpp"
#include "Vulkan/VkImage.hpp"

#include <sys/ipc.h>
#include <sys/shm.h>

#include <memory>

namespace vk {

bool getWindowSizeAndDepth(xcb_connection_t *connection, xcb_window_t window, VkExtent2D *windowExtent, int *depth)
{
	auto cookie = libXCB->xcb_get_geometry(connection, window);
	if(auto *geom = libXCB->xcb_get_geometry_reply(connection, cookie, nullptr))
	{
		windowExtent->width = static_cast<uint32_t>(geom->width);
		windowExtent->height = static_cast<uint32_t>(geom->height);
		*depth = static_cast<int>(geom->depth);
		free(geom);
		return true;
	}
	return false;
}

bool XcbSurfaceKHR::isSupported()
{
	return libXCB.isPresent();
}

XcbSurfaceKHR::XcbSurfaceKHR(const VkXcbSurfaceCreateInfoKHR *pCreateInfo, void *mem)
    : connection(pCreateInfo->connection)
    , window(pCreateInfo->window)
{
	ASSERT(isSupported());

	gc = libXCB->xcb_generate_id(connection);
	uint32_t values[2] = { 0, 0xFFFFFFFF };
	libXCB->xcb_create_gc(connection, gc, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND, values);

	auto shmQuery = libXCB->xcb_get_extension_data(connection, libXCB->xcb_shm_id);
	if(shmQuery->present)
	{
		auto shmCookie = libXCB->xcb_shm_query_version(connection);
		if(auto *reply = libXCB->xcb_shm_query_version_reply(connection, shmCookie, nullptr))
		{
			mitSHM = reply && reply->shared_pixmaps;
			free(reply);
		}
	}

	auto geomCookie = libXCB->xcb_get_geometry(connection, window);
	if(auto *reply = libXCB->xcb_get_geometry_reply(connection, geomCookie, nullptr))
	{
		windowDepth = reply->depth;
		free(reply);
	}
	else
	{
		surfaceLost = true;
	}
}

void XcbSurfaceKHR::destroySurface(const VkAllocationCallbacks *pAllocator)
{
	libXCB->xcb_free_gc(connection, gc);
}

size_t XcbSurfaceKHR::ComputeRequiredAllocationSize(const VkXcbSurfaceCreateInfoKHR *pCreateInfo)
{
	return 0;
}

VkResult XcbSurfaceKHR::getSurfaceCapabilities(const void *pSurfaceInfoPNext, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities, void *pSurfaceCapabilitiesPNext) const
{
	if(surfaceLost)
	{
		return VK_ERROR_SURFACE_LOST_KHR;
	}

	VkExtent2D extent;
	int depth;
	if(!getWindowSizeAndDepth(connection, window, &extent, &depth))
	{
		surfaceLost = true;
		return VK_ERROR_SURFACE_LOST_KHR;
	}

	pSurfaceCapabilities->currentExtent = extent;
	pSurfaceCapabilities->minImageExtent = extent;
	pSurfaceCapabilities->maxImageExtent = extent;

	SetCommonSurfaceCapabilities(pSurfaceInfoPNext, pSurfaceCapabilities, pSurfaceCapabilitiesPNext);
	return VK_SUCCESS;
}

void *XcbSurfaceKHR::allocateImageMemory(PresentImage *image, const VkMemoryAllocateInfo &allocateInfo)
{
	if(!mitSHM)
	{
		return nullptr;
	}

	SHMPixmap &pixmap = pixmaps[image];
	int shmid = shmget(IPC_PRIVATE, allocateInfo.allocationSize, IPC_CREAT | SHM_R | SHM_W);
	pixmap.shmaddr = shmat(shmid, 0, 0);
	pixmap.shmseg = libXCB->xcb_generate_id(connection);
	libXCB->xcb_shm_attach(connection, pixmap.shmseg, shmid, false);
	shmctl(shmid, IPC_RMID, 0);

	int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	int bytesPerPixel = static_cast<int>(image->getImage()->getFormat(VK_IMAGE_ASPECT_COLOR_BIT).bytes());
	int width = stride / bytesPerPixel;
	int height = allocateInfo.allocationSize / stride;

	pixmap.pixmap = libXCB->xcb_generate_id(connection);
	libXCB->xcb_shm_create_pixmap(
	    connection,
	    pixmap.pixmap,
	    window,
	    width, height,
	    windowDepth,
	    pixmap.shmseg,
	    0);

	return pixmap.shmaddr;
}

void XcbSurfaceKHR::releaseImageMemory(PresentImage *image)
{
	if(mitSHM)
	{
		auto it = pixmaps.find(image);
		assert(it != pixmaps.end());
		libXCB->xcb_shm_detach(connection, it->second.shmseg);
		shmdt(it->second.shmaddr);
		libXCB->xcb_free_pixmap(connection, it->second.pixmap);
		pixmaps.erase(it);
	}
}

void XcbSurfaceKHR::attachImage(PresentImage *image)
{
}

void XcbSurfaceKHR::detachImage(PresentImage *image)
{
}

VkResult XcbSurfaceKHR::present(PresentImage *image)
{
	VkExtent2D windowExtent;
	int depth;
	// TODO(penghuang): getWindowSizeAndDepth() call needs a sync IPC, try to remove it.
	if(surfaceLost || !getWindowSizeAndDepth(connection, window, &windowExtent, &depth))
	{
		surfaceLost = true;
		return VK_ERROR_SURFACE_LOST_KHR;
	}

	const VkExtent3D &extent = image->getImage()->getExtent();

	if(windowExtent.width != extent.width || windowExtent.height != extent.height)
	{
		return VK_ERROR_OUT_OF_DATE_KHR;
	}

	if(!mitSHM)
	{
		// TODO: Convert image if not RGB888.
		int stride = image->getImage()->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
		int bytesPerPixel = static_cast<int>(image->getImage()->getFormat(VK_IMAGE_ASPECT_COLOR_BIT).bytes());
		int width = stride / bytesPerPixel;
		auto buffer = reinterpret_cast<uint8_t *>(image->getImageMemory()->getOffsetPointer(0));
		size_t max_request_size = static_cast<size_t>(libXCB->xcb_get_maximum_request_length(connection)) * 4;
		size_t max_strides = (max_request_size - sizeof(xcb_put_image_request_t)) / stride;
		for(size_t y = 0; y < extent.height; y += max_strides)
		{
			size_t num_strides = std::min(max_strides, extent.height - y);
			libXCB->xcb_put_image(
			    connection,
			    XCB_IMAGE_FORMAT_Z_PIXMAP,
			    window,
			    gc,
			    width,
			    num_strides,
			    0, y,                  // dst x, y
			    0,                     // left_pad
			    depth,
			    num_strides * stride,  // data_len
			    buffer + y * stride    // data
			);
		}
		assert(libXCB->xcb_connection_has_error(connection) == 0);
	}
	else
	{
		auto it = pixmaps.find(image);
		assert(it != pixmaps.end());
		libXCB->xcb_copy_area(
		    connection,
		    it->second.pixmap,
		    window,
		    gc,
		    0, 0,  // src x, y
		    0, 0,  // dst x, y
		    extent.width,
		    extent.height);
	}
	libXCB->xcb_flush(connection);

	return VK_SUCCESS;
}

}  // namespace vk
