#pragma once
#include "PVRShell/PVRShell.h"
#include <DynamicEgl.h>
#include <DynamicGles.h>
#if TARGET_OS_IPHONE
typedef void* NativeDisplay;
typedef void* NativeWindow;
#else
typedef EGLNativeDisplayType NativeDisplay;
#endif

#if TARGET_OS_IPHONE
typedef void VoidUIView;
struct NativePlatformHandle
{
	EAGLContext* context;
	VoidUIView* view;

	NativePlatformHandle() : context(0), view(0) {}
};
#else
struct NativePlatformHandle
{
	EGLDisplay display;
	EGLSurface drawSurface;
	EGLSurface readSurface;
	EGLContext context;

#if defined(Wayland)
	wl_egl_window* eglWindow;
#endif

	NativePlatformHandle() : display(EGL_NO_DISPLAY), drawSurface(EGL_NO_SURFACE), readSurface(EGL_NO_SURFACE), context(EGL_NO_CONTEXT) {}
};
#endif

struct NativeDisplayHandle
{
	NativeDisplay nativeDisplay;
	operator NativeDisplay&() { return nativeDisplay; }
	operator const NativeDisplay&() const { return nativeDisplay; }
};

struct EglContext
{
	bool init(pvr::OSWindow window, pvr::OSDisplay display, pvr::DisplayAttributes& attributes, pvr::Api minVersion = pvr::Api::Unspecified, pvr::Api maxVersion = pvr::Api::Unspecified);
	bool preInitialize(pvr::OSDisplay osdisplay, NativePlatformHandle& handles);
	void populateMaxApiVersion();
	bool swapBuffers();
	void release();
	bool isApiSupported(pvr::Api api)
	{
		if (_maxApiVersion == pvr::Api::Unspecified) { populateMaxApiVersion(); }
		return api <= _maxApiVersion;
	}
	bool makeCurrent();

	NativePlatformHandle _platformContextHandles;
	NativeDisplayHandle _displayHandle;
	int8_t _swapInterval;
	pvr::Api _apiType;
	pvr::Api _maxApiVersion;
	bool _isDiscardSupported;

private:
#if TARGET_OS_IPHONE
	bool isGlesVersionSupported(pvr::Api apiLevel);
#else
	bool isGlesVersionSupported(EGLDisplay display, pvr::Api graphicsapi, bool& isSupported);
	bool initializeContext(bool wantWindow, pvr::DisplayAttributes& original_attributes, NativePlatformHandle& handles, EGLConfig& config, pvr::Api graphicsapi);
#endif
};
