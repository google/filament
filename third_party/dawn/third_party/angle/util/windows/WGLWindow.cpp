//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGLWindow:
//   Implements initializing a WGL rendering context.
//

#include "util/windows/WGLWindow.h"

#include "common/string_utils.h"
#include "common/system_utils.h"
#include "util/OSWindow.h"

#include <iostream>

namespace
{
constexpr int kColorBits   = 24;
constexpr int kAlphaBits   = 8;
constexpr int kDepthBits   = 24;
constexpr int kStencilBits = 8;

PIXELFORMATDESCRIPTOR GetDefaultPixelFormatDescriptor()
{
    PIXELFORMATDESCRIPTOR pixelFormatDescriptor = {};
    pixelFormatDescriptor.nSize                 = sizeof(pixelFormatDescriptor);
    pixelFormatDescriptor.nVersion              = 1;
    pixelFormatDescriptor.dwFlags =
        PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixelFormatDescriptor.iPixelType   = PFD_TYPE_RGBA;
    pixelFormatDescriptor.cColorBits   = kColorBits;
    pixelFormatDescriptor.cAlphaBits   = kAlphaBits;
    pixelFormatDescriptor.cDepthBits   = kDepthBits;
    pixelFormatDescriptor.cStencilBits = kStencilBits;
    pixelFormatDescriptor.iLayerType   = PFD_MAIN_PLANE;

    return pixelFormatDescriptor;
}

PFNWGLGETPROCADDRESSPROC gCurrentWGLGetProcAddress = nullptr;
HMODULE gCurrentModule                             = nullptr;

GenericProc WINAPI GetProcAddressWithFallback(const char *name)
{
    GenericProc proc = reinterpret_cast<GenericProc>(gCurrentWGLGetProcAddress(name));
    if (proc)
    {
        return proc;
    }

    return reinterpret_cast<GenericProc>(GetProcAddress(gCurrentModule, name));
}

bool HasExtension(const std::vector<std::string> &extensions, const char *ext)
{
    return std::find(extensions.begin(), extensions.end(), ext) != extensions.end();
}

void DumpLastWindowsError()
{
    std::cerr << "Last Windows error code: 0x" << std::hex << GetLastError() << std::endl;
}

// Based on GetDefaultPixelFormatAttributes from wgl_utils.cpp
std::vector<int> GetPixelFormatAttributes(const ConfigParameters &configParams)
{
    std::vector<int> attribs;
    attribs.push_back(WGL_DRAW_TO_WINDOW_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_ACCELERATION_ARB);
    attribs.push_back(WGL_FULL_ACCELERATION_ARB);

    attribs.push_back(WGL_SUPPORT_OPENGL_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_DOUBLE_BUFFER_ARB);
    attribs.push_back(TRUE);

    attribs.push_back(WGL_PIXEL_TYPE_ARB);
    attribs.push_back(WGL_TYPE_RGBA_ARB);

    attribs.push_back(WGL_COLOR_BITS_ARB);
    attribs.push_back(kColorBits);

    attribs.push_back(WGL_ALPHA_BITS_ARB);
    attribs.push_back(kAlphaBits);

    attribs.push_back(WGL_DEPTH_BITS_ARB);
    attribs.push_back(kDepthBits);

    attribs.push_back(WGL_STENCIL_BITS_ARB);
    attribs.push_back(kStencilBits);

    attribs.push_back(WGL_SWAP_METHOD_ARB);
    attribs.push_back(WGL_SWAP_UNDEFINED_ARB);

    attribs.push_back(WGL_COLORSPACE_EXT);
    if (configParams.colorSpace == EGL_COLORSPACE_sRGB)
    {
        attribs.push_back(WGL_COLORSPACE_SRGB_EXT);
    }
    else
    {
        attribs.push_back(WGL_COLORSPACE_LINEAR_EXT);
    }

    attribs.push_back(0);

    return attribs;
}

}  // namespace

WGLWindow::WGLWindow(int majorVersion, int minorVersion)
    : GLWindowBase(majorVersion, minorVersion),
      mDeviceContext(nullptr),
      mWGLContext(nullptr),
      mWindow(nullptr)
{}

WGLWindow::~WGLWindow() {}

// Internally initializes GL resources.
GLWindowResult WGLWindow::initializeGLWithResult(OSWindow *osWindow,
                                                 angle::Library *glWindowingLibrary,
                                                 angle::GLESDriverType driverType,
                                                 const EGLPlatformParameters &platformParams,
                                                 const ConfigParameters &configParams)
{
    if (driverType != angle::GLESDriverType::SystemWGL)
    {
        std::cerr << "WGLWindow requires angle::GLESDriverType::SystemWGL.\n";
        return GLWindowResult::Error;
    }

    glWindowingLibrary->getAs("wglGetProcAddress", &gCurrentWGLGetProcAddress);

    if (!gCurrentWGLGetProcAddress)
    {
        std::cerr << "Error loading wglGetProcAddress." << std::endl;
        return GLWindowResult::Error;
    }

    gCurrentModule = reinterpret_cast<HMODULE>(glWindowingLibrary->getNative());
    LoadWGL(GetProcAddressWithFallback);

    mWindow                                           = osWindow->getNativeWindow();
    mDeviceContext                                    = GetDC(mWindow);
    const PIXELFORMATDESCRIPTOR pixelFormatDescriptor = GetDefaultPixelFormatDescriptor();

    int pixelFormat = 0;

    if (!_wglChoosePixelFormatARB)
    {
        std::cout << "Driver does not expose wglChoosePixelFormatARB." << std::endl;
    }
    else
    {
        std::vector<int> pixelFormatAttribs = GetPixelFormatAttributes(configParams);

        UINT matchingFormats = 0;
        _wglChoosePixelFormatARB(mDeviceContext, &pixelFormatAttribs[0], nullptr, 1u, &pixelFormat,
                                 &matchingFormats);
    }

    if (pixelFormat == 0 && configParams.colorSpace != EGL_COLORSPACE_LINEAR)
    {
        std::cerr << "Could not find a compatible pixel format for a non-linear color space."
                  << std::endl;
        return GLWindowResult::NoColorspaceSupport;
    }

    if (pixelFormat == 0)
    {
        pixelFormat = ChoosePixelFormat(mDeviceContext, &pixelFormatDescriptor);
    }

    if (pixelFormat == 0)
    {
        std::cerr << "Could not find a compatible pixel format." << std::endl;
        DumpLastWindowsError();
        return GLWindowResult::Error;
    }

    // According to the Windows docs, it is an error to set a pixel format twice.
    int currentPixelFormat = GetPixelFormat(mDeviceContext);
    if (currentPixelFormat != pixelFormat)
    {
        if (SetPixelFormat(mDeviceContext, pixelFormat, &pixelFormatDescriptor) != TRUE)
        {
            std::cerr << "Failed to set the pixel format." << std::endl;
            DumpLastWindowsError();
            return GLWindowResult::Error;
        }
    }

    mWGLContext = createContext(configParams, nullptr);
    if (mWGLContext == nullptr)
    {
        return GLWindowResult::Error;
    }

    if (!makeCurrent())
    {
        return GLWindowResult::Error;
    }

    mPlatform     = platformParams;
    mConfigParams = configParams;

    LoadUtilGLES(GetProcAddressWithFallback);
    return GLWindowResult::NoError;
}

bool WGLWindow::initializeGL(OSWindow *osWindow,
                             angle::Library *glWindowingLibrary,
                             angle::GLESDriverType driverType,
                             const EGLPlatformParameters &platformParams,
                             const ConfigParameters &configParams)
{
    return initializeGLWithResult(osWindow, glWindowingLibrary, driverType, platformParams,
                                  configParams) == GLWindowResult::NoError;
}

HGLRC WGLWindow::createContext(const ConfigParameters &configParams, HGLRC shareContext)
{
    HGLRC context = _wglCreateContext(mDeviceContext);
    if (!context)
    {
        std::cerr << "Failed to create a WGL context." << std::endl;
        return context;
    }

    if (!makeCurrent(context))
    {
        std::cerr << "Failed to make WGL context current." << std::endl;
        return context;
    }

    // Reload entry points to capture extensions.
    LoadWGL(GetProcAddressWithFallback);

    if (!_wglGetExtensionsStringARB)
    {
        std::cerr << "Driver does not expose wglGetExtensionsStringARB." << std::endl;
        return context;
    }

    const char *extensionsString = _wglGetExtensionsStringARB(mDeviceContext);

    std::vector<std::string> extensions;
    angle::SplitStringAlongWhitespace(extensionsString, &extensions);

    if (!HasExtension(extensions, "WGL_EXT_create_context_es2_profile"))
    {
        std::cerr << "Driver does not expose WGL_EXT_create_context_es2_profile." << std::endl;
        return context;
    }

    if (mConfigParams.webGLCompatibility.valid() || mConfigParams.robustResourceInit.valid())
    {
        std::cerr << "WGLWindow does not support the requested feature set." << std::endl;
        return context;
    }

    // Tear down the context and create another with ES2 compatibility.
    _wglDeleteContext(context);

    // This could be extended to cover ES1 compatibility.
    const int createAttribs[] = {WGL_CONTEXT_MAJOR_VERSION_ARB,
                                 mClientMajorVersion,
                                 WGL_CONTEXT_MINOR_VERSION_ARB,
                                 mClientMinorVersion,
                                 WGL_CONTEXT_PROFILE_MASK_ARB,
                                 WGL_CONTEXT_ES2_PROFILE_BIT_EXT,
                                 0,
                                 0};

    context = _wglCreateContextAttribsARB(mDeviceContext, shareContext, createAttribs);
    if (!context)
    {
        std::cerr << "Failed to create an ES2 compatible WGL context." << std::endl;
        return context;
    }

    return context;
}

void WGLWindow::destroyGL()
{
    if (mWGLContext)
    {
        _wglDeleteContext(mWGLContext);
        mWGLContext = nullptr;
    }

    if (mDeviceContext)
    {
        ReleaseDC(mWindow, mDeviceContext);
        mDeviceContext = nullptr;
    }
}

bool WGLWindow::isGLInitialized() const
{
    return mWGLContext != nullptr;
}

GLWindowContext WGLWindow::getCurrentContextGeneric()
{
    return reinterpret_cast<GLWindowContext>(mWGLContext);
}

GLWindowContext WGLWindow::createContextGeneric(GLWindowContext share)
{
    HGLRC shareContext = reinterpret_cast<HGLRC>(share);
    HGLRC newContext   = createContext(mConfigParams, shareContext);

    // createContext() calls makeCurrent(newContext), so we need to restore the current context.
    if (!makeCurrent())
    {
        return nullptr;
    }

    return reinterpret_cast<GLWindowContext>(newContext);
}

bool WGLWindow::makeCurrent()
{
    return makeCurrent(mWGLContext);
}

bool WGLWindow::makeCurrentGeneric(GLWindowContext context)
{
    HGLRC wglContext = reinterpret_cast<HGLRC>(context);
    return makeCurrent(wglContext);
}

bool WGLWindow::makeCurrent(HGLRC context)
{
    if (_wglMakeCurrent(mDeviceContext, context) == FALSE)
    {
        std::cerr << "Error during wglMakeCurrent.\n";
        return false;
    }

    return true;
}

WGLWindow::Image WGLWindow::createImage(GLWindowContext context,
                                        Enum target,
                                        ClientBuffer buffer,
                                        const Attrib *attrib_list)
{
    std::cerr << "WGLWindow::createImage not implemented.\n";
    return nullptr;
}

WGLWindow::Image WGLWindow::createImageKHR(GLWindowContext context,
                                           Enum target,
                                           ClientBuffer buffer,
                                           const AttribKHR *attrib_list)
{
    std::cerr << "WGLWindow::createImageKHR not implemented.\n";
    return nullptr;
}

EGLBoolean WGLWindow::destroyImage(Image image)
{
    std::cerr << "WGLWindow::destroyImage not implemented.\n";
    return EGL_FALSE;
}

EGLBoolean WGLWindow::destroyImageKHR(Image image)
{
    std::cerr << "WGLWindow::destroyImageKHR not implemented.\n";
    return EGL_FALSE;
}

WGLWindow::Sync WGLWindow::createSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    return nullptr;
}

WGLWindow::Sync WGLWindow::createSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
    return nullptr;
}

EGLBoolean WGLWindow::destroySync(EGLDisplay dpy, Sync sync)
{
    return EGL_FALSE;
}

EGLBoolean WGLWindow::destroySyncKHR(EGLDisplay dpy, Sync sync)
{
    return EGL_FALSE;
}

EGLint WGLWindow::clientWaitSync(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout)
{
    return EGL_FALSE;
}

EGLint WGLWindow::clientWaitSyncKHR(EGLDisplay dpy, Sync sync, EGLint flags, EGLTimeKHR timeout)
{
    return EGL_FALSE;
}

EGLint WGLWindow::getEGLError()
{
    return EGL_SUCCESS;
}

WGLWindow::Display WGLWindow::getCurrentDisplay()
{
    return nullptr;
}

WGLWindow::Surface WGLWindow::createPbufferSurface(const EGLint *attrib_list)
{
    std::cerr << "WGLWindow::createPbufferSurface not implemented.\n";
    return EGL_FALSE;
}

EGLBoolean WGLWindow::destroySurface(Surface surface)
{
    std::cerr << "WGLWindow::destroySurface not implemented.\n";
    return EGL_FALSE;
}

EGLBoolean WGLWindow::bindTexImage(EGLSurface surface, EGLint buffer)
{
    std::cerr << "WGLWindow::bindTexImage not implemented.\n";
    return EGL_FALSE;
}

EGLBoolean WGLWindow::releaseTexImage(EGLSurface surface, EGLint buffer)
{
    std::cerr << "WGLWindow::releaseTexImage not implemented.\n";
    return EGL_FALSE;
}

bool WGLWindow::makeCurrent(EGLSurface draw, EGLSurface read, EGLContext context)
{
    std::cerr << "WGLWindow::makeCurrent(draw, read, context) not implemented.\n";
    return EGL_FALSE;
}

bool WGLWindow::setSwapInterval(EGLint swapInterval)
{
    if (!_wglSwapIntervalEXT || _wglSwapIntervalEXT(swapInterval) == FALSE)
    {
        std::cerr << "Error during wglSwapIntervalEXT.\n";
        return false;
    }
    return true;
}

void WGLWindow::swap()
{
    if (SwapBuffers(mDeviceContext) == FALSE)
    {
        std::cerr << "Error during SwapBuffers.\n";
    }
}

bool WGLWindow::hasError() const
{
    return GetLastError() != S_OK;
}

GenericProc WGLWindow::getProcAddress(const char *name)
{
    return GetProcAddressWithFallback(name);
}

// static
WGLWindow *WGLWindow::New(int majorVersion, int minorVersion)
{
    return new WGLWindow(majorVersion, minorVersion);
}

// static
void WGLWindow::Delete(WGLWindow **window)
{
    delete *window;
    *window = nullptr;
}
