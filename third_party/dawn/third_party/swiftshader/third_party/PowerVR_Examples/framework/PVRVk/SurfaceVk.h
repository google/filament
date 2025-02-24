/*!
\brief The PVRVk Surface class
\file PVRVk/SurfaceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/DebugUtilsVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/TypesVk.h"

#if defined(VK_USE_PLATFORM_XCB_KHR)
#include <xcb/xcb.h>
#endif

namespace pvrvk {
namespace impl {
/// <summary>A surface represents a renderable part of the "screen", e.g. the inside part of the window.</summary>
class Surface_ : public PVRVkInstanceObjectBase<VkSurfaceKHR, ObjectType::e_SURFACE_KHR>
{
protected:
	/// <summary>Constructor for a surface object.</summary>
	/// <param name="instance">The instance which will be used to create the surface.</param>
	Surface_(Instance& instance);

	/// <summary>Destructor for a surface object.</summary>
	~Surface_();
};

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
/// <summary>An Android Surface.</summary>
class AndroidSurface_ : public Surface_
{
private:
	friend class Instance_;
	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend AndroidSurface_;
	};

	static AndroidSurface constructShared(Instance& instance, ANativeWindow* aNativewindow, AndroidSurfaceCreateFlagsKHR flags = AndroidSurfaceCreateFlagsKHR::e_NONE)
	{
		return std::make_shared<AndroidSurface_>(make_shared_enabler{}, instance, aNativewindow, flags);
	}

	ANativeWindow* _aNativeWindow;
	AndroidSurfaceCreateFlagsKHR _flags;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(AndroidSurface_)
	AndroidSurface_(make_shared_enabler, Instance& instance, ANativeWindow* aNativewindow, AndroidSurfaceCreateFlagsKHR flags);
	//!\endcond

	/// <summary>Get window handle</summary>
	/// <returns>ANativeWindow&</returns>
	const ANativeWindow* getANativeWindow() const { return _aNativeWindow; }

	/// <summary>Get AndroidSurfaceCreateFlagsKHR flags</summary>
	/// <returns>AndroidSurfaceCreateFlagsKHR&</returns>
	const AndroidSurfaceCreateFlagsKHR& getFlags() const { return _flags; }
};
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
/// <summary>An Win32 Surface.</summary>
class Win32Surface_ : public Surface_
{
private:
	friend class Instance_;
	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend Win32Surface_;
	};

	static Win32Surface constructShared(Instance& instance, HINSTANCE hinstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags = Win32SurfaceCreateFlagsKHR::e_NONE)
	{
		return std::make_shared<Win32Surface_>(make_shared_enabler{}, instance, hinstance, hwnd, flags);
	}

	HINSTANCE _hinstance;
	HWND _hwnd;
	Win32SurfaceCreateFlagsKHR _flags;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Win32Surface_)
	Win32Surface_(make_shared_enabler, Instance& instance, HINSTANCE hinstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags);
	//!\endcond

	/// <summary>Get hinstance</summary>
	/// <returns>HINSTANCE&</returns>
	const HINSTANCE& getHInstance() const { return _hinstance; }

	/// <summary>Get hwnd</summary>
	/// <returns>HWND&</returns>
	const HWND& getHwnd() const { return _hwnd; }

	/// <summary>Get Win32SurfaceCreateFlagsKHR flags</summary>
	/// <returns>Win32SurfaceCreateFlagsKHR&</returns>
	const Win32SurfaceCreateFlagsKHR& getFlags() const { return _flags; }
};
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
/// <summary>An Xcb Surface.</summary>
class XcbSurface_ : public Surface_
{
private:
	friend class Instance_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend XcbSurface_;
	};

	static XcbSurface constructShared(Instance& instance, xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags = XcbSurfaceCreateFlagsKHR::e_NONE)
	{
		return std::make_shared<XcbSurface_>(make_shared_enabler{}, instance, connection, window, flags);
	}

	xcb_connection_t* _connection;
	xcb_window_t _window;
	XcbSurfaceCreateFlagsKHR _flags;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(XcbSurface_)
	XcbSurface_(make_shared_enabler, Instance& instance, xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags);
	//!\endcond

	/// <summary>Get hinstance</summary>
	/// <returns>HINSTANCE&</returns>
	const xcb_connection_t& getConnection() const { return *_connection; }

	/// <summary>Get hwnd</summary>
	/// <returns>HWND&</returns>
	xcb_window_t getWindow() const { return _window; }

	/// <summary>Get XcbSurfaceCreateFlagsKHR flags</summary>
	/// <returns>XcbSurfaceCreateFlagsKHR&</returns>
	const XcbSurfaceCreateFlagsKHR& getFlags() const { return _flags; }
};
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
/// <summary>An Xlib Surface.</summary>
class XlibSurface_ : public Surface_
{
private:
	friend class Instance_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend XlibSurface_;
	};

	static XlibSurface constructShared(Instance& instance, ::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags = XlibSurfaceCreateFlagsKHR::e_NONE)
	{
		return std::make_shared<XlibSurface_>(make_shared_enabler{}, instance, dpy, window, flags);
	}

	::Display* _dpy;
	Window _window;
	XlibSurfaceCreateFlagsKHR _flags;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(XlibSurface_)
	XlibSurface_(make_shared_enabler, Instance& instance, ::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags);
	//!\endcond

	/// <summary>Get display</summary>
	/// <returns>Display&</returns>
	const ::Display& getDpy() const { return *_dpy; }

	/// <summary>Get Window</summary>
	/// <returns>Window&</returns>
	const Window& getWindow() const { return _window; }

	/// <summary>Get XlibSurfaceCreateFlagsKHR flags</summary>
	/// <returns>XlibSurfaceCreateFlagsKHR&</returns>
	const XlibSurfaceCreateFlagsKHR& getFlags() const { return _flags; }
};
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
/// <summary>A Wayland Surface.</summary>
class WaylandSurface_ : public Surface_
{
private:
	friend class Instance_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend WaylandSurface_;
	};

	static WaylandSurface constructShared(Instance& instance, wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags = WaylandSurfaceCreateFlagsKHR::e_NONE)
	{
		return std::make_shared<WaylandSurface_>(make_shared_enabler{}, instance, display, surface, flags);
	}

	wl_display* _display;
	wl_surface* _surface;
	WaylandSurfaceCreateFlagsKHR _flags;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(WaylandSurface_)
	WaylandSurface_(make_shared_enabler, Instance& instance, wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags);
	//!\endcond

	/// <summary>Get display</summary>
	/// <returns>wl_display&</returns>
	const wl_display* getDisplay() const { return _display; }

	/// <summary>Get Surface</summary>
	/// <returns>wl_surface&</returns>
	const wl_surface* getSurface() const { return _surface; }

	/// <summary>Get WaylandSurfaceCreateFlagsKHR flags</summary>
	/// <returns>WaylandSurfaceCreateFlagsKHR&</returns>
	const WaylandSurfaceCreateFlagsKHR& getFlags() const { return _flags; }
};
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK)
/// <summary>A MacOS Surface.</summary>
class MacOSSurface_ : public Surface_
{
private:
    friend class Instance_;
    
    class make_shared_enabler
    {
    protected:
        make_shared_enabler() {}
        friend MacOSSurface_;
    };
    
    static MacOSSurface constructShared(Instance& instance, void* view)
    {
        return std::make_shared<MacOSSurface_>(make_shared_enabler{}, instance, view);
    }
    
    void* _view;
    
public:
    //!\cond NO_DOXYGEN
    DECLARE_NO_COPY_SEMANTICS(MacOSSurface_)
    MacOSSurface_(make_shared_enabler, Instance& instance, void* view);
    //!\endcond
    
    /// <summary>Get view</summary>
    /// <returns>void*</returns>
    const void* getView() const
    {
        return _view;
    }
};
#endif

/// <summary>A DisplayPlane Surface.</summary>
class DisplayPlaneSurface_ : public Surface_
{
private:
	friend class Instance_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend DisplayPlaneSurface_;
	};

	static DisplayPlaneSurface constructShared(Instance& instance, const DisplayMode& displayMode, Extent2D imageExtent,
		const DisplaySurfaceCreateFlagsKHR flags = DisplaySurfaceCreateFlagsKHR::e_NONE, uint32_t planeIndex = 0, uint32_t planeStackIndex = 0,
		SurfaceTransformFlagsKHR transformFlags = SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR, float globalAlpha = 0.0f,
		DisplayPlaneAlphaFlagsKHR alphaFlags = DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR)
	{
		return std::make_shared<DisplayPlaneSurface_>(
			make_shared_enabler{}, instance, displayMode, imageExtent, flags, planeIndex, planeStackIndex, transformFlags, globalAlpha, alphaFlags);
	}

	DisplayModeWeakPtr _displayMode;
	DisplaySurfaceCreateFlagsKHR _flags;
	uint32_t _planeIndex;
	uint32_t _planeStackIndex;
	SurfaceTransformFlagsKHR _transformFlags;
	float _globalAlpha;
	DisplayPlaneAlphaFlagsKHR _alphaFlags;
	Extent2D _imageExtent;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DisplayPlaneSurface_)
	DisplayPlaneSurface_(make_shared_enabler, Instance& instance, const DisplayMode& displayMode, Extent2D imageExtent, const DisplaySurfaceCreateFlagsKHR flags,
		uint32_t planeIndex, uint32_t planeStackIndex, SurfaceTransformFlagsKHR transformFlags, float globalAlpha, DisplayPlaneAlphaFlagsKHR alphaFlags);
	//!\endcond

	/// <summary>Get Display Mode Properties</summary>
	/// <returns>DisplayModePropertiesKHR&</returns>
	const DisplayModeWeakPtr& getDisplayMode() const { return _displayMode; }

	/// <summary>Get DisplaySurfaceCreateFlagsKHR flags</summary>
	/// <returns>DisplaySurfaceCreateFlagsKHR&</returns>
	const DisplaySurfaceCreateFlagsKHR& getFlags() const { return _flags; }

	/// <summary>Get display plane index</summary>
	/// <returns>Plane index</returns>
	uint32_t getPlaneIndex() const { return _planeIndex; }

	/// <summary>Get display plane stack index</summary>
	/// <returns>Plane stack index</returns>
	uint32_t getPlaneStackIndex() const { return _planeStackIndex; }

	/// <summary>Get SurfaceTransformFlagsKHR flags</summary>
	/// <returns>SurfaceTransformFlagsKHR&</returns>
	const SurfaceTransformFlagsKHR& getTransformFlags() const { return _transformFlags; }

	/// <summary>Get display plane global alpha</summary>
	/// <returns>Plane global alpha</returns>
	float getGlobalAlpha() const { return _globalAlpha; }

	/// <summary>Get DisplayPlaneAlphaFlagsKHR flags</summary>
	/// <returns>DisplayPlaneAlphaFlagsKHR&</returns>
	const DisplayPlaneAlphaFlagsKHR& getAlphaFlags() const { return _alphaFlags; }

	/// <summary>Get Extent2D</summary>
	/// <returns>Extent2D&</returns>
	const Extent2D& getImageExtent() const { return _imageExtent; }
};
} // namespace impl
} // namespace pvrvk
