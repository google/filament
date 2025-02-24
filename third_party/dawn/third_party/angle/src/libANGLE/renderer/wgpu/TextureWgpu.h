//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureWgpu.h:
//    Defines the class interface for TextureWgpu, implementing TextureImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_TEXTUREWGPU_H_
#define LIBANGLE_RENDERER_WGPU_TEXTUREWGPU_H_

#include "libANGLE/renderer/TextureImpl.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/RenderTargetWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

namespace rx
{
class TextureWgpu : public TextureImpl
{
  public:
    TextureWgpu(const gl::TextureState &state);
    ~TextureWgpu() override;

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

    angle::Result generateMipmap(const gl::Context *context) override;

    angle::Result setBaseLevel(const gl::Context *context, GLuint baseLevel) override;

    angle::Result bindTexImage(const gl::Context *context, egl::Surface *surface) override;
    angle::Result releaseTexImage(const gl::Context *context) override;

    angle::Result syncState(const gl::Context *context,
                            const gl::Texture::DirtyBits &dirtyBits,
                            gl::Command source) override;

    angle::Result setStorageMultisample(const gl::Context *context,
                                        gl::TextureType type,
                                        GLsizei samples,
                                        GLint internalformat,
                                        const gl::Extents &size,
                                        bool fixedSampleLocations) override;

    angle::Result initializeContents(const gl::Context *context,
                                     GLenum binding,
                                     const gl::ImageIndex &imageIndex) override;

    angle::Result getAttachmentRenderTarget(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex,
                                            GLsizei samples,
                                            FramebufferAttachmentRenderTarget **rtOut) override;

    webgpu::ImageHelper *getImage() { return mImage; }

  private:
    angle::Result setImageImpl(const gl::Context *context,
                               GLenum internalFormat,
                               GLenum type,
                               const gl::ImageIndex &index,
                               const gl::Extents &size,
                               const gl::PixelUnpackState &unpack,
                               const uint8_t *pixels);

    angle::Result setSubImageImpl(const gl::Context *context,
                                  const webgpu::Format &webgpuFormat,
                                  GLenum type,
                                  const gl::ImageIndex &index,
                                  const gl::Box &area,
                                  const gl::PixelUnpackState &unpack,
                                  const uint8_t *pixels);

    angle::Result initializeImage(ContextWgpu *contextWgpu, ImageMipLevels mipLevels);

    angle::Result redefineLevel(const gl::Context *context,
                                const webgpu::Format &webgpuFormat,
                                const gl::ImageIndex &index,
                                const gl::Extents &size);

    uint32_t getMipLevelCount(ImageMipLevels mipLevels) const;

    uint32_t getMaxLevelCount() const;
    angle::Result respecifyImageStorageIfNecessary(ContextWgpu *contextWgpu, gl::Command source);
    void prepareForGenerateMipmap(ContextWgpu *contextWgpu);
    angle::Result maybeUpdateBaseMaxLevels(ContextWgpu *contextWgpu);
    angle::Result initSingleLayerRenderTargets(ContextWgpu *contextWgpu,
                                               GLuint layerCount,
                                               gl::LevelIndex levelIndex,
                                               gl::RenderToTextureImageIndex renderToTextureIndex);
    const webgpu::Format &getBaseLevelFormat(ContextWgpu *contextWgpu) const;

    webgpu::ImageHelper *mImage;
    gl::LevelIndex mCurrentBaseLevel;
    gl::LevelIndex mCurrentMaxLevel;
    gl::CubeFaceArray<gl::TexLevelMask> mRedefinedLevels;

    // Render targets stored as array of vector of vectors
    //
    // - First dimension: only RenderToTextureImageIndex::Default for now.
    // - Second dimension: level
    // - Third dimension: layer
    gl::RenderToTextureImageMap<std::vector<std::vector<RenderTargetWgpu>>>
        mSingleLayerRenderTargets;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_TEXTUREWGPU_H_
