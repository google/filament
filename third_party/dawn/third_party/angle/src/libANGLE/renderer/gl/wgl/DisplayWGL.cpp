//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayWGL.h: WGL implementation of egl::Display

#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"

#include "common/debug.h"
#include "common/system_utils.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/gl/wgl/ContextWGL.h"
#include "libANGLE/renderer/gl/wgl/D3DTextureSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/DXGISwapChainWindowSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/FunctionsWGL.h"
#include "libANGLE/renderer/gl/wgl/PbufferSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/RendererWGL.h"
#include "libANGLE/renderer/gl/wgl/WindowSurfaceWGL.h"
#include "libANGLE/renderer/gl/wgl/wgl_utils.h"
#include "platform/PlatformMethods.h"

#include <EGL/eglext.h>
#include <sstream>
#include <string>

namespace rx
{

namespace
{

std::string GetErrorMessage()
{
    DWORD errorCode     = GetLastError();
    LPSTR messageBuffer = nullptr;
    size_t size         = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    if (size == 0)
    {
        std::ostringstream stream;
        stream << "Failed to get the error message for '" << errorCode << "' due to the error '"
               << GetLastError() << "'";
        message = stream.str();
    }
    if (messageBuffer != nullptr)
    {
        LocalFree(messageBuffer);
    }
    return message;
}

}  // anonymous namespace

class FunctionsGLWindows : public FunctionsGL
{
  public:
    FunctionsGLWindows(HMODULE openGLModule, PFNWGLGETPROCADDRESSPROC getProcAddressWGL)
        : mOpenGLModule(openGLModule), mGetProcAddressWGL(getProcAddressWGL)
    {
        ASSERT(mOpenGLModule);
        ASSERT(mGetProcAddressWGL);
    }

    ~FunctionsGLWindows() override {}

  private:
    void *loadProcAddress(const std::string &function) const override
    {
        void *proc = reinterpret_cast<void *>(mGetProcAddressWGL(function.c_str()));
        if (!proc)
        {
            proc = reinterpret_cast<void *>(GetProcAddress(mOpenGLModule, function.c_str()));
        }
        return proc;
    }

    HMODULE mOpenGLModule;
    PFNWGLGETPROCADDRESSPROC mGetProcAddressWGL;
};

DisplayWGL::DisplayWGL(const egl::DisplayState &state)
    : DisplayGL(state),
      mRenderer(nullptr),
      mCurrentNativeContexts(),
      mOpenGLModule(nullptr),
      mFunctionsWGL(nullptr),
      mHasWGLCreateContextRobustness(false),
      mHasRobustness(false),
      mWindowClass(0),
      mWindow(nullptr),
      mDeviceContext(nullptr),
      mPixelFormat(0),
      mUseDXGISwapChains(false),
      mHasDXInterop(false),
      mDxgiModule(nullptr),
      mD3d11Module(nullptr),
      mD3D11DeviceHandle(nullptr),
      mD3D11Device(nullptr),
      mD3D11Device1(nullptr)
{}

DisplayWGL::~DisplayWGL() {}

egl::Error DisplayWGL::initialize(egl::Display *display)
{
    egl::Error error = initializeImpl(display);
    if (error.isError())
    {
        destroy();
        return error;
    }

    return DisplayGL::initialize(display);
}

egl::Error DisplayWGL::initializeImpl(egl::Display *display)
{
    mDisplayAttributes = display->getAttributeMap();

    mOpenGLModule = LoadLibraryExA("opengl32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!mOpenGLModule)
    {
        return egl::EglNotInitialized() << "Failed to load OpenGL library.";
    }

    mFunctionsWGL = new FunctionsWGL();
    mFunctionsWGL->initialize(mOpenGLModule, nullptr);

    // WGL can't grab extensions until it creates a context because it needs to load the driver's
    // DLLs first. Create a stub context to load the driver and determine which GL versions are
    // available.

    // Work around compile error from not defining "UNICODE" while Chromium does
    const LPWSTR idcArrow = MAKEINTRESOURCEW(32512);

    std::wostringstream stream;
    stream << L"ANGLE DisplayWGL " << gl::FmtHex<egl::Display *, wchar_t>(display)
           << L" Intermediate Window Class";
    std::wstring className = stream.str();

    WNDCLASSW intermediateClassDesc     = {};
    intermediateClassDesc.style         = CS_OWNDC;
    intermediateClassDesc.lpfnWndProc   = DefWindowProcW;
    intermediateClassDesc.cbClsExtra    = 0;
    intermediateClassDesc.cbWndExtra    = 0;
    intermediateClassDesc.hInstance     = GetModuleHandle(nullptr);
    intermediateClassDesc.hIcon         = nullptr;
    intermediateClassDesc.hCursor       = LoadCursorW(nullptr, idcArrow);
    intermediateClassDesc.hbrBackground = nullptr;
    intermediateClassDesc.lpszMenuName  = nullptr;
    intermediateClassDesc.lpszClassName = className.c_str();
    mWindowClass                        = RegisterClassW(&intermediateClassDesc);
    if (!mWindowClass)
    {
        return egl::EglNotInitialized() << "Failed to register intermediate OpenGL window class \""
                                        << gl::FmtHex<egl::Display *, char>(display)
                                        << "\":" << gl::FmtErr(HRESULT_CODE(GetLastError()));
    }

    HWND placeholderWindow =
        CreateWindowExW(0, reinterpret_cast<LPCWSTR>(mWindowClass), L"ANGLE Placeholder Window",
                        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                        CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
    if (!placeholderWindow)
    {
        return egl::EglNotInitialized() << "Failed to create placeholder OpenGL window.";
    }

    HDC placeholderDeviceContext = GetDC(placeholderWindow);
    if (!placeholderDeviceContext)
    {
        return egl::EglNotInitialized()
               << "Failed to get the device context of the placeholder OpenGL window.";
    }

    const PIXELFORMATDESCRIPTOR pixelFormatDescriptor = wgl::GetDefaultPixelFormatDescriptor();

    int placeholderPixelFormat =
        ChoosePixelFormat(placeholderDeviceContext, &pixelFormatDescriptor);
    if (placeholderPixelFormat == 0)
    {
        return egl::EglNotInitialized()
               << "Could not find a compatible pixel format for the placeholder OpenGL window.";
    }

    if (!SetPixelFormat(placeholderDeviceContext, placeholderPixelFormat, &pixelFormatDescriptor))
    {
        return egl::EglNotInitialized()
               << "Failed to set the pixel format on the intermediate OpenGL window.";
    }

    HGLRC placeholderWGLContext = mFunctionsWGL->createContext(placeholderDeviceContext);
    if (!placeholderDeviceContext)
    {
        return egl::EglNotInitialized()
               << "Failed to create a WGL context for the placeholder OpenGL window.";
    }

    if (!mFunctionsWGL->makeCurrent(placeholderDeviceContext, placeholderWGLContext))
    {
        return egl::EglNotInitialized() << "Failed to make the placeholder WGL context current.";
    }

    // Reinitialize the wgl functions to grab the extensions
    mFunctionsWGL->initialize(mOpenGLModule, placeholderDeviceContext);

    mHasWGLCreateContextRobustness =
        mFunctionsWGL->hasExtension("WGL_ARB_create_context_robustness");

    // Destroy the placeholder window and context
    mFunctionsWGL->makeCurrent(placeholderDeviceContext, nullptr);
    mFunctionsWGL->deleteContext(placeholderWGLContext);
    ReleaseDC(placeholderWindow, placeholderDeviceContext);
    DestroyWindow(placeholderWindow);

    const egl::AttributeMap &displayAttributes = display->getAttributeMap();
    EGLint requestedDisplayType                = static_cast<EGLint>(displayAttributes.get(
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE));
    if (requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE &&
        !mFunctionsWGL->hasExtension("WGL_EXT_create_context_es2_profile") &&
        !mFunctionsWGL->hasExtension("WGL_EXT_create_context_es_profile"))
    {
        return egl::EglNotInitialized() << "Cannot create an OpenGL ES platform on Windows without "
                                           "the WGL_EXT_create_context_es(2)_profile extension.";
    }

    // Create the real intermediate context and windows
    mWindow = CreateWindowExA(0, reinterpret_cast<const char *>(mWindowClass),
                              "ANGLE Intermediate Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr,
                              nullptr, nullptr);
    if (!mWindow)
    {
        return egl::EglNotInitialized() << "Failed to create intermediate OpenGL window.";
    }

    mDeviceContext = GetDC(mWindow);
    if (!mDeviceContext)
    {
        return egl::EglNotInitialized()
               << "Failed to get the device context of the intermediate OpenGL window.";
    }

    if (mFunctionsWGL->choosePixelFormatARB)
    {
        std::vector<int> attribs = wgl::GetDefaultPixelFormatAttributes(false);

        UINT matchingFormats = 0;
        mFunctionsWGL->choosePixelFormatARB(mDeviceContext, &attribs[0], nullptr, 1u, &mPixelFormat,
                                            &matchingFormats);
    }

    if (mPixelFormat == 0)
    {
        mPixelFormat = ChoosePixelFormat(mDeviceContext, &pixelFormatDescriptor);
    }

    if (mPixelFormat == 0)
    {
        return egl::EglNotInitialized()
               << "Could not find a compatible pixel format for the intermediate OpenGL window.";
    }

    if (!SetPixelFormat(mDeviceContext, mPixelFormat, &pixelFormatDescriptor))
    {
        return egl::EglNotInitialized()
               << "Failed to set the pixel format on the intermediate OpenGL window.";
    }

    ANGLE_TRY(createRenderer(&mRenderer));
    const FunctionsGL *functionsGL = mRenderer->getFunctions();

    mHasRobustness = functionsGL->getGraphicsResetStatus != nullptr;
    if (mHasWGLCreateContextRobustness != mHasRobustness)
    {
        WARN() << "WGL_ARB_create_context_robustness exists but unable to create a context with "
                  "robustness.";
    }

    // Intel OpenGL ES drivers are not currently supported due to bugs in the driver and ANGLE
    VendorID vendor = GetVendorID(functionsGL);
    if (requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE && IsIntel(vendor))
    {
        return egl::EglNotInitialized() << "Intel OpenGL ES drivers are not supported.";
    }

    // Create DXGI swap chains for windows that come from other processes.  Windows is unable to
    // SetPixelFormat on windows from other processes when a sandbox is enabled.
    HDC nativeDisplay = display->getNativeDisplayId();
    HWND nativeWindow = WindowFromDC(nativeDisplay);
    if (nativeWindow != nullptr)
    {
        DWORD currentProcessId = GetCurrentProcessId();
        DWORD windowProcessId;
        GetWindowThreadProcessId(nativeWindow, &windowProcessId);

        // AMD drivers advertise the WGL_NV_DX_interop and WGL_NV_DX_interop2 extensions but fail
        mUseDXGISwapChains = !IsAMD(vendor) && (currentProcessId != windowProcessId);
    }
    else
    {
        mUseDXGISwapChains = false;
    }

    mHasDXInterop = mFunctionsWGL->hasExtension("WGL_NV_DX_interop2");

    if (mUseDXGISwapChains)
    {
        if (mHasDXInterop)
        {
            ANGLE_TRY(initializeD3DDevice());
        }
        else
        {
            // Want to use DXGI swap chains but WGL_NV_DX_interop2 is not present, fail
            // initialization
            return egl::EglNotInitialized() << "WGL_NV_DX_interop2 is required but not present.";
        }
    }

    const gl::Version &maxVersion = mRenderer->getMaxSupportedESVersion();
    if (maxVersion < gl::Version(2, 0))
    {
        return egl::EglNotInitialized() << "OpenGL ES 2.0 is not supportable.";
    }

    return egl::NoError();
}

void DisplayWGL::terminate()
{
    DisplayGL::terminate();
    destroy();
}

void DisplayWGL::destroy()
{
    releaseD3DDevice(mD3D11DeviceHandle);

    mRenderer.reset();

    if (mFunctionsWGL)
    {
        if (mDeviceContext)
        {
            mFunctionsWGL->makeCurrent(mDeviceContext, nullptr);
        }
    }
    mCurrentNativeContexts.clear();

    SafeDelete(mFunctionsWGL);

    if (mDeviceContext)
    {
        ReleaseDC(mWindow, mDeviceContext);
        mDeviceContext = nullptr;
    }

    if (mWindow)
    {
        DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    if (mWindowClass)
    {
        if (!UnregisterClassA(reinterpret_cast<const char *>(mWindowClass),
                              GetModuleHandle(nullptr)))
        {
            WARN() << "Failed to unregister OpenGL window class: " << gl::FmtHex(mWindowClass);
        }
        mWindowClass = NULL;
    }

    if (mOpenGLModule)
    {
        FreeLibrary(mOpenGLModule);
        mOpenGLModule = nullptr;
    }

    SafeRelease(mD3D11Device);
    SafeRelease(mD3D11Device1);

    if (mDxgiModule)
    {
        FreeLibrary(mDxgiModule);
        mDxgiModule = nullptr;
    }

    if (mD3d11Module)
    {
        FreeLibrary(mD3d11Module);
        mD3d11Module = nullptr;
    }

    ASSERT(mRegisteredD3DDevices.empty());
}

SurfaceImpl *DisplayWGL::createWindowSurface(const egl::SurfaceState &state,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    EGLint orientation = static_cast<EGLint>(attribs.get(EGL_SURFACE_ORIENTATION_ANGLE, 0));
    // TODO(crbug.com/540829, anglebug.com/42266638) other orientations
    // are still unsupported, so allow fallback instead of crashing
    // later in eglCreateWindowSurface
    if (mUseDXGISwapChains && orientation == EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE)
    {
        egl::Error error = initializeD3DDevice();
        if (error.isError())
        {
            return nullptr;
        }

        return new DXGISwapChainWindowSurfaceWGL(
            state, mRenderer->getStateManager(), window, mD3D11Device, mD3D11DeviceHandle,
            mDeviceContext, mRenderer->getFunctions(), mFunctionsWGL, orientation);
    }
    else
    {
        return new WindowSurfaceWGL(state, window, mPixelFormat, mFunctionsWGL, orientation);
    }
}

SurfaceImpl *DisplayWGL::createPbufferSurface(const egl::SurfaceState &state,
                                              const egl::AttributeMap &attribs)
{
    EGLint width          = static_cast<EGLint>(attribs.get(EGL_WIDTH, 0));
    EGLint height         = static_cast<EGLint>(attribs.get(EGL_HEIGHT, 0));
    bool largest          = (attribs.get(EGL_LARGEST_PBUFFER, EGL_FALSE) == EGL_TRUE);
    EGLenum textureFormat = static_cast<EGLenum>(attribs.get(EGL_TEXTURE_FORMAT, EGL_NO_TEXTURE));
    EGLenum textureTarget = static_cast<EGLenum>(attribs.get(EGL_TEXTURE_TARGET, EGL_NO_TEXTURE));

    return new PbufferSurfaceWGL(state, width, height, textureFormat, textureTarget, largest,
                                 mPixelFormat, mDeviceContext, mFunctionsWGL);
}

SurfaceImpl *DisplayWGL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                       EGLenum buftype,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs)
{
    egl::Error error = initializeD3DDevice();
    if (error.isError())
    {
        return nullptr;
    }

    return new D3DTextureSurfaceWGL(state, mRenderer->getStateManager(), buftype, clientBuffer,
                                    this, mDeviceContext, mD3D11Device, mD3D11Device1,
                                    mRenderer->getFunctions(), mFunctionsWGL);
}

SurfaceImpl *DisplayWGL::createPixmapSurface(const egl::SurfaceState &state,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

rx::ContextImpl *DisplayWGL::createContext(const gl::State &state,
                                           gl::ErrorSet *errorSet,
                                           const egl::Config *configuration,
                                           const gl::Context *shareContext,
                                           const egl::AttributeMap &attribs)
{
    return new ContextWGL(state, errorSet, mRenderer);
}

egl::ConfigSet DisplayWGL::generateConfigs()
{
    egl::ConfigSet configs;

    int minSwapInterval = 1;
    int maxSwapInterval = 1;
    if (mFunctionsWGL->swapIntervalEXT)
    {
        // No defined maximum swap interval in WGL_EXT_swap_control, use a reasonable number
        minSwapInterval = 0;
        maxSwapInterval = 8;
    }

    const gl::Version &maxVersion = getMaxSupportedESVersion();
    ASSERT(maxVersion >= gl::Version(2, 0));
    bool supportsES3 = maxVersion >= gl::Version(3, 0);

    PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
    DescribePixelFormat(mDeviceContext, mPixelFormat, sizeof(pixelFormatDescriptor),
                        &pixelFormatDescriptor);

    auto getAttrib = [this](int attrib) {
        return wgl::QueryWGLFormatAttrib(mDeviceContext, mPixelFormat, attrib, mFunctionsWGL);
    };

    const EGLint optimalSurfaceOrientation =
        mUseDXGISwapChains ? EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE : 0;

    egl::Config config;
    config.renderTargetFormat = GL_RGBA8;  // TODO: use the bit counts to determine the format
    config.depthStencilFormat =
        GL_DEPTH24_STENCIL8;  // TODO: use the bit counts to determine the format
    config.bufferSize        = pixelFormatDescriptor.cColorBits;
    config.redSize           = pixelFormatDescriptor.cRedBits;
    config.greenSize         = pixelFormatDescriptor.cGreenBits;
    config.blueSize          = pixelFormatDescriptor.cBlueBits;
    config.luminanceSize     = 0;
    config.alphaSize         = pixelFormatDescriptor.cAlphaBits;
    config.alphaMaskSize     = 0;
    config.bindToTextureRGB  = (getAttrib(WGL_BIND_TO_TEXTURE_RGB_ARB) == TRUE);
    config.bindToTextureRGBA = (getAttrib(WGL_BIND_TO_TEXTURE_RGBA_ARB) == TRUE);
    config.colorBufferType   = EGL_RGB_BUFFER;
    config.configCaveat      = EGL_NONE;
    config.conformant        = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.depthSize         = pixelFormatDescriptor.cDepthBits;
    config.level             = 0;
    config.matchNativePixmap = EGL_NONE;
    config.maxPBufferWidth   = getAttrib(WGL_MAX_PBUFFER_WIDTH_ARB);
    config.maxPBufferHeight  = getAttrib(WGL_MAX_PBUFFER_HEIGHT_ARB);
    config.maxPBufferPixels  = getAttrib(WGL_MAX_PBUFFER_PIXELS_ARB);
    config.maxSwapInterval   = maxSwapInterval;
    config.minSwapInterval   = minSwapInterval;
    config.nativeRenderable  = EGL_TRUE;  // Direct rendering
    config.nativeVisualID    = 0;
    config.nativeVisualType  = EGL_NONE;
    config.renderableType    = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
    config.sampleBuffers     = 0;  // FIXME: enumerate multi-sampling
    config.samples           = 0;
    config.stencilSize       = pixelFormatDescriptor.cStencilBits;
    config.surfaceType =
        ((pixelFormatDescriptor.dwFlags & PFD_DRAW_TO_WINDOW) ? EGL_WINDOW_BIT : 0) |
        ((getAttrib(WGL_DRAW_TO_PBUFFER_ARB) == TRUE) ? EGL_PBUFFER_BIT : 0) |
        ((getAttrib(WGL_SWAP_METHOD_ARB) == WGL_SWAP_COPY_ARB) ? EGL_SWAP_BEHAVIOR_PRESERVED_BIT
                                                               : 0);
    config.optimalOrientation = optimalSurfaceOrientation;
    config.colorComponentType = EGL_COLOR_COMPONENT_TYPE_FIXED_EXT;

    config.transparentType       = EGL_NONE;
    config.transparentRedValue   = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue  = 0;

    configs.add(config);

    return configs;
}

bool DisplayWGL::testDeviceLost()
{
    return false;
}

egl::Error DisplayWGL::restoreLostDevice(const egl::Display *display)
{
    return egl::EglBadDisplay();
}

bool DisplayWGL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (IsWindow(window) == TRUE);
}

egl::Error DisplayWGL::validateClientBuffer(const egl::Config *configuration,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const
{
    switch (buftype)
    {
        case EGL_D3D_TEXTURE_ANGLE:
        case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
            ANGLE_TRY(const_cast<DisplayWGL *>(this)->initializeD3DDevice());
            return D3DTextureSurfaceWGL::ValidateD3DTextureClientBuffer(
                buftype, clientBuffer, mD3D11Device, mD3D11Device1);

        default:
            return DisplayGL::validateClientBuffer(configuration, buftype, clientBuffer, attribs);
    }
}

egl::Error DisplayWGL::initializeD3DDevice()
{
    if (mD3D11Device != nullptr)
    {
        return egl::NoError();
    }

    mDxgiModule = LoadLibrary(TEXT("dxgi.dll"));
    if (!mDxgiModule)
    {
        return egl::EglNotInitialized() << "Failed to load DXGI library.";
    }

    mD3d11Module = LoadLibrary(TEXT("d3d11.dll"));
    if (!mD3d11Module)
    {
        return egl::EglNotInitialized() << "Failed to load d3d11 library.";
    }

    PFN_D3D11_CREATE_DEVICE d3d11CreateDevice = nullptr;
    d3d11CreateDevice                         = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
        GetProcAddress(mD3d11Module, "D3D11CreateDevice"));
    if (d3d11CreateDevice == nullptr)
    {
        return egl::EglNotInitialized() << "Could not retrieve D3D11CreateDevice address.";
    }

    HRESULT result = d3d11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0,
                                       D3D11_SDK_VERSION, &mD3D11Device, nullptr, nullptr);
    if (FAILED(result))
    {
        return egl::EglNotInitialized() << "Could not create D3D11 device, " << gl::FmtHR(result);
    }

    mD3D11Device->QueryInterface(__uuidof(ID3D11Device1),
                                 reinterpret_cast<void **>(&mD3D11Device1));

    return registerD3DDevice(mD3D11Device, &mD3D11DeviceHandle);
}

void DisplayWGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    // Only enable the surface orientation  and post sub buffer for DXGI swap chain surfaces, they
    // prefer to swap with inverted Y.
    outExtensions->postSubBuffer      = mUseDXGISwapChains;
    outExtensions->surfaceOrientation = mUseDXGISwapChains;

    outExtensions->createContextRobustness = mHasRobustness;

    outExtensions->d3dTextureClientBuffer         = mHasDXInterop;
    outExtensions->d3dShareHandleClientBuffer     = mHasDXInterop;
    outExtensions->surfaceD3DTexture2DShareHandle = true;
    outExtensions->querySurfacePointer            = true;
    outExtensions->keyedMutex                     = true;

    // Contexts are virtualized so textures and semaphores can be shared globally
    outExtensions->displayTextureShareGroup   = true;
    outExtensions->displaySemaphoreShareGroup = true;

    outExtensions->surfacelessContext = true;

    DisplayGL::generateExtensions(outExtensions);
}

void DisplayWGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

egl::Error DisplayWGL::makeCurrentSurfaceless(gl::Context *context)
{
    // Nothing to do because WGL always uses the same context and the previous surface can be left
    // current.
    return egl::NoError();
}

egl::Error DisplayWGL::waitClient(const gl::Context *context)
{
    // Unimplemented as this is not needed for WGL
    return egl::NoError();
}

egl::Error DisplayWGL::waitNative(const gl::Context *context, EGLint engine)
{
    // Unimplemented as this is not needed for WGL
    return egl::NoError();
}

egl::Error DisplayWGL::makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context)
{
    CurrentNativeContext &currentContext =
        mCurrentNativeContexts[angle::GetCurrentThreadUniqueId()];

    HDC newDC = mDeviceContext;
    if (drawSurface)
    {
        SurfaceWGL *drawSurfaceWGL = GetImplAs<SurfaceWGL>(drawSurface);
        newDC                      = drawSurfaceWGL->getDC();
    }

    HGLRC newContext = currentContext.glrc;
    if (context)
    {
        ContextWGL *contextWGL = GetImplAs<ContextWGL>(context);
        newContext             = contextWGL->getContext();
    }
    else if (!mUseDXGISwapChains)
    {
        newContext = 0;
    }

    if (newDC != currentContext.dc || newContext != currentContext.glrc)
    {
        ASSERT(newDC != 0);

        if (!mFunctionsWGL->makeCurrent(newDC, newContext))
        {
            // TODO(geofflang): What error type here?
            return egl::EglContextLost() << "Failed to make the WGL context current.";
        }
        currentContext.dc   = newDC;
        currentContext.glrc = newContext;
    }

    return DisplayGL::makeCurrent(display, drawSurface, readSurface, context);
}

egl::Error DisplayWGL::registerD3DDevice(IUnknown *device, HANDLE *outHandle)
{
    ASSERT(device != nullptr);
    ASSERT(outHandle != nullptr);

    auto iter = mRegisteredD3DDevices.find(device);
    if (iter != mRegisteredD3DDevices.end())
    {
        iter->second.refCount++;
        *outHandle = iter->second.handle;
        return egl::NoError();
    }

    HANDLE handle = mFunctionsWGL->dxOpenDeviceNV(device);
    if (!handle)
    {
        return egl::EglBadParameter() << "Failed to open D3D device.";
    }

    device->AddRef();

    D3DObjectHandle newDeviceInfo;
    newDeviceInfo.handle          = handle;
    newDeviceInfo.refCount        = 1;
    mRegisteredD3DDevices[device] = newDeviceInfo;

    *outHandle = handle;
    return egl::NoError();
}

void DisplayWGL::releaseD3DDevice(HANDLE deviceHandle)
{
    for (auto iter = mRegisteredD3DDevices.begin(); iter != mRegisteredD3DDevices.end(); iter++)
    {
        if (iter->second.handle == deviceHandle)
        {
            iter->second.refCount--;
            if (iter->second.refCount == 0)
            {
                mFunctionsWGL->dxCloseDeviceNV(iter->second.handle);
                iter->first->Release();
                mRegisteredD3DDevices.erase(iter);
                break;
            }
        }
    }
}

gl::Version DisplayWGL::getMaxSupportedESVersion() const
{
    return mRenderer->getMaxSupportedESVersion();
}

void DisplayWGL::destroyNativeContext(HGLRC context)
{
    mFunctionsWGL->deleteContext(context);
}

HGLRC DisplayWGL::initializeContextAttribs(const egl::AttributeMap &eglAttributes) const
{
    EGLint requestedDisplayType = static_cast<EGLint>(
        eglAttributes.get(EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE));

    // Create a context of the requested version, if any.
    gl::Version requestedVersion(static_cast<EGLint>(eglAttributes.get(
                                     EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE)),
                                 static_cast<EGLint>(eglAttributes.get(
                                     EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE)));
    if (static_cast<EGLint>(requestedVersion.major) != EGL_DONT_CARE &&
        static_cast<EGLint>(requestedVersion.minor) != EGL_DONT_CARE)
    {
        int profileMask = 0;
        if (requestedDisplayType != EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE &&
            requestedVersion >= gl::Version(3, 2))
        {
            profileMask |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        }
        return createContextAttribs(requestedVersion, profileMask);
    }

    // Try all the GL version in order as a workaround for Mesa context creation where the driver
    // doesn't automatically return the highest version available.
    for (const auto &info : GenerateContextCreationToTry(requestedDisplayType, false))
    {
        int profileFlag = 0;
        if (info.type == ContextCreationTry::Type::DESKTOP_CORE)
        {
            profileFlag |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
        }
        else if (info.type == ContextCreationTry::Type::ES)
        {
            profileFlag |= WGL_CONTEXT_ES_PROFILE_BIT_EXT;
        }

        HGLRC context = createContextAttribs(info.version, profileFlag);
        if (context != nullptr)
        {
            return context;
        }
    }

    return nullptr;
}

HGLRC DisplayWGL::createContextAttribs(const gl::Version &version, int profileMask) const
{
    std::vector<int> attribs;

    if (mHasWGLCreateContextRobustness)
    {
        attribs.push_back(WGL_CONTEXT_FLAGS_ARB);
        attribs.push_back(WGL_CONTEXT_ROBUST_ACCESS_BIT_ARB);
        attribs.push_back(WGL_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB);
        attribs.push_back(WGL_LOSE_CONTEXT_ON_RESET_ARB);
    }

    attribs.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);
    attribs.push_back(version.major);

    attribs.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);
    attribs.push_back(version.minor);

    if (profileMask != 0)
    {
        attribs.push_back(WGL_CONTEXT_PROFILE_MASK_ARB);
        attribs.push_back(profileMask);
    }

    attribs.push_back(0);
    attribs.push_back(0);
    return mFunctionsWGL->createContextAttribsARB(mDeviceContext, nullptr, &attribs[0]);
}

egl::Error DisplayWGL::createRenderer(std::shared_ptr<RendererWGL> *outRenderer)
{
    HGLRC context = nullptr;

    if (mFunctionsWGL->createContextAttribsARB)
    {
        context = initializeContextAttribs(mDisplayAttributes);
    }

    // If wglCreateContextAttribsARB is unavailable or failed, try the standard wglCreateContext
    if (!context)
    {
        // Don't have control over GL versions
        context = mFunctionsWGL->createContext(mDeviceContext);
    }

    if (!context)
    {
        return egl::EglNotInitialized()
               << "Failed to create a WGL context for the intermediate OpenGL window."
               << GetErrorMessage();
    }

    if (!mFunctionsWGL->makeCurrent(mDeviceContext, context))
    {
        return egl::EglNotInitialized() << "Failed to make the intermediate WGL context current.";
    }
    CurrentNativeContext &currentContext =
        mCurrentNativeContexts[angle::GetCurrentThreadUniqueId()];
    currentContext.dc   = mDeviceContext;
    currentContext.glrc = context;

    std::unique_ptr<FunctionsGL> functionsGL(
        new FunctionsGLWindows(mOpenGLModule, mFunctionsWGL->getProcAddress));
    functionsGL->initialize(mDisplayAttributes);

    outRenderer->reset(new RendererWGL(std::move(functionsGL), mDisplayAttributes, this, context));

    return egl::NoError();
}

void DisplayWGL::initializeFrontendFeatures(angle::FrontendFeatures *features) const
{
    mRenderer->initializeFrontendFeatures(features);
}

void DisplayWGL::populateFeatureList(angle::FeatureList *features)
{
    mRenderer->getFeatures().populateFeatureList(features);
}

RendererGL *DisplayWGL::getRenderer() const
{
    return reinterpret_cast<RendererGL *>(mRenderer.get());
}

}  // namespace rx
