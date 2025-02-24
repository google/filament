//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVk.h:
//    Defines the class interface for DisplayVk, implementing DisplayImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPLAYVK_H_
#define LIBANGLE_RENDERER_VULKAN_DISPLAYVK_H_

#include "common/MemoryBuffer.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
class DisplayVk : public DisplayImpl, public vk::ErrorContext, public vk::GlobalOps
{
  public:
    DisplayVk(const egl::DisplayState &state);
    ~DisplayVk() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    egl::Error makeCurrent(egl::Display *display,
                           egl::Surface *drawSurface,
                           egl::Surface *readSurface,
                           gl::Context *context) override;

    bool testDeviceLost() override;
    egl::Error restoreLostDevice(const egl::Display *display) override;

    std::string getRendererDescription() override;
    std::string getVendorString() override;
    std::string getVersionString(bool includeFullVersion) override;

    DeviceImpl *createDevice() override;

    egl::Error waitClient(const gl::Context *context) override;
    egl::Error waitNative(const gl::Context *context, EGLint engine) override;

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

    EGLSyncImpl *createSync() override;

    gl::Version getMaxSupportedESVersion() const override;
    gl::Version getMaxConformantESVersion() const override;

    egl::Error validateImageClientBuffer(const gl::Context *context,
                                         EGLenum target,
                                         EGLClientBuffer clientBuffer,
                                         const egl::AttributeMap &attribs) const override;
    ExternalImageSiblingImpl *createExternalImageSibling(const gl::Context *context,
                                                         EGLenum target,
                                                         EGLClientBuffer buffer,
                                                         const egl::AttributeMap &attribs) override;
    virtual const char *getWSIExtension() const = 0;
    virtual const char *getWSILayer() const;

    // Determine if a config with given formats and sample counts is supported.  This callback may
    // modify the config to add or remove platform specific attributes such as nativeVisualID.  If
    // the config is not supported by the window system, it removes the EGL_WINDOW_BIT from
    // surfaceType, which would still allow the config to be used for pbuffers.
    virtual void checkConfigSupport(egl::Config *config) = 0;

    angle::ScratchBuffer *getScratchBuffer() { return &mScratchBuffer; }

    void handleError(VkResult result,
                     const char *file,
                     const char *function,
                     unsigned int line) override;

    void initializeFrontendFeatures(angle::FrontendFeatures *features) const override;

    void populateFeatureList(angle::FeatureList *features) override;

    ShareGroupImpl *createShareGroup(const egl::ShareGroupState &state) override;

    bool isConfigFormatSupported(VkFormat format) const;
    bool isSurfaceFormatColorspacePairSupported(VkSurfaceKHR surface,
                                                VkFormat format,
                                                VkColorSpaceKHR colorspace) const;

    void lockVulkanQueue() override;
    void unlockVulkanQueue() override;

    egl::Error querySupportedCompressionRates(const egl::Config *configuration,
                                              const egl::AttributeMap &attributes,
                                              EGLint *rates,
                                              EGLint rate_size,
                                              EGLint *num_rates) const override;

  protected:
    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;

  private:
    virtual SurfaceImpl *createWindowSurfaceVk(const egl::SurfaceState &state,
                                               EGLNativeWindowType window) = 0;
    void generateCaps(egl::Caps *outCaps) const override;

    virtual angle::Result waitNativeImpl();

    bool isColorspaceSupported(VkColorSpaceKHR colorspace) const;
    void initSupportedSurfaceFormatColorspaces();

    // vk::GlobalOps
    void putBlob(const angle::BlobCacheKey &key, const angle::MemoryBuffer &value) override;
    bool getBlob(const angle::BlobCacheKey &key, angle::BlobCacheValue *valueOut) override;
    std::shared_ptr<angle::WaitableEvent> postMultiThreadWorkerTask(
        const std::shared_ptr<angle::Closure> &task) override;
    void notifyDeviceLost() override;

    angle::ScratchBuffer mScratchBuffer;

    // Map of supported colorspace and associated surface format set.
    angle::HashMap<VkColorSpaceKHR, std::unordered_set<VkFormat>> mSupportedColorspaceFormatsMap;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPLAYVK_H_
