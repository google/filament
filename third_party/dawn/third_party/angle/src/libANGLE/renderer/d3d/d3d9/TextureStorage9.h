//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureStorage9.h: Defines the abstract rx::TextureStorage9 class and its concrete derived
// classes TextureStorage9_2D and TextureStorage9_Cube, which act as the interface to the
// D3D9 texture.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_TEXTURESTORAGE9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_TEXTURESTORAGE9_H_

#include "common/debug.h"
#include "libANGLE/renderer/d3d/TextureStorage.h"

namespace rx
{
class EGLImageD3D;
class Renderer9;
class SwapChain9;
class RenderTargetD3D;
class RenderTarget9;

class TextureStorage9 : public TextureStorage
{
  public:
    ~TextureStorage9() override;

    static DWORD GetTextureUsage(GLenum internalformat, bool renderTarget);

    D3DPOOL getPool() const;
    DWORD getUsage() const;

    virtual angle::Result getSurfaceLevel(const gl::Context *context,
                                          gl::TextureTarget target,
                                          int level,
                                          bool dirty,
                                          IDirect3DSurface9 **outSurface)    = 0;
    virtual angle::Result getBaseTexture(const gl::Context *context,
                                         IDirect3DBaseTexture9 **outTexture) = 0;

    int getTopLevel() const override;
    bool isRenderTarget() const override;
    bool isUnorderedAccess() const override { return false; }
    bool isManaged() const override;
    bool supportsNativeMipmapFunction() const override;
    int getLevelCount() const override;
    bool isMultiplanar(const gl::Context *context) override;

    angle::Result setData(const gl::Context *context,
                          const gl::ImageIndex &index,
                          ImageD3D *image,
                          const gl::Box *destBox,
                          GLenum type,
                          const gl::PixelUnpackState &unpack,
                          const uint8_t *pixelData) override;

  protected:
    int mTopLevel;
    size_t mMipLevels;
    size_t mTextureWidth;
    size_t mTextureHeight;
    GLenum mInternalFormat;
    D3DFORMAT mTextureFormat;

    Renderer9 *mRenderer;

    TextureStorage9(Renderer9 *renderer, DWORD usage, const std::string &label);

  private:
    const DWORD mD3DUsage;
    const D3DPOOL mD3DPool;
};

class TextureStorage9_2D : public TextureStorage9
{
  public:
    TextureStorage9_2D(Renderer9 *renderer, SwapChain9 *swapchain, const std::string &label);
    TextureStorage9_2D(Renderer9 *renderer,
                       GLenum internalformat,
                       bool renderTarget,
                       GLsizei width,
                       GLsizei height,
                       int levels,
                       const std::string &label);
    ~TextureStorage9_2D() override;

    angle::Result getSurfaceLevel(const gl::Context *context,
                                  gl::TextureTarget target,
                                  int level,
                                  bool dirty,
                                  IDirect3DSurface9 **outSurface) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;
    angle::Result getBaseTexture(const gl::Context *context,
                                 IDirect3DBaseTexture9 **outTexture) override;
    angle::Result generateMipmap(const gl::Context *context,
                                 const gl::ImageIndex &sourceIndex,
                                 const gl::ImageIndex &destIndex) override;
    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

  private:
    IDirect3DTexture9 *mTexture;
    std::vector<RenderTarget9 *> mRenderTargets;
};

class TextureStorage9_EGLImage final : public TextureStorage9
{
  public:
    TextureStorage9_EGLImage(Renderer9 *renderer,
                             EGLImageD3D *image,
                             RenderTarget9 *renderTarget9,
                             const std::string &label);
    ~TextureStorage9_EGLImage() override;

    angle::Result getSurfaceLevel(const gl::Context *context,
                                  gl::TextureTarget target,
                                  int level,
                                  bool dirty,
                                  IDirect3DSurface9 **outSurface) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;
    angle::Result getBaseTexture(const gl::Context *context,
                                 IDirect3DBaseTexture9 **outTexture) override;
    angle::Result generateMipmap(const gl::Context *context,
                                 const gl::ImageIndex &sourceIndex,
                                 const gl::ImageIndex &destIndex) override;
    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

  private:
    EGLImageD3D *mImage;
};

class TextureStorage9_Cube : public TextureStorage9
{
  public:
    TextureStorage9_Cube(Renderer9 *renderer,
                         GLenum internalformat,
                         bool renderTarget,
                         int size,
                         int levels,
                         bool hintLevelZeroOnly,
                         const std::string &label);

    ~TextureStorage9_Cube() override;

    angle::Result getSurfaceLevel(const gl::Context *context,
                                  gl::TextureTarget target,
                                  int level,
                                  bool dirty,
                                  IDirect3DSurface9 **outSurface) override;
    angle::Result findRenderTarget(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLsizei samples,
                                   RenderTargetD3D **outRT) const override;
    angle::Result getRenderTarget(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLsizei samples,
                                  RenderTargetD3D **outRT) override;
    angle::Result getBaseTexture(const gl::Context *context,
                                 IDirect3DBaseTexture9 **outTexture) override;
    angle::Result generateMipmap(const gl::Context *context,
                                 const gl::ImageIndex &sourceIndex,
                                 const gl::ImageIndex &destIndex) override;
    angle::Result copyToStorage(const gl::Context *context, TextureStorage *destStorage) override;

  private:
    IDirect3DCubeTexture9 *mTexture;
    RenderTarget9 *mRenderTarget[gl::kCubeFaceCount];
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_TEXTURESTORAGE9_H_
