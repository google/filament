/*!
\brief Native object handles (display, window, view etc.) for the EAGL (iOS) implementation of PVRNativeApi.
\file PVRUtils/EAGL/EaglPlatformHandles.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#pragma once

#include "PVRCore/types/Types.h"
#include "PVRCore/Log.h"
#include "OpenGLES/ES2/gl.h"
#include "OpenGLES/ES2/glext.h"
#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else
class EAGLContext;
#endif
namespace pvr {
namespace platform {
typedef void* NativeDisplay; //!< void pointer representing the OS Display
typedef void* NativeWindow; //!< void pointer representing the OS Window

// Objective-C Type Definitions
typedef void VoidUIView;
typedef void VoidUIApplicationDelegate;

typedef VoidUIApplicationDelegate* OSApplication;
typedef void* OSDisplay;
typedef VoidUIView* OSWindow;
typedef void* OSSurface;
typedef void* OSDATA;

struct NativePlatformHandles_
{
	EAGLContext* context;
	VoidUIView* view;

	GLint numDiscardAttachments = 0;
	GLenum discardAttachments[3];
	GLuint framebuffer = 0;
	GLuint renderbuffer = 0;
	GLuint depthBuffer = 0;

	GLuint msaaFrameBuffer = 0;
	GLuint msaaColorBuffer = 0;
	GLuint msaaDepthBuffer = 0;
    
	NativePlatformHandles_() {}
};

/*! \brief Forward-declare friendly container for the native display */
struct NativeDisplayHandle_
{
	NativeDisplay nativeDisplay;
	operator NativeDisplay&() { return nativeDisplay; }
	operator const NativeDisplay&() const { return nativeDisplay; }
};
/*! \brief Forward-declare friendly container for the native window */
struct NativeWindowHandle_
{
	NativeWindow nativeWindow;
	operator NativeWindow&() { return nativeWindow; }
	operator const NativeWindow&() const { return nativeWindow; }
};

/// <summary>Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.</summary>
typedef std::shared_ptr<NativePlatformHandles_> NativePlatformHandles;

/// <summary>Pointer to a struct of platform handles. Used to pass around the undefined NativePlatformHandles_ struct.</summary>
typedef std::shared_ptr<NativeDisplayHandle_> NativeDisplayHandle;

} // namespace platform
} // namespace pvr
//!\endcond
