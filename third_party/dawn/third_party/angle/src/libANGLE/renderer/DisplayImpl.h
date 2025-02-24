//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.h: Implementation methods of egl::Display

#ifndef LIBANGLE_RENDERER_DISPLAYIMPL_H_
#define LIBANGLE_RENDERER_DISPLAYIMPL_H_

#include "common/Optional.h"
#include "common/angleutils.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Config.h"
#include "libANGLE/Error.h"
#include "libANGLE/Observer.h"
#include "libANGLE/Stream.h"
#include "libANGLE/Version.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "platform/Feature.h"

#include <set>
#include <vector>

namespace angle
{
struct FrontendFeatures;
}  // namespace angle

namespace egl
{
class AttributeMap;
class BlobCache;
class Display;
struct DisplayState;
struct Config;
class Surface;
class ImageSibling;
class Thread;
}  // namespace egl

namespace gl
{
class Context;
}  // namespace gl

namespace rx
{
class SurfaceImpl;
class ImageImpl;
struct ConfigDesc;
class DeviceImpl;
class StreamProducerImpl;

class DisplayImpl : public EGLImplFactory, public angle::Subject
{
  public:
    DisplayImpl(const egl::DisplayState &state);
    ~DisplayImpl() override;

    virtual egl::Error initialize(egl::Display *display) = 0;
    virtual void terminate()                             = 0;
    virtual egl::Error prepareForCall();
    virtual egl::Error releaseThread();

    virtual egl::Error makeCurrent(egl::Display *display,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface,
                                   gl::Context *context) = 0;

    virtual egl::ConfigSet generateConfigs() = 0;

    virtual bool testDeviceLost()                                     = 0;
    virtual egl::Error restoreLostDevice(const egl::Display *display) = 0;

    virtual bool isValidNativeWindow(EGLNativeWindowType window) const = 0;
    virtual egl::Error validateClientBuffer(const egl::Config *configuration,
                                            EGLenum buftype,
                                            EGLClientBuffer clientBuffer,
                                            const egl::AttributeMap &attribs) const;
    virtual egl::Error validateImageClientBuffer(const gl::Context *context,
                                                 EGLenum target,
                                                 EGLClientBuffer clientBuffer,
                                                 const egl::AttributeMap &attribs) const;
    virtual egl::Error validatePixmap(const egl::Config *config,
                                      EGLNativePixmapType pixmap,
                                      const egl::AttributeMap &attributes) const;

    virtual std::string getRendererDescription()                  = 0;
    virtual std::string getVendorString()                         = 0;
    virtual std::string getVersionString(bool includeFullVersion) = 0;

    virtual DeviceImpl *createDevice();

    virtual egl::Error waitClient(const gl::Context *context)                = 0;
    virtual egl::Error waitNative(const gl::Context *context, EGLint engine) = 0;
    virtual gl::Version getMaxSupportedESVersion() const                     = 0;
    virtual gl::Version getMaxConformantESVersion() const                    = 0;
    const egl::Caps &getCaps() const;

    virtual void setBlobCacheFuncs(EGLSetBlobFuncANDROID set, EGLGetBlobFuncANDROID get) {}

    const egl::DisplayExtensions &getExtensions() const;

    void setBlobCache(egl::BlobCache *blobCache) { mBlobCache = blobCache; }
    egl::BlobCache *getBlobCache() const { return mBlobCache; }

    virtual void initializeFrontendFeatures(angle::FrontendFeatures *features) const {}

    virtual void populateFeatureList(angle::FeatureList *features) = 0;

    const egl::DisplayState &getState() const { return mState; }

    virtual egl::Error handleGPUSwitch();
    virtual egl::Error forceGPUSwitch(EGLint gpuIDHigh, EGLint gpuIDLow);

    virtual egl::Error waitUntilWorkScheduled();

    virtual void lockVulkanQueue() { UNREACHABLE(); }
    virtual void unlockVulkanQueue() { UNREACHABLE(); }

    virtual angle::NativeWindowSystem getWindowSystem() const;

    virtual bool supportsDmaBufFormat(EGLint format) const;
    virtual egl::Error queryDmaBufFormats(EGLint max_formats, EGLint *formats, EGLint *num_formats);
    virtual egl::Error queryDmaBufModifiers(EGLint format,
                                            EGLint max_modifiers,
                                            EGLuint64KHR *modifiers,
                                            EGLBoolean *external_only,
                                            EGLint *num_modifiers);

    virtual egl::Error querySupportedCompressionRates(const egl::Config *configuration,
                                                      const egl::AttributeMap &attributes,
                                                      EGLint *rates,
                                                      EGLint rate_size,
                                                      EGLint *num_rates) const;

  protected:
    const egl::DisplayState &mState;

  private:
    virtual void generateExtensions(egl::DisplayExtensions *outExtensions) const = 0;
    virtual void generateCaps(egl::Caps *outCaps) const                          = 0;

    mutable bool mExtensionsInitialized;
    mutable egl::DisplayExtensions mExtensions;

    mutable bool mCapsInitialized;
    mutable egl::Caps mCaps;

    egl::BlobCache *mBlobCache;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_DISPLAYIMPL_H_
