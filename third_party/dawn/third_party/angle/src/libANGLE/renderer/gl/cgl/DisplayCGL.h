//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayCGL.h: CGL implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_CGL_DISPLAYCGL_H_
#define LIBANGLE_RENDERER_GL_CGL_DISPLAYCGL_H_

#include <unordered_set>

#include "libANGLE/renderer/gl/DisplayGL.h"

struct _CGLContextObject;
typedef _CGLContextObject *CGLContextObj;

struct _CGLPixelFormatObject;
typedef _CGLPixelFormatObject *CGLPixelFormatObj;

namespace rx
{

struct EnsureCGLContextIsCurrent : angle::NonCopyable
{
  public:
    EnsureCGLContextIsCurrent(CGLContextObj context);
    ~EnsureCGLContextIsCurrent();

  private:
    CGLContextObj mOldContext;
    bool mResetContext;
};

class DisplayCGL : public DisplayGL
{
  public:
    DisplayCGL(const egl::DisplayState &state);
    ~DisplayCGL() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;
    egl::Error prepareForCall() override;
    egl::Error releaseThread() override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    SurfaceImpl *createWindowSurface(const egl::SurfaceState &state,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::SurfaceState &state,
                                      const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                               EGLenum buftype,
                                               EGLClientBuffer clientBuffer,
                                               const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPixmapSurface(const egl::SurfaceState &state,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override;

    ContextImpl *createContext(const gl::State &state,
                               gl::ErrorSet *errorSet,
                               const egl::Config *configuration,
                               const gl::Context *shareContext,
                               const egl::AttributeMap &attribs) override;

    egl::ConfigSet generateConfigs() override;

    bool testDeviceLost() override;
    egl::Error restoreLostDevice(const egl::Display *display) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;
    egl::Error validateClientBuffer(const egl::Config *configuration,
                                    EGLenum buftype,
                                    EGLClientBuffer clientBuffer,
                                    const egl::AttributeMap &attribs) const override;

    DeviceImpl *createDevice() override;

    egl::Error waitClient(const gl::Context *context) override;
    egl::Error waitUntilWorkScheduled() override;
    egl::Error waitNative(const gl::Context *context, EGLint engine) override;

    gl::Version getMaxSupportedESVersion() const override;

    CGLContextObj getCGLContext() const;
    CGLPixelFormatObj getCGLPixelFormat() const;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    void populateFeatureList(angle::FeatureList *features) override;

    RendererGL *getRenderer() const override;

    // Support for dual-GPU MacBook Pros. Used only by ContextCGL. The use of
    // these entry points is gated by the presence of dual GPUs.
    egl::Error referenceDiscreteGPU();
    egl::Error unreferenceDiscreteGPU();
    egl::Error handleGPUSwitch() override;
    egl::Error forceGPUSwitch(EGLint gpuIDHigh, EGLint gpuIDLow) override;

  private:
    egl::Error makeCurrentSurfaceless(gl::Context *context) override;

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    void checkDiscreteGPUStatus();
    void setContextToGPU(uint64_t gpuID, GLint virtualScreen);

    std::shared_ptr<RendererGL> mRenderer;

    egl::Display *mEGLDisplay;
    CGLContextObj mContext;
    std::unordered_set<uint64_t> mThreadsWithCurrentContext;
    CGLPixelFormatObj mPixelFormat;
    bool mSupportsGPUSwitching;
    uint64_t mCurrentGPUID;
    CGLPixelFormatObj mDiscreteGPUPixelFormat;
    int mDiscreteGPURefs;
    // This comes from the ANGLE platform's DefaultMonotonicallyIncreasingTime. If the discrete GPU
    // is unref'd for the last time, this is set to the time of that last unref. If it isn't
    // activated again in 10 seconds, the discrete GPU pixel format is deleted.
    double mLastDiscreteGPUUnrefTime;
    bool mDeviceContextIsVolatile = false;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_CGL_DISPLAYCGL_H_
