//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureMtl.h:
//    Defines the class interface for TextureMtl, implementing TextureImpl.
//

#ifndef LIBANGLE_RENDERER_METAL_TEXTUREMTL_H_
#define LIBANGLE_RENDERER_METAL_TEXTUREMTL_H_

#include <map>

#include "common/PackedEnums.h"
#include "libANGLE/renderer/TextureImpl.h"
#include "libANGLE/renderer/metal/RenderTargetMtl.h"
#include "libANGLE/renderer/metal/SurfaceMtl.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_context_device.h"
#include "libANGLE/renderer/metal/mtl_resources.h"
namespace rx
{

// structure represents one image definition of a texture created by glTexImage* call.
struct ImageDefinitionMtl
{
    mtl::TextureRef image;
    angle::FormatID formatID = angle::FormatID::NONE;
};

class TextureMtl : public TextureImpl
{
  public:
    using TextureViewVector           = std::vector<mtl::TextureRef>;
    using LayerLevelTextureViewVector = std::vector<TextureViewVector>;

    TextureMtl(const gl::TextureState &state);
    // Texture  view
    TextureMtl(const TextureMtl &mtl, GLenum format);
    ~TextureMtl() override;
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
                              GLint sourceLevel,
                              bool unpackFlipY,
                              bool unpackPremultiplyAlpha,
                              bool unpackUnmultiplyAlpha,
                              const gl::Texture *source) override;
    angle::Result copySubTexture(const gl::Context *context,
                                 const gl::ImageIndex &index,
                                 const gl::Offset &destOffset,
                                 GLint sourceLevel,
                                 const gl::Box &sourceBox,
                                 bool unpackFlipY,
                                 bool unpackPremultiplyAlpha,
                                 bool unpackUnmultiplyAlpha,
                                 const gl::Texture *source) override;

    angle::Result copyCompressedTexture(const gl::Context *context,
                                        const gl::Texture *source) override;

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

    angle::Result setStorageMultisample(const gl::Context *context,
                                        gl::TextureType type,
                                        GLsizei samples,
                                        GLint internalFormat,
                                        const gl::Extents &size,
                                        bool fixedSampleLocations) override;

    angle::Result setEGLImageTarget(const gl::Context *context,
                                    gl::TextureType type,
                                    egl::Image *image) override;

    angle::Result setImageExternal(const gl::Context *context,
                                   gl::TextureType type,
                                   egl::Stream *stream,
                                   const egl::Stream::GLTextureDescription &desc) override;

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

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    // The texture's data is initially initialized and stored in an array
    // of images through glTexImage*/glCopyTex* calls. During draw calls, the caller must make sure
    // the actual texture is created by calling this method to transfer the stored images data
    // to the actual texture.
    angle::Result ensureNativeStorageCreated(const gl::Context *context);

    angle::Result bindToShader(const gl::Context *context,
                               mtl::RenderCommandEncoder *cmdEncoder,
                               gl::ShaderType shaderType,
                               gl::Sampler *sampler, /** nullable */
                               int textureSlotIndex,
                               int samplerSlotIndex);

    angle::Result bindToShaderImage(const gl::Context *context,
                                    mtl::RenderCommandEncoder *cmdEncoder,
                                    gl::ShaderType shaderType,
                                    int textureSlotIndex,
                                    int level,
                                    int layer,
                                    GLenum format);

    const mtl::Format &getFormat() const { return mFormat; }

  private:
    void deallocateNativeStorage(bool keepImages, bool keepSamplerStateAndFormat = false);
    angle::Result createNativeStorage(const gl::Context *context,
                                      gl::TextureType type,
                                      GLuint mips,
                                      GLuint samples,
                                      const gl::Extents &size);
    angle::Result onBaseMaxLevelsChanged(const gl::Context *context);
    angle::Result ensureSamplerStateCreated(const gl::Context *context);
    // Ensure image at given index is created:
    angle::Result ensureImageCreated(const gl::Context *context, const gl::ImageIndex &index);
    // Ensure all image views at all faces/levels are retained.
    void retainImageDefinitions();
    mtl::TextureRef createImageViewFromTextureStorage(GLuint cubeFaceOrZero, GLuint glLevel);
    angle::Result createViewFromBaseToMaxLevel();
    angle::Result ensureLevelViewsWithinBaseMaxCreated();
    angle::Result checkForEmulatedChannels(const gl::Context *context,
                                           const mtl::Format &mtlFormat,
                                           const mtl::TextureRef &texture);
    mtl::TextureRef &getImage(const gl::ImageIndex &imageIndex);
    ImageDefinitionMtl &getImageDefinition(const gl::ImageIndex &imageIndex);
    angle::Result getRenderTarget(ContextMtl *context,
                                  const gl::ImageIndex &imageIndex,
                                  GLsizei implicitSamples,
                                  RenderTargetMtl **renderTargetOut);
    mtl::TextureRef &getImplicitMSTexture(const gl::ImageIndex &imageIndex);

    angle::Result setStorageImpl(const gl::Context *context,
                                 gl::TextureType type,
                                 GLuint mips,
                                 GLuint samples,
                                 const mtl::Format &mtlFormat,
                                 const gl::Extents &size);

    angle::Result redefineImage(const gl::Context *context,
                                const gl::ImageIndex &index,
                                const mtl::Format &mtlFormat,
                                const gl::Extents &size);

    angle::Result setImageImpl(const gl::Context *context,
                               const gl::ImageIndex &index,
                               const gl::InternalFormat &dstFormatInfo,
                               const gl::Extents &size,
                               GLenum srcFormat,
                               GLenum srcType,
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
                                  const uint8_t *pixels);

    angle::Result copySubImageImpl(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   const gl::Offset &destOffset,
                                   const gl::Rectangle &sourceArea,
                                   const gl::InternalFormat &internalFormat,
                                   const FramebufferMtl *source,
                                   const RenderTargetMtl *sourceRtt);
    angle::Result copySubImageWithDraw(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::Offset &destOffset,
                                       const gl::Rectangle &sourceArea,
                                       const gl::InternalFormat &internalFormat,
                                       const FramebufferMtl *source,
                                       const RenderTargetMtl *sourceRtt);
    angle::Result copySubImageCPU(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  const gl::Offset &destOffset,
                                  const gl::Rectangle &sourceArea,
                                  const gl::InternalFormat &internalFormat,
                                  const FramebufferMtl *source,
                                  const RenderTargetMtl *sourceRtt);

    angle::Result copySubTextureImpl(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Offset &destOffset,
                                     const gl::InternalFormat &internalFormat,
                                     GLint sourceLevel,
                                     const gl::Box &sourceBox,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha,
                                     const gl::Texture *source);

    angle::Result copySubTextureWithDraw(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         const gl::Offset &destOffset,
                                         const gl::InternalFormat &internalFormat,
                                         const mtl::MipmapNativeLevel &sourceNativeLevel,
                                         const gl::Box &sourceBox,
                                         const angle::Format &sourceAngleFormat,
                                         bool unpackFlipY,
                                         bool unpackPremultiplyAlpha,
                                         bool unpackUnmultiplyAlpha,
                                         const mtl::TextureRef &sourceTexture);

    angle::Result copySubTextureCPU(const gl::Context *context,
                                    const gl::ImageIndex &index,
                                    const gl::Offset &destOffset,
                                    const gl::InternalFormat &internalFormat,
                                    const mtl::MipmapNativeLevel &sourceNativeLevel,
                                    const gl::Box &sourceBox,
                                    const angle::Format &sourceAngleFormat,
                                    bool unpackFlipY,
                                    bool unpackPremultiplyAlpha,
                                    bool unpackUnmultiplyAlpha,
                                    const mtl::TextureRef &sourceTexture);

    // Copy data to texture's per array's slice/cube's face. NOTE: This function doesn't upload
    // data to 3D texture's z layer. Metal treats 3D texture's z layer & array texture's slice
    // differently. For array/cube texture, it is only possible to upload to one slice at a time.
    angle::Result setPerSliceSubImage(const gl::Context *context,
                                      int slice,
                                      const MTLRegion &mtlArea,
                                      const gl::InternalFormat &internalFormat,
                                      GLenum type,
                                      const angle::Format &pixelsAngleFormat,
                                      size_t pixelsRowPitch,
                                      size_t pixelsDepthPitch,
                                      gl::Buffer *unpackBuffer,
                                      const uint8_t *pixels,
                                      const mtl::TextureRef &image);

    // Convert pixels to suported format before uploading to texture
    angle::Result convertAndSetPerSliceSubImage(const gl::Context *context,
                                                int slice,
                                                const MTLRegion &mtlArea,
                                                const gl::InternalFormat &internalFormat,
                                                GLenum type,
                                                const angle::Format &pixelsAngleFormat,
                                                size_t pixelsRowPitch,
                                                size_t pixelsDepthPitch,
                                                gl::Buffer *unpackBuffer,
                                                const uint8_t *pixels,
                                                const mtl::TextureRef &image);

    angle::Result generateMipmapCPU(const gl::Context *context);

    bool needsFormatViewForPixelLocalStorage(const ShPixelLocalStorageOptions &) const;
    bool isImmutableOrPBuffer() const;

    mtl::Format mFormat;
    egl::Surface *mBoundSurface = nullptr;
    class NativeTextureWrapper;
    class NativeTextureWrapperWithViewSupport;
    // The real texture used by Metal.
    // For non-immutable texture, this usually contains levels from (GL base level -> GL max level).
    // For immutable texture, this contains levels allocated with glTexStorage which could be
    // outside (GL base level -> GL max level) range.
    std::unique_ptr<NativeTextureWrapperWithViewSupport> mNativeTextureStorage;
    // The view of mNativeTextureStorage from (GL base level -> GL max level)
    std::unique_ptr<NativeTextureWrapper> mViewFromBaseToMaxLevel;
    id<MTLSamplerState> mMetalSamplerState = nil;

    // Number of slices
    uint32_t mSlices = 1;

    // Stored images array defined by glTexImage/glCopy*.
    // Once the images array is complete, they will be transferred to real texture object.
    // NOTE:
    //  - For Cube map, there will be at most 6 entries in the map table, one for each face. This is
    //  because the Cube map's image is defined per face & per level.
    //  - For other texture types, there will be only one entry in the map table. All other textures
    //  except Cube map has texture image defined per level (all slices included).
    //  - The second dimension is indexed by GL level.
    std::map<int, gl::TexLevelArray<ImageDefinitionMtl>> mTexImageDefs;
    // 1st index = image index, 2nd index = samples count.
    std::map<gl::ImageIndex, gl::RenderToTextureImageMap<RenderTargetMtl>> mRenderTargets;
    std::map<gl::ImageIndex, gl::RenderToTextureImageMap<mtl::TextureRef>> mImplicitMSTextures;

    // Lazily populated 2D views for shader storage images.
    // May have different formats than the original texture.
    // Indexed by format, then layer, then level.
    std::map<MTLPixelFormat, LayerLevelTextureViewVector> mShaderImageViews;

    // Mipmap views are indexed from (base GL level -> max GL level):
    mtl::NativeTexLevelArray mLevelViewsWithinBaseMax;

    // The swizzled or stencil view used for shader sampling.
    mtl::TextureRef mSwizzleStencilSamplingView;
};

}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_TEXTUREMTL_H_ */
