//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer9.cpp: Implements the Framebuffer9 class.

#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/TextureStorage9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{
Framebuffer9::Framebuffer9(const gl::FramebufferState &data, Renderer9 *renderer)
    : FramebufferD3D(data, renderer), mRenderer(renderer)
{
    ASSERT(mRenderer != nullptr);
}

Framebuffer9::~Framebuffer9() {}

angle::Result Framebuffer9::discard(const gl::Context *context,
                                    size_t count,
                                    const GLenum *attachments)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Framebuffer9::invalidate(const gl::Context *context,
                                       size_t count,
                                       const GLenum *attachments)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Framebuffer9::invalidateSub(const gl::Context *context,
                                          size_t count,
                                          const GLenum *attachments,
                                          const gl::Rectangle &area)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Framebuffer9::clearImpl(const gl::Context *context,
                                      const ClearParameters &clearParams)
{
    ANGLE_TRY(mRenderer->applyRenderTarget(context, mRenderTargetCache.getColors()[0],
                                           mRenderTargetCache.getDepthStencil()));

    const gl::State &glState = context->getState();
    float nearZ              = glState.getNearPlane();
    float farZ               = glState.getFarPlane();
    mRenderer->setViewport(glState.getViewport(), nearZ, farZ, gl::PrimitiveMode::Triangles,
                           glState.getRasterizerState().frontFace, true);

    mRenderer->setScissorRectangle(glState.getScissor(), glState.isScissorTestEnabled());

    mRenderer->clear(clearParams, mRenderTargetCache.getColors()[0],
                     mRenderTargetCache.getDepthStencil());
    return angle::Result::Continue;
}

angle::Result Framebuffer9::readPixelsImpl(const gl::Context *context,
                                           const gl::Rectangle &area,
                                           GLenum format,
                                           GLenum type,
                                           size_t outputPitch,
                                           const gl::PixelPackState &pack,
                                           gl::Buffer *packBuffer,
                                           uint8_t *pixels)
{
    const gl::FramebufferAttachment *colorbuffer = mState.getColorAttachment(0);
    ASSERT(colorbuffer);

    RenderTarget9 *renderTarget = nullptr;
    ANGLE_TRY(colorbuffer->getRenderTarget(context, 0, &renderTarget));
    ASSERT(renderTarget);

    IDirect3DSurface9 *surface = renderTarget->getSurface();
    ASSERT(surface);

    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);

    Context9 *context9 = GetImplAs<Context9>(context);

    if (desc.MultiSampleType != D3DMULTISAMPLE_NONE)
    {
        UNIMPLEMENTED();  // FIXME: Requires resolve using StretchRect into non-multisampled render
                          // target
        SafeRelease(surface);
        ANGLE_TRY_HR(context9, E_OUTOFMEMORY,
                     "ReadPixels is unimplemented for multisampled framebuffer attachments.");
    }

    IDirect3DDevice9 *device = mRenderer->getDevice();
    ASSERT(device);

    HRESULT result;
    IDirect3DSurface9 *systemSurface = nullptr;
    bool directToPixels =
        !pack.reverseRowOrder && pack.alignment <= 4 && mRenderer->getShareHandleSupport() &&
        area.x == 0 && area.y == 0 && static_cast<UINT>(area.width) == desc.Width &&
        static_cast<UINT>(area.height) == desc.Height && desc.Format == D3DFMT_A8R8G8B8 &&
        format == GL_BGRA_EXT && type == GL_UNSIGNED_BYTE;
    if (directToPixels)
    {
        // Use the pixels ptr as a shared handle to write directly into client's memory
        result = device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                     D3DPOOL_SYSTEMMEM, &systemSurface,
                                                     reinterpret_cast<void **>(&pixels));
        if (FAILED(result))
        {
            // Try again without the shared handle
            directToPixels = false;
        }
    }

    if (!directToPixels)
    {
        result = device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                     D3DPOOL_SYSTEMMEM, &systemSurface, nullptr);
        if (FAILED(result))
        {
            ASSERT(result == D3DERR_OUTOFVIDEOMEMORY || result == E_OUTOFMEMORY);
            SafeRelease(surface);
            ANGLE_TRY_HR(context9, E_OUTOFMEMORY,
                         "Failed to allocate internal texture for ReadPixels.");
        }
    }

    result = device->GetRenderTargetData(surface, systemSurface);
    SafeRelease(surface);

    if (FAILED(result))
    {
        SafeRelease(systemSurface);

        // It turns out that D3D will sometimes produce more error
        // codes than those documented.
        if (d3d9::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
        }
        else
        {
            UNREACHABLE();
        }

        ANGLE_TRY_HR(context9, E_OUTOFMEMORY, "Failed to read internal render target data.");
    }

    if (directToPixels)
    {
        SafeRelease(systemSurface);
        return angle::Result::Continue;
    }

    RECT rect;
    rect.left   = gl::clamp(area.x, 0L, static_cast<LONG>(desc.Width));
    rect.top    = gl::clamp(area.y, 0L, static_cast<LONG>(desc.Height));
    rect.right  = gl::clamp(area.x + area.width, 0L, static_cast<LONG>(desc.Width));
    rect.bottom = gl::clamp(area.y + area.height, 0L, static_cast<LONG>(desc.Height));

    D3DLOCKED_RECT lock;
    result = systemSurface->LockRect(&lock, &rect, D3DLOCK_READONLY);

    if (FAILED(result))
    {
        UNREACHABLE();
        SafeRelease(systemSurface);

        ANGLE_TRY_HR(context9, E_OUTOFMEMORY, "Failed to lock internal render target.");
    }

    uint8_t *source = static_cast<uint8_t *>(lock.pBits);
    int inputPitch  = lock.Pitch;

    const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(desc.Format);

    gl::FormatType formatType(format, type);

    PackPixelsParams packParams;
    packParams.area.x          = rect.left;
    packParams.area.y          = rect.top;
    packParams.area.width      = rect.right - rect.left;
    packParams.area.height     = rect.bottom - rect.top;
    packParams.destFormat      = &GetFormatFromFormatType(format, type);
    packParams.outputPitch     = static_cast<GLuint>(outputPitch);
    packParams.reverseRowOrder = pack.reverseRowOrder;

    PackPixels(packParams, d3dFormatInfo.info(), inputPitch, source, pixels);

    systemSurface->UnlockRect();
    SafeRelease(systemSurface);

    return angle::Result::Continue;
}

angle::Result Framebuffer9::blitImpl(const gl::Context *context,
                                     const gl::Rectangle &sourceArea,
                                     const gl::Rectangle &destArea,
                                     const gl::Rectangle *scissor,
                                     bool blitRenderTarget,
                                     bool blitDepth,
                                     bool blitStencil,
                                     GLenum filter,
                                     const gl::Framebuffer *sourceFramebuffer)
{
    ASSERT(filter == GL_NEAREST);

    IDirect3DDevice9 *device = mRenderer->getDevice();
    ASSERT(device);

    mRenderer->endScene();

    Context9 *context9 = GetImplAs<Context9>(context);

    if (blitRenderTarget)
    {
        const gl::FramebufferAttachment *readBuffer = sourceFramebuffer->getColorAttachment(0);
        ASSERT(readBuffer);

        RenderTarget9 *readRenderTarget = nullptr;
        ANGLE_TRY(readBuffer->getRenderTarget(context, 0, &readRenderTarget));
        ASSERT(readRenderTarget);

        const gl::FramebufferAttachment *drawBuffer = mState.getColorAttachment(0);
        ASSERT(drawBuffer);

        RenderTarget9 *drawRenderTarget = nullptr;
        ANGLE_TRY(
            drawBuffer->getRenderTarget(context, drawBuffer->getSamples(), &drawRenderTarget));
        ASSERT(drawRenderTarget);

        // The getSurface calls do an AddRef so save them until after no errors are possible
        IDirect3DSurface9 *readSurface = readRenderTarget->getSurface();
        ASSERT(readSurface);

        IDirect3DSurface9 *drawSurface = drawRenderTarget->getSurface();
        ASSERT(drawSurface);

        gl::Extents srcSize(readRenderTarget->getWidth(), readRenderTarget->getHeight(), 1);
        gl::Extents dstSize(drawRenderTarget->getWidth(), drawRenderTarget->getHeight(), 1);

        RECT srcRect;
        srcRect.left   = sourceArea.x;
        srcRect.right  = sourceArea.x + sourceArea.width;
        srcRect.top    = sourceArea.y;
        srcRect.bottom = sourceArea.y + sourceArea.height;

        RECT dstRect;
        dstRect.left   = destArea.x;
        dstRect.right  = destArea.x + destArea.width;
        dstRect.top    = destArea.y;
        dstRect.bottom = destArea.y + destArea.height;

        // Clip the rectangles to the scissor rectangle
        if (scissor)
        {
            if (dstRect.left < scissor->x)
            {
                srcRect.left += (scissor->x - dstRect.left);
                dstRect.left = scissor->x;
            }
            if (dstRect.top < scissor->y)
            {
                srcRect.top += (scissor->y - dstRect.top);
                dstRect.top = scissor->y;
            }
            if (dstRect.right > scissor->x + scissor->width)
            {
                srcRect.right -= (dstRect.right - (scissor->x + scissor->width));
                dstRect.right = scissor->x + scissor->width;
            }
            if (dstRect.bottom > scissor->y + scissor->height)
            {
                srcRect.bottom -= (dstRect.bottom - (scissor->y + scissor->height));
                dstRect.bottom = scissor->y + scissor->height;
            }
        }

        // Clip the rectangles to the destination size
        if (dstRect.left < 0)
        {
            srcRect.left += -dstRect.left;
            dstRect.left = 0;
        }
        if (dstRect.right > dstSize.width)
        {
            srcRect.right -= (dstRect.right - dstSize.width);
            dstRect.right = dstSize.width;
        }
        if (dstRect.top < 0)
        {
            srcRect.top += -dstRect.top;
            dstRect.top = 0;
        }
        if (dstRect.bottom > dstSize.height)
        {
            srcRect.bottom -= (dstRect.bottom - dstSize.height);
            dstRect.bottom = dstSize.height;
        }

        // Clip the rectangles to the source size
        if (srcRect.left < 0)
        {
            dstRect.left += -srcRect.left;
            srcRect.left = 0;
        }
        if (srcRect.right > srcSize.width)
        {
            dstRect.right -= (srcRect.right - srcSize.width);
            srcRect.right = srcSize.width;
        }
        if (srcRect.top < 0)
        {
            dstRect.top += -srcRect.top;
            srcRect.top = 0;
        }
        if (srcRect.bottom > srcSize.height)
        {
            dstRect.bottom -= (srcRect.bottom - srcSize.height);
            srcRect.bottom = srcSize.height;
        }

        HRESULT result =
            device->StretchRect(readSurface, &srcRect, drawSurface, &dstRect, D3DTEXF_NONE);

        SafeRelease(readSurface);
        SafeRelease(drawSurface);

        ANGLE_TRY_HR(context9, result, "Internal blit failed.");
    }

    if (blitDepth || blitStencil)
    {
        const gl::FramebufferAttachment *readBuffer =
            sourceFramebuffer->getDepthOrStencilAttachment();
        ASSERT(readBuffer);

        RenderTarget9 *readDepthStencil = nullptr;
        ANGLE_TRY(readBuffer->getRenderTarget(context, 0, &readDepthStencil));
        ASSERT(readDepthStencil);

        const gl::FramebufferAttachment *drawBuffer = mState.getDepthOrStencilAttachment();
        ASSERT(drawBuffer);

        RenderTarget9 *drawDepthStencil = nullptr;
        ANGLE_TRY(
            drawBuffer->getRenderTarget(context, drawBuffer->getSamples(), &drawDepthStencil));
        ASSERT(drawDepthStencil);

        // The getSurface calls do an AddRef so save them until after no errors are possible
        IDirect3DSurface9 *readSurface = readDepthStencil->getSurface();
        ASSERT(readDepthStencil);

        IDirect3DSurface9 *drawSurface = drawDepthStencil->getSurface();
        ASSERT(drawDepthStencil);

        HRESULT result =
            device->StretchRect(readSurface, nullptr, drawSurface, nullptr, D3DTEXF_NONE);

        SafeRelease(readSurface);
        SafeRelease(drawSurface);

        ANGLE_TRY_HR(context9, result, "Internal blit failed.");
    }

    return angle::Result::Continue;
}

const gl::InternalFormat &Framebuffer9::getImplementationColorReadFormat(
    const gl::Context *context) const
{
    GLenum sizedFormat = mState.getReadAttachment()->getFormat().info->sizedInternalFormat;
    const d3d9::TextureFormat &textureFormat = d3d9::GetTextureFormatInfo(sizedFormat);
    const d3d9::D3DFormat &d3dFormatInfo     = d3d9::GetD3DFormatInfo(textureFormat.renderFormat);
    const angle::Format &angleFormat         = angle::Format::Get(d3dFormatInfo.formatID);
    return gl::GetSizedInternalFormatInfo(angleFormat.fboImplementationInternalFormat);
}

angle::Result Framebuffer9::getSamplePosition(const gl::Context *context,
                                              size_t index,
                                              GLfloat *xy) const
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context9>(context));
    return angle::Result::Stop;
}

angle::Result Framebuffer9::syncState(const gl::Context *context,
                                      GLenum binding,
                                      const gl::Framebuffer::DirtyBits &dirtyBits,
                                      gl::Command command)
{
    ANGLE_TRY(FramebufferD3D::syncState(context, binding, dirtyBits, command));
    ANGLE_TRY(mRenderTargetCache.update(context, mState, dirtyBits));
    return angle::Result::Continue;
}
}  // namespace rx
