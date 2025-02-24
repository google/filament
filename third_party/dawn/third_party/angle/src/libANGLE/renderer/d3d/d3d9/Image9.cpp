//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Image9.cpp: Implements the rx::Image9 class, which acts as the interface to
// the actual underlying surfaces of a Texture.

#include "libANGLE/renderer/d3d/d3d9/Image9.h"

#include "common/utilities.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/copyvertex.h"
#include "libANGLE/renderer/d3d/d3d9/Context9.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/TextureStorage9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"

namespace rx
{

Image9::Image9(Renderer9 *renderer)
{
    mSurface  = nullptr;
    mRenderer = nullptr;

    mD3DPool   = D3DPOOL_SYSTEMMEM;
    mD3DFormat = D3DFMT_UNKNOWN;

    mRenderer = renderer;
}

Image9::~Image9()
{
    SafeRelease(mSurface);
}

// static
angle::Result Image9::GenerateMip(Context9 *context9,
                                  IDirect3DSurface9 *destSurface,
                                  IDirect3DSurface9 *sourceSurface)
{
    D3DSURFACE_DESC destDesc;
    HRESULT result = destSurface->GetDesc(&destDesc);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result,
                 "Failed to query the source surface description for mipmap generation");

    D3DSURFACE_DESC sourceDesc;
    result = sourceSurface->GetDesc(&sourceDesc);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result,
                 "Failed to query the destination surface description for mipmap generation");

    ASSERT(sourceDesc.Format == destDesc.Format);
    ASSERT(sourceDesc.Width == 1 || sourceDesc.Width / 2 == destDesc.Width);
    ASSERT(sourceDesc.Height == 1 || sourceDesc.Height / 2 == destDesc.Height);

    const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(sourceDesc.Format);
    ASSERT(d3dFormatInfo.info().mipGenerationFunction != nullptr);

    D3DLOCKED_RECT sourceLocked = {};
    result                      = sourceSurface->LockRect(&sourceLocked, nullptr, D3DLOCK_READONLY);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result, "Failed to lock the source surface for mipmap generation");

    D3DLOCKED_RECT destLocked = {};
    result                    = destSurface->LockRect(&destLocked, nullptr, 0);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result, "Failed to lock the destination surface for mipmap generation");

    const uint8_t *sourceData = static_cast<const uint8_t *>(sourceLocked.pBits);
    uint8_t *destData         = static_cast<uint8_t *>(destLocked.pBits);

    ASSERT(sourceData && destData);

    d3dFormatInfo.info().mipGenerationFunction(sourceDesc.Width, sourceDesc.Height, 1, sourceData,
                                               sourceLocked.Pitch, 0, destData, destLocked.Pitch,
                                               0);

    destSurface->UnlockRect();
    sourceSurface->UnlockRect();

    return angle::Result::Continue;
}

// static
angle::Result Image9::GenerateMipmap(Context9 *context9, Image9 *dest, Image9 *source)
{
    IDirect3DSurface9 *sourceSurface = nullptr;
    ANGLE_TRY(source->getSurface(context9, &sourceSurface));

    IDirect3DSurface9 *destSurface = nullptr;
    ANGLE_TRY(dest->getSurface(context9, &destSurface));

    ANGLE_TRY(GenerateMip(context9, destSurface, sourceSurface));

    dest->markDirty();

    return angle::Result::Continue;
}

// static
angle::Result Image9::CopyLockableSurfaces(Context9 *context9,
                                           IDirect3DSurface9 *dest,
                                           IDirect3DSurface9 *source)
{
    D3DLOCKED_RECT sourceLock = {};
    D3DLOCKED_RECT destLock   = {};

    HRESULT result;

    result = source->LockRect(&sourceLock, nullptr, 0);
    ANGLE_TRY_HR(context9, result, "Failed to lock source surface for copy");

    result = dest->LockRect(&destLock, nullptr, 0);
    if (FAILED(result))
    {
        source->UnlockRect();
    }
    ANGLE_TRY_HR(context9, result, "Failed to lock destination surface for copy");

    ASSERT(sourceLock.pBits && destLock.pBits);

    D3DSURFACE_DESC desc;
    source->GetDesc(&desc);

    const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(desc.Format);
    unsigned int rows                    = desc.Height / d3dFormatInfo.blockHeight;

    unsigned int bytes = d3d9::ComputeBlockSize(desc.Format, desc.Width, d3dFormatInfo.blockHeight);
    ASSERT(bytes <= static_cast<unsigned int>(sourceLock.Pitch) &&
           bytes <= static_cast<unsigned int>(destLock.Pitch));

    for (unsigned int i = 0; i < rows; i++)
    {
        memcpy((char *)destLock.pBits + destLock.Pitch * i,
               (char *)sourceLock.pBits + sourceLock.Pitch * i, bytes);
    }

    source->UnlockRect();
    dest->UnlockRect();

    return angle::Result::Continue;
}

// static
angle::Result Image9::CopyImage(const gl::Context *context,
                                Image9 *dest,
                                Image9 *source,
                                const gl::Rectangle &sourceRect,
                                const gl::Offset &destOffset,
                                bool unpackFlipY,
                                bool unpackPremultiplyAlpha,
                                bool unpackUnmultiplyAlpha)
{
    Context9 *context9 = GetImplAs<Context9>(context);

    IDirect3DSurface9 *sourceSurface = nullptr;
    ANGLE_TRY(source->getSurface(context9, &sourceSurface));

    IDirect3DSurface9 *destSurface = nullptr;
    ANGLE_TRY(dest->getSurface(context9, &destSurface));

    D3DSURFACE_DESC destDesc;
    HRESULT result = destSurface->GetDesc(&destDesc);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result, "Failed to query the source surface description for CopyImage");
    const d3d9::D3DFormat &destD3DFormatInfo = d3d9::GetD3DFormatInfo(destDesc.Format);

    D3DSURFACE_DESC sourceDesc;
    result = sourceSurface->GetDesc(&sourceDesc);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result,
                 "Failed to query the destination surface description for CopyImage");
    const d3d9::D3DFormat &sourceD3DFormatInfo = d3d9::GetD3DFormatInfo(sourceDesc.Format);

    D3DLOCKED_RECT sourceLocked = {};
    result                      = sourceSurface->LockRect(&sourceLocked, nullptr, D3DLOCK_READONLY);
    ASSERT(SUCCEEDED(result));
    ANGLE_TRY_HR(context9, result, "Failed to lock the source surface for CopyImage");

    D3DLOCKED_RECT destLocked = {};
    result                    = destSurface->LockRect(&destLocked, nullptr, 0);
    ASSERT(SUCCEEDED(result));
    if (FAILED(result))
    {
        sourceSurface->UnlockRect();
    }
    ANGLE_TRY_HR(context9, result, "Failed to lock the destination surface for CopyImage");

    const uint8_t *sourceData = static_cast<const uint8_t *>(sourceLocked.pBits) +
                                sourceRect.x * sourceD3DFormatInfo.pixelBytes +
                                sourceRect.y * sourceLocked.Pitch;
    uint8_t *destData = static_cast<uint8_t *>(destLocked.pBits) +
                        destOffset.x * destD3DFormatInfo.pixelBytes +
                        destOffset.y * destLocked.Pitch;
    ASSERT(sourceData && destData);

    CopyImageCHROMIUM(sourceData, sourceLocked.Pitch, sourceD3DFormatInfo.pixelBytes, 0,
                      sourceD3DFormatInfo.info().pixelReadFunction, destData, destLocked.Pitch,
                      destD3DFormatInfo.pixelBytes, 0, destD3DFormatInfo.info().pixelWriteFunction,
                      gl::GetUnsizedFormat(dest->getInternalFormat()),
                      destD3DFormatInfo.info().componentType, sourceRect.width, sourceRect.height,
                      1, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha);

    dest->markDirty();

    destSurface->UnlockRect();
    sourceSurface->UnlockRect();

    return angle::Result::Continue;
}

bool Image9::redefine(gl::TextureType type,
                      GLenum internalformat,
                      const gl::Extents &size,
                      bool forceRelease)
{
    // 3D textures are not supported by the D3D9 backend.
    ASSERT(size.depth <= 1);

    // Only 2D and cube texture are supported by the D3D9 backend.
    ASSERT(type == gl::TextureType::_2D || type == gl::TextureType::CubeMap);

    if (mWidth != size.width || mHeight != size.height || mDepth != size.depth ||
        mInternalFormat != internalformat || forceRelease)
    {
        mWidth          = size.width;
        mHeight         = size.height;
        mDepth          = size.depth;
        mType           = type;
        mInternalFormat = internalformat;

        // compute the d3d format that will be used
        const d3d9::TextureFormat &d3d9FormatInfo = d3d9::GetTextureFormatInfo(internalformat);
        mD3DFormat                                = d3d9FormatInfo.texFormat;
        mRenderable                               = (d3d9FormatInfo.renderFormat != D3DFMT_UNKNOWN);

        SafeRelease(mSurface);
        mDirty = (d3d9FormatInfo.dataInitializerFunction != nullptr);

        return true;
    }

    return false;
}

angle::Result Image9::createSurface(Context9 *context9)
{
    if (mSurface)
    {
        return angle::Result::Continue;
    }

    IDirect3DTexture9 *newTexture = nullptr;
    IDirect3DSurface9 *newSurface = nullptr;
    const D3DPOOL poolToUse       = D3DPOOL_SYSTEMMEM;
    const D3DFORMAT d3dFormat     = getD3DFormat();

    if (mWidth != 0 && mHeight != 0)
    {
        int levelToFetch      = 0;
        GLsizei requestWidth  = mWidth;
        GLsizei requestHeight = mHeight;
        d3d9::MakeValidSize(true, d3dFormat, &requestWidth, &requestHeight, &levelToFetch);

        IDirect3DDevice9 *device = mRenderer->getDevice();

        HRESULT result = device->CreateTexture(requestWidth, requestHeight, levelToFetch + 1, 0,
                                               d3dFormat, poolToUse, &newTexture, nullptr);

        ANGLE_TRY_HR(context9, result, "Failed to create image surface");

        newTexture->GetSurfaceLevel(levelToFetch, &newSurface);
        SafeRelease(newTexture);

        const d3d9::TextureFormat &d3dFormatInfo = d3d9::GetTextureFormatInfo(mInternalFormat);
        if (d3dFormatInfo.dataInitializerFunction != nullptr)
        {
            RECT entireRect;
            entireRect.left   = 0;
            entireRect.right  = mWidth;
            entireRect.top    = 0;
            entireRect.bottom = mHeight;

            D3DLOCKED_RECT lockedRect;
            result = newSurface->LockRect(&lockedRect, &entireRect, 0);
            ASSERT(SUCCEEDED(result));
            ANGLE_TRY_HR(context9, result, "Failed to lock image surface");

            d3dFormatInfo.dataInitializerFunction(
                mWidth, mHeight, 1, static_cast<uint8_t *>(lockedRect.pBits), lockedRect.Pitch, 0);

            result = newSurface->UnlockRect();
            ASSERT(SUCCEEDED(result));
            ANGLE_TRY_HR(context9, result, "Failed to unlock image surface");
        }
    }

    mSurface = newSurface;
    mDirty   = false;
    mD3DPool = poolToUse;

    return angle::Result::Continue;
}

angle::Result Image9::lock(Context9 *context9, D3DLOCKED_RECT *lockedRect, const RECT &rect)
{
    ANGLE_TRY(createSurface(context9));

    if (mSurface)
    {
        HRESULT result = mSurface->LockRect(lockedRect, &rect, 0);
        ASSERT(SUCCEEDED(result));
        ANGLE_TRY_HR(context9, result, "Failed to lock image surface");
        mDirty = true;
    }

    return angle::Result::Continue;
}

void Image9::unlock()
{
    if (mSurface)
    {
        HRESULT result = mSurface->UnlockRect();
        ASSERT(SUCCEEDED(result));
    }
}

D3DFORMAT Image9::getD3DFormat() const
{
    // this should only happen if the image hasn't been redefined first
    // which would be a bug by the caller
    ASSERT(mD3DFormat != D3DFMT_UNKNOWN);

    return mD3DFormat;
}

bool Image9::isDirty() const
{
    // Make sure to that this image is marked as dirty even if the staging texture hasn't been
    // created yet if initialization is required before use.
    return (mSurface ||
            d3d9::GetTextureFormatInfo(mInternalFormat).dataInitializerFunction != nullptr) &&
           mDirty;
}

angle::Result Image9::getSurface(Context9 *context9, IDirect3DSurface9 **outSurface)
{
    ANGLE_TRY(createSurface(context9));
    *outSurface = mSurface;
    return angle::Result::Continue;
}

angle::Result Image9::setManagedSurface2D(const gl::Context *context,
                                          TextureStorage *storage,
                                          int level)
{
    IDirect3DSurface9 *surface = nullptr;
    TextureStorage9 *storage9  = GetAs<TextureStorage9>(storage);
    ANGLE_TRY(storage9->getSurfaceLevel(context, gl::TextureTarget::_2D, level, false, &surface));
    return setManagedSurface(GetImplAs<Context9>(context), surface);
}

angle::Result Image9::setManagedSurfaceCube(const gl::Context *context,
                                            TextureStorage *storage,
                                            int face,
                                            int level)
{
    IDirect3DSurface9 *surface = nullptr;
    TextureStorage9 *storage9  = GetAs<TextureStorage9>(storage);
    ANGLE_TRY(storage9->getSurfaceLevel(context, gl::CubeFaceIndexToTextureTarget(face), level,
                                        false, &surface));
    return setManagedSurface(GetImplAs<Context9>(context), surface);
}

angle::Result Image9::setManagedSurface(Context9 *context9, IDirect3DSurface9 *surface)
{
    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);
    ASSERT(desc.Pool == D3DPOOL_MANAGED);

    if ((GLsizei)desc.Width == mWidth && (GLsizei)desc.Height == mHeight)
    {
        if (mSurface)
        {
            angle::Result result = CopyLockableSurfaces(context9, surface, mSurface);
            SafeRelease(mSurface);
            ANGLE_TRY(result);
        }

        mSurface = surface;
        mD3DPool = desc.Pool;
    }

    return angle::Result::Continue;
}

angle::Result Image9::copyToStorage(const gl::Context *context,
                                    TextureStorage *storage,
                                    const gl::ImageIndex &index,
                                    const gl::Box &region)
{
    ANGLE_TRY(createSurface(GetImplAs<Context9>(context)));

    TextureStorage9 *storage9      = GetAs<TextureStorage9>(storage);
    IDirect3DSurface9 *destSurface = nullptr;
    ANGLE_TRY(storage9->getSurfaceLevel(context, index.getTarget(), index.getLevelIndex(), true,
                                        &destSurface));

    angle::Result result = copyToSurface(GetImplAs<Context9>(context), destSurface, region);
    SafeRelease(destSurface);
    return result;
}

angle::Result Image9::copyToSurface(Context9 *context9,
                                    IDirect3DSurface9 *destSurface,
                                    const gl::Box &area)
{
    ASSERT(area.width > 0 && area.height > 0 && area.depth == 1);
    ASSERT(destSurface);

    IDirect3DSurface9 *sourceSurface = nullptr;
    ANGLE_TRY(getSurface(context9, &sourceSurface));

    ASSERT(sourceSurface && sourceSurface != destSurface);

    RECT rect;
    rect.left   = area.x;
    rect.top    = area.y;
    rect.right  = area.x + area.width;
    rect.bottom = area.y + area.height;

    POINT point = {rect.left, rect.top};

    IDirect3DDevice9 *device = mRenderer->getDevice();

    if (mD3DPool == D3DPOOL_MANAGED)
    {
        D3DSURFACE_DESC desc;
        sourceSurface->GetDesc(&desc);

        IDirect3DSurface9 *surf = 0;
        HRESULT result = device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                             D3DPOOL_SYSTEMMEM, &surf, nullptr);
        ANGLE_TRY_HR(context9, result, "Internal CreateOffscreenPlainSurface call failed");

        auto err = CopyLockableSurfaces(context9, surf, sourceSurface);
        result   = device->UpdateSurface(surf, &rect, destSurface, &point);
        SafeRelease(surf);
        ANGLE_TRY(err);
        ASSERT(SUCCEEDED(result));
        ANGLE_TRY_HR(context9, result, "Internal UpdateSurface call failed");
    }
    else
    {
        // UpdateSurface: source must be SYSTEMMEM, dest must be DEFAULT pools
        HRESULT result = device->UpdateSurface(sourceSurface, &rect, destSurface, &point);
        ASSERT(SUCCEEDED(result));
        ANGLE_TRY_HR(context9, result, "Internal UpdateSurface call failed");
    }

    return angle::Result::Continue;
}

// Store the pixel rectangle designated by xoffset,yoffset,width,height with pixels stored as
// format/type at input into the target pixel rectangle.
angle::Result Image9::loadData(const gl::Context *context,
                               const gl::Box &area,
                               const gl::PixelUnpackState &unpack,
                               GLenum type,
                               const void *input,
                               bool applySkipImages)
{
    // 3D textures are not supported by the D3D9 backend.
    ASSERT(area.z == 0 && area.depth == 1);

    Context9 *context9 = GetImplAs<Context9>(context);

    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(mInternalFormat);
    GLuint inputRowPitch                 = 0;
    ANGLE_CHECK_GL_MATH(context9, formatInfo.computeRowPitch(type, area.width, unpack.alignment,
                                                             unpack.rowLength, &inputRowPitch));
    ASSERT(!applySkipImages);
    ASSERT(unpack.skipPixels == 0);
    ASSERT(unpack.skipRows == 0);

    const d3d9::TextureFormat &d3dFormatInfo = d3d9::GetTextureFormatInfo(mInternalFormat);
    ASSERT(d3dFormatInfo.loadFunction != nullptr);

    RECT lockRect = {area.x, area.y, area.x + area.width, area.y + area.height};

    D3DLOCKED_RECT locked;
    ANGLE_TRY(lock(context9, &locked, lockRect));

    d3dFormatInfo.loadFunction(context9->getImageLoadContext(), area.width, area.height, area.depth,
                               static_cast<const uint8_t *>(input), inputRowPitch, 0,
                               static_cast<uint8_t *>(locked.pBits), locked.Pitch, 0);

    unlock();

    return angle::Result::Continue;
}

angle::Result Image9::loadCompressedData(const gl::Context *context,
                                         const gl::Box &area,
                                         const void *input)
{
    // 3D textures are not supported by the D3D9 backend.
    ASSERT(area.z == 0 && area.depth == 1);

    Context9 *context9 = GetImplAs<Context9>(context);

    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(mInternalFormat);
    GLuint inputRowPitch                 = 0;
    ANGLE_CHECK_GL_MATH(
        context9, formatInfo.computeRowPitch(GL_UNSIGNED_BYTE, area.width, 1, 0, &inputRowPitch));

    GLuint inputDepthPitch = 0;
    ANGLE_CHECK_GL_MATH(
        context9, formatInfo.computeDepthPitch(area.height, 0, inputRowPitch, &inputDepthPitch));

    const d3d9::TextureFormat &d3d9FormatInfo = d3d9::GetTextureFormatInfo(mInternalFormat);

    ASSERT(area.x % d3d9::GetD3DFormatInfo(d3d9FormatInfo.texFormat).blockWidth == 0);
    ASSERT(area.y % d3d9::GetD3DFormatInfo(d3d9FormatInfo.texFormat).blockHeight == 0);

    ASSERT(d3d9FormatInfo.loadFunction != nullptr);

    RECT lockRect = {area.x, area.y, area.x + area.width, area.y + area.height};

    D3DLOCKED_RECT locked;
    ANGLE_TRY(lock(context9, &locked, lockRect));

    d3d9FormatInfo.loadFunction(context9->getImageLoadContext(), area.width, area.height,
                                area.depth, static_cast<const uint8_t *>(input), inputRowPitch,
                                inputDepthPitch, static_cast<uint8_t *>(locked.pBits), locked.Pitch,
                                0);

    unlock();

    return angle::Result::Continue;
}

// This implements glCopyTex[Sub]Image2D for non-renderable internal texture formats and incomplete
// textures
angle::Result Image9::copyFromRTInternal(Context9 *context9,
                                         const gl::Offset &destOffset,
                                         const gl::Rectangle &sourceArea,
                                         RenderTargetD3D *source)
{
    ASSERT(source);

    // ES3.0 only behaviour to copy into a 3d texture
    ASSERT(destOffset.z == 0);

    RenderTarget9 *renderTarget = GetAs<RenderTarget9>(source);

    angle::ComPtr<IDirect3DSurface9> surface = renderTarget->getSurface();
    ASSERT(surface);

    IDirect3DDevice9 *device = mRenderer->getDevice();

    angle::ComPtr<IDirect3DSurface9> renderTargetData = nullptr;
    D3DSURFACE_DESC description;
    surface->GetDesc(&description);

    HRESULT hr = device->CreateOffscreenPlainSurface(description.Width, description.Height,
                                                     description.Format, D3DPOOL_SYSTEMMEM,
                                                     &renderTargetData, nullptr);

    ANGLE_TRY_HR(context9, hr, "Could not create matching destination surface");

    hr = device->GetRenderTargetData(surface.Get(), renderTargetData.Get());

    ANGLE_TRY_HR(context9, hr, "GetRenderTargetData unexpectedly failed");

    int width  = sourceArea.width;
    int height = sourceArea.height;

    RECT sourceRect = {sourceArea.x, sourceArea.y, sourceArea.x + width, sourceArea.y + height};
    RECT destRect   = {destOffset.x, destOffset.y, destOffset.x + width, destOffset.y + height};

    D3DLOCKED_RECT sourceLock = {};
    hr                        = renderTargetData->LockRect(&sourceLock, &sourceRect, 0);

    ANGLE_TRY_HR(context9, hr, "Failed to lock the source surface (rectangle might be invalid)");

    D3DLOCKED_RECT destLock = {};
    angle::Result result    = lock(context9, &destLock, destRect);
    if (result == angle::Result::Stop)
    {
        renderTargetData->UnlockRect();
    }
    ANGLE_TRY(result);

    ASSERT(destLock.pBits && sourceLock.pBits);

    unsigned char *sourcePixels = (unsigned char *)sourceLock.pBits;
    unsigned char *destPixels   = (unsigned char *)destLock.pBits;

    switch (description.Format)
    {
        case D3DFMT_X8R8G8B8:
        case D3DFMT_A8R8G8B8:
            switch (getD3DFormat())
            {
                case D3DFMT_X8R8G8B8:
                case D3DFMT_A8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        memcpy(destPixels, sourcePixels, 4 * width);
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_L8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            destPixels[x] = sourcePixels[x * 4 + 2];
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A8L8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            destPixels[x * 2 + 0] = sourcePixels[x * 4 + 2];
                            destPixels[x * 2 + 1] = sourcePixels[x * 4 + 3];
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A4L4:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned char r = sourcePixels[x * 4 + 2];
                            unsigned char a = sourcePixels[x * 4 + 3];
                            destPixels[x]   = ((a >> 4) << 4) | (r >> 4);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case D3DFMT_R5G6B5:
            switch (getD3DFormat())
            {
                case D3DFMT_X8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned short rgb  = ((unsigned short *)sourcePixels)[x];
                            unsigned char red   = static_cast<unsigned char>((rgb & 0xF800) >> 8);
                            unsigned char green = static_cast<unsigned char>((rgb & 0x07E0) >> 3);
                            unsigned char blue  = static_cast<unsigned char>((rgb & 0x001F) << 3);
                            destPixels[x + 0]   = blue | (blue >> 5);
                            destPixels[x + 1]   = green | (green >> 6);
                            destPixels[x + 2]   = red | (red >> 5);
                            destPixels[x + 3]   = 0xFF;
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_L8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned char red = sourcePixels[x * 2 + 1] & 0xF8;
                            destPixels[x]     = red | (red >> 5);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case D3DFMT_A1R5G5B5:
            switch (getD3DFormat())
            {
                case D3DFMT_X8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned short argb = ((unsigned short *)sourcePixels)[x];
                            unsigned char red   = static_cast<unsigned char>((argb & 0x7C00) >> 7);
                            unsigned char green = static_cast<unsigned char>((argb & 0x03E0) >> 2);
                            unsigned char blue  = static_cast<unsigned char>((argb & 0x001F) << 3);
                            destPixels[x + 0]   = blue | (blue >> 5);
                            destPixels[x + 1]   = green | (green >> 5);
                            destPixels[x + 2]   = red | (red >> 5);
                            destPixels[x + 3]   = 0xFF;
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned short argb = ((unsigned short *)sourcePixels)[x];
                            unsigned char red   = static_cast<unsigned char>((argb & 0x7C00) >> 7);
                            unsigned char green = static_cast<unsigned char>((argb & 0x03E0) >> 2);
                            unsigned char blue  = static_cast<unsigned char>((argb & 0x001F) << 3);
                            unsigned char alpha = (signed short)argb >> 15;
                            destPixels[x + 0]   = blue | (blue >> 5);
                            destPixels[x + 1]   = green | (green >> 5);
                            destPixels[x + 2]   = red | (red >> 5);
                            destPixels[x + 3]   = alpha;
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_L8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned char red = sourcePixels[x * 2 + 1] & 0x7C;
                            destPixels[x]     = (red << 1) | (red >> 4);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A8L8:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned char red     = sourcePixels[x * 2 + 1] & 0x7C;
                            destPixels[x * 2 + 0] = (red << 1) | (red >> 4);
                            destPixels[x * 2 + 1] = (signed char)sourcePixels[x * 2 + 1] >> 7;
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A4L4:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            unsigned char r = (sourcePixels[x * 2 + 1] & 0x7C) >> 3;
                            unsigned char a = (sourcePixels[x * 2 + 1] >> 7) ? 0xF : 0x0;
                            destPixels[x]   = (a << 4) | (r >> 3);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case D3DFMT_A16B16G16R16F:
            switch (getD3DFormat())
            {
                case D3DFMT_X8R8G8B8:
                case D3DFMT_A8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        const uint16_t *sourcePixels16F =
                            reinterpret_cast<uint16_t *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r = gl::float16ToFloat32(sourcePixels16F[x * 4 + 0]);
                            float g = gl::float16ToFloat32(sourcePixels16F[x * 4 + 1]);
                            float b = gl::float16ToFloat32(sourcePixels16F[x * 4 + 2]);
                            float a = gl::float16ToFloat32(sourcePixels16F[x * 4 + 3]);
                            destPixels[x * 4 + 0] = gl::floatToNormalized<uint8_t>(b);
                            destPixels[x * 4 + 1] = gl::floatToNormalized<uint8_t>(g);
                            destPixels[x * 4 + 2] = gl::floatToNormalized<uint8_t>(r);
                            destPixels[x * 4 + 3] = gl::floatToNormalized<uint8_t>(a);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_L8:
                    for (int y = 0; y < height; y++)
                    {
                        const uint16_t *sourcePixels16F =
                            reinterpret_cast<uint16_t *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r       = gl::float16ToFloat32(sourcePixels16F[x * 4]);
                            destPixels[x] = gl::floatToNormalized<uint8_t>(r);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A8L8:
                    for (int y = 0; y < height; y++)
                    {
                        const uint16_t *sourcePixels16F =
                            reinterpret_cast<uint16_t *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r = gl::float16ToFloat32(sourcePixels16F[x * 4 + 0]);
                            float a = gl::float16ToFloat32(sourcePixels16F[x * 4 + 3]);
                            destPixels[x * 2 + 0] = gl::floatToNormalized<uint8_t>(r);
                            destPixels[x * 2 + 1] = gl::floatToNormalized<uint8_t>(a);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A4L4:
                    for (int y = 0; y < height; y++)
                    {
                        const uint16_t *sourcePixels16F =
                            reinterpret_cast<uint16_t *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r       = gl::float16ToFloat32(sourcePixels16F[x * 4 + 0]);
                            float a       = gl::float16ToFloat32(sourcePixels16F[x * 4 + 3]);
                            destPixels[x] = gl::floatToNormalized<4, uint8_t>(r) |
                                            (gl::floatToNormalized<4, uint8_t>(a) << 4);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        case D3DFMT_A32B32G32R32F:
            switch (getD3DFormat())
            {
                case D3DFMT_X8R8G8B8:
                case D3DFMT_A8R8G8B8:
                    for (int y = 0; y < height; y++)
                    {
                        const float *sourcePixels32F = reinterpret_cast<float *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r               = sourcePixels32F[x * 4 + 0];
                            float g               = sourcePixels32F[x * 4 + 1];
                            float b               = sourcePixels32F[x * 4 + 2];
                            float a               = sourcePixels32F[x * 4 + 3];
                            destPixels[x * 4 + 0] = gl::floatToNormalized<uint8_t>(b);
                            destPixels[x * 4 + 1] = gl::floatToNormalized<uint8_t>(g);
                            destPixels[x * 4 + 2] = gl::floatToNormalized<uint8_t>(r);
                            destPixels[x * 4 + 3] = gl::floatToNormalized<uint8_t>(a);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_L8:
                    for (int y = 0; y < height; y++)
                    {
                        const float *sourcePixels32F = reinterpret_cast<float *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r       = sourcePixels32F[x * 4];
                            destPixels[x] = gl::floatToNormalized<uint8_t>(r);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A8L8:
                    for (int y = 0; y < height; y++)
                    {
                        const float *sourcePixels32F = reinterpret_cast<float *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r               = sourcePixels32F[x * 4 + 0];
                            float a               = sourcePixels32F[x * 4 + 3];
                            destPixels[x * 2 + 0] = gl::floatToNormalized<uint8_t>(r);
                            destPixels[x * 2 + 1] = gl::floatToNormalized<uint8_t>(a);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                case D3DFMT_A4L4:
                    for (int y = 0; y < height; y++)
                    {
                        const float *sourcePixels32F = reinterpret_cast<float *>(sourcePixels);
                        for (int x = 0; x < width; x++)
                        {
                            float r       = sourcePixels32F[x * 4 + 0];
                            float a       = sourcePixels32F[x * 4 + 3];
                            destPixels[x] = gl::floatToNormalized<4, uint8_t>(r) |
                                            (gl::floatToNormalized<4, uint8_t>(a) << 4);
                        }
                        sourcePixels += sourceLock.Pitch;
                        destPixels += destLock.Pitch;
                    }
                    break;
                default:
                    UNREACHABLE();
            }
            break;
        default:
            UNREACHABLE();
    }

    unlock();
    renderTargetData->UnlockRect();

    mDirty = true;
    return angle::Result::Continue;
}

angle::Result Image9::copyFromTexStorage(const gl::Context *context,
                                         const gl::ImageIndex &imageIndex,
                                         TextureStorage *source)
{
    RenderTargetD3D *renderTarget = nullptr;
    ANGLE_TRY(source->getRenderTarget(context, imageIndex, 0, &renderTarget));

    gl::Rectangle sourceArea(0, 0, mWidth, mHeight);
    return copyFromRTInternal(GetImplAs<Context9>(context), gl::Offset(), sourceArea, renderTarget);
}

angle::Result Image9::copyFromFramebuffer(const gl::Context *context,
                                          const gl::Offset &destOffset,
                                          const gl::Rectangle &sourceArea,
                                          const gl::Framebuffer *source)
{
    const gl::FramebufferAttachment *srcAttachment = source->getReadColorAttachment();
    ASSERT(srcAttachment);

    RenderTargetD3D *renderTarget = nullptr;
    ANGLE_TRY(srcAttachment->getRenderTarget(context, 0, &renderTarget));
    ASSERT(renderTarget);
    return copyFromRTInternal(GetImplAs<Context9>(context), destOffset, sourceArea, renderTarget);
}

}  // namespace rx
