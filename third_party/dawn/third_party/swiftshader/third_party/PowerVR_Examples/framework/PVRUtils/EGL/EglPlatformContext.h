/*!
\brief Contains the declaration of the PlatformContext class, the main wrapper for the Platform specific part of a
Graphics context.
\file PVRUtils/EGL/EglPlatformContext.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once

#include "PVRCore/stream/Stream.h"
#include "PVRCore/math/Rectangle.h"
#include "PVRCore/strings/StringFunctions.h"
#if TARGET_OS_IPHONE
#include "PVRUtils/EAGL/EaglPlatformHandles.h"
#else
#include "PVRUtils/EGL/EglPlatformHandles.h"
#endif
#include <map>

/// <summary>Main PowerVR Framework namespace</summary>
namespace pvr {
/// <summary>The pvr::platform namespace contains low-level, system-communication classes and functions</summary>
namespace platform {

/// <summary>The EglContext context is the class wrapping all platform-specific objects required to power an
/// OpenGL implementation (Displays, windows, configurations etc.).</summary>
class EglContext_
{
public:
	EglContext_()
		: _platformContextHandles(), _swapInterval(-2), _initialized(false), _preInitialized(false), _apiType(Api::Unspecified), _maxApiVersion(Api::Unspecified), _attributes(0),
		  _isDiscardSupported(false), _parentContext(0)
	{}

	virtual ~EglContext_() { release(); }

	/// <summary>Release this object</summary>
	void release();

	/// <summary>Get maximum api version supported</summary>
	/// <returns>The maximum api version supported</returns>
	Api getMaxApiVersion();

	/// <summary>Return true if the specified api is supported</summary>
	/// <param name="api">The api to check for support</param>
	/// <returns>True if supported, otherwse false</returns>
	bool isApiSupported(Api api);

	/// <summary>Present back buffer</summary>
	void swapBuffers();

	/// <summary>Make this platform context current. In non-binding contexts(e.g. vulkan) does nothing</summary>
	void makeCurrent();

	/// <summary>Get native platform handles (const)</summary>
	/// <returns>A platform/api specific object containing the platform handles (surface, window etc.)</returns>
	const NativePlatformHandles_& getNativePlatformHandles() const { return *_platformContextHandles; }

	/// <summary>Get native platform handles (const)</summary>
	/// <returns>A platform/api specific object containing the platform handles (context, queues, fences/semaphores
	/// etc.)</returns>
	NativePlatformHandles_& getNativePlatformHandles() { return *_platformContextHandles; }

	/// <summary>Get information on this object</summary>
	/// <returns>Information on this object, typically device name etc.</returns>
	std::string getInfo();

	/// <summary>Return true if this object is initialized</summary>
	/// <returns>true if this object is initialized</returns>
	bool isInitialized() const { return (_platformContextHandles.get() != 0) && _initialized; }

	/// <summary>Getter for the Api version of the EGL platform context created.</summary>
	/// <returns>Returns the pvr::Api version of the EGL platform context created.</returns>
	Api getApiVersion();

	/// <summary>Initialize the EglPlatform Context object</summary>
	/// <param name="window">The window specifies an EGLNativeWindowType which is used to create the windowing surface used by the EGL platform context</param>
	/// <param name="display">The display specifies an EGLDisplay which is used to create the windowing surface used by the EGL platform context</param>
	/// <param name="attributes">The DisplayAttributes structure specifies the configuration with which to use when initialising the EGL platform context</param>
	/// <param name="minVersion">The minimum version of EGL platform context which must be supported for initialisation to be successful</param>
	/// <param name="maxVersion">The maximum version of EGL platform context which must be initialised</param>
	/// <returns>Returns 'true' on success 'false' in all other cases.</returns>
	void init(OSWindow window, OSDisplay display, DisplayAttributes& attributes, Api minVersion = Api::Unspecified, Api maxVersion = Api::Unspecified);

	/// <summary>Getter for the on screen frame buffer object. The default or on screen frame buffer is not guaranteed to be zero on all platforms.</summary>
	/// <returns>Returns the OpenGL ES handle for the on screen frame buffer object.</returns>
	uint32_t getOnScreenFbo();

	/// <summary>Creates an instance of a shared platform context</summary>
	/// <returns>A unique pointer to a EglContext_ instance.</returns>
	std::unique_ptr<EglContext_> createSharedContextFromEGLContext();

private:
	EglContext_(const EglContext_& rhs); // deleted
	NativePlatformHandles _platformContextHandles;
	NativeDisplayHandle _displayHandle;
	int8_t _swapInterval;
	bool _initialized;
	bool _preInitialized;
	Api _apiType;
	Api _maxApiVersion;
	DisplayAttributes* _attributes;
	bool _isDiscardSupported;
	platform::EglContext_* _parentContext;

	// Must be called after the context has been active in order to query the driver for resource limitations.
	void populateMaxApiVersion();
};

} // namespace platform
} // namespace pvr

namespace pvr {
/// <summary>A unique pointer to a pvr::platform::EglContext_ instance. Typdeffed to provide easier access and use of the EGLContext.</summary>
typedef std::unique_ptr<platform::EglContext_> EglContext;

/// <summary>Creates an instance of an EGL platform context</summary>
/// <returns>A unique pointer to a pvr::platform::EglContext_ instance.</returns>
std::unique_ptr<platform::EglContext_> createEglContext();
} // namespace pvr
