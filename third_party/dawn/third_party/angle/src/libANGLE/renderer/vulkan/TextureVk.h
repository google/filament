//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureVk.h:
//    Defines the class interface for TextureVk, implementing TextureImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_TEXTUREVK_H_
#define LIBANGLE_RENDERER_VULKAN_TEXTUREVK_H_

#include "libANGLE/renderer/TextureImpl.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/SamplerVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_resource.h"

namespace rx
{

enum class TextureUpdateResult
{
    ImageUnaffected,
    ImageRespecified,
};

class TextureVk : public TextureImpl, public angle::ObserverInterface
{
  public:
    TextureVk(const gl::TextureState &state, vk::Renderer *renderer);
    ~TextureVk() override;
    void onDestroy(const gl::Context *context) override;

    angle::Result setImage(const gl::Context *context,
                           const gl::ImageIndex &index,
                           GLenum internalFormat,
                           const gl::Extents &size,
                           GLenum format,
                           GLenum type,
                           const gl::PixelUnpackState &unpack,
                           gl::Buffer *unpackBuffer,
                           const uint8_t *pixels) override;
    angle::Result setSubImage(const gl::Context *context,
                              const gl::ImageIndex &index,
                              const gl::Box &area,
                              GLenum format,
                              GLenum type,
                              const gl::PixelUnpackState &unpack,
                              gl::Buffer *unpackBuffer,
                              const uint8_t *pixels) override;

    angle::Result setCompressedImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     GLenum internalFormat,
                                     const gl::Extents &size,
                                     const gl::PixelUnpackState &unpack,
                                     size_t imageSize,
                                     const uint8_t *pixels) override;
    angle::Result setCompressedSubImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Box &area,
                                        GLenum format,
                                        const gl::PixelUnpackState &unpack,
                                        size_t imageSize,
                                        const uint8_t *pixels) override;

    angle::Result copyImage(const gl::Context *context,
                            const gl::ImageIndex &index,
                            const gl::Rectangle &sourceArea,
                            GLenum internalFormat,
                            gl::Framebuffer *source) override;
    angle::Result copySubImage(const gl::Context *context,
                               const gl::ImageIndex &index,
                               const gl::Offset &destOffset,
                               const gl::Rectangle &sourceArea,
                               gl::Framebuffer *source) override;

    angle::Result copyTexture(const gl::Context *context,
                              const gl::ImageIndex &index,
                              GLenum internalFormat,
                              GLenum type,
                              GLint sourceLevelGL,
                              bool unpackFlipY,
                              bool unpackPremultiplyAlpha,
                              bool unpackUnmultiplyAlpha,
                              const gl::Texture *source) override;
    angle::Result copySubTexture(const gl::Context *context,
                                 const gl::ImageIndex &index,
                                 const gl::Offset &destOffset,
                                 GLint sourceLevelGL,
                                 const gl::Box &sourceBox,
                                 bool unpackFlipY,
                                 bool unpackPremultiplyAlpha,
                                 bool unpackUnmultiplyAlpha,
                                 const gl::Texture *source) override;

    angle::Result copyRenderbufferSubData(const gl::Context *context,
                                          const gl::Renderbuffer *srcBuffer,
                                          GLint srcLevel,
                                          GLint srcX,
                                          GLint srcY,
                                          GLint srcZ,
                                          GLint dstLevel,
                                          GLint dstX,
                                          GLint dstY,
                                          GLint dstZ,
                                          GLsizei srcWidth,
                                          GLsizei srcHeight,
                                          GLsizei srcDepth) override;

    angle::Result copyTextureSubData(const gl::Context *context,
                                     const gl::Texture *srcTexture,
                                     GLint srcLevel,
                                     GLint srcX,
                                     GLint srcY,
                                     GLint srcZ,
                                     GLint dstLevel,
                                     GLint dstX,
                                     GLint dstY,
                                     GLint dstZ,
                                     GLsizei srcWidth,
                                     GLsizei srcHeight,
                                     GLsizei srcDepth) override;

    angle::Result copyCompressedTexture(const gl::Context *context,
                                        const gl::Texture *source) override;

    angle::Result clearImage(const gl::Context *context,
                             GLint level,
                             GLenum format,
                             GLenum type,
                             const uint8_t *data) override;

    angle::Result clearSubImage(const gl::Context *context,
                                GLint level,
                                const gl::Box &area,
                                GLenum format,
                                GLenum type,
                                const uint8_t *data) override;

    angle::Result setStorage(const gl::Context *context,
                             gl::TextureType type,
                             size_t levels,
                             GLenum internalFormat,
                             const gl::Extents &size) override;

    angle::Result setStorageExternalMemory(const gl::Context *context,
                                           gl::TextureType type,
                                           size_t levels,
                                           GLenum internalFormat,
                                           const gl::Extents &size,
                                           gl::MemoryObject *memoryObject,
                                           GLuint64 offset,
                                           GLbitfield createFlags,
                                           GLbitfield usageFlags,
                                           const void *imageCreateInfoPNext) override;

    angle::Result setEGLImageTarget(const gl::Context *context,
                                    gl::TextureType type,
                                    egl::Image *image) override;

    angle::Result setImageExternal(const gl::Context *context,
                                   gl::TextureType type,
                                   egl::Stream *stream,
                                   const egl::Stream::GLTextureDescription &desc) override;

    angle::Result setBuffer(const gl::Context *context, GLenum internalFormat) override;

    angle::Result generateMipmap(const gl::Context *context) override;

    angle::Result setBaseLevel(const gl::Context *context, GLuint baseLevel) override;

    angle::Result bindTexImage(const gl::Context *context, egl::Surface *surface) override;
    angle::Result releaseTexImage(const gl::Context *context) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    angle::Result syncState(const gl::Context *context,
                            const gl::Texture::DirtyBits &dirtyBits,
                            gl::Command source) override;

    angle::Result setStorageMultisample(const gl::Context *context,
                                        gl::TextureType type,
                                        GLsizei samples,
                                        GLint internalformat,
                                        const gl::Extents &size,
                                        bool fixedSampleLocations) override;

    angle::Result setStorageAttribs(const gl::Context *context,
                                    gl::TextureType type,
                                    size_t levels,
                                    GLint internalFormat,
                                    const gl::Extents &size,
                                    const GLint *attribList) override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    angle::Result initializeContentsWithBlack(const gl::Context *context,
                                              GLenum binding,
                                              const gl::ImageIndex &imageIndex);

    GLint getRequiredExternalTextureImageUnits([[maybe_unused]] const gl::Context *context) override
    {
        // For now, we assume that only one image unit is needed to support
        // external GL textures in the Vulkan backend.
        return 1;
    }

    const vk::ImageHelper &getImage() const
    {
        ASSERT(mImage && mImage->valid());
        return *mImage;
    }

    vk::ImageHelper &getImage()
    {
        ASSERT(mImage && mImage->valid());
        return *mImage;
    }

    void retainBufferViews(vk::CommandBufferHelperCommon *commandBufferHelper)
    {
        commandBufferHelper->retainResource(&mBufferViews);
    }

    bool isImmutable() { return mState.getImmutableFormat(); }
    bool imageValid() const { return (mImage && mImage->valid()); }

    void releaseOwnershipOfImage(const gl::Context *context);

    const vk::ImageView &getReadImageView(GLenum srgbDecode,
                                          bool texelFetchStaticUse,
                                          bool samplerExternal2DY2YEXT) const;

    angle::Result getBufferView(vk::ErrorContext *context,
                                const vk::Format *imageUniformFormat,
                                const gl::SamplerBinding *samplerBinding,
                                bool isImage,
                                const vk::BufferView **viewOut);

    // A special view used for texture copies that shouldn't perform swizzle.
    const vk::ImageView &getCopyImageView() const;
    angle::Result getStorageImageView(vk::ErrorContext *context,
                                      const gl::ImageUnit &binding,
                                      const vk::ImageView **imageViewOut);

    const vk::SamplerHelper &getSampler(bool isSamplerExternalY2Y) const
    {
        if (isSamplerExternalY2Y)
        {
            ASSERT(mY2YSampler->valid());
            return *mY2YSampler.get();
        }
        ASSERT(mSampler->valid());
        return *mSampler.get();
    }

    void resetSampler()
    {
        mSampler.reset();
        mY2YSampler.reset();
    }

    // Normally, initialize the image with enabled mipmap level counts.
    angle::Result ensureImageInitialized(ContextVk *contextVk, ImageMipLevels mipLevels);

    vk::ImageOrBufferViewSubresourceSerial getImageViewSubresourceSerial(
        const gl::SamplerState &samplerState,
        bool staticTexelFetchAccess) const
    {
        ASSERT(mImage != nullptr);
        gl::SrgbDecode srgbDecode = (samplerState.getSRGBDecode() == GL_SKIP_DECODE_EXT)
                                        ? gl::SrgbDecode::Skip
                                        : gl::SrgbDecode::Default;
        mImageView.updateSrgbDecode(*mImage, srgbDecode);
        mImageView.updateStaticTexelFetch(*mImage, staticTexelFetchAccess);

        if (mImageView.getColorspaceForRead() == vk::ImageViewColorspace::SRGB)
        {
            ASSERT(getImageViewSubresourceSerialImpl(vk::ImageViewColorspace::SRGB) ==
                   mCachedImageViewSubresourceSerialSRGBDecode);
            return mCachedImageViewSubresourceSerialSRGBDecode;
        }
        else
        {
            ASSERT(getImageViewSubresourceSerialImpl(vk::ImageViewColorspace::Linear) ==
                   mCachedImageViewSubresourceSerialSkipDecode);
            return mCachedImageViewSubresourceSerialSkipDecode;
        }
    }

    vk::ImageOrBufferViewSubresourceSerial getBufferViewSerial() const;
    vk::ImageOrBufferViewSubresourceSerial getStorageImageViewSerial(
        const gl::ImageUnit &binding) const;

    GLenum getColorReadFormat(const gl::Context *context) override;
    GLenum getColorReadType(const gl::Context *context) override;

    angle::Result getTexImage(const gl::Context *context,
                              const gl::PixelPackState &packState,
                              gl::Buffer *packBuffer,
                              gl::TextureTarget target,
                              GLint level,
                              GLenum format,
                              GLenum type,
                              void *pixels) override;

    angle::Result getCompressedTexImage(const gl::Context *context,
                                        const gl::PixelPackState &packState,
                                        gl::Buffer *packBuffer,
                                        gl::TextureTarget target,
                                        GLint level,
                                        void *pixels) override;

    ANGLE_INLINE bool hasBeenBoundAsImage() const { return mState.hasBeenBoundAsImage(); }
    ANGLE_INLINE const gl::OffsetBindingPointer<gl::Buffer> &getBuffer() const
    {
        return mState.getBuffer();
    }
    vk::BufferHelper *getPossiblyEmulatedTextureBuffer(vk::ErrorContext *context) const;

    bool isSRGBOverrideEnabled() const
    {
        return mState.getSRGBOverride() != gl::SrgbOverride::Default;
    }

    angle::Result updateSrgbDecodeState(ContextVk *contextVk, const gl::SamplerState &samplerState)
    {
        ASSERT(mImage != nullptr && mImage->valid());
        gl::SrgbDecode srgbDecode = (samplerState.getSRGBDecode() == GL_SKIP_DECODE_EXT)
                                        ? gl::SrgbDecode::Skip
                                        : gl::SrgbDecode::Default;
        mImageView.updateSrgbDecode(*mImage, srgbDecode);
        if (mImageView.hasColorspaceOverrideForRead(*mImage))
        {
            ANGLE_TRY(ensureMutable(contextVk));
        }
        return angle::Result::Continue;
    }

    angle::Result ensureRenderable(ContextVk *contextVk, TextureUpdateResult *updateResultOut);

    bool getAndResetImmutableSamplerDirtyState()
    {
        bool isDirty           = mImmutableSamplerDirty;
        mImmutableSamplerDirty = false;
        return isDirty;
    }

    angle::Result onLabelUpdate(const gl::Context *context) override;

    void onNewDescriptorSet(const vk::SharedDescriptorSetCacheKey &sharedCacheKey)
    {
        mDescriptorSetCacheManager.addKey(sharedCacheKey);
    }

    // Check if the texture is consistently specified. Used for flushing mutable textures.
    bool isMutableTextureConsistentlySpecifiedForFlush();
    bool isMipImageDescDefined(gl::TextureTarget textureTarget, size_t level);

    GLint getImageCompressionRate(const gl::Context *context) override;
    GLint getFormatSupportedCompressionRates(const gl::Context *context,
                                             GLenum internalformat,
                                             GLsizei bufSize,
                                             GLint *rates) override;

  private:
    // Transform an image index from the frontend into one that can be used on the backing
    // ImageHelper, taking into account mipmap or cube face offsets
    gl::ImageIndex getNativeImageIndex(const gl::ImageIndex &inputImageIndex) const;
    gl::LevelIndex getNativeImageLevel(gl::LevelIndex frontendLevel) const;
    uint32_t getNativeImageLayer(uint32_t frontendLayer) const;

    // Get the layer count for views.
    uint32_t getImageViewLayerCount() const;
    // Get the level count for views.
    uint32_t getImageViewLevelCount() const;

    void releaseAndDeleteImageAndViews(ContextVk *contextVk);
    angle::Result ensureImageAllocated(ContextVk *contextVk, const vk::Format &format);
    void setImageHelper(ContextVk *contextVk,
                        vk::ImageHelper *imageHelper,
                        gl::TextureType imageType,
                        uint32_t imageLevelOffset,
                        uint32_t imageLayerOffset,
                        bool selfOwned,
                        UniqueSerial siblingSerial);

    vk::ImageViewHelper &getImageViews() { return mImageView; }
    const vk::ImageViewHelper &getImageViews() const { return mImageView; }

    angle::Result ensureRenderableWithFormat(ContextVk *contextVk,
                                             const vk::Format &format,
                                             TextureUpdateResult *updateResultOut);
    angle::Result ensureRenderableIfCopyTextureCannotTransfer(ContextVk *contextVk,
                                                              const gl::InternalFormat &dstFormat,
                                                              bool unpackFlipY,
                                                              bool unpackPremultiplyAlpha,
                                                              bool unpackUnmultiplyAlpha,
                                                              TextureVk *source);
    angle::Result ensureRenderableIfCopyTexImageCannotTransfer(
        ContextVk *contextVk,
        const gl::InternalFormat &internalFormat,
        gl::Framebuffer *source);

    // Redefine a mip level of the texture.  If the new size and format don't match the allocated
    // image, the image may be released.  When redefining a mip of a multi-level image, updates are
    // forced to be staged, as another mip of the image may be bound to a framebuffer.  For example,
    // assume texture has two mips, and framebuffer is bound to mip 0.  Redefining mip 1 to an
    // incompatible size shouldn't affect the framebuffer, especially if the redefinition comes from
    // something like glCopyTexSubImage2D() (which simultaneously is reading from said framebuffer,
    // i.e. mip 0 of the texture).
    angle::Result redefineLevel(const gl::Context *context,
                                const gl::ImageIndex &index,
                                const vk::Format &format,
                                const gl::Extents &size);

    angle::Result setImageImpl(const gl::Context *context,
                               const gl::ImageIndex &index,
                               const gl::InternalFormat &formatInfo,
                               const gl::Extents &size,
                               GLenum type,
                               const gl::PixelUnpackState &unpack,
                               gl::Buffer *unpackBuffer,
                               const uint8_t *pixels);
    angle::Result setSubImageImpl(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  const gl::Box &area,
                                  const gl::InternalFormat &formatInfo,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  gl::Buffer *unpackBuffer,
                                  const uint8_t *pixels,
                                  const vk::Format &vkFormat);

    // Used to clear a texture to a given value in part or whole.
    angle::Result clearSubImageImpl(const gl::Context *context,
                                    GLint level,
                                    const gl::Box &clearArea,
                                    vk::ClearTextureMode clearMode,
                                    GLenum format,
                                    GLenum type,
                                    const uint8_t *data);

    angle::Result ensureImageInitializedIfUpdatesNeedStageOrFlush(ContextVk *contextVk,
                                                                  gl::LevelIndex level,
                                                                  const vk::Format &vkFormat,
                                                                  vk::ApplyImageUpdate applyUpdate,
                                                                  bool usesBufferForUpdate);

    angle::Result copyImageDataToBufferAndGetData(ContextVk *contextVk,
                                                  gl::LevelIndex sourceLevelGL,
                                                  uint32_t layerCount,
                                                  const gl::Box &sourceArea,
                                                  RenderPassClosureReason reason,
                                                  vk::BufferHelper *copyBuffer,
                                                  uint8_t **outDataPtr);

    angle::Result copyBufferDataToImage(ContextVk *contextVk,
                                        vk::BufferHelper *srcBuffer,
                                        const gl::ImageIndex index,
                                        uint32_t rowLength,
                                        uint32_t imageHeight,
                                        const gl::Box &sourceArea,
                                        size_t offset,
                                        VkImageAspectFlags aspectFlags);

    // Called from syncState to prepare the image for mipmap generation.
    void prepareForGenerateMipmap(ContextVk *contextVk);

    // Generate mipmaps from level 0 into the rest of the mips.  This requires the image to have
    // STORAGE usage.
    angle::Result generateMipmapsWithCompute(ContextVk *contextVk);

    angle::Result generateMipmapsWithCPU(const gl::Context *context);

    angle::Result generateMipmapLevelsWithCPU(ContextVk *contextVk,
                                              const angle::Format &sourceFormat,
                                              GLuint layer,
                                              gl::LevelIndex firstMipLevel,
                                              gl::LevelIndex maxMipLevel,
                                              const size_t sourceWidth,
                                              const size_t sourceHeight,
                                              const size_t sourceDepth,
                                              const size_t sourceRowPitch,
                                              const size_t sourceDepthPitch,
                                              uint8_t *sourceData);

    angle::Result copySubImageImpl(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   const gl::Offset &destOffset,
                                   const gl::Rectangle &sourceArea,
                                   const gl::InternalFormat &internalFormat,
                                   gl::Framebuffer *source);

    angle::Result copySubTextureImpl(ContextVk *contextVk,
                                     const gl::ImageIndex &index,
                                     const gl::Offset &dstOffset,
                                     const gl::InternalFormat &dstFormat,
                                     gl::LevelIndex sourceLevelGL,
                                     const gl::Box &sourceBox,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha,
                                     TextureVk *source);

    angle::Result copySubImageImplWithTransfer(ContextVk *contextVk,
                                               const gl::ImageIndex &index,
                                               const gl::Offset &dstOffset,
                                               const vk::Format &dstFormat,
                                               gl::LevelIndex sourceLevelGL,
                                               size_t sourceLayer,
                                               const gl::Box &sourceBox,
                                               vk::ImageHelper *srcImage);

    angle::Result copySubImageImplWithDraw(ContextVk *contextVk,
                                           const gl::ImageIndex &index,
                                           const gl::Offset &dstOffset,
                                           const vk::Format &dstFormat,
                                           gl::LevelIndex sourceLevelGL,
                                           const gl::Box &sourceBox,
                                           bool isSrcFlipY,
                                           bool unpackFlipY,
                                           bool unpackPremultiplyAlpha,
                                           bool unpackUnmultiplyAlpha,
                                           vk::ImageHelper *srcImage,
                                           const vk::ImageView *srcView,
                                           SurfaceRotation srcFramebufferRotation);

    angle::Result initImage(ContextVk *contextVk,
                            angle::FormatID intendedImageFormatID,
                            angle::FormatID actualImageFormatID,
                            ImageMipLevels mipLevels);
    void releaseImage(ContextVk *contextVk);
    void releaseImageViews(ContextVk *contextVk);
    void releaseStagedUpdates(ContextVk *contextVk);
    uint32_t getMipLevelCount(ImageMipLevels mipLevels) const;
    uint32_t getMaxLevelCount() const;
    angle::Result copyAndStageImageData(ContextVk *contextVk,
                                        gl::LevelIndex previousFirstAllocateLevel,
                                        vk::ImageHelper *srcImage,
                                        vk::ImageHelper *dstImage);
    angle::Result reinitImageAsRenderable(ContextVk *contextVk, const vk::Format &format);
    angle::Result initImageViews(ContextVk *contextVk, uint32_t levelCount);
    void initSingleLayerRenderTargets(ContextVk *contextVk,
                                      GLuint layerCount,
                                      gl::LevelIndex levelIndexGL,
                                      gl::RenderToTextureImageIndex renderToTextureIndex);
    RenderTargetVk *getMultiLayerRenderTarget(ContextVk *contextVk,
                                              gl::LevelIndex level,
                                              GLuint layerIndex,
                                              GLuint layerCount);
    angle::Result getLevelLayerImageView(vk::ErrorContext *context,
                                         gl::LevelIndex levelGL,
                                         size_t layer,
                                         const vk::ImageView **imageViewOut);

    // Flush image's staged updates for all levels and layers.
    angle::Result flushImageStagedUpdates(ContextVk *contextVk);

    // For various reasons, the underlying image may need to be respecified.  For example because
    // base/max level changed, usage/create flags have changed, the format needs modification to
    // become renderable, generate mipmap is adding levels, etc.  This function is called by
    // syncState and getAttachmentRenderTarget.  The latter calls this function to be able to sync
    // the texture's image while an attached framebuffer is being synced.  Note that we currently
    // sync framebuffers before textures so that the deferred clear optimization works.
    angle::Result respecifyImageStorageIfNecessary(ContextVk *contextVk, gl::Command source);

    const gl::InternalFormat &getImplementationSizedFormat(const gl::Context *context) const;
    const vk::Format &getBaseLevelFormat(vk::Renderer *renderer) const;
    // Queues a flush of any modified image attributes. The image will be reallocated with its new
    // attributes at the next opportunity.
    angle::Result respecifyImageStorage(ContextVk *contextVk);

    // Update base and max levels, and re-create image if needed.
    angle::Result maybeUpdateBaseMaxLevels(ContextVk *contextVk,
                                           TextureUpdateResult *changeResultOut);

    bool isFastUnpackPossible(const vk::Format &vkFormat,
                              size_t offset,
                              const vk::Format &bufferVkFormat) const;

    bool updateMustBeStaged(gl::LevelIndex textureLevelIndexGL, angle::FormatID dstFormatID) const;
    bool updateMustBeFlushed(gl::LevelIndex textureLevelIndexGL, angle::FormatID dstFormatID) const;
    bool shouldUpdateBeFlushed(gl::LevelIndex textureLevelIndexGL,
                               angle::FormatID dstFormatID) const
    {
        return updateMustBeFlushed(textureLevelIndexGL, dstFormatID) ||
               !updateMustBeStaged(textureLevelIndexGL, dstFormatID);
    }

    // We monitor the staging buffer and set dirty bits if the staging buffer changes. Note that we
    // support changes in the staging buffer even outside the TextureVk class.
    void onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message) override;

    ANGLE_INLINE VkImageTiling getTilingMode()
    {
        return mImage != nullptr && mImage->valid() ? mImage->getTilingMode()
                                                    : VK_IMAGE_TILING_OPTIMAL;
    }

    angle::Result ensureMutable(ContextVk *contextVk);
    angle::Result refreshImageViews(ContextVk *contextVk);
    void initImageUsageFlags(ContextVk *contextVk, angle::FormatID actualFormatID);
    void handleImmutableSamplerTransition(const vk::ImageHelper *previousImage,
                                          const vk::ImageHelper *nextImage);

    vk::ImageAccess getRequiredImageAccess() const { return mRequiredImageAccess; }

    void stageSelfAsSubresourceUpdates(ContextVk *contextVk);

    vk::ImageOrBufferViewSubresourceSerial getImageViewSubresourceSerialImpl(
        vk::ImageViewColorspace colorspace) const;

    void updateCachedImageViewSerials();

    angle::Result updateTextureLabel(ContextVk *contextVk);

    vk::BufferHelper *getRGBAConversionBufferHelper(vk::Renderer *renderer,
                                                    angle::FormatID formatID) const;
    angle::Result convertBufferToRGBA(ContextVk *contextVk, size_t &conversionBufferSize);
    bool isCompressedFormatEmulated(const gl::Context *context,
                                    const gl::TextureTarget target,
                                    GLint level);

    angle::Result setStorageImpl(ContextVk *contextVk,
                                 gl::TextureType type,
                                 const vk::Format &format);

    GLint getFormatSupportedCompressionRatesImpl(vk::Renderer *renderer,
                                                 const vk::Format &format,
                                                 GLsizei bufSize,
                                                 GLint *rates);

    bool mOwnsImage;
    // Generated from ImageVk if EGLImage target, or from throw-away generator if Surface target.
    UniqueSerial mImageSiblingSerial;

    bool mRequiresMutableStorage;
    vk::ImageAccess mRequiredImageAccess;
    bool mImmutableSamplerDirty;

    // Only valid if this texture is an "EGLImage target" and the associated EGL Image was
    // originally sourced from an OpenGL texture. Such EGL Images can be a slice of the underlying
    // resource. The layer and level offsets are used to track the location of the slice.
    gl::TextureType mEGLImageNativeType;
    uint32_t mEGLImageLayerOffset;
    uint32_t mEGLImageLevelOffset;

    // If multisampled rendering to texture, an intermediate multisampled image is created for use
    // as renderpass color attachment. A map of an array of images and image views are used where -
    //
    // The map is keyed based on the number of samples used with multisampled rendering to texture.
    // Index 0 corresponds to the non-multisampled-render-to-texture usage of the texture.
    // - index 0: Unused.  See description of |mImage|.
    // - index N: intermediate multisampled image used for multisampled rendering to texture with
    //            1 << N samples
    //
    // Each element in the array corresponds to a mip-level
    //
    // - mMultisampledImages[N][M]: intermediate multisampled image with 1 << N samples
    //                              for level index M
    using MultiSampleImages = gl::RenderToTextureImageMap<gl::TexLevelArray<vk::ImageHelper>>;
    std::unique_ptr<MultiSampleImages> mMultisampledImages;

    // If multisampled rendering to texture, contains views for mMultisampledImages.
    //
    // - index 0: Unused.  See description of |mImageView|.
    // - mMultisampledImageViews[N][M]: views for mMultisampledImages[N][M]
    using MultiSampleImageViews =
        gl::RenderToTextureImageMap<gl::TexLevelArray<vk::ImageViewHelper>>;
    std::unique_ptr<MultiSampleImageViews> mMultisampledImageViews;

    // Texture buffers create texel buffer views instead.  |BufferViewHelper| contains the views
    // corresponding to the attached buffer range.
    vk::BufferViewHelper mBufferViews;

    // Render targets stored as array of vector of vectors
    //
    // - First dimension: index N contains render targets with views from mMultisampledImageViews[N]
    // - Second dimension: level M contains render targets with views from
    // mMultisampledImageViews[N][M]
    // - Third dimension: layer
    gl::RenderToTextureImageMap<std::vector<RenderTargetVector>> mSingleLayerRenderTargets;
    // Multi-layer render targets stored as a hash map.  This is used for layered attachments
    // which covers the entire layer (glFramebufferTextureLayer) or multiview attachments which
    // cover a range of layers (glFramebufferTextureMultiviewOVR).
    angle::HashMap<vk::ImageSubresourceRange, std::unique_ptr<RenderTargetVk>>
        mMultiLayerRenderTargets;

    // |mImage| wraps a VkImage and VkDeviceMemory that represents the gl::Texture. |mOwnsImage|
    // indicates that |TextureVk| owns the image. Otherwise it is a weak pointer shared with another
    // class. Due to this sharing, for example through EGL images, the image must always be
    // dynamically allocated as the texture can release ownership for example and it can be
    // transferred to another |TextureVk|.
    vk::ImageHelper *mImage;
    // The view is always owned by the Texture and is not shared like |mImage|. It also has
    // different lifetimes and can be reallocated independently of |mImage| on state changes.
    vk::ImageViewHelper mImageView;

    // |mSampler| contains the relevant Vulkan sampler states representing the OpenGL Texture
    // sampling states for the Texture.
    vk::SharedSamplerPtr mSampler;
    // |mY2YSampler| contains a version of mSampler that is meant for use with
    // __samplerExternal2DY2YEXT (i.e., skipping conversion of YUV to RGB).
    vk::SharedSamplerPtr mY2YSampler;

    // The created vkImage usage flag.
    VkImageUsageFlags mImageUsageFlags;

    // Additional image create flags
    VkImageCreateFlags mImageCreateFlags;

    // If an image level is incompatibly redefined, the image lives through the call that did this
    // (i.e. set and copy levels), because the image may be used by the framebuffer in the very same
    // call.  As a result, updates to this redefined level are staged (in both the call that
    // redefines it, and any future calls such as subimage updates).  This array flags redefined
    // levels so that their updates will be force-staged until image is recreated.  Each member of
    // the array is a bitmask per level, and it's an array of cube faces because GL allows
    // redefining each cube map face separately.  For other texture types, only index 0 is
    // meaningful as all array levels are redefined simultaneously.
    //
    // In common cases with mipmapped textures, the base/max level would need adjusting as the
    // texture is no longer mip-complete.  However, if every level is redefined such that at the end
    // the image becomes mip-complete again, no reinitialization of the image is done.  This array
    // is additionally used to ensure the image is recreated in the next syncState, if not already.
    //
    // Note: the elements of this array are bitmasks indexed by gl::LevelIndex, not vk::LevelIndex
    gl::CubeFaceArray<gl::TexLevelMask> mRedefinedLevels;

    angle::ObserverBinding mImageObserverBinding;

    // Saved between updates.
    gl::LevelIndex mCurrentBaseLevel;
    gl::LevelIndex mCurrentMaxLevel;

    // Cached subresource indexes.
    vk::ImageOrBufferViewSubresourceSerial mCachedImageViewSubresourceSerialSRGBDecode;
    vk::ImageOrBufferViewSubresourceSerial mCachedImageViewSubresourceSerialSkipDecode;

    // Manages the texture descriptor set cache that created with this texture
    vk::DescriptorSetCacheManager mDescriptorSetCacheManager;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_TEXTUREVK_H_
