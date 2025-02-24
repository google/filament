/*!
\brief Function implementations for the Surface class
\file PVRVk/SurfaceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/SurfaceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/DisplayModeVk.h"
#ifdef VK_USE_PLATFORM_XCB_KHR
#include <dlfcn.h>
#endif

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
Surface_::Surface_(Instance& instance) : PVRVkInstanceObjectBase(instance) {}

Surface_::~Surface_()
{
	if (getVkHandle() != VK_NULL_HANDLE)
	{
		if (!_instance.expired())
		{
			Instance instancePtr = getInstance();
			instancePtr->getVkBindings().vkDestroySurfaceKHR(instancePtr->getVkHandle(), getVkHandle(), NULL);
			_vkHandle = VK_NULL_HANDLE;
		}
		else
		{
			assert(false && "Attempted to destroy object of type [Surface] after its corresponding VkInstance");
		}
	}
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
AndroidSurface_::AndroidSurface_(make_shared_enabler, Instance& instance, ANativeWindow* aNativewindow, AndroidSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrAndroidSurfaceEnabled)
	{
		_aNativeWindow = aNativewindow;
		_flags = flags;

		// Create an Android Surface
		VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = static_cast<VkStructureType>(StructureType::e_ANDROID_SURFACE_CREATE_INFO_KHR);
		surfaceInfo.pNext = NULL;
		surfaceInfo.flags = static_cast<VkAndroidSurfaceCreateFlagsKHR>(_flags);
		surfaceInfo.window = _aNativeWindow;
		vkThrowIfFailed(instance->getVkBindings().vkCreateAndroidSurfaceKHR(instance->getVkHandle(), &surfaceInfo, NULL, &_vkHandle), "Failed to create Android Surface");
	}
	else
	{
		throw ErrorUnknown("Android Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
Win32Surface_::Win32Surface_(make_shared_enabler, Instance& instance, HINSTANCE hinstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrWin32SurfaceEnabled)
	{
		_hinstance = hinstance;
		_hwnd = hwnd;
		_flags = flags;

		// Create a Win32 Surface
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_WIN32_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.hinstance = _hinstance;
		surfaceCreateInfo.hwnd = _hwnd;
		surfaceCreateInfo.flags = static_cast<VkWin32SurfaceCreateFlagsKHR>(_flags);
		vkThrowIfFailed(instance->getVkBindings().vkCreateWin32SurfaceKHR(instance->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle), "failed to create Win32 Window surface");
	}
	else
	{
		throw ErrorUnknown("Win32 Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
XcbSurface_::XcbSurface_(make_shared_enabler, Instance& instance, xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrXcbSurfaceEnabled)
	{
		_connection = connection;
		_window = window;
		_flags = flags;

		VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_XCB_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.connection = connection;
		surfaceCreateInfo.window = window;
		vkThrowIfFailed(instance->getVkBindings().vkCreateXcbSurfaceKHR(instance->getVkHandle(), &surfaceCreateInfo, nullptr, &_vkHandle), "Failed to create Xcb Window surface");
	}
	else
	{
		throw ErrorUnknown("Xcb Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
XlibSurface_::XlibSurface_(make_shared_enabler, Instance& instance, ::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrXlibSurfaceEnabled)
	{
		_dpy = dpy;
		_window = window;
		_flags = flags;

		// Create an Xlib Surface
		VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_XLIB_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.dpy = _dpy;
		surfaceCreateInfo.window = _window;
		surfaceCreateInfo.flags = static_cast<VkXlibSurfaceCreateFlagsKHR>(_flags);
		vkThrowIfFailed(instance->getVkBindings().vkCreateXlibSurfaceKHR(instance->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle), "Failed to create Xlib Window surface");
	}
	else
	{
		throw ErrorUnknown("Xlib Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
WaylandSurface_::WaylandSurface_(make_shared_enabler, Instance& instance, wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrWaylandSurfaceEnabled)
	{
		_display = display;
		_surface = surface;
		_flags = flags;

		// Create a Wayland Surface
		VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_WAYLAND_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.display = _display;
		surfaceCreateInfo.surface = _surface;
		vkThrowIfFailed(instance->getVkBindings().vkCreateWaylandSurfaceKHR(instance->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle), "Failed to create Wayland Window surface");
	}
	else
	{
		throw ErrorUnknown("Wayland Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK)
MacOSSurface_::MacOSSurface_(make_shared_enabler, Instance& instance, void* view) : Surface_(instance)
{
	if (instance->getEnabledExtensionTable().mvkMacosSurfaceEnabled)
	{
		_view = view;

		// Create the MacOS surface info, passing the NSView handle
		VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceCreateInfo.pNext = 0;
		surfaceCreateInfo.flags = 0;
		// pView must be a valid NSView and must be backed by a CALayer instance of type CAMetalLayer.
		surfaceCreateInfo.pView = _view;

		vkThrowIfFailed(instance->getVkBindings().vkCreateMacOSSurfaceMVK(instance->getVkHandle(), &surfaceCreateInfo, NULL, &_vkHandle), "Failed to create MacOS surface");
	}
	else
	{
		throw ErrorUnknown("MacOS Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
#endif

DisplayPlaneSurface_::DisplayPlaneSurface_(make_shared_enabler, Instance& instance, const DisplayMode& displayMode, Extent2D imageExtent, const DisplaySurfaceCreateFlagsKHR flags,
	uint32_t planeIndex, uint32_t planeStackIndex, SurfaceTransformFlagsKHR transformFlags, float globalAlpha, DisplayPlaneAlphaFlagsKHR alphaFlags)
	: Surface_(instance)
{
	if (instance->getEnabledExtensionTable().khrDisplayEnabled)
	{
		_displayMode = displayMode;
		_flags = flags;
		_planeIndex = planeIndex;
		_planeStackIndex = planeStackIndex;
		_transformFlags = transformFlags;
		_globalAlpha = globalAlpha;
		_alphaFlags = alphaFlags;
		_imageExtent = imageExtent;

		// Create a DisplayPlane Surface
		VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DISPLAY_SURFACE_CREATE_INFO_KHR);
		surfaceCreateInfo.displayMode = displayMode->getVkHandle();
		surfaceCreateInfo.planeIndex = _planeIndex;
		surfaceCreateInfo.planeStackIndex = _planeStackIndex;
		surfaceCreateInfo.transform = static_cast<VkSurfaceTransformFlagBitsKHR>(_transformFlags);
		surfaceCreateInfo.globalAlpha = _globalAlpha;
		surfaceCreateInfo.alphaMode = static_cast<VkDisplayPlaneAlphaFlagBitsKHR>(_alphaFlags);
		surfaceCreateInfo.imageExtent = _imageExtent.get();

		vkThrowIfFailed(
			instance->getVkBindings().vkCreateDisplayPlaneSurfaceKHR(instance->getVkHandle(), &surfaceCreateInfo, nullptr, &_vkHandle), "Could not create DisplayPlane Surface");
	}
	else
	{
		throw ErrorUnknown("Display Plane Platform Surface extensions have not been enabled when creating the VkInstance.");
	}
}
//!\endcond
} // namespace impl
} // namespace pvrvk
