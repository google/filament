//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Blit9.cpp: Surface copy utility class.

#ifndef LIBANGLE_RENDERER_D3D_D3D9_BLIT9_H_
#define LIBANGLE_RENDERER_D3D_D3D9_BLIT9_H_

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"

namespace gl
{
class Context;
class Framebuffer;
class Texture;
}  // namespace gl

namespace rx
{
class Context9;
class Renderer9;
class TextureStorage;

namespace d3d
{
class Context;
}  // namespace d3d

class Blit9 : angle::NonCopyable
{
  public:
    explicit Blit9(Renderer9 *renderer);
    ~Blit9();

    angle::Result initialize(Context9 *context9);

    // Copy from source surface to dest surface.
    // sourceRect, xoffset, yoffset are in D3D coordinates (0,0 in upper-left)
    angle::Result copy2D(const gl::Context *context,
                         const gl::Framebuffer *framebuffer,
                         const RECT &sourceRect,
                         GLenum destFormat,
                         const gl::Offset &destOffset,
                         TextureStorage *storage,
                         GLint level);
    angle::Result copyCube(const gl::Context *context,
                           const gl::Framebuffer *framebuffer,
                           const RECT &sourceRect,
                           GLenum destFormat,
                           const gl::Offset &destOffset,
                           TextureStorage *storage,
                           gl::TextureTarget target,
                           GLint level);
    angle::Result copyTexture(const gl::Context *context,
                              const gl::Texture *source,
                              GLint sourceLevel,
                              const RECT &sourceRect,
                              GLenum destFormat,
                              const gl::Offset &destOffset,
                              TextureStorage *storage,
                              gl::TextureTarget destTarget,
                              GLint destLevel,
                              bool flipY,
                              bool premultiplyAlpha,
                              bool unmultiplyAlpha);

    // 2x2 box filter sample from source to dest.
    // Requires that source is RGB(A) and dest has the same format as source.
    angle::Result boxFilter(Context9 *context9, IDirect3DSurface9 *source, IDirect3DSurface9 *dest);

  private:
    Renderer9 *mRenderer;

    bool mGeometryLoaded;
    IDirect3DVertexBuffer9 *mQuadVertexBuffer;
    IDirect3DVertexDeclaration9 *mQuadVertexDeclaration;

    // Copy from source texture to dest surface.
    // sourceRect, xoffset, yoffset are in D3D coordinates (0,0 in upper-left)
    // source is interpreted as RGBA and destFormat specifies the desired result format. For
    // example, if destFormat = GL_RGB, the alpha channel will be forced to 0.
    angle::Result formatConvert(Context9 *context9,
                                IDirect3DBaseTexture9 *source,
                                const RECT &sourceRect,
                                const gl::Extents &sourceSize,
                                GLenum destFormat,
                                const gl::Offset &destOffset,
                                IDirect3DSurface9 *dest,
                                bool flipY,
                                bool premultiplyAlpha,
                                bool unmultiplyAlpha);
    angle::Result setFormatConvertShaders(Context9 *context9,
                                          GLenum destFormat,
                                          bool flipY,
                                          bool premultiplyAlpha,
                                          bool unmultiplyAlpha);

    angle::Result copy(Context9 *context9,
                       IDirect3DSurface9 *source,
                       IDirect3DBaseTexture9 *sourceTexture,
                       const RECT &sourceRect,
                       GLenum destFormat,
                       const gl::Offset &destOffset,
                       IDirect3DSurface9 *dest,
                       bool flipY,
                       bool premultiplyAlpha,
                       bool unmultiplyAlpha);
    angle::Result copySurfaceToTexture(Context9 *context9,
                                       IDirect3DSurface9 *surface,
                                       const RECT &sourceRect,
                                       angle::ComPtr<IDirect3DBaseTexture9> *outTexture);
    void setViewportAndShaderConstants(const RECT &sourceRect,
                                       const gl::Extents &sourceSize,
                                       const RECT &destRect,
                                       bool flipY);
    void setCommonBlitState();
    RECT getSurfaceRect(IDirect3DSurface9 *surface) const;
    gl::Extents getSurfaceSize(IDirect3DSurface9 *surface) const;

    // This enum is used to index mCompiledShaders and mShaderSource.
    enum ShaderId
    {
        SHADER_VS_STANDARD,
        SHADER_PS_PASSTHROUGH,
        SHADER_PS_LUMINANCE,
        SHADER_PS_LUMINANCE_PREMULTIPLY_ALPHA,
        SHADER_PS_LUMINANCE_UNMULTIPLY_ALPHA,
        SHADER_PS_COMPONENTMASK,
        SHADER_PS_COMPONENTMASK_PREMULTIPLY_ALPHA,
        SHADER_PS_COMPONENTMASK_UNMULTIPLY_ALPHA,
        SHADER_COUNT,
    };

    // This actually contains IDirect3DVertexShader9 or IDirect3DPixelShader9 casted to IUnknown.
    IUnknown *mCompiledShaders[SHADER_COUNT];

    template <class D3DShaderType>
    angle::Result setShader(Context9 *,
                            ShaderId source,
                            const char *profile,
                            angle::Result (Renderer9::*createShader)(d3d::Context *context,
                                                                     const DWORD *,
                                                                     size_t length,
                                                                     D3DShaderType **outShader),
                            HRESULT (WINAPI IDirect3DDevice9::*setShader)(D3DShaderType *));

    angle::Result setVertexShader(Context9 *context9, ShaderId shader);
    angle::Result setPixelShader(Context9 *context9, ShaderId shader);
    void render();

    void saveState();
    void restoreState();
    IDirect3DStateBlock9 *mSavedStateBlock;
    IDirect3DSurface9 *mSavedRenderTarget;
    IDirect3DSurface9 *mSavedDepthStencil;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_D3D9_BLIT9_H_
