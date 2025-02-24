//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayNULL.cpp:
//    Implements the class methods for DisplayNULL.
//

#include "libANGLE/renderer/null/DisplayNULL.h"

#include "common/debug.h"

#include "libANGLE/Display.h"
#include "libANGLE/renderer/null/ContextNULL.h"
#include "libANGLE/renderer/null/DeviceNULL.h"
#include "libANGLE/renderer/null/ImageNULL.h"
#include "libANGLE/renderer/null/SurfaceNULL.h"

namespace rx
{

DisplayNULL::DisplayNULL(const egl::DisplayState &state) : DisplayImpl(state) {}

DisplayNULL::~DisplayNULL() {}

egl::Error DisplayNULL::initialize(egl::Display *display)
{
    constexpr size_t kMaxTotalAllocationSize = 1 << 28;  // 256MB
    mAllocationTracker.reset(new AllocationTrackerNULL(kMaxTotalAllocationSize));

    return egl::NoError();
}

void DisplayNULL::terminate()
{
    mAllocationTracker.reset();
}

egl::Error DisplayNULL::makeCurrent(egl::Display *display,
                                    egl::Surface *drawSurface,
                                    egl::Surface *readSurface,
                                    gl::Context *context)
{
    // Ensure that the correct global DebugAnnotator is installed when the end2end tests change
    // the ANGLE back-end (done frequently).
    display->setGlobalDebugAnnotator();

    return egl::NoError();
}

egl::ConfigSet DisplayNULL::generateConfigs()
{
    egl::Config config;
    config.renderTargetFormat    = GL_RGBA8;
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
    config.maxPBufferWidth       = 4096;
    config.maxPBufferHeight      = 4096;
    config.maxPBufferPixels      = 4096 * 4096;
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

bool DisplayNULL::testDeviceLost()
{
    return false;
}

egl::Error DisplayNULL::restoreLostDevice(const egl::Display *display)
{
    return egl::NoError();
}

bool DisplayNULL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayNULL::getRendererDescription()
{
    return "NULL";
}

std::string DisplayNULL::getVendorString()
{
    return "NULL";
}

std::string DisplayNULL::getVersionString(bool includeFullVersion)
{
    return std::string();
}

DeviceImpl *DisplayNULL::createDevice()
{
    return new DeviceNULL();
}

egl::Error DisplayNULL::waitClient(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error DisplayNULL::waitNative(const gl::Context *context, EGLint engine)
{
    return egl::NoError();
}

gl::Version DisplayNULL::getMaxSupportedESVersion() const
{
    return gl::Version(3, 2);
}

gl::Version DisplayNULL::getMaxConformantESVersion() const
{
    return getMaxSupportedESVersion();
}

SurfaceImpl *DisplayNULL::createWindowSurface(const egl::SurfaceState &state,
                                              EGLNativeWindowType window,
                                              const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPbufferSurface(const egl::SurfaceState &state,
                                               const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                        EGLenum buftype,
                                                        EGLClientBuffer buffer,
                                                        const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPixmapSurface(const egl::SurfaceState &state,
                                              NativePixmapType nativePixmap,
                                              const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

ImageImpl *DisplayNULL::createImage(const egl::ImageState &state,
                                    const gl::Context *context,
                                    EGLenum target,
                                    const egl::AttributeMap &attribs)
{
    return new ImageNULL(state);
}

rx::ContextImpl *DisplayNULL::createContext(const gl::State &state,
                                            gl::ErrorSet *errorSet,
                                            const egl::Config *configuration,
                                            const gl::Context *shareContext,
                                            const egl::AttributeMap &attribs)
{
    return new ContextNULL(state, errorSet, mAllocationTracker.get());
}

StreamProducerImpl *DisplayNULL::createStreamProducerD3DTexture(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ShareGroupImpl *DisplayNULL::createShareGroup(const egl::ShareGroupState &state)
{
    return new ShareGroupNULL(state);
}

void DisplayNULL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness            = true;
    outExtensions->postSubBuffer                      = true;
    outExtensions->createContext                      = true;
    outExtensions->image                              = true;
    outExtensions->imageBase                          = true;
    outExtensions->glTexture2DImage                   = true;
    outExtensions->glTextureCubemapImage              = true;
    outExtensions->glTexture3DImage                   = true;
    outExtensions->glRenderbufferImage                = true;
    outExtensions->getAllProcAddresses                = true;
    outExtensions->noConfigContext                    = true;
    outExtensions->directComposition                  = true;
    outExtensions->createContextNoError               = true;
    outExtensions->createContextWebGLCompatibility    = true;
    outExtensions->createContextBindGeneratesResource = true;
    outExtensions->swapBuffersWithDamage              = true;
    outExtensions->pixelFormatFloat                   = true;
    outExtensions->surfacelessContext                 = true;
    outExtensions->displayTextureShareGroup           = true;
    outExtensions->displaySemaphoreShareGroup         = true;
    outExtensions->createContextClientArrays          = true;
    outExtensions->programCacheControlANGLE           = true;
    outExtensions->robustResourceInitializationANGLE  = true;
}

void DisplayNULL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

}  // namespace rx
