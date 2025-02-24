//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image11.h: Defines the rx::Image11 class, which acts as the interface to
// the actual underlying resources of a Texture

#ifndef LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_

#include "common/debug.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/renderer/d3d/ImageD3D.h"
#include "libANGLE/renderer/d3d/d3d11/MappedSubresourceVerifier11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace gl
{
class Framebuffer;
}

namespace d3d11
{
template <typename T>
class ScopedUnmapper;
}  // namespace d3d11

namespace rx
{
class Renderer11;
class TextureHelper11;
class TextureStorage11;
struct Renderer11DeviceCaps;

class Image11 : public ImageD3D
{
  public:
    Image11(Renderer11 *renderer);
    ~Image11() override;

    static angle::Result GenerateMipmap(const gl::Context *context,
                                        Image11 *dest,
                                        Image11 *src,
                                        const Renderer11DeviceCaps &rendererCaps);
    static angle::Result CopyImage(const gl::Context *context,
                                   Image11 *dest,
                                   Image11 *source,
                                   const gl::Box &sourceBox,
                                   const gl::Offset &destOffset,
                                   bool unpackFlipY,
                                   bool unpackPremultiplyAlpha,
                                   bool unpackUnmultiplyAlpha,
                                   const Renderer11DeviceCaps &rendererCaps);

    bool isDirty() const override;

    angle::Result copyToStorage(const gl::Context *context,
                                TextureStorage *storage,
                                const gl::ImageIndex &index,
                                const gl::Box &region) override;

    bool redefine(gl::TextureType type,
                  GLenum internalformat,
                  const gl::Extents &size,
                  bool forceRelease) override;

    DXGI_FORMAT getDXGIFormat() const;

    angle::Result loadData(const gl::Context *context,
                           const gl::Box &area,
                           const gl::PixelUnpackState &unpack,
                           GLenum type,
                           const void *input,
                           bool applySkipImages) override;
    angle::Result loadCompressedData(const gl::Context *context,
                                     const gl::Box &area,
                                     const void *input) override;

    angle::Result copyFromTexStorage(const gl::Context *context,
                                     const gl::ImageIndex &imageIndex,
                                     TextureStorage *source) override;
    angle::Result copyFromFramebuffer(const gl::Context *context,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      const gl::Framebuffer *source) override;

    angle::Result recoverFromAssociatedStorage(const gl::Context *context);
    void verifyAssociatedStorageValid(TextureStorage11 *textureStorageEXT) const;
    void disassociateStorage();

    angle::Result getStagingTexture(const gl::Context *context,
                                    const TextureHelper11 **outStagingTexture,
                                    unsigned int *outSubresourceIndex);

  protected:
    template <typename T>
    friend class d3d11::ScopedUnmapper;
    angle::Result map(const gl::Context *context, D3D11_MAP mapType, D3D11_MAPPED_SUBRESOURCE *map);
    void unmap();

  private:
    angle::Result copyWithoutConversion(const gl::Context *context,
                                        const gl::Offset &destOffset,
                                        const gl::Box &sourceArea,
                                        const TextureHelper11 &textureHelper,
                                        UINT sourceSubResource);

    angle::Result createStagingTexture(const gl::Context *context);
    void releaseStagingTexture();

    Renderer11 *mRenderer;

    DXGI_FORMAT mDXGIFormat;
    TextureHelper11 mStagingTexture;
    unsigned int mStagingSubresource;
    MappedSubresourceVerifier11 mStagingTextureSubresourceVerifier;

    bool mRecoverFromStorage;
    TextureStorage11 *mAssociatedStorage;
    gl::ImageIndex mAssociatedImageIndex;
    unsigned int mRecoveredFromStorageCount;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D11_IMAGE11_H_
