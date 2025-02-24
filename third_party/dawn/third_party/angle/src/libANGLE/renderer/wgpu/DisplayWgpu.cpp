//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayWgpu.cpp:
//    Implements the class methods for DisplayWgpu.
//

#include "libANGLE/renderer/wgpu/DisplayWgpu.h"

#include <dawn/dawn_proc.h>

#include "common/debug.h"
#include "common/platform.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/DeviceWgpu.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu_api.h"
#include "libANGLE/renderer/wgpu/ImageWgpu.h"
#include "libANGLE/renderer/wgpu/SurfaceWgpu.h"

namespace rx
{

DisplayWgpu::DisplayWgpu(const egl::DisplayState &state) : DisplayImpl(state) {}

DisplayWgpu::~DisplayWgpu() {}

egl::Error DisplayWgpu::initialize(egl::Display *display)
{
    ANGLE_TRY(createWgpuDevice());

    mQueue = mDevice.GetQueue();
    mFormatTable.initialize();

    wgpu::SupportedLimits supportedLimits;
    mDevice.GetLimits(&supportedLimits);
    mLimitsWgpu = supportedLimits.limits;

    webgpu::GenerateCaps(mLimitsWgpu, &mGLCaps, &mGLTextureCaps, &mGLExtensions, &mGLLimitations,
                         &mEGLCaps, &mEGLExtensions, &mMaxSupportedClientVersion);

    return egl::NoError();
}

void DisplayWgpu::terminate() {}

egl::Error DisplayWgpu::makeCurrent(egl::Display *display,
                                    egl::Surface *drawSurface,
                                    egl::Surface *readSurface,
                                    gl::Context *context)
{
    // Ensure that the correct global DebugAnnotator is installed when the end2end tests change
    // the ANGLE back-end (done frequently).
    display->setGlobalDebugAnnotator();

    return egl::NoError();
}

egl::ConfigSet DisplayWgpu::generateConfigs()
{
    egl::Config config;
    config.renderTargetFormat    = GL_BGRA8_EXT;
    config.depthStencilFormat    = GL_DEPTH24_STENCIL8;
    config.bufferSize            = 32;
    config.redSize               = 8;
    config.greenSize             = 8;
    config.blueSize              = 8;
    config.alphaSize             = 8;
    config.alphaMaskSize         = 0;
    config.bindToTextureRGB      = EGL_TRUE;
    config.bindToTextureRGBA     = EGL_TRUE;
    config.colorBufferType       = EGL_RGB_BUFFER;
    config.configCaveat          = EGL_NONE;
    config.conformant            = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT;
    config.depthSize             = 24;
    config.level                 = 0;
    config.matchNativePixmap     = EGL_NONE;
    config.maxPBufferWidth       = 0;
    config.maxPBufferHeight      = 0;
    config.maxPBufferPixels      = 0;
    config.maxSwapInterval       = 1;
    config.minSwapInterval       = 1;
    config.nativeRenderable      = EGL_TRUE;
    config.nativeVisualID        = 0;
    config.nativeVisualType      = EGL_NONE;
    config.renderableType        = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT;
    config.sampleBuffers         = 0;
    config.samples               = 0;
    config.stencilSize           = 8;
    config.surfaceType           = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
    config.optimalOrientation    = 0;
    config.transparentType       = EGL_NONE;
    config.transparentRedValue   = 0;
    config.transparentGreenValue = 0;
    config.transparentBlueValue  = 0;

    egl::ConfigSet configSet;
    configSet.add(config);
    return configSet;
}

bool DisplayWgpu::testDeviceLost()
{
    return false;
}

egl::Error DisplayWgpu::restoreLostDevice(const egl::Display *display)
{
    return egl::NoError();
}

bool DisplayWgpu::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayWgpu::getRendererDescription()
{
    return "Wgpu";
}

std::string DisplayWgpu::getVendorString()
{
    return "Wgpu";
}

std::string DisplayWgpu::getVersionString(bool includeFullVersion)
{
    return std::string();
}

DeviceImpl *DisplayWgpu::createDevice()
{
    return new DeviceWgpu();
}

egl::Error DisplayWgpu::waitClient(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error DisplayWgpu::waitNative(const gl::Context *context, EGLint engine)
{
    return egl::NoError();
}

gl::Version DisplayWgpu::getMaxSupportedESVersion() const
{
    return mMaxSupportedClientVersion;
}

gl::Version DisplayWgpu::getMaxConformantESVersion() const
{
    return mMaxSupportedClientVersion;
}

SurfaceImpl *DisplayWgpu::createWindowSurface(const egl::SurfaceState &state,
                                              EGLNativeWindowType window,
                                              const egl::AttributeMap &attribs)
{
    return CreateWgpuWindowSurface(state, window);
}

SurfaceImpl *DisplayWgpu::createPbufferSurface(const egl::SurfaceState &state,
                                               const egl::AttributeMap &attribs)
{
    return new OffscreenSurfaceWgpu(state);
}

SurfaceImpl *DisplayWgpu::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                        EGLenum buftype,
                                                        EGLClientBuffer buffer,
                                                        const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayWgpu::createPixmapSurface(const egl::SurfaceState &state,
                                              NativePixmapType nativePixmap,
                                              const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ImageImpl *DisplayWgpu::createImage(const egl::ImageState &state,
                                    const gl::Context *context,
                                    EGLenum target,
                                    const egl::AttributeMap &attribs)
{
    return new ImageWgpu(state);
}

rx::ContextImpl *DisplayWgpu::createContext(const gl::State &state,
                                            gl::ErrorSet *errorSet,
                                            const egl::Config *configuration,
                                            const gl::Context *shareContext,
                                            const egl::AttributeMap &attribs)
{
    return new ContextWgpu(state, errorSet, this);
}

StreamProducerImpl *DisplayWgpu::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ShareGroupImpl *DisplayWgpu::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupWgpu(state);
}

angle::NativeWindowSystem DisplayWgpu::getWindowSystem() const
{
#if defined(ANGLE_PLATFORM_LINUX)
#    if defined(ANGLE_USE_X11)
    return angle::NativeWindowSystem::X11;
#    elif defined(ANGLE_USE_WAYLAND)
    return angle::NativeWindowSystem::Wayland;
#    endif
#else
    return angle::NativeWindowSystem::Other;
#endif
}

void DisplayWgpu::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    *outExtensions = mEGLExtensions;
}

void DisplayWgpu::generateCaps(egl::Caps *outCaps) const
{
    *outCaps = mEGLCaps;
}

egl::Error DisplayWgpu::createWgpuDevice()
{
    dawnProcSetProcs(&dawn::native::GetProcs());

    dawn::native::DawnInstanceDescriptor dawnInstanceDescriptor;

    wgpu::InstanceDescriptor instanceDescriptor;
    instanceDescriptor.features.timedWaitAnyEnable = true;
    instanceDescriptor.nextInChain                 = &dawnInstanceDescriptor;
    mInstance                                      = wgpu::CreateInstance(&instanceDescriptor);

    struct RequestAdapterResult
    {
        WGPURequestAdapterStatus status;
        wgpu::Adapter adapter;
        std::string message;
    };
    RequestAdapterResult adapterResult;

    wgpu::RequestAdapterOptions requestAdapterOptions;

    wgpu::RequestAdapterCallbackInfo callbackInfo;
    callbackInfo.mode     = wgpu::CallbackMode::WaitAnyOnly;
    callbackInfo.callback = [](WGPURequestAdapterStatus status, WGPUAdapter adapter,
                               char const *message, void *userdata) {
        RequestAdapterResult *result = reinterpret_cast<RequestAdapterResult *>(userdata);
        result->status               = status;
        result->adapter              = wgpu::Adapter::Acquire(adapter);
        result->message              = message ? message : "";
    };
    callbackInfo.userdata = &adapterResult;

    wgpu::FutureWaitInfo futureWaitInfo;
    futureWaitInfo.future = mInstance.RequestAdapter(&requestAdapterOptions, callbackInfo);

    wgpu::WaitStatus status = mInstance.WaitAny(1, &futureWaitInfo, -1);
    if (webgpu::IsWgpuError(status))
    {
        return egl::EglBadAlloc() << "Failed to get WebGPU adapter: " << adapterResult.message;
    }

    mAdapter = adapterResult.adapter;

    std::vector<wgpu::FeatureName> requiredFeatures;  // empty for now

    wgpu::DeviceDescriptor deviceDesc;
    deviceDesc.requiredFeatureCount = requiredFeatures.size();
    deviceDesc.requiredFeatures     = requiredFeatures.data();

    mDevice = mAdapter.CreateDevice(&deviceDesc);
    mDevice.SetUncapturedErrorCallback(
        [](WGPUErrorType type, const char *message, void *userdata) {
            ERR() << "Error: " << type << " - message: " << message;
        },
        nullptr);
    return egl::NoError();
}

DisplayImpl *CreateWgpuDisplay(const egl::DisplayState &state)
{
    return new DisplayWgpu(state);
}

}  // namespace rx
