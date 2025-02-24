//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayMtl.h:
//    Defines the class interface for DisplayMtl, implementing DisplayImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_DISPLAYMTL_H_
#define LIBANGLE_RENDERER_METAL_DISPLAYMTL_H_

#include "common/PackedEnums.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/ShareGroupImpl.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_library_cache.h"
#include "libANGLE/renderer/metal/mtl_render_utils.h"
#include "libANGLE/renderer/metal/mtl_state_cache.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
#include "platform/autogen/FeaturesMtl_autogen.h"

namespace egl
{
class Surface;
}

namespace rx
{
class ShareGroupMtl : public ShareGroupImpl
{
  public:
    ShareGroupMtl(const egl::ShareGroupState &state) : ShareGroupImpl(state) {}
};

class ContextMtl;

struct DefaultShaderAsyncInfoMtl;

class DisplayMtl : public DisplayImpl
{
  public:
    DisplayMtl(const egl::DisplayState &state);
    ~DisplayMtl() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    egl::Display *getDisplay() const { return mDisplay; }

    bool testDeviceLost() override;
    egl::Error restoreLostDevice(const egl::Display *display) override;

    std::string getRendererDescription() override;
    std::string getVendorString() override;
    std::string getVersionString(bool includeFullVersion) override;

    DeviceImpl *createDevice() override;

    egl::Error waitClient(const gl::Context *context) override;
    egl::Error waitNative(const gl::Context *context, EGLint engine) override;

    egl::Error waitUntilWorkScheduled() override;

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

    ImageImpl *createImage(const egl::ImageState &state,
                           const gl::Context *context,
                           EGLenum target,
                           const egl::AttributeMap &attribs) override;

    ContextImpl *createContext(const gl::State &state,
                               gl::ErrorSet *errorSet,
                               const egl::Config *configuration,
                               const gl::Context *shareContext,
                               const egl::AttributeMap &attribs) override;

    StreamProducerImpl *createStreamProducerD3DTexture(egl::Stream::ConsumerType consumerType,
                                                       const egl::AttributeMap &attribs) override;

    ShareGroupImpl *createShareGroup(const egl::ShareGroupState &state) override;

    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;
    gl::Version getMaxSupportedESVersion() const override;
    gl::Version getMaxConformantESVersion() const override;

    EGLSyncImpl *createSync() override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    void populateFeatureList(angle::FeatureList *features) override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    egl::Error validateClientBuffer(const egl::Config *configuration,
                                    EGLenum buftype,
                                    EGLClientBuffer clientBuffer,
                                    const egl::AttributeMap &attribs) const override;

    egl::Error validateImageClientBuffer(const gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer clientBuffer,
                                         const egl::AttributeMap &attribs) const override;

    egl::ConfigSet generateConfigs() override;

    gl::Caps getNativeCaps() const;
    const gl::TextureCapsMap &getNativeTextureCaps() const;
    const gl::Extensions &getNativeExtensions() const;
    const gl::Limitations &getNativeLimitations() const;
    const ShPixelLocalStorageOptions &getNativePixelLocalStorageOptions() const;
    const angle::FeaturesMtl &getFeatures() const { return mFeatures; }

    // Check whether either of the specified iOS or Mac GPU family is supported
    bool supportsEitherGPUFamily(uint8_t iOSFamily, uint8_t macFamily) const;
    bool supportsAppleGPUFamily(uint8_t iOSFamily) const;
    bool supportsMacGPUFamily(uint8_t macFamily) const;
    bool supportsDepth24Stencil8PixelFormat() const;
    bool supports32BitFloatFiltering() const;
    bool supportsBCTextureCompression() const;
    bool isAMD() const;
    bool isAMDBronzeDriver() const;
    bool isAMDFireProDevice() const;
    bool isIntel() const;
    bool isNVIDIA() const;
    bool isSimulator() const;

    id<MTLDevice> getMetalDevice() const { return mMetalDevice; }

    mtl::CommandQueue &cmdQueue() { return mCmdQueue; }
    const mtl::FormatTable &getFormatTable() const { return mFormatTable; }
    mtl::RenderUtils &getUtils() { return *mUtils; }
    mtl::StateCache &getStateCache() { return mStateCache; }
    mtl::LibraryCache &getLibraryCache() { return mLibraryCache; }
    uint32_t getMaxColorTargetBits() { return mMaxColorTargetBits; }
    bool hasFragmentMemoryBarriers() const { return mHasFragmentMemoryBarriers; }

    id<MTLLibrary> getDefaultShadersLib();

    const mtl::Format &getPixelFormat(angle::FormatID angleFormatId) const
    {
        return mFormatTable.getPixelFormat(angleFormatId);
    }
    const mtl::FormatCaps &getNativeFormatCaps(MTLPixelFormat mtlFormat) const
    {
        return mFormatTable.getNativeFormatCaps(mtlFormat);
    }

    // See mtl::FormatTable::getVertexFormat()
    const mtl::VertexFormat &getVertexFormat(angle::FormatID angleFormatId,
                                             bool tightlyPacked) const
    {
        return mFormatTable.getVertexFormat(angleFormatId, tightlyPacked);
    }

    mtl::AutoObjCPtr<MTLSharedEventListener *> getOrCreateSharedEventListener();

  protected:
    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

  private:
    angle::Result initializeImpl(egl::Display *display);
    void ensureCapsInitialized() const;
    void initializeCaps() const;
    void initializeExtensions() const;
    void initializeTextureCaps() const;
    void initializeFeatures();
    void initializeLimitations();
    EGLenum EGLDrawingBufferTextureTarget();
    mtl::AutoObjCPtr<id<MTLDevice>> getMetalDeviceMatchingAttribute(
        const egl::AttributeMap &attribs);
    angle::Result initializeShaderLibrary();

    egl::Display *mDisplay;

    mtl::AutoObjCPtr<id<MTLDevice>> mMetalDevice = nil;
    uint32_t mMetalDeviceVendorId                = 0;

    // Expensive-to-compute AMD Bronze driver detection
    mutable bool mComputedAMDBronze = false;
    mutable bool mIsAMDBronze       = false;

    mtl::CommandQueue mCmdQueue;

    mutable mtl::FormatTable mFormatTable;
    mtl::StateCache mStateCache;
    mtl::LibraryCache mLibraryCache;
    std::unique_ptr<mtl::RenderUtils> mUtils;

    // Built-in Shaders
    mtl::AutoObjCPtr<id<MTLLibrary>> mDefaultShaders;
    mtl::AutoObjCPtr<MTLSharedEventListener *> mSharedEventListener;

    mutable bool mCapsInitialized;
    mutable gl::TextureCapsMap mNativeTextureCaps;
    mutable gl::Extensions mNativeExtensions;
    mutable gl::Caps mNativeCaps;
    mutable gl::Limitations mNativeLimitations;
    mutable ShPixelLocalStorageOptions mNativePLSOptions;
    mutable uint32_t mMaxColorTargetBits = 0;
    mutable bool mHasFragmentMemoryBarriers;

    angle::FeaturesMtl mFeatures;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_DISPLAYMTL_H_ */
