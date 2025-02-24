
//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureMtl.mm:
//    Implements the class methods for TextureMtl.
//

#include "libANGLE/renderer/metal/TextureMtl.h"

#include <algorithm>
#include <initializer_list>

#include "common/Color.h"
#include "common/MemoryBuffer.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "image_util/imageformats.h"
#include "image_util/loadimage.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/metal/BufferMtl.h"
#include "libANGLE/renderer/metal/ContextMtl.h"
#include "libANGLE/renderer/metal/DisplayMtl.h"
#include "libANGLE/renderer/metal/FrameBufferMtl.h"
#include "libANGLE/renderer/metal/ImageMtl.h"
#include "libANGLE/renderer/metal/SamplerMtl.h"
#include "libANGLE/renderer/metal/SurfaceMtl.h"
#include "libANGLE/renderer/metal/mtl_common.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_utils.h"
#include "libANGLE/renderer/renderer_utils.h"

namespace rx
{

namespace
{

gl::ImageIndex GetZeroLevelIndex(const mtl::TextureRef &image)
{
    switch (image->textureType())
    {
        case MTLTextureType2D:
            return gl::ImageIndex::Make2D(0);
        case MTLTextureTypeCube:
            return gl::ImageIndex::MakeFromType(gl::TextureType::CubeMap, 0);
        case MTLTextureType2DArray:
            return gl::ImageIndex::Make2DArray(0 /** entire layers */);
        case MTLTextureType2DMultisample:
            return gl::ImageIndex::Make2DMultisample();
        case MTLTextureType3D:
            return gl::ImageIndex::Make3D(0 /** entire layers */);
        default:
            UNREACHABLE();
            break;
    }

    return gl::ImageIndex();
}

// Slice is ignored if texture type is not Cube or 2D array
gl::ImageIndex GetCubeOrArraySliceMipIndex(const mtl::TextureRef &image,
                                           uint32_t slice,
                                           uint32_t level)
{
    switch (image->textureType())
    {
        case MTLTextureType2D:
            return gl::ImageIndex::Make2D(level);
        case MTLTextureTypeCube:
        {
            auto cubeFace = static_cast<gl::TextureTarget>(
                static_cast<int>(gl::TextureTarget::CubeMapPositiveX) + slice);
            return gl::ImageIndex::MakeCubeMapFace(cubeFace, level);
        }
        case MTLTextureType2DArray:
            return gl::ImageIndex::Make2DArray(level, slice);
        case MTLTextureType2DMultisample:
            return gl::ImageIndex::Make2DMultisample();
        case MTLTextureType3D:
            return gl::ImageIndex::Make3D(level);
        default:
            UNREACHABLE();
            break;
    }

    return gl::ImageIndex();
}

// layer is ignored if texture type is not Cube or 2D array or 3D
gl::ImageIndex GetLayerMipIndex(const mtl::TextureRef &image, uint32_t layer, uint32_t level)
{
    switch (image->textureType())
    {
        case MTLTextureType2D:
            return gl::ImageIndex::Make2D(level);
        case MTLTextureTypeCube:
        {
            auto cubeFace = static_cast<gl::TextureTarget>(
                static_cast<int>(gl::TextureTarget::CubeMapPositiveX) + layer);
            return gl::ImageIndex::MakeCubeMapFace(cubeFace, level);
        }
        case MTLTextureType2DArray:
            return gl::ImageIndex::Make2DArray(level, layer);
        case MTLTextureType2DMultisample:
            return gl::ImageIndex::Make2DMultisample();
        case MTLTextureType3D:
            return gl::ImageIndex::Make3D(level, layer);
        default:
            UNREACHABLE();
            break;
    }

    return gl::ImageIndex();
}

GLuint GetImageLayerIndexFrom(const gl::ImageIndex &index)
{
    switch (index.getType())
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::Rectangle:
            return 0;
        case gl::TextureType::CubeMap:
            return index.cubeMapFaceIndex();
        case gl::TextureType::_2DArray:
        case gl::TextureType::_3D:
            return index.getLayerIndex();
        default:
            UNREACHABLE();
    }

    return 0;
}

GLuint GetImageCubeFaceIndexOrZeroFrom(const gl::ImageIndex &index)
{
    switch (index.getType())
    {
        case gl::TextureType::CubeMap:
            return index.cubeMapFaceIndex();
        default:
            break;
    }

    return 0;
}

// Given texture type, get texture type of one image for a glTexImage call.
// For example, for texture 2d, one image is also texture 2d.
// for texture cube, one image is texture 2d.
gl::TextureType GetTextureImageType(gl::TextureType texType)
{
    switch (texType)
    {
        case gl::TextureType::CubeMap:
            return gl::TextureType::_2D;
        case gl::TextureType::_2D:
        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::_3D:
        case gl::TextureType::Rectangle:
            return texType;
        default:
            UNREACHABLE();
            return gl::TextureType::InvalidEnum;
    }
}

// D24X8 by default writes depth data to high 24 bits of 32 bit integers. However, Metal separate
// depth stencil blitting expects depth data to be in low 24 bits of the data.
void WriteDepthStencilToDepth24(const uint8_t *srcPtr, uint8_t *dstPtr)
{
    auto src = reinterpret_cast<const angle::DepthStencil *>(srcPtr);
    auto dst = reinterpret_cast<uint32_t *>(dstPtr);
    *dst     = gl::floatToNormalized<24, uint32_t>(static_cast<float>(src->depth));
}

void CopyTextureData(const MTLSize &regionSize,
                     size_t srcRowPitch,
                     size_t src2DImageSize,
                     const uint8_t *psrc,
                     size_t destRowPitch,
                     size_t dest2DImageSize,
                     uint8_t *pdst)
{
    {
        size_t rowCopySize = std::min(srcRowPitch, destRowPitch);
        for (NSUInteger d = 0; d < regionSize.depth; ++d)
        {
            for (NSUInteger r = 0; r < regionSize.height; ++r)
            {
                const uint8_t *pCopySrc = psrc + d * src2DImageSize + r * srcRowPitch;
                uint8_t *pCopyDst       = pdst + d * dest2DImageSize + r * destRowPitch;
                memcpy(pCopyDst, pCopySrc, rowCopySize);
            }
        }
    }
}

void ConvertDepthStencilData(const MTLSize &regionSize,
                             const angle::Format &srcAngleFormat,
                             size_t srcRowPitch,
                             size_t src2DImageSize,
                             const uint8_t *psrc,
                             const angle::Format &dstAngleFormat,
                             rx::PixelWriteFunction pixelWriteFunctionOverride,
                             size_t destRowPitch,
                             size_t dest2DImageSize,
                             uint8_t *pdst)
{
    if (srcAngleFormat.id == dstAngleFormat.id)
    {
        size_t rowCopySize = std::min(srcRowPitch, destRowPitch);
        for (NSUInteger d = 0; d < regionSize.depth; ++d)
        {
            for (NSUInteger r = 0; r < regionSize.height; ++r)
            {
                const uint8_t *pCopySrc = psrc + d * src2DImageSize + r * srcRowPitch;
                uint8_t *pCopyDst       = pdst + d * dest2DImageSize + r * destRowPitch;
                memcpy(pCopyDst, pCopySrc, rowCopySize);
            }
        }
    }
    else
    {
        rx::PixelWriteFunction pixelWriteFunction = pixelWriteFunctionOverride
                                                        ? pixelWriteFunctionOverride
                                                        : dstAngleFormat.pixelWriteFunction;
        // This is only for depth & stencil case.
        ASSERT(srcAngleFormat.depthBits || srcAngleFormat.stencilBits);
        ASSERT(srcAngleFormat.pixelReadFunction && pixelWriteFunction);

        // cache to store read result of source pixel
        angle::DepthStencil depthStencilData;
        auto sourcePixelReadData = reinterpret_cast<uint8_t *>(&depthStencilData);
        ASSERT(srcAngleFormat.pixelBytes <= sizeof(depthStencilData));

        for (NSUInteger d = 0; d < regionSize.depth; ++d)
        {
            for (NSUInteger r = 0; r < regionSize.height; ++r)
            {
                for (NSUInteger c = 0; c < regionSize.width; ++c)
                {
                    const uint8_t *sourcePixelData =
                        psrc + d * src2DImageSize + r * srcRowPitch + c * srcAngleFormat.pixelBytes;

                    uint8_t *destPixelData = pdst + d * dest2DImageSize + r * destRowPitch +
                                             c * dstAngleFormat.pixelBytes;

                    srcAngleFormat.pixelReadFunction(sourcePixelData, sourcePixelReadData);
                    pixelWriteFunction(sourcePixelReadData, destPixelData);
                }
            }
        }
    }
}

mtl::BlitCommandEncoder *GetBlitCommandEncoderForResources(
    ContextMtl *contextMtl,
    const std::initializer_list<const mtl::Resource *> &resources)
{
    if (std::none_of(resources.begin(), resources.end(), [contextMtl](const mtl::Resource *res) {
            return res->hasPendingRenderWorks(contextMtl);
        }))
    {
        // If no resource has pending render works waiting to be submitted, then it's safe to
        // create a blit encoder without ending current render pass. The blit commands
        // will run before any pending render commands.
        return contextMtl->getBlitCommandEncoderWithoutEndingRenderEncoder();
    }
    return contextMtl->getBlitCommandEncoder();
}

angle::Result CopyDepthStencilTextureContentsToStagingBuffer(
    ContextMtl *contextMtl,
    const angle::Format &textureAngleFormat,
    const angle::Format &stagingAngleFormat,
    rx::PixelWriteFunction pixelWriteFunctionOverride,
    const MTLSize &regionSize,
    const uint8_t *data,
    size_t bytesPerRow,
    size_t bytesPer2DImage,
    size_t *bufferRowPitchOut,
    size_t *buffer2DImageSizeOut,
    mtl::BufferRef *bufferOut)
{
    size_t stagingBufferRowPitch    = regionSize.width * stagingAngleFormat.pixelBytes;
    size_t stagingBuffer2DImageSize = stagingBufferRowPitch * regionSize.height;
    size_t stagingBufferSize        = stagingBuffer2DImageSize * regionSize.depth;
    mtl::BufferRef stagingBuffer;
    ANGLE_TRY(mtl::Buffer::MakeBuffer(contextMtl, stagingBufferSize, nullptr, &stagingBuffer));

    uint8_t *pdst = stagingBuffer->map(contextMtl);

    ConvertDepthStencilData(regionSize, textureAngleFormat, bytesPerRow, bytesPer2DImage, data,
                            stagingAngleFormat, pixelWriteFunctionOverride, stagingBufferRowPitch,
                            stagingBuffer2DImageSize, pdst);

    stagingBuffer->unmap(contextMtl);

    *bufferOut            = stagingBuffer;
    *bufferRowPitchOut    = stagingBufferRowPitch;
    *buffer2DImageSizeOut = stagingBuffer2DImageSize;

    return angle::Result::Continue;
}

angle::Result CopyTextureContentsToStagingBuffer(ContextMtl *contextMtl,
                                                 const angle::Format &textureAngleFormat,
                                                 const MTLSize &regionSize,
                                                 const uint8_t *data,
                                                 size_t bytesPerRow,
                                                 size_t bytesPer2DImage,
                                                 size_t *bufferRowPitchOut,
                                                 size_t *buffer2DImageSizeOut,
                                                 mtl::BufferRef *bufferOut)
{
    size_t stagingBufferRowPitch    = regionSize.width * textureAngleFormat.pixelBytes;
    size_t stagingBuffer2DImageSize = stagingBufferRowPitch * regionSize.height;
    size_t stagingBufferSize        = stagingBuffer2DImageSize * regionSize.depth;
    mtl::BufferRef stagingBuffer;
    ANGLE_TRY(mtl::Buffer::MakeBuffer(contextMtl, stagingBufferSize, nullptr, &stagingBuffer));

    uint8_t *pdst = stagingBuffer->map(contextMtl);
    CopyTextureData(regionSize, bytesPerRow, bytesPer2DImage, data, stagingBufferRowPitch,
                    stagingBuffer2DImageSize, pdst);

    stagingBuffer->unmap(contextMtl);

    *bufferOut            = stagingBuffer;
    *bufferRowPitchOut    = stagingBufferRowPitch;
    *buffer2DImageSizeOut = stagingBuffer2DImageSize;

    return angle::Result::Continue;
}

angle::Result CopyCompressedTextureContentsToStagingBuffer(ContextMtl *contextMtl,
                                                           const angle::Format &textureAngleFormat,
                                                           const MTLSize &regionSizeInBlocks,
                                                           const uint8_t *data,
                                                           size_t bytesPerBlockRow,
                                                           size_t bytesPer2DImage,
                                                           size_t *bufferRowPitchOut,
                                                           size_t *buffer2DImageSizeOut,
                                                           mtl::BufferRef *bufferOut)
{
    size_t stagingBufferRowPitch    = bytesPerBlockRow;
    size_t stagingBuffer2DImageSize = bytesPer2DImage;
    size_t stagingBufferSize        = stagingBuffer2DImageSize * regionSizeInBlocks.depth;
    mtl::BufferRef stagingBuffer;
    ANGLE_TRY(mtl::Buffer::MakeBuffer(contextMtl, stagingBufferSize, nullptr, &stagingBuffer));

    uint8_t *pdst = stagingBuffer->map(contextMtl);
    CopyTextureData(regionSizeInBlocks, bytesPerBlockRow, bytesPer2DImage, data,
                    stagingBufferRowPitch, stagingBuffer2DImageSize, pdst);

    stagingBuffer->unmap(contextMtl);

    *bufferOut            = stagingBuffer;
    *bufferRowPitchOut    = stagingBufferRowPitch;
    *buffer2DImageSizeOut = stagingBuffer2DImageSize;

    return angle::Result::Continue;
}

angle::Result SaturateDepth(ContextMtl *contextMtl,
                            mtl::BufferRef srcBuffer,
                            mtl::BufferRef dstBuffer,
                            uint32_t srcBufferOffset,
                            uint32_t srcPitch,
                            MTLSize size)
{
    static_assert(gl::IMPLEMENTATION_MAX_2D_TEXTURE_SIZE <= UINT_MAX);
    mtl::DepthSaturationParams params;
    params.srcBuffer       = srcBuffer;
    params.dstBuffer       = dstBuffer;
    params.srcBufferOffset = srcBufferOffset;
    params.dstWidth        = static_cast<uint32_t>(size.width);
    params.dstHeight       = static_cast<uint32_t>(size.height);
    params.srcPitch        = srcPitch;
    ANGLE_TRY(contextMtl->getDisplay()->getUtils().saturateDepth(contextMtl, params));

    return angle::Result::Continue;
}

// This will copy a buffer to:
// - the respective level & slice of an original texture if the "dst" texture is a view.
// - the "dst" texture if it is not a view.
// Notes:
// - dstSlice is a slice in the "dst" texture not original texture.
// - dstLevel is a level in the "dst" texture not original texture.
// This function is needed because some GPUs such as the ones having AMD Bronze driver
// have a bug when copying a buffer to a view of a 3D texture.
void CopyBufferToOriginalTextureIfDstIsAView(ContextMtl *contextMtl,
                                             mtl::BlitCommandEncoder *blitEncoder,
                                             const mtl::BufferRef &src,
                                             size_t srcOffset,
                                             size_t srcBytesPerRow,
                                             size_t srcBytesPerImage,
                                             MTLSize srcSize,
                                             const mtl::TextureRef &dst,
                                             const uint32_t dstSlice,
                                             const mtl::MipmapNativeLevel &dstLevel,
                                             MTLOrigin dstOrigin,
                                             MTLBlitOption blitOption)
{
    mtl::TextureRef correctedTexture      = dst;
    mtl::MipmapNativeLevel correctedLevel = dstLevel;
    uint32_t correctedSlice               = dstSlice;
    // TODO(b/343734719): Simulator has bug in parentRelativeSlice() so skip this step
    // on simulator.
    if (!contextMtl->getDisplay()->isSimulator() && correctedTexture->parentTexture())
    {
        correctedLevel = correctedLevel + correctedTexture->parentRelativeLevel().get();
        correctedSlice += correctedTexture->parentRelativeSlice();
        correctedTexture = correctedTexture->parentTexture();
    }

    blitEncoder->copyBufferToTexture(src, srcOffset, srcBytesPerRow, srcBytesPerImage, srcSize,
                                     correctedTexture, correctedSlice, correctedLevel, dstOrigin,
                                     blitOption);
}

angle::Result UploadDepthStencilTextureContentsWithStagingBuffer(
    ContextMtl *contextMtl,
    const angle::Format &textureAngleFormat,
    MTLRegion region,
    const mtl::MipmapNativeLevel &mipmapLevel,
    uint32_t slice,
    const uint8_t *data,
    size_t bytesPerRow,
    size_t bytesPer2DImage,
    const mtl::TextureRef &texture)
{
    ASSERT(texture && texture->valid());

    ASSERT(!texture->isCPUAccessible());

    ASSERT(!textureAngleFormat.depthBits || !textureAngleFormat.stencilBits);

    // Depth and stencil textures cannot be of 3D type;
    // arrays and cube maps must be uploaded per-slice.
    ASSERT(region.size.depth == 1);

    // Copy data to staging buffer
    size_t stagingBufferRowPitch;
    size_t stagingBuffer2DImageSize;
    mtl::BufferRef stagingBuffer;
    ANGLE_TRY(CopyDepthStencilTextureContentsToStagingBuffer(
        contextMtl, textureAngleFormat, textureAngleFormat, textureAngleFormat.pixelWriteFunction,
        region.size, data, bytesPerRow, bytesPer2DImage, &stagingBufferRowPitch,
        &stagingBuffer2DImageSize, &stagingBuffer));

    if (textureAngleFormat.id == angle::FormatID::D32_FLOAT)
    {
        ANGLE_TRY(SaturateDepth(contextMtl, stagingBuffer, stagingBuffer, 0,
                                static_cast<uint32_t>(region.size.width), region.size));
    }

    // Copy staging buffer to texture.
    mtl::BlitCommandEncoder *encoder =
        GetBlitCommandEncoderForResources(contextMtl, {stagingBuffer.get(), texture.get()});

    CopyBufferToOriginalTextureIfDstIsAView(
        contextMtl, encoder, stagingBuffer, 0, stagingBufferRowPitch, stagingBuffer2DImageSize,
        region.size, texture, slice, mipmapLevel, region.origin, MTLBlitOptionNone);

    return angle::Result::Continue;
}

// Packed depth stencil upload using staging buffer
angle::Result UploadPackedDepthStencilTextureContentsWithStagingBuffer(
    ContextMtl *contextMtl,
    const angle::Format &textureAngleFormat,
    MTLRegion region,
    const mtl::MipmapNativeLevel &mipmapLevel,
    uint32_t slice,
    const uint8_t *data,
    size_t bytesPerRow,
    size_t bytesPer2DImage,
    const mtl::TextureRef &texture)
{
    ASSERT(texture && texture->valid());

    ASSERT(!texture->isCPUAccessible());

    ASSERT(textureAngleFormat.depthBits && textureAngleFormat.stencilBits);

    // Depth and stencil textures cannot be of 3D type;
    // arrays and cube maps must be uploaded per-slice.
    ASSERT(region.size.depth == 1);

    // We have to split the depth & stencil data into 2 buffers.
    angle::FormatID stagingDepthBufferFormatId;
    angle::FormatID stagingStencilBufferFormatId;
    // Custom depth write function. We cannot use those in imageformats.cpp since Metal has some
    // special cases.
    rx::PixelWriteFunction stagingDepthBufferWriteFunctionOverride = nullptr;

    switch (textureAngleFormat.id)
    {
        case angle::FormatID::D24_UNORM_S8_UINT:
            // D24_UNORM_X8_UINT writes depth data to high 24 bits. But Metal expects depth data to
            // be in low 24 bits.
            stagingDepthBufferFormatId              = angle::FormatID::D24_UNORM_X8_UINT;
            stagingDepthBufferWriteFunctionOverride = WriteDepthStencilToDepth24;
            stagingStencilBufferFormatId            = angle::FormatID::S8_UINT;
            break;
        case angle::FormatID::D32_FLOAT_S8X24_UINT:
            stagingDepthBufferFormatId   = angle::FormatID::D32_FLOAT;
            stagingStencilBufferFormatId = angle::FormatID::S8_UINT;
            break;
        default:
            ANGLE_GL_UNREACHABLE(contextMtl);
    }

    const angle::Format &angleStagingDepthFormat = angle::Format::Get(stagingDepthBufferFormatId);
    const angle::Format &angleStagingStencilFormat =
        angle::Format::Get(stagingStencilBufferFormatId);

    size_t stagingDepthBufferRowPitch, stagingStencilBufferRowPitch;
    size_t stagingDepthBuffer2DImageSize, stagingStencilBuffer2DImageSize;
    mtl::BufferRef stagingDepthBuffer, stagingStencilBuffer;

    // Copy depth data to staging depth buffer
    ANGLE_TRY(CopyDepthStencilTextureContentsToStagingBuffer(
        contextMtl, textureAngleFormat, angleStagingDepthFormat,
        stagingDepthBufferWriteFunctionOverride, region.size, data, bytesPerRow, bytesPer2DImage,
        &stagingDepthBufferRowPitch, &stagingDepthBuffer2DImageSize, &stagingDepthBuffer));

    // Copy stencil data to staging stencil buffer
    ANGLE_TRY(CopyDepthStencilTextureContentsToStagingBuffer(
        contextMtl, textureAngleFormat, angleStagingStencilFormat, nullptr, region.size, data,
        bytesPerRow, bytesPer2DImage, &stagingStencilBufferRowPitch,
        &stagingStencilBuffer2DImageSize, &stagingStencilBuffer));

    if (angleStagingDepthFormat.id == angle::FormatID::D32_FLOAT)
    {
        ANGLE_TRY(SaturateDepth(contextMtl, stagingDepthBuffer, stagingDepthBuffer, 0,
                                static_cast<uint32_t>(region.size.width), region.size));
    }

    mtl::BlitCommandEncoder *encoder = GetBlitCommandEncoderForResources(
        contextMtl, {stagingDepthBuffer.get(), stagingStencilBuffer.get(), texture.get()});

    CopyBufferToOriginalTextureIfDstIsAView(
        contextMtl, encoder, stagingDepthBuffer, 0, stagingDepthBufferRowPitch,
        stagingDepthBuffer2DImageSize, region.size, texture, slice, mipmapLevel, region.origin,
        MTLBlitOptionDepthFromDepthStencil);
    CopyBufferToOriginalTextureIfDstIsAView(
        contextMtl, encoder, stagingStencilBuffer, 0, stagingStencilBufferRowPitch,
        stagingStencilBuffer2DImageSize, region.size, texture, slice, mipmapLevel, region.origin,
        MTLBlitOptionStencilFromDepthStencil);

    return angle::Result::Continue;
}

angle::Result UploadTextureContentsWithStagingBuffer(ContextMtl *contextMtl,
                                                     const angle::Format &textureAngleFormat,
                                                     MTLRegion region,
                                                     const mtl::MipmapNativeLevel &mipmapLevel,
                                                     uint32_t slice,
                                                     const uint8_t *data,
                                                     size_t bytesPerRow,
                                                     size_t bytesPer2DImage,
                                                     const mtl::TextureRef &texture)
{
    ASSERT(texture && texture->valid());

    angle::FormatID stagingBufferFormatID   = textureAngleFormat.id;
    const angle::Format &angleStagingFormat = angle::Format::Get(stagingBufferFormatID);

    size_t stagingBufferRowPitch;
    size_t stagingBuffer2DImageSize;
    mtl::BufferRef stagingBuffer;

    // Block-compressed formats need a bit of massaging for copy.
    if (textureAngleFormat.isBlock)
    {
        GLenum internalFormat         = textureAngleFormat.glInternalFormat;
        const gl::InternalFormat &fmt = gl::GetSizedInternalFormatInfo(internalFormat);
        MTLRegion newRegion           = region;
        bytesPerRow =
            (region.size.width + fmt.compressedBlockWidth - 1) / fmt.compressedBlockWidth * 16;
        bytesPer2DImage = (region.size.height + fmt.compressedBlockHeight - 1) /
                          fmt.compressedBlockHeight * bytesPerRow;
        newRegion.size.width =
            (region.size.width + fmt.compressedBlockWidth - 1) / fmt.compressedBlockWidth;
        newRegion.size.height =
            (region.size.height + fmt.compressedBlockHeight - 1) / fmt.compressedBlockHeight;
        ANGLE_TRY(CopyCompressedTextureContentsToStagingBuffer(
            contextMtl, angleStagingFormat, newRegion.size, data, bytesPerRow, bytesPer2DImage,
            &stagingBufferRowPitch, &stagingBuffer2DImageSize, &stagingBuffer));
    }
    // Copy to staging buffer before uploading to texture.
    else
    {
        ANGLE_TRY(CopyTextureContentsToStagingBuffer(
            contextMtl, angleStagingFormat, region.size, data, bytesPerRow, bytesPer2DImage,
            &stagingBufferRowPitch, &stagingBuffer2DImageSize, &stagingBuffer));
    }
    mtl::BlitCommandEncoder *encoder =
        GetBlitCommandEncoderForResources(contextMtl, {stagingBuffer.get(), texture.get()});

    CopyBufferToOriginalTextureIfDstIsAView(
        contextMtl, encoder, stagingBuffer, 0, stagingBufferRowPitch, stagingBuffer2DImageSize,
        region.size, texture, slice, mipmapLevel, region.origin, 0);

    return angle::Result::Continue;
}

angle::Result UploadTextureContents(const gl::Context *context,
                                    const angle::Format &textureAngleFormat,
                                    const MTLRegion &region,
                                    const mtl::MipmapNativeLevel &mipmapLevel,
                                    uint32_t slice,
                                    const uint8_t *data,
                                    size_t bytesPerRow,
                                    size_t bytesPer2DImage,
                                    bool avoidStagingBuffers,
                                    const mtl::TextureRef &texture)

{
    ASSERT(texture && texture->valid());
    ContextMtl *contextMtl       = mtl::GetImpl(context);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(textureAngleFormat.id);

    bool preferGPUInitialization =
        !avoidStagingBuffers &&
        PreferStagedTextureUploads(context, texture, mtlFormat, mtl::StagingPurpose::Upload);
    if (texture->isCPUAccessible() && !preferGPUInitialization)
    {
        if (mtlFormat.isPVRTC())
        {
            // Replace Region Validation: rowBytes must be 0
            bytesPerRow = 0;
        }

        // If texture is CPU accessible, just call replaceRegion() directly.
        texture->replaceRegion(contextMtl, region, mipmapLevel, slice, data, bytesPerRow,
                               bytesPer2DImage);

        return angle::Result::Continue;
    }

    // Texture is not CPU accessible or staging is forced due to a workaround
    if (!textureAngleFormat.depthBits && !textureAngleFormat.stencilBits)
    {
        // Upload color data
        ANGLE_TRY(UploadTextureContentsWithStagingBuffer(contextMtl, textureAngleFormat, region,
                                                         mipmapLevel, slice, data, bytesPerRow,
                                                         bytesPer2DImage, texture));
    }
    else if (textureAngleFormat.depthBits && textureAngleFormat.stencilBits)
    {
        // Packed depth-stencil
        ANGLE_TRY(UploadPackedDepthStencilTextureContentsWithStagingBuffer(
            contextMtl, textureAngleFormat, region, mipmapLevel, slice, data, bytesPerRow,
            bytesPer2DImage, texture));
    }
    else
    {
        // Depth or stencil
        ANGLE_TRY(UploadDepthStencilTextureContentsWithStagingBuffer(
            contextMtl, textureAngleFormat, region, mipmapLevel, slice, data, bytesPerRow,
            bytesPer2DImage, texture));
    }

    return angle::Result::Continue;
}

// This might be unused on platform not supporting swizzle.
ANGLE_APPLE_UNUSED
GLenum OverrideSwizzleValue(const gl::Context *context,
                            GLenum swizzle,
                            const mtl::Format &format,
                            const gl::InternalFormat &glInternalFormat)
{
    if (format.actualAngleFormat().hasDepthOrStencilBits())
    {
        ASSERT(!format.swizzled);
        if (context->getState().getClientMajorVersion() >= 3 && glInternalFormat.sized)
        {
            // ES 3.1 spec: treat depth and stencil textures as red textures during sampling.
            if (swizzle == GL_GREEN || swizzle == GL_BLUE)
            {
                return GL_NONE;
            }
            else if (swizzle == GL_ALPHA)
            {
                return GL_ONE;
            }
        }
        else
        {
            // https://www.khronos.org/registry/OpenGL/extensions/OES/OES_depth_texture.txt
            // Treat depth texture as luminance texture during sampling.
            if (swizzle == GL_GREEN || swizzle == GL_BLUE)
            {
                return GL_RED;
            }
            else if (swizzle == GL_ALPHA)
            {
                return GL_ONE;
            }
        }
    }
    else if (format.swizzled)
    {
        // Combine the swizzles
        switch (swizzle)
        {
            case GL_RED:
                return format.swizzle[0];
            case GL_GREEN:
                return format.swizzle[1];
            case GL_BLUE:
                return format.swizzle[2];
            case GL_ALPHA:
                return format.swizzle[3];
            default:
                break;
        }
    }

    return swizzle;
}

mtl::TextureRef &GetLayerLevelTextureView(
    TextureMtl::LayerLevelTextureViewVector *layerLevelTextureViews,
    uint32_t layer,
    uint32_t level,
    uint32_t layerCount,
    uint32_t levelCount)
{
    // Lazily allocate the full layer and level count to not trigger any std::vector reallocations.
    if (layerLevelTextureViews->empty())
    {
        layerLevelTextureViews->resize(layerCount);
    }
    ASSERT(layerLevelTextureViews->size() > layer);

    TextureMtl::TextureViewVector &levelTextureViews = (*layerLevelTextureViews)[layer];

    if (levelTextureViews.empty())
    {
        levelTextureViews.resize(levelCount);
    }
    ASSERT(levelTextureViews.size() > level);

    return levelTextureViews[level];
}

}  // namespace

// TextureMtl::NativeTextureWrapper implementation.
// This class uses GL level instead of mtl::MipmapNativeLevel.
// It seamlessly translates GL level to native level based on the base GL information passed in the
// constructor. The base GL level is unchanged thoughout the lifetime of this object.
// Note that NativeTextureWrapper's base GL level doesn't necessarily mean it's the same as a GL
// texture's real base level.
// - If NativeTextureWrapper holds a native storage of a non-immutable texture,
// its base GL level is indeed equal to the GL texture's base level.
// - If NativeTextureWrapper holds a native storage of an immutable texture,
// it base GL level is actually 0.
// - If NativeTextureWrapper holds a view from base level to max level of a GL texture,
// then its base GL level is equal to the GL texture's base level.
class TextureMtl::NativeTextureWrapper : angle::NonCopyable
{
  public:
    NativeTextureWrapper(mtl::TextureRef texture, GLuint baseGLLevel)
        : mNativeTexture(std::move(texture)), mBaseGLLevel(baseGLLevel)
    {
        ASSERT(mNativeTexture && mNativeTexture->valid());
    }

    operator const mtl::TextureRef &() const { return mNativeTexture; }
    const mtl::TextureRef &getNativeTexture() const { return mNativeTexture; }

    void replaceRegion(ContextMtl *context,
                       const MTLRegion &region,
                       GLuint glLevel,
                       uint32_t slice,
                       const uint8_t *data,
                       size_t bytesPerRow,
                       size_t bytesPer2DImage)
    {
        mNativeTexture->replaceRegion(context, region, getNativeLevel(glLevel), slice, data,
                                      bytesPerRow, bytesPer2DImage);
    }

    void getBytes(ContextMtl *context,
                  size_t bytesPerRow,
                  size_t bytesPer2DInage,
                  const MTLRegion &region,
                  GLuint glLevel,
                  uint32_t slice,
                  uint8_t *dataOut)
    {
        mNativeTexture->getBytes(context, bytesPerRow, bytesPer2DInage, region,
                                 getNativeLevel(glLevel), slice, dataOut);
    }

    GLuint getBaseGLLevel() const { return mBaseGLLevel; }
    // Get max addressable GL level that this texture supports.
    GLuint getMaxSupportedGLLevel() const { return mBaseGLLevel + mipmapLevels() - 1; }
    // Check whether a GL level refers to a valid mip in this texture.
    bool isGLLevelSupported(GLuint glLevel)
    {
        return glLevel >= mBaseGLLevel && glLevel <= getMaxSupportedGLLevel();
    }
    mtl::MipmapNativeLevel getNativeLevel(GLuint glLevel) const
    {
        return mtl::GetNativeMipLevel(glLevel, mBaseGLLevel);
    }
    GLuint getGLLevel(const mtl::MipmapNativeLevel &nativeLevel) const
    {
        return mtl::GetGLMipLevel(nativeLevel, mBaseGLLevel);
    }

    mtl::TextureRef getStencilView() { return mNativeTexture->getStencilView(); }

    MTLTextureType textureType() const { return mNativeTexture->textureType(); }
    MTLPixelFormat pixelFormat() const { return mNativeTexture->pixelFormat(); }

    uint32_t mipmapLevels() const { return mNativeTexture->mipmapLevels(); }
    uint32_t arrayLength() const { return mNativeTexture->arrayLength(); }
    uint32_t cubeFaces() const { return mNativeTexture->cubeFaces(); }
    uint32_t cubeFacesOrArrayLength() const { return mNativeTexture->cubeFacesOrArrayLength(); }

    uint32_t width(GLuint glLevel) const { return mNativeTexture->width(getNativeLevel(glLevel)); }
    uint32_t height(GLuint glLevel) const
    {
        return mNativeTexture->height(getNativeLevel(glLevel));
    }
    uint32_t depth(GLuint glLevel) const { return mNativeTexture->depth(getNativeLevel(glLevel)); }

    gl::Extents size(GLuint glLevel) const { return mNativeTexture->size(getNativeLevel(glLevel)); }

    // Get width, height, depth, size at base level.
    uint32_t widthAt0() const { return width(mBaseGLLevel); }
    uint32_t heightAt0() const { return height(mBaseGLLevel); }
    uint32_t depthAt0() const { return depth(mBaseGLLevel); }
    gl::Extents sizeAt0() const { return size(mBaseGLLevel); }

  protected:
    mtl::TextureRef mNativeTexture;
    const GLuint mBaseGLLevel;
};

// This class extends NativeTextureWrapper with support for view creation
class TextureMtl::NativeTextureWrapperWithViewSupport : public NativeTextureWrapper
{
  public:
    NativeTextureWrapperWithViewSupport(mtl::TextureRef texture, GLuint baseGLLevel)
        : NativeTextureWrapper(std::move(texture), baseGLLevel)
    {}

    // Create a view of one slice at a level.
    mtl::TextureRef createSliceMipView(uint32_t slice, GLuint glLevel)
    {
        return mNativeTexture->createSliceMipView(slice, getNativeLevel(glLevel));
    }
    // Create a levels range view
    mtl::TextureRef createMipsView(GLuint glLevel, uint32_t levels)
    {
        return mNativeTexture->createMipsView(getNativeLevel(glLevel), levels);
    }
    // Create a view of a level.
    mtl::TextureRef createMipView(GLuint glLevel)
    {
        return mNativeTexture->createMipView(getNativeLevel(glLevel));
    }
    // Create a view for a shader image binding.
    mtl::TextureRef createShaderImageView2D(GLuint glLevel, int layer, MTLPixelFormat format)
    {
        return mNativeTexture->createShaderImageView2D(getNativeLevel(glLevel), layer, format);
    }

    // Create a swizzled view
    mtl::TextureRef createMipsSwizzleView(GLuint glLevel,
                                          uint32_t levels,
                                          MTLPixelFormat format,
                                          const MTLTextureSwizzleChannels &swizzle)
    {
        return mNativeTexture->createMipsSwizzleView(getNativeLevel(glLevel), levels, format,
                                                     swizzle);
    }
};

// TextureMtl implementation
TextureMtl::TextureMtl(const gl::TextureState &state) : TextureImpl(state) {}

TextureMtl::~TextureMtl() = default;

void TextureMtl::onDestroy(const gl::Context *context)
{
    deallocateNativeStorage(/*keepImages=*/false);
    mBoundSurface = nullptr;
}

void TextureMtl::deallocateNativeStorage(bool keepImages, bool keepSamplerStateAndFormat)
{

    if (!keepImages)
    {
        mTexImageDefs.clear();
        mShaderImageViews.clear();
    }
    else if (mNativeTextureStorage)
    {
        // Release native texture but keep its image definitions.
        retainImageDefinitions();
    }

    mNativeTextureStorage       = nullptr;
    mViewFromBaseToMaxLevel     = nullptr;
    mSwizzleStencilSamplingView = nullptr;

    // Clear render target cache for each texture's image. We don't erase them because they
    // might still be referenced by a framebuffer.
    for (auto &samplesMapRenderTargets : mRenderTargets)
    {
        for (RenderTargetMtl &perSampleCountRenderTarget : samplesMapRenderTargets.second)
        {
            perSampleCountRenderTarget.reset();
        }
    }

    for (auto &samplesMapMSTextures : mImplicitMSTextures)
    {
        for (mtl::TextureRef &perSampleCountMSTexture : samplesMapMSTextures.second)
        {
            perSampleCountMSTexture.reset();
        }
    }

    for (mtl::TextureRef &view : mLevelViewsWithinBaseMax)
    {
        view.reset();
    }

    if (!keepSamplerStateAndFormat)
    {
        mMetalSamplerState = nil;
        mFormat            = mtl::Format();
    }
}

angle::Result TextureMtl::ensureNativeStorageCreated(const gl::Context *context)
{
    if (mNativeTextureStorage)
    {
        return angle::Result::Continue;
    }

    // This should not be called from immutable texture.
    ASSERT(!isImmutableOrPBuffer());
    ASSERT(mState.getType() != gl::TextureType::_2DMultisample);
    ASSERT(mState.getType() != gl::TextureType::_2DMultisampleArray);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    // Create actual texture object:
    GLuint mips        = mState.getMipmapMaxLevel() - mState.getEffectiveBaseLevel() + 1;
    gl::ImageDesc desc = mState.getBaseLevelDesc();
    ANGLE_CHECK(contextMtl, desc.format.valid(), gl::err::kInternalError, GL_INVALID_OPERATION);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(desc.format.info->sizedInternalFormat);
    mFormat = contextMtl->getPixelFormat(angleFormatId);

    ANGLE_TRY(createNativeStorage(context, mState.getType(), mips, 0, desc.size));

    // Transfer data from defined images to actual texture object
    int numCubeFaces = static_cast<int>(mNativeTextureStorage->cubeFaces());
    for (int face = 0; face < numCubeFaces; ++face)
    {
        for (mtl::MipmapNativeLevel actualMip = mtl::kZeroNativeMipLevel; actualMip.get() < mips;
             ++actualMip)
        {
            GLuint imageMipLevel             = mNativeTextureStorage->getGLLevel(actualMip);
            mtl::TextureRef &imageToTransfer = mTexImageDefs[face][imageMipLevel].image;

            // Only transfer if this mip & slice image has been defined and in correct size &
            // format.
            gl::Extents actualMipSize = mNativeTextureStorage->size(imageMipLevel);
            if (imageToTransfer && imageToTransfer->sizeAt0() == actualMipSize &&
                imageToTransfer->arrayLength() == mNativeTextureStorage->arrayLength() &&
                imageToTransfer->pixelFormat() == mNativeTextureStorage->pixelFormat())
            {
                mtl::BlitCommandEncoder *encoder = GetBlitCommandEncoderForResources(
                    contextMtl,
                    {imageToTransfer.get(), mNativeTextureStorage->getNativeTexture().get()});

                encoder->copyTexture(imageToTransfer, 0, mtl::kZeroNativeMipLevel,
                                     *mNativeTextureStorage, face, actualMip,
                                     imageToTransfer->arrayLength(), 1);

                // Invalidate texture image definition at this index so that we can make it a
                // view of the native texture at this index later.
                imageToTransfer = nullptr;
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureMtl::createNativeStorage(const gl::Context *context,
                                              gl::TextureType type,
                                              GLuint mips,
                                              GLuint samples,
                                              const gl::Extents &size)
{
    ASSERT(samples == 0 || mips == 0);
    ContextMtl *contextMtl = mtl::GetImpl(context);

    // Create actual texture object:
    mSlices              = 1;
    bool allowFormatView = mFormat.hasDepthAndStencilBits() ||
                           needsFormatViewForPixelLocalStorage(
                               contextMtl->getDisplay()->getNativePixelLocalStorageOptions());
    mtl::TextureRef nativeTextureStorage;
    switch (type)
    {
        case gl::TextureType::_2D:
            ANGLE_TRY(mtl::Texture::Make2DTexture(
                contextMtl, mFormat, size.width, size.height, mips,
                /** renderTargetOnly */ false, allowFormatView, &nativeTextureStorage));
            break;
        case gl::TextureType::CubeMap:
            mSlices = 6;
            ANGLE_TRY(mtl::Texture::MakeCubeTexture(contextMtl, mFormat, size.width, mips,
                                                    /** renderTargetOnly */ false, allowFormatView,
                                                    &nativeTextureStorage));
            break;
        case gl::TextureType::_3D:
            ANGLE_TRY(mtl::Texture::Make3DTexture(
                contextMtl, mFormat, size.width, size.height, size.depth, mips,
                /** renderTargetOnly */ false, allowFormatView, &nativeTextureStorage));
            break;
        case gl::TextureType::_2DArray:
            mSlices = size.depth;
            ANGLE_TRY(mtl::Texture::Make2DArrayTexture(
                contextMtl, mFormat, size.width, size.height, mips, mSlices,
                /** renderTargetOnly */ false, allowFormatView, &nativeTextureStorage));
            break;
        case gl::TextureType::_2DMultisample:
            ANGLE_TRY(mtl::Texture::Make2DMSTexture(
                contextMtl, mFormat, size.width, size.height, samples,
                /** renderTargetOnly */ false, allowFormatView, &nativeTextureStorage));
            break;
        default:
            UNREACHABLE();
    }

    if (mState.getImmutableFormat())
    {
        mNativeTextureStorage = std::make_unique<NativeTextureWrapperWithViewSupport>(
            std::move(nativeTextureStorage), /*baseGLLevel=*/0);
    }
    else
    {
        mNativeTextureStorage = std::make_unique<NativeTextureWrapperWithViewSupport>(
            std::move(nativeTextureStorage), /*baseGLLevel=*/mState.getEffectiveBaseLevel());
    }

    ANGLE_TRY(checkForEmulatedChannels(context, mFormat, *mNativeTextureStorage));

    ANGLE_TRY(createViewFromBaseToMaxLevel());

    // Create sampler state
    ANGLE_TRY(ensureSamplerStateCreated(context));

    return angle::Result::Continue;
}

angle::Result TextureMtl::ensureSamplerStateCreated(const gl::Context *context)
{
    if (mMetalSamplerState)
    {
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);

    mtl::SamplerDesc samplerDesc(mState.getSamplerState());

    if (mFormat.actualAngleFormat().depthBits && !mFormat.getCaps().filterable)
    {
        // On devices not supporting filtering for depth textures, we need to convert to nearest
        // here.
        samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
        samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
        if (samplerDesc.mipFilter != MTLSamplerMipFilterNotMipmapped)
        {
            samplerDesc.mipFilter = MTLSamplerMipFilterNearest;
        }

        samplerDesc.maxAnisotropy = 1;
    }

    // OpenGL ES 3.x: The rules for texel selection are modified
    // for cube maps so that texture wrap modes are ignored.
    if ((mState.getType() == gl::TextureType::CubeMap ||
         mState.getType() == gl::TextureType::CubeMapArray) &&
        context->getState().getClientMajorVersion() >= 3)
    {
        samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
        samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
        samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
    }
    mMetalSamplerState = contextMtl->getDisplay()->getStateCache().getSamplerState(
        contextMtl->getMetalDevice(), samplerDesc);

    return angle::Result::Continue;
}

angle::Result TextureMtl::createViewFromBaseToMaxLevel()
{
    ASSERT(mNativeTextureStorage);
    uint32_t maxLevel =
        std::min(mNativeTextureStorage->getMaxSupportedGLLevel(), mState.getEffectiveMaxLevel());

    mtl::TextureRef nativeViewFromBaseToMaxLevelRef;
    if (maxLevel == mNativeTextureStorage->getMaxSupportedGLLevel() &&
        mState.getEffectiveBaseLevel() == mNativeTextureStorage->getBaseGLLevel())
    {
        // If base & max level are the same in mNativeTextureStorage, we don't need
        // a dedicated view. Furthermore, Intel driver has some bugs when sampling a view
        // of a stencil texture.
        nativeViewFromBaseToMaxLevelRef = mNativeTextureStorage->getNativeTexture();
    }
    else
    {
        uint32_t baseToMaxLevels = maxLevel - mState.getEffectiveBaseLevel() + 1;
        nativeViewFromBaseToMaxLevelRef =
            mNativeTextureStorage->createMipsView(mState.getEffectiveBaseLevel(), baseToMaxLevels);
    }

    mViewFromBaseToMaxLevel = std::make_unique<NativeTextureWrapper>(
        nativeViewFromBaseToMaxLevelRef, mState.getEffectiveBaseLevel());

    // Recreate in bindToShader()
    mSwizzleStencilSamplingView = nullptr;
    return angle::Result::Continue;
}

angle::Result TextureMtl::onBaseMaxLevelsChanged(const gl::Context *context)
{
    if (!mNativeTextureStorage)
    {
        return angle::Result::Continue;
    }

    if (isImmutableOrPBuffer())
    {
        // For immutable texture, only recreate base-max view.
        ANGLE_TRY(createViewFromBaseToMaxLevel());
        // Invalidate base-max per level views so that they can be recreated
        // in generateMipmap()
        for (mtl::TextureRef &view : mLevelViewsWithinBaseMax)
        {
            view.reset();
        }
        return angle::Result::Continue;
    }

    if (mState.getEffectiveBaseLevel() == mNativeTextureStorage->getBaseGLLevel() &&
        mState.getMipmapMaxLevel() == mNativeTextureStorage->getMaxSupportedGLLevel())
    {
        ASSERT(mState.getBaseLevelDesc().size == mNativeTextureStorage->sizeAt0());
        // If level range remain the same, don't recreate the texture storage.
        // This might feel unnecessary at first since the front-end might prevent redundant base/max
        // level change already. However, there are cases that cause native storage to be created
        // before base/max level dirty bit is passed to Metal backend and lead to unwanted problems.
        // Example:
        // 1. texture with a non-default base/max level state is set.
        // 2. The texture is used first as a framebuffer attachment. This operation does not fully
        //    sync the texture state and therefore does not unset base/max level dirty bits.
        // 3. The same texture is then used for sampling; this operation fully syncs the texture
        //    state. Base/max level dirty bits may lead to recreating the texture storage thus
        //    invalidating native render target references created in step 2.
        // 4. If the framebuffer created in step 2 is used again, its native render target
        //    references will not be updated to point to the new storage because everything is in
        //    sync from the frontend point of view.
        // 5. Note: if the new range is different, it is expected that native render target
        //    references will be updated during draw framebuffer sync.
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);

    // We need to recreate a new native texture storage with number of levels = max level - base
    // level + 1. This can be achieved by simply deleting the old storage. The storage will be
    // lazily recreated later via ensureNativeStorageCreated().
    // Note: We release the native texture storage but keep old image definitions. So that when the
    // storage is recreated, its levels can be recreated with data from the old image definitions
    // respectively.
    deallocateNativeStorage(/*keepImages=*/true, /*keepSamplerStateAndFormat=*/true);

    // Tell context to rebind textures
    contextMtl->invalidateCurrentTextures();

    return angle::Result::Continue;
}

angle::Result TextureMtl::ensureImageCreated(const gl::Context *context,
                                             const gl::ImageIndex &index)
{
    mtl::TextureRef &image = getImage(index);
    if (!image)
    {
        // Image at this level hasn't been defined yet. We need to define it:
        const gl::ImageDesc &desc = mState.getImageDesc(index);
        ANGLE_TRY(redefineImage(context, index, mFormat, desc.size));
    }
    return angle::Result::Continue;
}

angle::Result TextureMtl::ensureLevelViewsWithinBaseMaxCreated()
{
    ASSERT(mViewFromBaseToMaxLevel);
    for (mtl::MipmapNativeLevel mip = mtl::kZeroNativeMipLevel;
         mip.get() < mViewFromBaseToMaxLevel->mipmapLevels(); ++mip)
    {
        if (mLevelViewsWithinBaseMax[mip])
        {
            continue;
        }

        GLuint mipGLLevel = mViewFromBaseToMaxLevel->getGLLevel(mip);

        if (mViewFromBaseToMaxLevel->textureType() != MTLTextureTypeCube &&
            mTexImageDefs[0][mipGLLevel].image)
        {
            // Reuse texture image view.
            mLevelViewsWithinBaseMax[mip] = mTexImageDefs[0][mipGLLevel].image;
        }
        else
        {
            mLevelViewsWithinBaseMax[mip] = mNativeTextureStorage->createMipView(mipGLLevel);
        }
    }
    return angle::Result::Continue;
}

mtl::TextureRef TextureMtl::createImageViewFromTextureStorage(GLuint cubeFaceOrZero, GLuint glLevel)
{
    mtl::TextureRef image;
    if (mNativeTextureStorage->textureType() == MTLTextureTypeCube)
    {
        // Cube texture's image is per face.
        image = mNativeTextureStorage->createSliceMipView(cubeFaceOrZero, glLevel);
    }
    else
    {
        if (mViewFromBaseToMaxLevel->isGLLevelSupported(glLevel))
        {
            mtl::MipmapNativeLevel nativeLevel = mViewFromBaseToMaxLevel->getNativeLevel(glLevel);
            if (mLevelViewsWithinBaseMax[nativeLevel])
            {
                // Reuse the native level view
                image = mLevelViewsWithinBaseMax[nativeLevel];
            }
        }

        if (!image)
        {
            image = mNativeTextureStorage->createMipView(glLevel);
        }
    }

    return image;
}

void TextureMtl::retainImageDefinitions()
{
    if (!mNativeTextureStorage)
    {
        return;
    }
    const GLuint mips = mNativeTextureStorage->mipmapLevels();

    int numCubeFaces = 1;
    switch (mState.getType())
    {
        case gl::TextureType::CubeMap:
            numCubeFaces = 6;
            break;
        default:
            break;
    }

    // Create image view per cube face, per mip level
    for (int face = 0; face < numCubeFaces; ++face)
    {
        for (mtl::MipmapNativeLevel mip = mtl::kZeroNativeMipLevel; mip.get() < mips; ++mip)
        {
            GLuint imageMipLevel         = mNativeTextureStorage->getGLLevel(mip);
            ImageDefinitionMtl &imageDef = mTexImageDefs[face][imageMipLevel];
            if (imageDef.image)
            {
                continue;
            }
            imageDef.image    = createImageViewFromTextureStorage(face, imageMipLevel);
            imageDef.formatID = mFormat.intendedFormatId;
        }
    }
}

mtl::TextureRef &TextureMtl::getImage(const gl::ImageIndex &imageIndex)
{
    return getImageDefinition(imageIndex).image;
}

ImageDefinitionMtl &TextureMtl::getImageDefinition(const gl::ImageIndex &imageIndex)
{
    GLuint cubeFaceOrZero        = GetImageCubeFaceIndexOrZeroFrom(imageIndex);
    ImageDefinitionMtl &imageDef = mTexImageDefs[cubeFaceOrZero][imageIndex.getLevelIndex()];

    if (!imageDef.image && mNativeTextureStorage)
    {
        // If native texture is already created, and the image at this index is not available,
        // then create a view of native texture at this index, so that modifications of the image
        // are reflected back to native texture's respective index.
        if (!mNativeTextureStorage->isGLLevelSupported(imageIndex.getLevelIndex()))
        {
            // Image outside native texture's mip levels is skipped.
            return imageDef;
        }

        imageDef.image =
            createImageViewFromTextureStorage(cubeFaceOrZero, imageIndex.getLevelIndex());
        imageDef.formatID = mFormat.intendedFormatId;
    }

    return imageDef;
}
angle::Result TextureMtl::getRenderTarget(ContextMtl *context,
                                          const gl::ImageIndex &imageIndex,
                                          GLsizei implicitSamples,
                                          RenderTargetMtl **renderTargetOut)
{
    ASSERT(imageIndex.getType() == gl::TextureType::_2D ||
           imageIndex.getType() == gl::TextureType::Rectangle ||
           imageIndex.getType() == gl::TextureType::_2DMultisample || imageIndex.hasLayer());

    const gl::RenderToTextureImageIndex renderToTextureIndex =
        implicitSamples <= 1
            ? gl::RenderToTextureImageIndex::Default
            : static_cast<gl::RenderToTextureImageIndex>(PackSampleCount(implicitSamples));

    GLuint layer         = GetImageLayerIndexFrom(imageIndex);
    RenderTargetMtl &rtt = mRenderTargets[imageIndex][renderToTextureIndex];
    if (!rtt.getTexture())
    {
        // Lazy initialization of render target:
        mtl::TextureRef &image = getImage(imageIndex);
        if (image)
        {
            if (imageIndex.getType() == gl::TextureType::CubeMap)
            {
                // Cube map is special, the image is already the view of its layer
                rtt.set(image, mtl::kZeroNativeMipLevel, 0, mFormat);
            }
            else
            {
                rtt.set(image, mtl::kZeroNativeMipLevel, layer, mFormat);
            }
        }
    }

    if (implicitSamples > 1 && !rtt.getImplicitMSTexture())
    {
        // This format must supports implicit resolve
        ANGLE_CHECK(context, mFormat.getCaps().resolve, gl::err::kInternalError, GL_INVALID_VALUE);
        mtl::TextureRef &msTexture = mImplicitMSTextures[imageIndex][renderToTextureIndex];
        if (!msTexture)
        {
            const gl::ImageDesc &desc = mState.getImageDesc(imageIndex);
            ANGLE_TRY(mtl::Texture::MakeMemoryLess2DMSTexture(
                context, mFormat, desc.size.width, desc.size.height, implicitSamples, &msTexture));
        }
        rtt.setImplicitMSTexture(msTexture);
    }

    *renderTargetOut = &rtt;

    return angle::Result::Continue;
}

angle::Result TextureMtl::setImage(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   GLenum internalFormat,
                                   const gl::Extents &size,
                                   GLenum format,
                                   GLenum type,
                                   const gl::PixelUnpackState &unpack,
                                   gl::Buffer *unpackBuffer,
                                   const uint8_t *pixels)
{
    const gl::InternalFormat &dstFormatInfo = gl::GetInternalFormatInfo(internalFormat, type);

    return setImageImpl(context, index, dstFormatInfo, size, format, type, unpack, unpackBuffer,
                        pixels);
}

angle::Result TextureMtl::setSubImage(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      const gl::Box &area,
                                      GLenum format,
                                      GLenum type,
                                      const gl::PixelUnpackState &unpack,
                                      gl::Buffer *unpackBuffer,
                                      const uint8_t *pixels)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, type);

    return setSubImageImpl(context, index, area, formatInfo, type, unpack, unpackBuffer, pixels);
}

angle::Result TextureMtl::setCompressedImage(const gl::Context *context,
                                             const gl::ImageIndex &index,
                                             GLenum internalFormat,
                                             const gl::Extents &size,
                                             const gl::PixelUnpackState &unpack,
                                             size_t imageSize,
                                             const uint8_t *pixels)
{
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    const gl::State &glState             = context->getState();
    gl::Buffer *unpackBuffer             = glState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    return setImageImpl(context, index, formatInfo, size, internalFormat, GL_UNSIGNED_BYTE, unpack,
                        unpackBuffer, pixels);
}

angle::Result TextureMtl::setCompressedSubImage(const gl::Context *context,
                                                const gl::ImageIndex &index,
                                                const gl::Box &area,
                                                GLenum format,
                                                const gl::PixelUnpackState &unpack,
                                                size_t imageSize,
                                                const uint8_t *pixels)
{

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, GL_UNSIGNED_BYTE);

    const gl::State &glState = context->getState();
    gl::Buffer *unpackBuffer = glState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    return setSubImageImpl(context, index, area, formatInfo, GL_UNSIGNED_BYTE, unpack, unpackBuffer,
                           pixels);
}

angle::Result TextureMtl::copyImage(const gl::Context *context,
                                    const gl::ImageIndex &index,
                                    const gl::Rectangle &sourceArea,
                                    GLenum internalFormat,
                                    gl::Framebuffer *source)
{
    gl::Extents newImageSize(sourceArea.width, sourceArea.height, 1);
    const gl::InternalFormat &internalFormatInfo =
        gl::GetInternalFormatInfo(internalFormat, GL_UNSIGNED_BYTE);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(internalFormatInfo.sizedInternalFormat);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(angleFormatId);

    FramebufferMtl *srcFramebufferMtl = mtl::GetImpl(source);
    RenderTargetMtl *srcReadRT        = srcFramebufferMtl->getColorReadRenderTarget(context);
    RenderTargetMtl colorReadRT;
    if (srcReadRT)
    {
        // Need to duplicate RenderTargetMtl since the srcReadRT would be invalidated in
        // redefineImage(). This can happen if the source and this texture are the same texture.
        // Duplication ensures the copyImage() will be able to proceed even if the source texture
        // will be redefined.
        colorReadRT.duplicateFrom(*srcReadRT);
    }

    ANGLE_TRY(redefineImage(context, index, mtlFormat, newImageSize));

    gl::Extents fbSize = source->getReadColorAttachment()->getSize();
    gl::Rectangle fbRect(0, 0, fbSize.width, fbSize.height);
    if (context->isWebGL() && !fbRect.encloses(sourceArea))
    {
        ANGLE_TRY(initializeContents(context, GL_NONE, index));
    }

    return copySubImageImpl(context, index, gl::Offset(0, 0, 0), sourceArea, internalFormatInfo,
                            srcFramebufferMtl, &colorReadRT);
}

angle::Result TextureMtl::copySubImage(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::Offset &destOffset,
                                       const gl::Rectangle &sourceArea,
                                       gl::Framebuffer *source)
{
    const gl::InternalFormat &currentFormat = *mState.getImageDesc(index).format.info;
    FramebufferMtl *srcFramebufferMtl       = mtl::GetImpl(source);
    RenderTargetMtl *colorReadRT            = srcFramebufferMtl->getColorReadRenderTarget(context);
    return copySubImageImpl(context, index, destOffset, sourceArea, currentFormat,
                            srcFramebufferMtl, colorReadRT);
}

angle::Result TextureMtl::copyTexture(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      GLenum internalFormat,
                                      GLenum type,
                                      GLint sourceLevel,
                                      bool unpackFlipY,
                                      bool unpackPremultiplyAlpha,
                                      bool unpackUnmultiplyAlpha,
                                      const gl::Texture *source)
{
    const gl::ImageDesc &sourceImageDesc = source->getTextureState().getImageDesc(
        NonCubeTextureTypeToTarget(source->getType()), sourceLevel);
    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat, type);

    // Only 2D textures are supported.
    ASSERT(sourceImageDesc.size.depth == 1);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(internalFormatInfo.sizedInternalFormat);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(angleFormatId);

    ANGLE_TRY(redefineImage(context, index, mtlFormat, sourceImageDesc.size));

    return copySubTextureImpl(
        context, index, gl::Offset(0, 0, 0), internalFormatInfo, sourceLevel,
        gl::Box(0, 0, 0, sourceImageDesc.size.width, sourceImageDesc.size.height, 1), unpackFlipY,
        unpackPremultiplyAlpha, unpackUnmultiplyAlpha, source);
}

angle::Result TextureMtl::copySubTexture(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         const gl::Offset &destOffset,
                                         GLint sourceLevel,
                                         const gl::Box &sourceBox,
                                         bool unpackFlipY,
                                         bool unpackPremultiplyAlpha,
                                         bool unpackUnmultiplyAlpha,
                                         const gl::Texture *source)
{
    const gl::InternalFormat &currentFormat = *mState.getImageDesc(index).format.info;

    return copySubTextureImpl(context, index, destOffset, currentFormat, sourceLevel, sourceBox,
                              unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha, source);
}

angle::Result TextureMtl::copyCompressedTexture(const gl::Context *context,
                                                const gl::Texture *source)
{
    UNIMPLEMENTED();

    return angle::Result::Stop;
}

angle::Result TextureMtl::setStorage(const gl::Context *context,
                                     gl::TextureType type,
                                     size_t mipmaps,
                                     GLenum internalFormat,
                                     const gl::Extents &size)
{
    ContextMtl *contextMtl               = mtl::GetImpl(context);
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(formatInfo.sizedInternalFormat);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(angleFormatId);

    return setStorageImpl(context, type, mState.getImmutableLevels(), 0, mtlFormat, size);
}

angle::Result TextureMtl::setStorageExternalMemory(const gl::Context *context,
                                                   gl::TextureType type,
                                                   size_t levels,
                                                   GLenum internalFormat,
                                                   const gl::Extents &size,
                                                   gl::MemoryObject *memoryObject,
                                                   GLuint64 offset,
                                                   GLbitfield createFlags,
                                                   GLbitfield usageFlags,
                                                   const void *imageCreateInfoPNext)
{
    UNIMPLEMENTED();

    return angle::Result::Stop;
}

angle::Result TextureMtl::setStorageMultisample(const gl::Context *context,
                                                gl::TextureType type,
                                                GLsizei samples,
                                                GLint internalFormat,
                                                const gl::Extents &size,
                                                bool fixedSampleLocations)
{
    ContextMtl *contextMtl               = mtl::GetImpl(context);
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(formatInfo.sizedInternalFormat);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(angleFormatId);

    return setStorageImpl(context, type, 0, mState.getLevelZeroDesc().samples, mtlFormat, size);
}

angle::Result TextureMtl::setEGLImageTarget(const gl::Context *context,
                                            gl::TextureType type,
                                            egl::Image *image)
{
    deallocateNativeStorage(/*keepImages=*/false);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    ImageMtl *imageMtl = mtl::GetImpl(image);
    if (type != imageMtl->getImageTextureType())
    {
        return angle::Result::Stop;
    }

    mNativeTextureStorage = std::make_unique<NativeTextureWrapperWithViewSupport>(
        imageMtl->getTexture(), /*baseGLLevel=*/0);

    const angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(image->getFormat().info->sizedInternalFormat);
    mFormat = contextMtl->getPixelFormat(angleFormatId);

    mSlices = mNativeTextureStorage->cubeFacesOrArrayLength();

    ANGLE_TRY(ensureSamplerStateCreated(context));
    ANGLE_TRY(createViewFromBaseToMaxLevel());

    // Tell context to rebind textures
    contextMtl->invalidateCurrentTextures();

    return angle::Result::Continue;
}

angle::Result TextureMtl::setImageExternal(const gl::Context *context,
                                           gl::TextureType type,
                                           egl::Stream *stream,
                                           const egl::Stream::GLTextureDescription &desc)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

angle::Result TextureMtl::generateMipmap(const gl::Context *context)
{
    ANGLE_TRY(ensureNativeStorageCreated(context));

    ContextMtl *contextMtl = mtl::GetImpl(context);
    if (!mViewFromBaseToMaxLevel)
    {
        return angle::Result::Continue;
    }

    const mtl::FormatCaps &caps = mFormat.getCaps();
    //
    bool sRGB = mFormat.actualInternalFormat().colorEncoding == GL_SRGB;

    bool avoidGPUPath =
        contextMtl->getDisplay()->getFeatures().forceNonCSBaseMipmapGeneration.enabled &&
        mViewFromBaseToMaxLevel->widthAt0() < 5;

    if (!avoidGPUPath && caps.writable && mState.getType() == gl::TextureType::_3D)
    {
        // http://anglebug.com/42263496.
        // Use compute for 3D mipmap generation.
        ANGLE_TRY(ensureLevelViewsWithinBaseMaxCreated());
        ANGLE_TRY(contextMtl->getDisplay()->getUtils().generateMipmapCS(
            contextMtl, *mViewFromBaseToMaxLevel, sRGB, &mLevelViewsWithinBaseMax));
    }
    else if (!avoidGPUPath && caps.filterable && caps.colorRenderable)
    {
        mtl::BlitCommandEncoder *blitEncoder = GetBlitCommandEncoderForResources(
            contextMtl, {mViewFromBaseToMaxLevel->getNativeTexture().get()});
        blitEncoder->generateMipmapsForTexture(*mViewFromBaseToMaxLevel);
    }
    else
    {
        ANGLE_TRY(generateMipmapCPU(context));
    }

    return angle::Result::Continue;
}

angle::Result TextureMtl::generateMipmapCPU(const gl::Context *context)
{
    ASSERT(mViewFromBaseToMaxLevel);

    ContextMtl *contextMtl           = mtl::GetImpl(context);
    const angle::Format &angleFormat = mFormat.actualAngleFormat();
    // This format must have mip generation function.
    ANGLE_CHECK(contextMtl, angleFormat.mipGenerationFunction, gl::err::kInternalError,
                GL_INVALID_OPERATION);

    for (uint32_t slice = 0; slice < mSlices; ++slice)
    {
        GLuint baseGLLevel = mViewFromBaseToMaxLevel->getBaseGLLevel();

        uint32_t prevLevelWidth    = mViewFromBaseToMaxLevel->widthAt0();
        uint32_t prevLevelHeight   = mViewFromBaseToMaxLevel->heightAt0();
        uint32_t prevLevelDepth    = mViewFromBaseToMaxLevel->depthAt0();
        size_t prevLevelRowPitch   = angleFormat.pixelBytes * prevLevelWidth;
        size_t prevLevelDepthPitch = prevLevelRowPitch * prevLevelHeight;
        std::unique_ptr<uint8_t[]> prevLevelData(new (std::nothrow)
                                                     uint8_t[prevLevelDepthPitch * prevLevelDepth]);
        ANGLE_CHECK_GL_ALLOC(contextMtl, prevLevelData);
        std::unique_ptr<uint8_t[]> dstLevelData;

        // Download base level data
        mViewFromBaseToMaxLevel->getBytes(
            contextMtl, prevLevelRowPitch, prevLevelDepthPitch,
            MTLRegionMake3D(0, 0, 0, prevLevelWidth, prevLevelHeight, prevLevelDepth), baseGLLevel,
            slice, prevLevelData.get());

        for (GLuint mip = 1; mip < mViewFromBaseToMaxLevel->mipmapLevels(); ++mip)
        {
            GLuint glLevel     = baseGLLevel + mip;
            uint32_t dstWidth  = mViewFromBaseToMaxLevel->width(glLevel);
            uint32_t dstHeight = mViewFromBaseToMaxLevel->height(glLevel);
            uint32_t dstDepth  = mViewFromBaseToMaxLevel->depth(glLevel);

            size_t dstRowPitch   = angleFormat.pixelBytes * dstWidth;
            size_t dstDepthPitch = dstRowPitch * dstHeight;
            size_t dstDataSize   = dstDepthPitch * dstDepth;
            if (!dstLevelData)
            {
                // Allocate once and reuse the buffer
                dstLevelData.reset(new (std::nothrow) uint8_t[dstDataSize]);
                ANGLE_CHECK_GL_ALLOC(contextMtl, dstLevelData);
            }

            // Generate mip level
            angleFormat.mipGenerationFunction(
                prevLevelWidth, prevLevelHeight, 1, prevLevelData.get(), prevLevelRowPitch,
                prevLevelDepthPitch, dstLevelData.get(), dstRowPitch, dstDepthPitch);

            mtl::MipmapNativeLevel nativeLevel = mViewFromBaseToMaxLevel->getNativeLevel(glLevel);

            // Upload to texture
            ANGLE_TRY(UploadTextureContents(context, angleFormat,
                                            MTLRegionMake3D(0, 0, 0, dstWidth, dstHeight, dstDepth),
                                            nativeLevel, slice, dstLevelData.get(), dstRowPitch,
                                            dstDepthPitch, false, *mViewFromBaseToMaxLevel));

            prevLevelWidth      = dstWidth;
            prevLevelHeight     = dstHeight;
            prevLevelDepth      = dstDepth;
            prevLevelRowPitch   = dstRowPitch;
            prevLevelDepthPitch = dstDepthPitch;
            std::swap(prevLevelData, dstLevelData);
        }  // for mip level

    }  // For layers

    return angle::Result::Continue;
}

bool TextureMtl::needsFormatViewForPixelLocalStorage(
    const ShPixelLocalStorageOptions &plsOptions) const
{
    // On iOS devices with GPU family 5 and later, Metal doesn't apply lossless compression to
    // the texture if we set MTLTextureUsagePixelFormatView. This shouldn't be a problem though
    // because iOS devices implement pixel local storage with framebuffer fetch instead of shader
    // images.
    if (plsOptions.type == ShPixelLocalStorageType::ImageLoadStore)
    {
        switch (mFormat.metalFormat)
        {
            case MTLPixelFormatRGBA8Unorm:
            case MTLPixelFormatRGBA8Uint:
            case MTLPixelFormatRGBA8Sint:
                return !plsOptions.supportsNativeRGBA8ImageFormats;
            default:
                break;
        }
    }
    return false;
}

bool TextureMtl::isImmutableOrPBuffer() const
{
    return mState.getImmutableFormat() || mBoundSurface;
}

angle::Result TextureMtl::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    return onBaseMaxLevelsChanged(context);
}

angle::Result TextureMtl::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    deallocateNativeStorage(/*keepImages=*/false);

    mBoundSurface         = surface;
    auto pBuffer          = GetImplAs<OffscreenSurfaceMtl>(surface);
    mNativeTextureStorage = std::make_unique<NativeTextureWrapperWithViewSupport>(
        pBuffer->getColorTexture(), /*baseGLLevel=*/0);
    mFormat = pBuffer->getColorFormat();
    mSlices = mNativeTextureStorage->cubeFacesOrArrayLength();

    ANGLE_TRY(ensureSamplerStateCreated(context));
    ANGLE_TRY(createViewFromBaseToMaxLevel());

    // Tell context to rebind textures
    ContextMtl *contextMtl = mtl::GetImpl(context);
    contextMtl->invalidateCurrentTextures();

    return angle::Result::Continue;
}

angle::Result TextureMtl::releaseTexImage(const gl::Context *context)
{
    deallocateNativeStorage(/*keepImages=*/false);
    mBoundSurface = nullptr;
    return angle::Result::Continue;
}

angle::Result TextureMtl::getAttachmentRenderTarget(const gl::Context *context,
                                                    GLenum binding,
                                                    const gl::ImageIndex &imageIndex,
                                                    GLsizei samples,
                                                    FramebufferAttachmentRenderTarget **rtOut)
{
    ANGLE_TRY(ensureNativeStorageCreated(context));

    ContextMtl *contextMtl = mtl::GetImpl(context);
    ANGLE_CHECK(contextMtl, mNativeTextureStorage, gl::err::kInternalError, GL_INVALID_OPERATION);

    RenderTargetMtl *rtt;
    ANGLE_TRY(getRenderTarget(contextMtl, imageIndex, samples, &rtt));

    *rtOut = rtt;

    return angle::Result::Continue;
}

angle::Result TextureMtl::syncState(const gl::Context *context,
                                    const gl::Texture::DirtyBits &dirtyBits,
                                    gl::Command source)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    for (size_t dirtyBit : dirtyBits)
    {
        switch (dirtyBit)
        {
            case gl::Texture::DIRTY_BIT_COMPARE_MODE:
            case gl::Texture::DIRTY_BIT_COMPARE_FUNC:
                // Tell context to rebind textures so that ProgramMtl has a chance to verify
                // depth texture compare mode.
                contextMtl->invalidateCurrentTextures();
                // fall through
                OS_FALLTHROUGH;
            case gl::Texture::DIRTY_BIT_MIN_FILTER:
            case gl::Texture::DIRTY_BIT_MAG_FILTER:
            case gl::Texture::DIRTY_BIT_WRAP_S:
            case gl::Texture::DIRTY_BIT_WRAP_T:
            case gl::Texture::DIRTY_BIT_WRAP_R:
            case gl::Texture::DIRTY_BIT_MAX_ANISOTROPY:
            case gl::Texture::DIRTY_BIT_MIN_LOD:
            case gl::Texture::DIRTY_BIT_MAX_LOD:
            case gl::Texture::DIRTY_BIT_SRGB_DECODE:
            case gl::Texture::DIRTY_BIT_BORDER_COLOR:
                // Recreate sampler state
                mMetalSamplerState = nil;
                break;
            case gl::Texture::DIRTY_BIT_MAX_LEVEL:
            case gl::Texture::DIRTY_BIT_BASE_LEVEL:
                ANGLE_TRY(onBaseMaxLevelsChanged(context));
                break;
            case gl::Texture::DIRTY_BIT_SWIZZLE_RED:
            case gl::Texture::DIRTY_BIT_SWIZZLE_GREEN:
            case gl::Texture::DIRTY_BIT_SWIZZLE_BLUE:
            case gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA:
            case gl::Texture::DIRTY_BIT_DEPTH_STENCIL_TEXTURE_MODE:
            {
                // Recreate swizzle/stencil view.
                mSwizzleStencilSamplingView = nullptr;
            }
            break;
            default:
                break;
        }
    }

    ANGLE_TRY(ensureNativeStorageCreated(context));
    ANGLE_TRY(ensureSamplerStateCreated(context));

    return angle::Result::Continue;
}

angle::Result TextureMtl::bindToShader(const gl::Context *context,
                                       mtl::RenderCommandEncoder *cmdEncoder,
                                       gl::ShaderType shaderType,
                                       gl::Sampler *sampler,
                                       int textureSlotIndex,
                                       int samplerSlotIndex)
{
    ASSERT(mNativeTextureStorage);
    ASSERT(mViewFromBaseToMaxLevel);

    float minLodClamp;
    float maxLodClamp;
    id<MTLSamplerState> samplerState;

    if (!mSwizzleStencilSamplingView)
    {
        ContextMtl *contextMtl             = mtl::GetImpl(context);
        const angle::FeaturesMtl &features = contextMtl->getDisplay()->getFeatures();

        // Sampling from unused channels of depth and stencil textures is undefined in Metal.
        // ANGLE relies on swizzled views to enforce values required by OpenGL ES specs. Some
        // drivers fail to sample from a swizzled view of a stencil texture so skip this step.
        const bool skipStencilSwizzle = mFormat.actualFormatId == angle::FormatID::S8_UINT &&
                                        features.avoidStencilTextureSwizzle.enabled;
        if (!skipStencilSwizzle &&
            (mState.getSwizzleState().swizzleRequired() ||
             mFormat.actualAngleFormat().hasDepthOrStencilBits() || mFormat.swizzled) &&
            features.hasTextureSwizzle.enabled)
        {
            const gl::InternalFormat &glInternalFormat = *mState.getBaseLevelDesc().format.info;

            MTLTextureSwizzleChannels swizzle = MTLTextureSwizzleChannelsMake(
                mtl::GetTextureSwizzle(OverrideSwizzleValue(
                    context, mState.getSwizzleState().swizzleRed, mFormat, glInternalFormat)),
                mtl::GetTextureSwizzle(OverrideSwizzleValue(
                    context, mState.getSwizzleState().swizzleGreen, mFormat, glInternalFormat)),
                mtl::GetTextureSwizzle(OverrideSwizzleValue(
                    context, mState.getSwizzleState().swizzleBlue, mFormat, glInternalFormat)),
                mtl::GetTextureSwizzle(OverrideSwizzleValue(
                    context, mState.getSwizzleState().swizzleAlpha, mFormat, glInternalFormat)));

            MTLPixelFormat format = mViewFromBaseToMaxLevel->pixelFormat();
            if (mState.isStencilMode())
            {
                if (format == MTLPixelFormatDepth32Float_Stencil8)
                {
                    format = MTLPixelFormatX32_Stencil8;
                }
#    if TARGET_OS_OSX || TARGET_OS_MACCATALYST
                else if (format == MTLPixelFormatDepth24Unorm_Stencil8)
                {
                    format = MTLPixelFormatX24_Stencil8;
                }
#    endif
            }

            mSwizzleStencilSamplingView = mNativeTextureStorage->createMipsSwizzleView(
                mViewFromBaseToMaxLevel->getBaseGLLevel(), mViewFromBaseToMaxLevel->mipmapLevels(),
                format, swizzle);
        }
        else
        {
            mSwizzleStencilSamplingView = mState.isStencilMode()
                                              ? mViewFromBaseToMaxLevel->getStencilView()
                                              : mViewFromBaseToMaxLevel->getNativeTexture();
        }
    }

    if (!sampler)
    {
        samplerState = mMetalSamplerState;
        minLodClamp  = mState.getSamplerState().getMinLod();
        maxLodClamp  = mState.getSamplerState().getMaxLod();
    }
    else
    {
        SamplerMtl *samplerMtl = mtl::GetImpl(sampler);
        samplerState           = samplerMtl->getSampler(mtl::GetImpl(context));
        minLodClamp            = sampler->getSamplerState().getMinLod();
        maxLodClamp            = sampler->getSamplerState().getMaxLod();
    }

    minLodClamp = std::max(minLodClamp, 0.f);

    cmdEncoder->setTexture(shaderType, mSwizzleStencilSamplingView, textureSlotIndex);
    cmdEncoder->setSamplerState(shaderType, samplerState, minLodClamp, maxLodClamp,
                                samplerSlotIndex);

    return angle::Result::Continue;
}

angle::Result TextureMtl::bindToShaderImage(const gl::Context *context,
                                            mtl::RenderCommandEncoder *cmdEncoder,
                                            gl::ShaderType shaderType,
                                            int textureSlotIndex,
                                            int level,
                                            int layer,
                                            GLenum format)
{
    ASSERT(mNativeTextureStorage);
    ASSERT(mState.getImmutableFormat());
    ASSERT(0 <= level && static_cast<uint32_t>(level) < mState.getImmutableLevels());
    ASSERT(0 <= layer && static_cast<uint32_t>(layer) < mSlices);

    ContextMtl *contextMtl        = mtl::GetImpl(context);
    angle::FormatID angleFormatId = angle::Format::InternalFormatToID(format);
    mtl::Format imageAccessFormat = contextMtl->getPixelFormat(angleFormatId);
    LayerLevelTextureViewVector &textureViewVector =
        mShaderImageViews[imageAccessFormat.metalFormat];
    mtl::TextureRef &textureRef = GetLayerLevelTextureView(&textureViewVector, layer, level,
                                                           mSlices, mState.getImmutableLevels());

    if (textureRef == nullptr)
    {
        textureRef = mNativeTextureStorage->createShaderImageView2D(level, layer,
                                                                    imageAccessFormat.metalFormat);
    }

    cmdEncoder->setRWTexture(shaderType, textureRef, textureSlotIndex);
    return angle::Result::Continue;
}

angle::Result TextureMtl::redefineImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const mtl::Format &mtlFormat,
                                        const gl::Extents &size)
{
    bool imageWithinNativeStorageLevels = false;
    if (mNativeTextureStorage && mNativeTextureStorage->isGLLevelSupported(index.getLevelIndex()))
    {
        imageWithinNativeStorageLevels = true;
        GLuint glLevel                 = index.getLevelIndex();
        // Calculate the expected size for the index we are defining. If the size is different
        // from the given size, or the format is different, we are redefining the image so we
        // must release it.
        ASSERT(mNativeTextureStorage->textureType() == mtl::GetTextureType(index.getType()));
        if (mFormat != mtlFormat || size != mNativeTextureStorage->size(glLevel))
        {
            // Keep other images
            deallocateNativeStorage(/*keepImages=*/true);
        }
    }

    // Early-out on empty textures, don't create a zero-sized storage.
    if (size.empty())
    {
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);
    // Cache last defined image format:
    mFormat                      = mtlFormat;
    ImageDefinitionMtl &imageDef = getImageDefinition(index);

    // If native texture still exists, it means the size hasn't been changed, no need to create new
    // image
    if (mNativeTextureStorage && imageDef.image && imageWithinNativeStorageLevels)
    {
        ASSERT(imageDef.image->textureType() ==
                   mtl::GetTextureType(GetTextureImageType(index.getType())) &&
               imageDef.formatID == mFormat.intendedFormatId && imageDef.image->sizeAt0() == size);
    }
    else
    {
        imageDef.formatID    = mtlFormat.intendedFormatId;
        bool allowFormatView = mFormat.hasDepthAndStencilBits() ||
                               needsFormatViewForPixelLocalStorage(
                                   contextMtl->getDisplay()->getNativePixelLocalStorageOptions());
        // Create image to hold texture's data at this level & slice:
        switch (index.getType())
        {
            case gl::TextureType::_2D:
            case gl::TextureType::CubeMap:
                ANGLE_TRY(mtl::Texture::Make2DTexture(
                    contextMtl, mtlFormat, size.width, size.height, 1,
                    /** renderTargetOnly */ false, allowFormatView, &imageDef.image));
                break;
            case gl::TextureType::_3D:
                ANGLE_TRY(mtl::Texture::Make3DTexture(
                    contextMtl, mtlFormat, size.width, size.height, size.depth, 1,
                    /** renderTargetOnly */ false, allowFormatView, &imageDef.image));
                break;
            case gl::TextureType::_2DArray:
                ANGLE_TRY(mtl::Texture::Make2DArrayTexture(
                    contextMtl, mtlFormat, size.width, size.height, 1, size.depth,
                    /** renderTargetOnly */ false, allowFormatView, &imageDef.image));
                break;
            default:
                UNREACHABLE();
        }

        // Make sure emulated channels are properly initialized in this newly allocated texture.
        ANGLE_TRY(checkForEmulatedChannels(context, mtlFormat, imageDef.image));
    }

    // Tell context to rebind textures
    contextMtl->invalidateCurrentTextures();

    return angle::Result::Continue;
}

angle::Result TextureMtl::setStorageImpl(const gl::Context *context,
                                         gl::TextureType type,
                                         GLuint mips,
                                         GLuint samples,
                                         const mtl::Format &mtlFormat,
                                         const gl::Extents &size)
{
    // Don't need to hold old images data.
    deallocateNativeStorage(/*keepImages=*/false);

    ContextMtl *contextMtl = mtl::GetImpl(context);

    // Tell context to rebind textures
    contextMtl->invalidateCurrentTextures();

    mFormat = mtlFormat;

    ANGLE_TRY(createNativeStorage(context, type, mips, samples, size));
    ANGLE_TRY(createViewFromBaseToMaxLevel());

    return angle::Result::Continue;
}

angle::Result TextureMtl::setImageImpl(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::InternalFormat &dstFormatInfo,
                                       const gl::Extents &size,
                                       GLenum srcFormat,
                                       GLenum srcType,
                                       const gl::PixelUnpackState &unpack,
                                       gl::Buffer *unpackBuffer,
                                       const uint8_t *pixels)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    angle::FormatID angleFormatId =
        angle::Format::InternalFormatToID(dstFormatInfo.sizedInternalFormat);
    const mtl::Format &mtlFormat = contextMtl->getPixelFormat(angleFormatId);

    ANGLE_TRY(redefineImage(context, index, mtlFormat, size));

    // Early-out on empty textures, don't create a zero-sized storage.
    if (size.empty())
    {
        return angle::Result::Continue;
    }

    // Format of the supplied pixels.
    const gl::InternalFormat *srcFormatInfo;
    if (srcFormat != dstFormatInfo.format || srcType != dstFormatInfo.type)
    {
        srcFormatInfo = &gl::GetInternalFormatInfo(srcFormat, srcType);
    }
    else
    {
        srcFormatInfo = &dstFormatInfo;
    }
    return setSubImageImpl(context, index, gl::Box(0, 0, 0, size.width, size.height, size.depth),
                           *srcFormatInfo, srcType, unpack, unpackBuffer, pixels);
}

angle::Result TextureMtl::setSubImageImpl(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Box &area,
                                          const gl::InternalFormat &formatInfo,
                                          GLenum type,
                                          const gl::PixelUnpackState &unpack,
                                          gl::Buffer *unpackBuffer,
                                          const uint8_t *oriPixels)
{
    if (!oriPixels && !unpackBuffer)
    {
        return angle::Result::Continue;
    }

    ContextMtl *contextMtl = mtl::GetImpl(context);

    ANGLE_TRY(ensureImageCreated(context, index));
    mtl::TextureRef &image = getImage(index);

    GLuint sourceRowPitch   = 0;
    GLuint sourceDepthPitch = 0;
    GLuint sourceSkipBytes  = 0;
    ANGLE_CHECK_GL_MATH(contextMtl, formatInfo.computeRowPitch(type, area.width, unpack.alignment,
                                                               unpack.rowLength, &sourceRowPitch));
    ANGLE_CHECK_GL_MATH(
        contextMtl, formatInfo.computeDepthPitch(area.height, unpack.imageHeight, sourceRowPitch,
                                                 &sourceDepthPitch));
    ANGLE_CHECK_GL_MATH(contextMtl,
                        formatInfo.computeSkipBytes(type, sourceRowPitch, sourceDepthPitch, unpack,
                                                    index.usesTex3D(), &sourceSkipBytes));

    // Get corresponding source data's ANGLE format
    angle::FormatID srcAngleFormatId;
    if (formatInfo.sizedInternalFormat == GL_DEPTH_COMPONENT24)
    {
        // GL_DEPTH_COMPONENT24 is special case, its supplied data is 32 bit depth.
        srcAngleFormatId = angle::FormatID::D32_UNORM;
    }
    else
    {
        srcAngleFormatId = angle::Format::InternalFormatToID(formatInfo.sizedInternalFormat);
    }
    const angle::Format &srcAngleFormat = angle::Format::Get(srcAngleFormatId);

    const uint8_t *usablePixels = oriPixels + sourceSkipBytes;

    // Upload to texture
    if (index.getType() == gl::TextureType::_2DArray)
    {
        // OpenGL unifies texture array and texture 3d's box area by using z & depth as array start
        // index & length for texture array. However, Metal treats them differently. We need to
        // handle them in separate code.
        MTLRegion mtlRegion = MTLRegionMake3D(area.x, area.y, 0, area.width, area.height, 1);

        for (int slice = 0; slice < area.depth; ++slice)
        {
            int sliceIndex           = slice + area.z;
            const uint8_t *srcPixels = usablePixels + slice * sourceDepthPitch;
            ANGLE_TRY(setPerSliceSubImage(context, sliceIndex, mtlRegion, formatInfo, type,
                                          srcAngleFormat, sourceRowPitch, sourceDepthPitch,
                                          unpackBuffer, srcPixels, image));
        }
    }
    else
    {
        MTLRegion mtlRegion =
            MTLRegionMake3D(area.x, area.y, area.z, area.width, area.height, area.depth);

        ANGLE_TRY(setPerSliceSubImage(context, 0, mtlRegion, formatInfo, type, srcAngleFormat,
                                      sourceRowPitch, sourceDepthPitch, unpackBuffer, usablePixels,
                                      image));
    }

    return angle::Result::Continue;
}

angle::Result TextureMtl::setPerSliceSubImage(const gl::Context *context,
                                              int slice,
                                              const MTLRegion &mtlArea,
                                              const gl::InternalFormat &internalFormat,
                                              GLenum type,
                                              const angle::Format &pixelsAngleFormat,
                                              size_t pixelsRowPitch,
                                              size_t pixelsDepthPitch,
                                              gl::Buffer *unpackBuffer,
                                              const uint8_t *pixels,
                                              const mtl::TextureRef &image)
{
    // If source pixels are luminance or RGB8, we need to convert them to RGBA

    if (mFormat.needConversion(pixelsAngleFormat.id))
    {
        return convertAndSetPerSliceSubImage(context, slice, mtlArea, internalFormat, type,
                                             pixelsAngleFormat, pixelsRowPitch, pixelsDepthPitch,
                                             unpackBuffer, pixels, image);
    }

    // No conversion needed.
    ContextMtl *contextMtl = mtl::GetImpl(context);

    if (unpackBuffer)
    {
        uintptr_t offset = reinterpret_cast<uintptr_t>(pixels);
        GLuint minRowPitch;
        ANGLE_CHECK_GL_MATH(contextMtl, internalFormat.computeRowPitch(
                                            type, static_cast<GLint>(mtlArea.size.width),
                                            /** aligment */ 1, /** rowLength */ 0, &minRowPitch));
        if (offset % mFormat.actualAngleFormat().pixelBytes || pixelsRowPitch < minRowPitch)
        {
            // offset is not divisible by pixelByte or the source row pitch is smaller than minimum
            // row pitch, use convertAndSetPerSliceSubImage() function.
            return convertAndSetPerSliceSubImage(context, slice, mtlArea, internalFormat, type,
                                                 pixelsAngleFormat, pixelsRowPitch,
                                                 pixelsDepthPitch, unpackBuffer, pixels, image);
        }

        BufferMtl *unpackBufferMtl = mtl::GetImpl(unpackBuffer);

        if (mFormat.hasDepthAndStencilBits())
        {
            // NOTE(hqle): packed depth & stencil texture cannot copy from buffer directly, needs
            // to split its depth & stencil data and copy separately.
            const uint8_t *clientData = unpackBufferMtl->getBufferDataReadOnly(contextMtl);
            clientData += offset;
            ANGLE_TRY(UploadTextureContents(context, mFormat.actualAngleFormat(), mtlArea,
                                            mtl::kZeroNativeMipLevel, slice, clientData,
                                            pixelsRowPitch, pixelsDepthPitch, false, image));
        }
        else
        {
            mtl::BufferRef sourceBuffer = unpackBufferMtl->getCurrentBuffer();
            // PVRTC1 blocks are stored in a reflected Morton order
            // and need to be linearized for buffer uploads in Metal.
            // This step is skipped for textures that have only one block.
            if (mFormat.isPVRTC() && mtlArea.size.height > 4)
            {
                // PVRTC1 inherent requirement.
                ASSERT(gl::isPow2(mtlArea.size.width) && gl::isPow2(mtlArea.size.height));
                // Metal-specific limitation enforced by ANGLE validation.
                ASSERT(mtlArea.size.width == mtlArea.size.height);
                static_assert(gl::IMPLEMENTATION_MAX_2D_TEXTURE_SIZE <= 262144,
                              "The current kernel can handle up to 65536 blocks per dimension.");

                // Current command buffer implementation does not support 64-bit offsets.
                ANGLE_CHECK_GL_MATH(contextMtl, offset <= std::numeric_limits<uint32_t>::max());

                mtl::BufferRef stagingBuffer;
                ANGLE_TRY(
                    mtl::Buffer::MakeBuffer(contextMtl, pixelsDepthPitch, nullptr, &stagingBuffer));

                mtl::BlockLinearizationParams params;
                params.srcBuffer       = sourceBuffer;
                params.dstBuffer       = stagingBuffer;
                params.srcBufferOffset = static_cast<uint32_t>(offset);
                params.blocksWide =
                    static_cast<GLuint>(mtlArea.size.width) / internalFormat.compressedBlockWidth;
                params.blocksHigh =
                    static_cast<GLuint>(mtlArea.size.height) / internalFormat.compressedBlockHeight;

                // PVRTC1 textures always have at least 2 blocks in each dimension.
                // Enforce correct block layout for 8x8 textures that use 8x4 blocks.
                params.blocksWide = std::max(params.blocksWide, 2u);

                ANGLE_TRY(contextMtl->getDisplay()->getUtils().linearizeBlocks(contextMtl, params));

                sourceBuffer = stagingBuffer;
                offset       = 0;
            }
            else if (pixelsAngleFormat.id == angle::FormatID::D32_FLOAT)
            {
                // Current command buffer implementation does not support 64-bit offsets.
                ANGLE_CHECK_GL_MATH(contextMtl, offset <= std::numeric_limits<uint32_t>::max());
                mtl::BufferRef stagingBuffer;
                ANGLE_TRY(
                    mtl::Buffer::MakeBuffer(contextMtl, pixelsDepthPitch, nullptr, &stagingBuffer));

                ASSERT(pixelsAngleFormat.pixelBytes == 4 && offset % 4 == 0);
                ANGLE_TRY(SaturateDepth(contextMtl, sourceBuffer, stagingBuffer,
                                        static_cast<uint32_t>(offset),
                                        static_cast<uint32_t>(pixelsRowPitch) / 4, mtlArea.size));

                sourceBuffer = stagingBuffer;
                offset       = 0;
            }

            // Use blit encoder to copy
            mtl::BlitCommandEncoder *blitEncoder =
                GetBlitCommandEncoderForResources(contextMtl, {sourceBuffer.get(), image.get()});
            CopyBufferToOriginalTextureIfDstIsAView(
                contextMtl, blitEncoder, sourceBuffer, offset, pixelsRowPitch, pixelsDepthPitch,
                mtlArea.size, image, slice, mtl::kZeroNativeMipLevel, mtlArea.origin,
                mFormat.isPVRTC() ? MTLBlitOptionRowLinearPVRTC : MTLBlitOptionNone);
        }
    }
    else
    {
        ANGLE_TRY(UploadTextureContents(context, mFormat.actualAngleFormat(), mtlArea,
                                        mtl::kZeroNativeMipLevel, slice, pixels, pixelsRowPitch,
                                        pixelsDepthPitch, false, image));
    }
    return angle::Result::Continue;
}

angle::Result TextureMtl::convertAndSetPerSliceSubImage(const gl::Context *context,
                                                        int slice,
                                                        const MTLRegion &mtlArea,
                                                        const gl::InternalFormat &internalFormat,
                                                        GLenum type,
                                                        const angle::Format &pixelsAngleFormat,
                                                        size_t pixelsRowPitch,
                                                        size_t pixelsDepthPitch,
                                                        gl::Buffer *unpackBuffer,
                                                        const uint8_t *pixels,
                                                        const mtl::TextureRef &image)
{
    ASSERT(image && image->valid());

    ContextMtl *contextMtl = mtl::GetImpl(context);

    if (unpackBuffer)
    {
        ANGLE_CHECK_GL_MATH(contextMtl, reinterpret_cast<uintptr_t>(pixels) <=
                                            std::numeric_limits<uint32_t>::max());

        uint32_t offset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pixels));

        BufferMtl *unpackBufferMtl = mtl::GetImpl(unpackBuffer);
        if (!mFormat.getCaps().writable || mFormat.hasDepthOrStencilBits() ||
            mFormat.intendedAngleFormat().isBlock)
        {
            // Unsupported format, use CPU path.
            const uint8_t *clientData = unpackBufferMtl->getBufferDataReadOnly(contextMtl);
            clientData += offset;
            ANGLE_TRY(convertAndSetPerSliceSubImage(context, slice, mtlArea, internalFormat, type,
                                                    pixelsAngleFormat, pixelsRowPitch,
                                                    pixelsDepthPitch, nullptr, clientData, image));
        }
        else
        {
            // Use compute shader
            mtl::CopyPixelsFromBufferParams params;
            params.buffer            = unpackBufferMtl->getCurrentBuffer();
            params.bufferStartOffset = offset;
            params.bufferRowPitch    = static_cast<uint32_t>(pixelsRowPitch);
            params.bufferDepthPitch  = static_cast<uint32_t>(pixelsDepthPitch);
            params.texture           = image;
            params.textureArea       = mtl::MTLRegionToGLBox(mtlArea);

            // If texture is not array, slice must be zero, if texture is array, mtlArea.origin.z
            // must be zero.
            // This is because this function uses Metal convention: where slice is only used for
            // array textures, and z layer of mtlArea.origin is only used for 3D textures.
            ASSERT(slice == 0 || params.textureArea.z == 0);

            // For mtl::RenderUtils we convert to OpenGL convention: z layer is used as either array
            // texture's slice or 3D texture's layer index.
            params.textureArea.z += slice;

            ANGLE_TRY(contextMtl->getDisplay()->getUtils().unpackPixelsFromBufferToTexture(
                contextMtl, pixelsAngleFormat, params));
        }
    }  // if (unpackBuffer)
    else
    {
        LoadImageFunctionInfo loadFunctionInfo = mFormat.textureLoadFunctions
                                                     ? mFormat.textureLoadFunctions(type)
                                                     : LoadImageFunctionInfo();
        const angle::Format &dstFormat         = angle::Format::Get(mFormat.actualFormatId);
        const size_t dstRowPitch               = dstFormat.pixelBytes * mtlArea.size.width;

        // It is very important to avoid allocating a new buffer for each row during these
        // uploads.
        const bool kAvoidStagingBuffers = true;

        // Check if original image data is compressed:
        if (mFormat.intendedAngleFormat().isBlock)
        {
            if (mFormat.intendedFormatId != mFormat.actualFormatId)
            {
                ASSERT(loadFunctionInfo.loadFunction);

                // Need to create a buffer to hold entire decompressed image.
                const size_t dstDepthPitch = dstRowPitch * mtlArea.size.height;
                angle::MemoryBuffer decompressBuf;
                ANGLE_CHECK_GL_ALLOC(contextMtl,
                                     decompressBuf.resize(dstDepthPitch * mtlArea.size.depth));

                // Decompress
                loadFunctionInfo.loadFunction(contextMtl->getImageLoadContext(), mtlArea.size.width,
                                              mtlArea.size.height, mtlArea.size.depth, pixels,
                                              pixelsRowPitch, pixelsDepthPitch,
                                              decompressBuf.data(), dstRowPitch, dstDepthPitch);

                // Upload to texture
                ANGLE_TRY(UploadTextureContents(
                    context, dstFormat, mtlArea, mtl::kZeroNativeMipLevel, slice,
                    decompressBuf.data(), dstRowPitch, dstDepthPitch, kAvoidStagingBuffers, image));
            }
            else
            {
                // Assert that we're filling the level in it's entierety.
                ASSERT(mtlArea.size.width == static_cast<unsigned int>(image->sizeAt0().width));
                ASSERT(mtlArea.size.height == static_cast<unsigned int>(image->sizeAt0().height));
                const size_t dstDepthPitch = dstRowPitch * mtlArea.size.height;
                ANGLE_TRY(UploadTextureContents(
                    context, dstFormat, mtlArea, mtl::kZeroNativeMipLevel, slice, pixels,
                    dstRowPitch, dstDepthPitch, kAvoidStagingBuffers, image));
            }
        }  // if (mFormat.intendedAngleFormat().isBlock)
        else
        {
            // Create scratch row buffer
            angle::MemoryBuffer conversionRow;
            ANGLE_CHECK_GL_ALLOC(contextMtl, conversionRow.resize(dstRowPitch));

            // Convert row by row:
            MTLRegion mtlRow   = mtlArea;
            mtlRow.size.height = mtlRow.size.depth = 1;
            for (NSUInteger d = 0; d < mtlArea.size.depth; ++d)
            {
                mtlRow.origin.z = mtlArea.origin.z + d;
                for (NSUInteger r = 0; r < mtlArea.size.height; ++r)
                {
                    const uint8_t *psrc = pixels + d * pixelsDepthPitch + r * pixelsRowPitch;
                    mtlRow.origin.y     = mtlArea.origin.y + r;

                    // Convert pixels
                    if (loadFunctionInfo.loadFunction)
                    {
                        loadFunctionInfo.loadFunction(contextMtl->getImageLoadContext(),
                                                      mtlRow.size.width, 1, 1, psrc, pixelsRowPitch,
                                                      0, conversionRow.data(), dstRowPitch, 0);
                    }
                    else if (mFormat.hasDepthOrStencilBits())
                    {
                        ConvertDepthStencilData(mtlRow.size, pixelsAngleFormat, pixelsRowPitch, 0,
                                                psrc, dstFormat, nullptr, dstRowPitch, 0,
                                                conversionRow.data());
                    }
                    else
                    {
                        CopyImageCHROMIUM(psrc, pixelsRowPitch, pixelsAngleFormat.pixelBytes, 0,
                                          pixelsAngleFormat.pixelReadFunction, conversionRow.data(),
                                          dstRowPitch, dstFormat.pixelBytes, 0,
                                          dstFormat.pixelWriteFunction, internalFormat.format,
                                          dstFormat.componentType, mtlRow.size.width, 1, 1, false,
                                          false, false);
                    }

                    // Upload to texture
                    ANGLE_TRY(UploadTextureContents(
                        context, dstFormat, mtlRow, mtl::kZeroNativeMipLevel, slice,
                        conversionRow.data(), dstRowPitch, 0, kAvoidStagingBuffers, image));
                }
            }
        }  // if (mFormat.intendedAngleFormat().isBlock)
    }      // if (unpackBuffer)

    return angle::Result::Continue;
}

angle::Result TextureMtl::checkForEmulatedChannels(const gl::Context *context,
                                                   const mtl::Format &mtlFormat,
                                                   const mtl::TextureRef &texture)
{
    bool emulatedChannels = mtl::IsFormatEmulated(mtlFormat);

    // For emulated channels that GL texture intends to not have,
    // we need to initialize their content.
    if (emulatedChannels)
    {
        uint32_t mipmaps = texture->mipmapLevels();

        uint32_t layers = texture->cubeFacesOrArrayLength();
        for (uint32_t layer = 0; layer < layers; ++layer)
        {
            for (uint32_t mip = 0; mip < mipmaps; ++mip)
            {
                auto index = mtl::ImageNativeIndex::FromBaseZeroGLIndex(
                    GetCubeOrArraySliceMipIndex(texture, layer, mip));

                ANGLE_TRY(mtl::InitializeTextureContents(context, texture, mtlFormat, index));
            }
        }
    }
    return angle::Result::Continue;
}

angle::Result TextureMtl::initializeContents(const gl::Context *context,
                                             GLenum binding,
                                             const gl::ImageIndex &index)
{
    if (index.isLayered())
    {
        // InitializeTextureContents is only able to initialize one layer at a time.
        const gl::ImageDesc &desc = mState.getImageDesc(index);
        uint32_t layerCount;
        if (index.isEntireLevelCubeMap())
        {
            layerCount = 6;
        }
        else
        {
            layerCount = desc.size.depth;
        }

        gl::ImageIndexIterator ite = index.getLayerIterator(layerCount);
        while (ite.hasNext())
        {
            gl::ImageIndex layerIndex = ite.next();
            ANGLE_TRY(initializeContents(context, GL_NONE, layerIndex));
        }
        return angle::Result::Continue;
    }
    else if (index.getLayerCount() > 1)
    {
        for (int layer = 0; layer < index.getLayerCount(); ++layer)
        {
            int layerIdx = layer + index.getLayerIndex();
            gl::ImageIndex layerIndex =
                gl::ImageIndex::MakeFromType(index.getType(), index.getLevelIndex(), layerIdx);
            ANGLE_TRY(initializeContents(context, GL_NONE, layerIndex));
        }
        return angle::Result::Continue;
    }

    ASSERT(index.getLayerCount() == 1 && !index.isLayered());
    ANGLE_TRY(ensureImageCreated(context, index));
    ContextMtl *contextMtl       = mtl::GetImpl(context);
    ImageDefinitionMtl &imageDef = getImageDefinition(index);
    const mtl::TextureRef &image = imageDef.image;
    const mtl::Format &format    = contextMtl->getPixelFormat(imageDef.formatID);
    // For Texture's image definition, we always use zero mip level.
    if (format.metalFormat == MTLPixelFormatInvalid)
    {
        return angle::Result::Stop;
    }
    return mtl::InitializeTextureContents(
        context, image, format,
        mtl::ImageNativeIndex::FromBaseZeroGLIndex(
            GetLayerMipIndex(image, GetImageLayerIndexFrom(index), /** level */ 0)));
}

angle::Result TextureMtl::copySubImageImpl(const gl::Context *context,
                                           const gl::ImageIndex &index,
                                           const gl::Offset &destOffset,
                                           const gl::Rectangle &sourceArea,
                                           const gl::InternalFormat &internalFormat,
                                           const FramebufferMtl *source,
                                           const RenderTargetMtl *colorReadRT)
{
    if (!colorReadRT || !colorReadRT->getTexture())
    {
        // Is this an error?
        return angle::Result::Continue;
    }

    gl::Extents fbSize = colorReadRT->getTexture()->size(colorReadRT->getLevelIndex());
    gl::Rectangle clippedSourceArea;
    if (!ClipRectangle(sourceArea, gl::Rectangle(0, 0, fbSize.width, fbSize.height),
                       &clippedSourceArea))
    {
        return angle::Result::Continue;
    }

    // If negative offsets are given, clippedSourceArea ensures we don't read from those offsets.
    // However, that changes the sourceOffset->destOffset mapping.  Here, destOffset is shifted by
    // the same amount as clipped to correct the error.
    const gl::Offset modifiedDestOffset(destOffset.x + clippedSourceArea.x - sourceArea.x,
                                        destOffset.y + clippedSourceArea.y - sourceArea.y, 0);

    ANGLE_TRY(ensureImageCreated(context, index));

    if (!mFormat.getCaps().isRenderable())
    {
        return copySubImageCPU(context, index, modifiedDestOffset, clippedSourceArea,
                               internalFormat, source, colorReadRT);
    }

    // NOTE(hqle): Use compute shader.
    return copySubImageWithDraw(context, index, modifiedDestOffset, clippedSourceArea,
                                internalFormat, source, colorReadRT);
}

angle::Result TextureMtl::copySubImageWithDraw(const gl::Context *context,
                                               const gl::ImageIndex &index,
                                               const gl::Offset &modifiedDestOffset,
                                               const gl::Rectangle &clippedSourceArea,
                                               const gl::InternalFormat &internalFormat,
                                               const FramebufferMtl *source,
                                               const RenderTargetMtl *colorReadRT)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    DisplayMtl *displayMtl = contextMtl->getDisplay();

    RenderTargetMtl *imageRtt;
    ANGLE_TRY(getRenderTarget(contextMtl, index, /*implicitSamples=*/0, &imageRtt));

    mtl::RenderCommandEncoder *cmdEncoder = contextMtl->getRenderTargetCommandEncoder(*imageRtt);
    mtl::ColorBlitParams blitParams;

    blitParams.dstTextureSize = imageRtt->getTexture()->size(imageRtt->getLevelIndex());
    blitParams.dstRect        = gl::Rectangle(modifiedDestOffset.x, modifiedDestOffset.y,
                                              clippedSourceArea.width, clippedSourceArea.height);
    blitParams.dstScissorRect = blitParams.dstRect;

    blitParams.enabledBuffers.set(0);

    blitParams.src      = colorReadRT->getTexture();
    blitParams.srcLevel = colorReadRT->getLevelIndex();
    blitParams.srcLayer = colorReadRT->getLayerIndex();

    blitParams.srcNormalizedCoords = mtl::NormalizedCoords(
        clippedSourceArea, colorReadRT->getTexture()->size(blitParams.srcLevel));
    blitParams.srcYFlipped  = source->flipY();
    blitParams.dstLuminance = internalFormat.isLUMA();

    return displayMtl->getUtils().blitColorWithDraw(
        context, cmdEncoder, colorReadRT->getFormat().actualAngleFormat(), blitParams);
}

angle::Result TextureMtl::copySubImageCPU(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &modifiedDestOffset,
                                          const gl::Rectangle &clippedSourceArea,
                                          const gl::InternalFormat &internalFormat,
                                          const FramebufferMtl *source,
                                          const RenderTargetMtl *colorReadRT)
{
    mtl::TextureRef &image = getImage(index);
    ASSERT(image && image->valid());

    ContextMtl *contextMtl = mtl::GetImpl(context);

    const angle::Format &dstFormat = angle::Format::Get(mFormat.actualFormatId);
    const int dstRowPitch          = dstFormat.pixelBytes * clippedSourceArea.width;
    angle::MemoryBuffer conversionRow;
    ANGLE_CHECK_GL_ALLOC(contextMtl, conversionRow.resize(dstRowPitch));

    gl::Rectangle srcRowArea = gl::Rectangle(clippedSourceArea.x, 0, clippedSourceArea.width, 1);
    MTLRegion mtlDstRowArea  = MTLRegionMake2D(modifiedDestOffset.x, 0, clippedSourceArea.width, 1);
    uint32_t dstSlice        = 0;
    switch (index.getType())
    {
        case gl::TextureType::_2D:
        case gl::TextureType::CubeMap:
            dstSlice = 0;
            break;
        case gl::TextureType::_2DArray:
            ASSERT(index.hasLayer());
            dstSlice = index.getLayerIndex();
            break;
        case gl::TextureType::_3D:
            ASSERT(index.hasLayer());
            dstSlice               = 0;
            mtlDstRowArea.origin.z = index.getLayerIndex();
            break;
        default:
            UNREACHABLE();
    }

    // It is very important to avoid allocating a new buffer for each row during these
    // uploads.
    const bool kAvoidStagingBuffers = true;

    // Copy row by row:
    for (int r = 0; r < clippedSourceArea.height; ++r)
    {
        mtlDstRowArea.origin.y = modifiedDestOffset.y + r;
        srcRowArea.y           = clippedSourceArea.y + r;

        PackPixelsParams packParams(srcRowArea, dstFormat, dstRowPitch, false, nullptr, 0);

        // Read pixels from framebuffer to memory:
        gl::Rectangle flippedSrcRowArea = source->getCorrectFlippedReadArea(context, srcRowArea);
        ANGLE_TRY(source->readPixelsImpl(context, flippedSrcRowArea, packParams, colorReadRT,
                                         conversionRow.data()));

        // Upload to texture
        ANGLE_TRY(UploadTextureContents(context, dstFormat, mtlDstRowArea, mtl::kZeroNativeMipLevel,
                                        dstSlice, conversionRow.data(), dstRowPitch, 0,
                                        kAvoidStagingBuffers, image));
    }

    return angle::Result::Continue;
}

angle::Result TextureMtl::copySubTextureImpl(const gl::Context *context,
                                             const gl::ImageIndex &index,
                                             const gl::Offset &destOffset,
                                             const gl::InternalFormat &internalFormat,
                                             GLint sourceLevel,
                                             const gl::Box &sourceBox,
                                             bool unpackFlipY,
                                             bool unpackPremultiplyAlpha,
                                             bool unpackUnmultiplyAlpha,
                                             const gl::Texture *source)
{
    // Only 2D textures are supported.
    ASSERT(sourceBox.depth == 1);
    ASSERT(source->getType() == gl::TextureType::_2D);
    gl::ImageIndex sourceIndex = gl::ImageIndex::Make2D(sourceLevel);

    ContextMtl *contextMtl = mtl::GetImpl(context);
    TextureMtl *sourceMtl  = mtl::GetImpl(source);

    ANGLE_TRY(ensureImageCreated(context, index));

    ANGLE_TRY(sourceMtl->ensureImageCreated(context, sourceIndex));

    const ImageDefinitionMtl &srcImageDef  = sourceMtl->getImageDefinition(sourceIndex);
    const mtl::TextureRef &sourceImage     = srcImageDef.image;
    const mtl::Format &sourceFormat        = contextMtl->getPixelFormat(srcImageDef.formatID);
    const angle::Format &sourceAngleFormat = sourceFormat.actualAngleFormat();

    if (!mFormat.getCaps().isRenderable())
    {
        return copySubTextureCPU(context, index, destOffset, internalFormat,
                                 mtl::kZeroNativeMipLevel, sourceBox, sourceAngleFormat,
                                 unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha,
                                 sourceImage);
    }
    return copySubTextureWithDraw(
        context, index, destOffset, internalFormat, mtl::kZeroNativeMipLevel, sourceBox,
        sourceAngleFormat, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha, sourceImage);
}

angle::Result TextureMtl::copySubTextureWithDraw(const gl::Context *context,
                                                 const gl::ImageIndex &index,
                                                 const gl::Offset &destOffset,
                                                 const gl::InternalFormat &internalFormat,
                                                 const mtl::MipmapNativeLevel &sourceNativeLevel,
                                                 const gl::Box &sourceBox,
                                                 const angle::Format &sourceAngleFormat,
                                                 bool unpackFlipY,
                                                 bool unpackPremultiplyAlpha,
                                                 bool unpackUnmultiplyAlpha,
                                                 const mtl::TextureRef &sourceTexture)
{
    ContextMtl *contextMtl = mtl::GetImpl(context);
    DisplayMtl *displayMtl = contextMtl->getDisplay();

    mtl::TextureRef image = getImage(index);
    ASSERT(image && image->valid());

    if (internalFormat.colorEncoding == GL_SRGB)
    {
        image = image->getLinearColorView();
    }

    mtl::RenderCommandEncoder *cmdEncoder = contextMtl->getTextureRenderCommandEncoder(
        image, mtl::ImageNativeIndex::FromBaseZeroGLIndex(GetZeroLevelIndex(image)));
    mtl::ColorBlitParams blitParams;

    blitParams.dstTextureSize = image->sizeAt0();
    blitParams.dstRect =
        gl::Rectangle(destOffset.x, destOffset.y, sourceBox.width, sourceBox.height);
    blitParams.dstScissorRect = blitParams.dstRect;

    blitParams.enabledBuffers.set(0);

    blitParams.src      = sourceTexture;
    blitParams.srcLevel = sourceNativeLevel;
    blitParams.srcLayer = 0;
    blitParams.srcNormalizedCoords =
        mtl::NormalizedCoords(sourceBox.toRect(), sourceTexture->size(sourceNativeLevel));
    blitParams.srcYFlipped            = false;
    blitParams.dstLuminance           = internalFormat.isLUMA();
    blitParams.unpackFlipY            = unpackFlipY;
    blitParams.unpackPremultiplyAlpha = unpackPremultiplyAlpha;
    blitParams.unpackUnmultiplyAlpha  = unpackUnmultiplyAlpha;
    blitParams.transformLinearToSrgb  = sourceAngleFormat.isSRGB;

    return displayMtl->getUtils().copyTextureWithDraw(context, cmdEncoder, sourceAngleFormat,
                                                      mFormat.actualAngleFormat(), blitParams);
}

angle::Result TextureMtl::copySubTextureCPU(const gl::Context *context,
                                            const gl::ImageIndex &index,
                                            const gl::Offset &destOffset,
                                            const gl::InternalFormat &internalFormat,
                                            const mtl::MipmapNativeLevel &sourceNativeLevel,
                                            const gl::Box &sourceBox,
                                            const angle::Format &sourceAngleFormat,
                                            bool unpackFlipY,
                                            bool unpackPremultiplyAlpha,
                                            bool unpackUnmultiplyAlpha,
                                            const mtl::TextureRef &sourceTexture)
{
    mtl::TextureRef &image = getImage(index);
    ASSERT(image && image->valid());

    ContextMtl *contextMtl = mtl::GetImpl(context);

    const angle::Format &dstAngleFormat = mFormat.actualAngleFormat();
    const int srcRowPitch               = sourceAngleFormat.pixelBytes * sourceBox.width;
    const int srcImageSize              = srcRowPitch * sourceBox.height;
    const int convRowPitch              = dstAngleFormat.pixelBytes * sourceBox.width;
    const int convImageSize             = convRowPitch * sourceBox.height;
    angle::MemoryBuffer conversionSrc, conversionDst;
    ANGLE_CHECK_GL_ALLOC(contextMtl, conversionSrc.resize(srcImageSize));
    ANGLE_CHECK_GL_ALLOC(contextMtl, conversionDst.resize(convImageSize));

    MTLRegion mtlSrcArea =
        MTLRegionMake2D(sourceBox.x, sourceBox.y, sourceBox.width, sourceBox.height);
    MTLRegion mtlDstArea =
        MTLRegionMake2D(destOffset.x, destOffset.y, sourceBox.width, sourceBox.height);

    // Read pixels from source to memory:
    sourceTexture->getBytes(contextMtl, srcRowPitch, 0, mtlSrcArea, sourceNativeLevel, 0,
                            conversionSrc.data());

    // Convert to destination format
    CopyImageCHROMIUM(conversionSrc.data(), srcRowPitch, sourceAngleFormat.pixelBytes, 0,
                      sourceAngleFormat.pixelReadFunction, conversionDst.data(), convRowPitch,
                      dstAngleFormat.pixelBytes, 0, dstAngleFormat.pixelWriteFunction,
                      internalFormat.format, internalFormat.componentType, sourceBox.width,
                      sourceBox.height, 1, unpackFlipY, unpackPremultiplyAlpha,
                      unpackUnmultiplyAlpha);

    // Upload to texture
    ANGLE_TRY(UploadTextureContents(context, dstAngleFormat, mtlDstArea, mtl::kZeroNativeMipLevel,
                                    0, conversionDst.data(), convRowPitch, 0, false, image));

    return angle::Result::Continue;
}

}  // namespace rx
