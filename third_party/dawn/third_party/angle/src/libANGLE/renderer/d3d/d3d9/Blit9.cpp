//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Blit9.cpp: Surface copy utility class.

#include "libANGLE/renderer/d3d/d3d9/Blit9.h"

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/TextureStorage9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

namespace
{
// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/componentmaskpremultps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/componentmaskps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/componentmaskunmultps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/luminancepremultps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/luminanceps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/luminanceunmultps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/passthroughps.h"
#include "libANGLE/renderer/d3d/d3d9/shaders/compiled/standardvs.h"

const BYTE *const g_shaderCode[] = {
    g_vs20_standardvs,
    g_ps20_passthroughps,
    g_ps20_luminanceps,
    g_ps20_luminancepremultps,
    g_ps20_luminanceunmultps,
    g_ps20_componentmaskps,
    g_ps20_componentmaskpremultps,
    g_ps20_componentmaskunmultps,
};

const size_t g_shaderSize[] = {
    sizeof(g_vs20_standardvs),
    sizeof(g_ps20_passthroughps),
    sizeof(g_ps20_luminanceps),
    sizeof(g_ps20_luminancepremultps),
    sizeof(g_ps20_luminanceunmultps),
    sizeof(g_ps20_componentmaskps),
    sizeof(g_ps20_componentmaskpremultps),
    sizeof(g_ps20_componentmaskunmultps),
};
}  // namespace

namespace rx
{

Blit9::Blit9(Renderer9 *renderer)
    : mRenderer(renderer),
      mGeometryLoaded(false),
      mQuadVertexBuffer(nullptr),
      mQuadVertexDeclaration(nullptr),
      mSavedStateBlock(nullptr),
      mSavedRenderTarget(nullptr),
      mSavedDepthStencil(nullptr)
{
    memset(mCompiledShaders, 0, sizeof(mCompiledShaders));
}

Blit9::~Blit9()
{
    SafeRelease(mSavedStateBlock);
    SafeRelease(mQuadVertexBuffer);
    SafeRelease(mQuadVertexDeclaration);

    for (int i = 0; i < SHADER_COUNT; i++)
    {
        SafeRelease(mCompiledShaders[i]);
    }
}

angle::Result Blit9::initialize(Context9 *context9)
{
    if (mGeometryLoaded)
    {
        return angle::Result::Continue;
    }

    static const float quad[] = {-1, -1, -1, 1, 1, -1, 1, 1};

    IDirect3DDevice9 *device = mRenderer->getDevice();

    HRESULT result = device->CreateVertexBuffer(sizeof(quad), D3DUSAGE_WRITEONLY, 0,
                                                D3DPOOL_DEFAULT, &mQuadVertexBuffer, nullptr);

    ANGLE_TRY_HR(context9, result, "Failed to create internal blit vertex shader");

    void *lockPtr = nullptr;
    result        = mQuadVertexBuffer->Lock(0, 0, &lockPtr, 0);

    ANGLE_TRY_HR(context9, result, "Failed to lock internal blit vertex shader");
    ASSERT(lockPtr);

    memcpy(lockPtr, quad, sizeof(quad));
    mQuadVertexBuffer->Unlock();

    static const D3DVERTEXELEMENT9 elements[] = {
        {0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, D3DDECL_END()};

    result = device->CreateVertexDeclaration(elements, &mQuadVertexDeclaration);
    ANGLE_TRY_HR(context9, result, "Failed to create internal blit vertex shader declaration");

    mGeometryLoaded = true;
    return angle::Result::Continue;
}

template <class D3DShaderType>
angle::Result Blit9::setShader(Context9 *context9,
                               ShaderId source,
                               const char *profile,
                               angle::Result (Renderer9::*createShader)(d3d::Context *,
                                                                        const DWORD *,
                                                                        size_t length,
                                                                        D3DShaderType **outShader),
                               HRESULT (WINAPI IDirect3DDevice9::*setShader)(D3DShaderType *))
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    D3DShaderType *shader = nullptr;

    if (mCompiledShaders[source] != nullptr)
    {
        shader = static_cast<D3DShaderType *>(mCompiledShaders[source]);
    }
    else
    {
        const BYTE *shaderCode = g_shaderCode[source];
        size_t shaderSize      = g_shaderSize[source];
        ANGLE_TRY((mRenderer->*createShader)(context9, reinterpret_cast<const DWORD *>(shaderCode),
                                             shaderSize, &shader));
        mCompiledShaders[source] = shader;
    }

    HRESULT hr = (device->*setShader)(shader);
    ANGLE_TRY_HR(context9, hr, "Failed to set shader for blit operation");
    return angle::Result::Continue;
}

angle::Result Blit9::setVertexShader(Context9 *context9, ShaderId shader)
{
    return setShader<IDirect3DVertexShader9>(context9, shader, "vs_2_0",
                                             &Renderer9::createVertexShader,
                                             &IDirect3DDevice9::SetVertexShader);
}

angle::Result Blit9::setPixelShader(Context9 *context9, ShaderId shader)
{
    return setShader<IDirect3DPixelShader9>(context9, shader, "ps_2_0",
                                            &Renderer9::createPixelShader,
                                            &IDirect3DDevice9::SetPixelShader);
}

RECT Blit9::getSurfaceRect(IDirect3DSurface9 *surface) const
{
    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);

    RECT rect;
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = desc.Width;
    rect.bottom = desc.Height;

    return rect;
}

gl::Extents Blit9::getSurfaceSize(IDirect3DSurface9 *surface) const
{
    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);

    return gl::Extents(desc.Width, desc.Height, 1);
}

angle::Result Blit9::boxFilter(Context9 *context9,
                               IDirect3DSurface9 *source,
                               IDirect3DSurface9 *dest)
{
    ANGLE_TRY(initialize(context9));

    angle::ComPtr<IDirect3DBaseTexture9> texture = nullptr;
    ANGLE_TRY(copySurfaceToTexture(context9, source, getSurfaceRect(source), &texture));

    IDirect3DDevice9 *device = mRenderer->getDevice();

    saveState();

    device->SetTexture(0, texture.Get());
    device->SetRenderTarget(0, dest);

    ANGLE_TRY(setVertexShader(context9, SHADER_VS_STANDARD));
    ANGLE_TRY(setPixelShader(context9, SHADER_PS_PASSTHROUGH));

    setCommonBlitState();
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    setViewportAndShaderConstants(getSurfaceRect(source), getSurfaceSize(source),
                                  getSurfaceRect(dest), false);

    render();

    restoreState();

    return angle::Result::Continue;
}

angle::Result Blit9::copy2D(const gl::Context *context,
                            const gl::Framebuffer *framebuffer,
                            const RECT &sourceRect,
                            GLenum destFormat,
                            const gl::Offset &destOffset,
                            TextureStorage *storage,
                            GLint level)
{
    Context9 *context9 = GetImplAs<Context9>(context);

    ANGLE_TRY(initialize(context9));

    const gl::FramebufferAttachment *colorbuffer = framebuffer->getColorAttachment(0);
    ASSERT(colorbuffer);

    RenderTarget9 *renderTarget9 = nullptr;
    ANGLE_TRY(colorbuffer->getRenderTarget(context, 0, &renderTarget9));
    ASSERT(renderTarget9);

    angle::ComPtr<IDirect3DSurface9> source = renderTarget9->getSurface();
    ASSERT(source);

    angle::ComPtr<IDirect3DSurface9> destSurface = nullptr;
    TextureStorage9 *storage9                    = GetAs<TextureStorage9>(storage);
    ANGLE_TRY(
        storage9->getSurfaceLevel(context, gl::TextureTarget::_2D, level, true, &destSurface));
    ASSERT(destSurface);

    ANGLE_TRY(copy(context9, source.Get(), nullptr, sourceRect, destFormat, destOffset,
                   destSurface.Get(), false, false, false));
    return angle::Result::Continue;
}

angle::Result Blit9::copyCube(const gl::Context *context,
                              const gl::Framebuffer *framebuffer,
                              const RECT &sourceRect,
                              GLenum destFormat,
                              const gl::Offset &destOffset,
                              TextureStorage *storage,
                              gl::TextureTarget target,
                              GLint level)
{
    Context9 *context9 = GetImplAs<Context9>(context);

    ANGLE_TRY(initialize(context9));

    const gl::FramebufferAttachment *colorbuffer = framebuffer->getColorAttachment(0);
    ASSERT(colorbuffer);

    RenderTarget9 *renderTarget9 = nullptr;
    ANGLE_TRY(colorbuffer->getRenderTarget(context, 0, &renderTarget9));
    ASSERT(renderTarget9);

    angle::ComPtr<IDirect3DSurface9> source = renderTarget9->getSurface();
    ASSERT(source);

    angle::ComPtr<IDirect3DSurface9> destSurface = nullptr;
    TextureStorage9 *storage9                    = GetAs<TextureStorage9>(storage);
    ANGLE_TRY(storage9->getSurfaceLevel(context, target, level, true, &destSurface));
    ASSERT(destSurface);

    return copy(context9, source.Get(), nullptr, sourceRect, destFormat, destOffset,
                destSurface.Get(), false, false, false);
}

angle::Result Blit9::copyTexture(const gl::Context *context,
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
                                 bool unmultiplyAlpha)
{
    Context9 *context9 = GetImplAs<Context9>(context);
    ANGLE_TRY(initialize(context9));

    TextureD3D *sourceD3D = GetImplAs<TextureD3D>(source);

    TextureStorage *sourceStorage = nullptr;
    ANGLE_TRY(sourceD3D->getNativeTexture(context, &sourceStorage));

    TextureStorage9_2D *sourceStorage9 = GetAs<TextureStorage9_2D>(sourceStorage);
    ASSERT(sourceStorage9);

    TextureStorage9 *destStorage9 = GetAs<TextureStorage9>(storage);
    ASSERT(destStorage9);

    ASSERT(sourceLevel == 0);
    IDirect3DBaseTexture9 *sourceTexture = nullptr;
    ANGLE_TRY(sourceStorage9->getBaseTexture(context, &sourceTexture));

    angle::ComPtr<IDirect3DSurface9> sourceSurface = nullptr;
    ANGLE_TRY(sourceStorage9->getSurfaceLevel(context, gl::TextureTarget::_2D, sourceLevel, true,
                                              &sourceSurface));

    angle::ComPtr<IDirect3DSurface9> destSurface = nullptr;
    ANGLE_TRY(destStorage9->getSurfaceLevel(context, destTarget, destLevel, true, &destSurface));

    return copy(context9, sourceSurface.Get(), sourceTexture, sourceRect, destFormat, destOffset,
                destSurface.Get(), flipY, premultiplyAlpha, unmultiplyAlpha);
}

angle::Result Blit9::copy(Context9 *context9,
                          IDirect3DSurface9 *source,
                          IDirect3DBaseTexture9 *sourceTexture,
                          const RECT &sourceRect,
                          GLenum destFormat,
                          const gl::Offset &destOffset,
                          IDirect3DSurface9 *dest,
                          bool flipY,
                          bool premultiplyAlpha,
                          bool unmultiplyAlpha)
{
    ASSERT(source != nullptr && dest != nullptr);

    IDirect3DDevice9 *device = mRenderer->getDevice();

    D3DSURFACE_DESC sourceDesc;
    D3DSURFACE_DESC destDesc;
    source->GetDesc(&sourceDesc);
    dest->GetDesc(&destDesc);

    // Check if it's possible to use StetchRect
    if (sourceDesc.Format == destDesc.Format && (destDesc.Usage & D3DUSAGE_RENDERTARGET) &&
        d3d9_gl::IsFormatChannelEquivalent(destDesc.Format, destFormat) && !flipY &&
        premultiplyAlpha == unmultiplyAlpha)
    {
        RECT destRect  = {destOffset.x, destOffset.y,
                         destOffset.x + (sourceRect.right - sourceRect.left),
                         destOffset.y + (sourceRect.bottom - sourceRect.top)};
        HRESULT result = device->StretchRect(source, &sourceRect, dest, &destRect, D3DTEXF_POINT);
        ANGLE_TRY_HR(context9, result, "StretchRect failed to blit between textures");
        return angle::Result::Continue;
    }

    angle::ComPtr<IDirect3DBaseTexture9> texture = sourceTexture;
    RECT adjustedSourceRect                      = sourceRect;
    gl::Extents sourceSize(sourceDesc.Width, sourceDesc.Height, 1);

    if (texture == nullptr)
    {
        ANGLE_TRY(copySurfaceToTexture(context9, source, sourceRect, &texture));

        // copySurfaceToTexture only copies in the sourceRect area of the source surface.
        // Adjust sourceRect so that it is now covering the entire source texture
        adjustedSourceRect.left   = 0;
        adjustedSourceRect.right  = sourceRect.right - sourceRect.left;
        adjustedSourceRect.top    = 0;
        adjustedSourceRect.bottom = sourceRect.bottom - sourceRect.top;

        sourceSize.width  = sourceRect.right - sourceRect.left;
        sourceSize.height = sourceRect.bottom - sourceRect.top;
    }

    ANGLE_TRY(formatConvert(context9, texture.Get(), adjustedSourceRect, sourceSize, destFormat,
                            destOffset, dest, flipY, premultiplyAlpha, unmultiplyAlpha));
    return angle::Result::Continue;
}

angle::Result Blit9::formatConvert(Context9 *context9,
                                   IDirect3DBaseTexture9 *source,
                                   const RECT &sourceRect,
                                   const gl::Extents &sourceSize,
                                   GLenum destFormat,
                                   const gl::Offset &destOffset,
                                   IDirect3DSurface9 *dest,
                                   bool flipY,
                                   bool premultiplyAlpha,
                                   bool unmultiplyAlpha)
{
    ANGLE_TRY(initialize(context9));

    IDirect3DDevice9 *device = mRenderer->getDevice();

    saveState();

    device->SetTexture(0, source);
    device->SetRenderTarget(0, dest);

    RECT destRect;
    destRect.left   = destOffset.x;
    destRect.right  = destOffset.x + (sourceRect.right - sourceRect.left);
    destRect.top    = destOffset.y;
    destRect.bottom = destOffset.y + (sourceRect.bottom - sourceRect.top);

    setViewportAndShaderConstants(sourceRect, sourceSize, destRect, flipY);

    setCommonBlitState();

    angle::Result result =
        setFormatConvertShaders(context9, destFormat, flipY, premultiplyAlpha, unmultiplyAlpha);
    if (result == angle::Result::Continue)
    {
        render();
    }

    restoreState();

    return result;
}

angle::Result Blit9::setFormatConvertShaders(Context9 *context9,
                                             GLenum destFormat,
                                             bool flipY,
                                             bool premultiplyAlpha,
                                             bool unmultiplyAlpha)
{
    ANGLE_TRY(setVertexShader(context9, SHADER_VS_STANDARD));

    switch (destFormat)
    {
        case GL_RGBA:
        case GL_BGRA_EXT:
        case GL_RGB:
        case GL_RG_EXT:
        case GL_RED_EXT:
        case GL_ALPHA:
            if (premultiplyAlpha == unmultiplyAlpha)
            {
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_COMPONENTMASK));
            }
            else if (premultiplyAlpha)
            {
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_COMPONENTMASK_PREMULTIPLY_ALPHA));
            }
            else
            {
                ASSERT(unmultiplyAlpha);
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_COMPONENTMASK_UNMULTIPLY_ALPHA));
            }
            break;

        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            if (premultiplyAlpha == unmultiplyAlpha)
            {
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_LUMINANCE));
            }
            else if (premultiplyAlpha)
            {
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_LUMINANCE_PREMULTIPLY_ALPHA));
            }
            else
            {
                ASSERT(unmultiplyAlpha);
                ANGLE_TRY(setPixelShader(context9, SHADER_PS_LUMINANCE_UNMULTIPLY_ALPHA));
            }
            break;

        default:
            UNREACHABLE();
    }

    enum
    {
        X = 0,
        Y = 1,
        Z = 2,
        W = 3
    };

    // The meaning of this constant depends on the shader that was selected.
    // See the shader assembly code above for details.
    // Allocate one array for both registers and split it into two float4's.
    float psConst[8] = {0};
    float *multConst = &psConst[0];
    float *addConst  = &psConst[4];

    switch (destFormat)
    {
        case GL_RGBA:
        case GL_BGRA_EXT:
            multConst[X] = 1;
            multConst[Y] = 1;
            multConst[Z] = 1;
            multConst[W] = 1;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 0;
            break;

        case GL_RGB:
            multConst[X] = 1;
            multConst[Y] = 1;
            multConst[Z] = 1;
            multConst[W] = 0;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 1;
            break;

        case GL_RG_EXT:
            multConst[X] = 1;
            multConst[Y] = 1;
            multConst[Z] = 0;
            multConst[W] = 0;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 1;
            break;

        case GL_RED_EXT:
            multConst[X] = 1;
            multConst[Y] = 0;
            multConst[Z] = 0;
            multConst[W] = 0;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 1;
            break;

        case GL_ALPHA:
            multConst[X] = 0;
            multConst[Y] = 0;
            multConst[Z] = 0;
            multConst[W] = 1;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 0;
            break;

        case GL_LUMINANCE:
            multConst[X] = 1;
            multConst[Y] = 0;
            multConst[Z] = 0;
            multConst[W] = 0;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 1;
            break;

        case GL_LUMINANCE_ALPHA:
            multConst[X] = 1;
            multConst[Y] = 0;
            multConst[Z] = 0;
            multConst[W] = 1;
            addConst[X]  = 0;
            addConst[Y]  = 0;
            addConst[Z]  = 0;
            addConst[W]  = 0;
            break;

        default:
            UNREACHABLE();
    }

    mRenderer->getDevice()->SetPixelShaderConstantF(0, psConst, 2);

    return angle::Result::Continue;
}

angle::Result Blit9::copySurfaceToTexture(Context9 *context9,
                                          IDirect3DSurface9 *surface,
                                          const RECT &sourceRect,
                                          angle::ComPtr<IDirect3DBaseTexture9> *outTexture)
{
    ASSERT(surface);

    IDirect3DDevice9 *device = mRenderer->getDevice();

    D3DSURFACE_DESC sourceDesc;
    surface->GetDesc(&sourceDesc);

    // Copy the render target into a texture
    angle::ComPtr<IDirect3DTexture9> texture;
    HRESULT result = device->CreateTexture(
        sourceRect.right - sourceRect.left, sourceRect.bottom - sourceRect.top, 1,
        D3DUSAGE_RENDERTARGET, sourceDesc.Format, D3DPOOL_DEFAULT, &texture, nullptr);
    ANGLE_TRY_HR(context9, result, "Failed to allocate internal texture for blit");

    angle::ComPtr<IDirect3DSurface9> textureSurface;
    result = texture->GetSurfaceLevel(0, &textureSurface);
    ANGLE_TRY_HR(context9, result, "Failed to query surface of internal blit texture");

    mRenderer->endScene();
    result = device->StretchRect(surface, &sourceRect, textureSurface.Get(), nullptr, D3DTEXF_NONE);
    ANGLE_TRY_HR(context9, result, "Failed to copy between internal blit textures");
    *outTexture = texture;

    return angle::Result::Continue;
}

void Blit9::setViewportAndShaderConstants(const RECT &sourceRect,
                                          const gl::Extents &sourceSize,
                                          const RECT &destRect,
                                          bool flipY)
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    D3DVIEWPORT9 vp;
    vp.X      = destRect.left;
    vp.Y      = destRect.top;
    vp.Width  = destRect.right - destRect.left;
    vp.Height = destRect.bottom - destRect.top;
    vp.MinZ   = 0.0f;
    vp.MaxZ   = 1.0f;
    device->SetViewport(&vp);

    float vertexConstants[8] = {
        // halfPixelAdjust
        -1.0f / vp.Width,
        1.0f / vp.Height,
        0,
        0,
        // texcoordOffset
        static_cast<float>(sourceRect.left) / sourceSize.width,
        static_cast<float>(flipY ? sourceRect.bottom : sourceRect.top) / sourceSize.height,
        static_cast<float>(sourceRect.right - sourceRect.left) / sourceSize.width,
        static_cast<float>(flipY ? sourceRect.top - sourceRect.bottom
                                 : sourceRect.bottom - sourceRect.top) /
            sourceSize.height,
    };

    device->SetVertexShaderConstantF(0, vertexConstants, 2);
}

void Blit9::setCommonBlitState()
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    device->SetDepthStencilSurface(nullptr);

    device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    device->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
    device->SetRenderState(D3DRS_COLORWRITEENABLE,
                           D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_BLUE |
                               D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_RED);
    device->SetRenderState(D3DRS_SRGBWRITEENABLE, FALSE);
    device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    device->SetSamplerState(0, D3DSAMP_SRGBTEXTURE, FALSE);
    device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

    RECT scissorRect = {};  // Scissoring is disabled for flipping, but we need this to capture and
                            // restore the old rectangle
    device->SetScissorRect(&scissorRect);

    for (int i = 0; i < gl::MAX_VERTEX_ATTRIBS; i++)
    {
        device->SetStreamSourceFreq(i, 1);
    }
}

void Blit9::render()
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    device->SetStreamSource(0, mQuadVertexBuffer, 0, 2 * sizeof(float));
    device->SetVertexDeclaration(mQuadVertexDeclaration);

    mRenderer->startScene();
    device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}

void Blit9::saveState()
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    HRESULT hr;

    device->GetDepthStencilSurface(&mSavedDepthStencil);
    device->GetRenderTarget(0, &mSavedRenderTarget);

    if (mSavedStateBlock == nullptr)
    {
        hr = device->BeginStateBlock();
        ASSERT(SUCCEEDED(hr) || hr == D3DERR_OUTOFVIDEOMEMORY || hr == E_OUTOFMEMORY);

        setCommonBlitState();

        static const float mockConst[8] = {0};

        device->SetVertexShader(nullptr);
        device->SetVertexShaderConstantF(0, mockConst, 2);
        device->SetPixelShader(nullptr);
        device->SetPixelShaderConstantF(0, mockConst, 2);

        D3DVIEWPORT9 mockVp;
        mockVp.X      = 0;
        mockVp.Y      = 0;
        mockVp.Width  = 1;
        mockVp.Height = 1;
        mockVp.MinZ   = 0;
        mockVp.MaxZ   = 1;

        device->SetViewport(&mockVp);

        device->SetTexture(0, nullptr);

        device->SetStreamSource(0, mQuadVertexBuffer, 0, 0);

        device->SetVertexDeclaration(mQuadVertexDeclaration);

        hr = device->EndStateBlock(&mSavedStateBlock);
        ASSERT(SUCCEEDED(hr) || hr == D3DERR_OUTOFVIDEOMEMORY || hr == E_OUTOFMEMORY);
    }

    ASSERT(mSavedStateBlock != nullptr);

    if (mSavedStateBlock != nullptr)
    {
        hr = mSavedStateBlock->Capture();
        ASSERT(SUCCEEDED(hr));
    }
}

void Blit9::restoreState()
{
    IDirect3DDevice9 *device = mRenderer->getDevice();

    device->SetDepthStencilSurface(mSavedDepthStencil);
    SafeRelease(mSavedDepthStencil);

    device->SetRenderTarget(0, mSavedRenderTarget);
    SafeRelease(mSavedRenderTarget);

    ASSERT(mSavedStateBlock != nullptr);

    if (mSavedStateBlock != nullptr)
    {
        mSavedStateBlock->Apply();
    }
}
}  // namespace rx
