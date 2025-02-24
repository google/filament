//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureVk.cpp:
//    Implements the class methods for TextureVk.
//

#include "libANGLE/renderer/vulkan/TextureVk.h"
#include <vulkan/vulkan.h>

#include "common/debug.h"
#include "image_util/generatemip.inc"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/Image.h"
#include "libANGLE/MemoryObject.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ImageVk.h"
#include "libANGLE/renderer/vulkan/MemoryObjectVk.h"
#include "libANGLE/renderer/vulkan/RenderbufferVk.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"
#include "libANGLE/renderer/vulkan/UtilsVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_renderer.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{
namespace
{
constexpr VkImageUsageFlags kTransferImageFlags =
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

constexpr VkImageUsageFlags kColorAttachmentImageFlags =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

constexpr VkImageUsageFlags kDrawStagingImageFlags =
    kTransferImageFlags | kColorAttachmentImageFlags;

constexpr VkFormatFeatureFlags kBlitFeatureFlags =
    VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;

constexpr VkImageAspectFlags kDepthStencilAspects =
    VK_IMAGE_ASPECT_STENCIL_BIT | VK_IMAGE_ASPECT_DEPTH_BIT;

constexpr angle::SubjectIndex kTextureImageSubjectIndex = 0;

// Test whether a texture level is within the range of levels for which the current image is
// allocated.  This is used to ensure out-of-range updates are staged in the image, and not
// attempted to be directly applied.
bool IsTextureLevelInAllocatedImage(const vk::ImageHelper &image,
                                    gl::LevelIndex textureLevelIndexGL)
{
    gl::LevelIndex imageFirstAllocateLevel = image.getFirstAllocatedLevel();
    if (textureLevelIndexGL < imageFirstAllocateLevel)
    {
        return false;
    }

    vk::LevelIndex imageLevelIndexVk = image.toVkLevel(textureLevelIndexGL);
    return imageLevelIndexVk < vk::LevelIndex(image.getLevelCount());
}

// Test whether a redefined texture level is compatible with the currently allocated image.  Returns
// true if the given size and format match the corresponding mip in the allocated image (taking
// base level into account).  This could return false when:
//
// - Defining a texture level that is outside the range of the image levels.  In this case, changes
//   to this level should remain staged until the texture is redefined to include this level.
// - Redefining a texture level that is within the range of the image levels, but has a different
//   size or format.  In this case too, changes to this level should remain staged as the texture
//   is no longer complete as is.
bool IsTextureLevelDefinitionCompatibleWithImage(const vk::ImageHelper &image,
                                                 gl::LevelIndex textureLevelIndexGL,
                                                 const gl::Extents &size,
                                                 angle::FormatID intendedFormatID,
                                                 angle::FormatID actualFormatID)
{
    if (!IsTextureLevelInAllocatedImage(image, textureLevelIndexGL))
    {
        return false;
    }

    vk::LevelIndex imageLevelIndexVk = image.toVkLevel(textureLevelIndexGL);
    return size == image.getLevelExtents(imageLevelIndexVk) &&
           intendedFormatID == image.getIntendedFormatID() &&
           actualFormatID == image.getActualFormatID();
}

bool CanCopyWithTransferForTexImage(vk::Renderer *renderer,
                                    angle::FormatID srcIntendedFormatID,
                                    angle::FormatID srcActualFormatID,
                                    VkImageTiling srcTilingMode,
                                    angle::FormatID dstIntendedFormatID,
                                    angle::FormatID dstActualFormatID,
                                    VkImageTiling dstTilingMode,
                                    bool isViewportFlipY)
{
    // For glTex[Sub]Image, only accept same-format transfers.
    // There are cases that two images' actual format is the same, but intended formats are
    // different due to one is using the fallback format (for example, RGB fallback to RGBA). In
    // these situations CanCopyWithTransfer will say yes. But if we use transfer to do copy, the
    // alpha channel will be also be copied with source data which is wrong.
    bool isFormatCompatible =
        srcIntendedFormatID == dstIntendedFormatID && srcActualFormatID == dstActualFormatID;

    return !isViewportFlipY && isFormatCompatible &&
           vk::CanCopyWithTransfer(renderer, srcActualFormatID, srcTilingMode, dstActualFormatID,
                                   dstTilingMode);
}

bool CanCopyWithTransferForCopyTexture(vk::Renderer *renderer,
                                       const vk::ImageHelper &srcImage,
                                       VkImageTiling srcTilingMode,
                                       angle::FormatID destIntendedFormatID,
                                       angle::FormatID destActualFormatID,
                                       VkImageTiling destTilingMode,
                                       bool unpackFlipY,
                                       bool unpackPremultiplyAlpha,
                                       bool unpackUnmultiplyAlpha)
{
    if (unpackFlipY || unpackPremultiplyAlpha || unpackUnmultiplyAlpha)
    {
        return false;
    }

    if (!vk::CanCopyWithTransfer(renderer, srcImage.getActualFormatID(), srcTilingMode,
                                 destActualFormatID, destTilingMode))
    {
        return false;
    }

    // If the formats are identical, we can always transfer between them.
    if (srcImage.getIntendedFormatID() == destIntendedFormatID &&
        srcImage.getActualFormatID() == destActualFormatID)
    {
        return true;
    }

    // If either format is emulated, cannot transfer.
    if (srcImage.hasEmulatedImageFormat() ||
        vk::HasEmulatedImageFormat(destIntendedFormatID, destActualFormatID))
    {
        return false;
    }

    // Otherwise, allow transfer between compatible formats.  This is derived from the specification
    // of CHROMIUM_copy_texture.
    const angle::Format &srcAngleFormat  = srcImage.getActualFormat();
    const angle::Format &destAngleFormat = angle::Format::Get(destActualFormatID);

    const bool srcIsBGRA   = srcAngleFormat.isBGRA();
    const bool srcHasR8    = srcAngleFormat.redBits == 8;
    const bool srcHasG8    = srcAngleFormat.greenBits == 8;
    const bool srcHasB8    = srcAngleFormat.blueBits == 8;
    const bool srcHasA8    = srcAngleFormat.alphaBits == 8;
    const bool srcIsSigned = srcAngleFormat.isSnorm() || srcAngleFormat.isSint();

    const bool destIsBGRA   = destAngleFormat.isBGRA();
    const bool destHasR8    = destAngleFormat.redBits == 8;
    const bool destHasG8    = destAngleFormat.greenBits == 8;
    const bool destHasB8    = destAngleFormat.blueBits == 8;
    const bool destHasA8    = destAngleFormat.alphaBits == 8;
    const bool destIsSigned = destAngleFormat.isSnorm() || destAngleFormat.isSint();

    // Copy is allowed as long as they have the same number, ordering and sign of (8-bit) channels.
    // CHROMIUM_copy_texture expects verbatim copy between these format, so this copy is done
    // regardless of sRGB, normalized, etc.
    return srcIsBGRA == destIsBGRA && srcHasR8 == destHasR8 && srcHasG8 == destHasG8 &&
           srcHasB8 == destHasB8 && srcHasA8 == destHasA8 && srcIsSigned == destIsSigned;
}

bool CanCopyWithDraw(vk::Renderer *renderer,
                     const angle::FormatID srcFormatID,
                     VkImageTiling srcTilingMode,
                     const angle::FormatID dstFormatID,
                     VkImageTiling destTilingMode)
{
    // Checks that the formats in copy by drawing have the appropriate feature bits
    bool srcFormatHasNecessaryFeature = vk::FormatHasNecessaryFeature(
        renderer, srcFormatID, srcTilingMode, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    bool dstFormatHasNecessaryFeature = vk::FormatHasNecessaryFeature(
        renderer, dstFormatID, destTilingMode, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);

    return srcFormatHasNecessaryFeature && dstFormatHasNecessaryFeature;
}

bool CanGenerateMipmapWithCompute(vk::Renderer *renderer,
                                  VkImageType imageType,
                                  angle::FormatID formatID,
                                  GLint samples,
                                  bool canBeRespecified)
{
    // Feature needs to be enabled
    if (!renderer->getFeatures().allowGenerateMipmapWithCompute.enabled)
    {
        return false;
    }

    // We need to be able to respecify the backing image
    if (!canBeRespecified)
    {
        return false;
    }

    const angle::Format &angleFormat = angle::Format::Get(formatID);
    // Format must have STORAGE support.
    const bool hasStorageSupport =
        renderer->hasImageFormatFeatureBits(formatID, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

    // No support for sRGB formats yet.
    const bool isSRGB = angleFormat.isSRGB;

    // No support for integer formats yet.
    const bool isInt = angleFormat.isInt();

    // Only 2D images are supported.
    const bool is2D = imageType == VK_IMAGE_TYPE_2D;

    // No support for multisampled images yet.
    const bool isMultisampled = samples > 1;

    // Only color formats are supported.
    const bool isColorFormat = !angleFormat.hasDepthOrStencilBits();

    return hasStorageSupport && !isSRGB && !isInt && is2D && !isMultisampled && isColorFormat;
}

void GetRenderTargetLayerCountAndIndex(vk::ImageHelper *image,
                                       const gl::ImageIndex &index,
                                       GLuint *layerIndex,
                                       GLuint *layerCount,
                                       GLuint *imageLayerCount)
{
    *layerIndex = index.hasLayer() ? index.getLayerIndex() : 0;
    *layerCount = index.getLayerCount();

    switch (index.getType())
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::External:
            ASSERT(*layerIndex == 0 &&
                   (*layerCount == 1 ||
                    *layerCount == static_cast<GLuint>(gl::ImageIndex::kEntireLevel)));
            *imageLayerCount = 1;
            break;

        case gl::TextureType::CubeMap:
            ASSERT(!index.hasLayer() ||
                   *layerIndex == static_cast<GLuint>(index.cubeMapFaceIndex()));
            *imageLayerCount = gl::kCubeFaceCount;
            break;

        case gl::TextureType::_3D:
        {
            gl::LevelIndex levelGL(index.getLevelIndex());
            *imageLayerCount = image->getLevelExtents(image->toVkLevel(levelGL)).depth;
            break;
        }

        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisampleArray:
        case gl::TextureType::CubeMapArray:
            *imageLayerCount = image->getLayerCount();
            break;

        default:
            UNREACHABLE();
    }

    if (*layerCount == static_cast<GLuint>(gl::ImageIndex::kEntireLevel))
    {
        ASSERT(*layerIndex == 0);
        *layerCount = *imageLayerCount;
    }
}

void Set3DBaseArrayLayerAndLayerCount(VkImageSubresourceLayers *Subresource)
{
    // If the srcImage/dstImage parameters are of VkImageType VK_IMAGE_TYPE_3D, the baseArrayLayer
    // and layerCount members of the corresponding subresource must be 0 and 1, respectively.
    Subresource->baseArrayLayer = 0;
    Subresource->layerCount     = 1;
}

const vk::Format *AdjustStorageViewFormatPerWorkarounds(vk::Renderer *renderer,
                                                        const vk::Format *intended,
                                                        vk::ImageAccess access)
{
    // r32f images are emulated with r32ui.
    if (renderer->getFeatures().emulateR32fImageAtomicExchange.enabled &&
        intended->getActualImageFormatID(access) == angle::FormatID::R32_FLOAT)
    {
        return &renderer->getFormat(angle::FormatID::R32_UINT);
    }

    return intended;
}

const vk::Format *AdjustViewFormatForSampler(vk::Renderer *renderer,
                                             const vk::Format *intended,
                                             gl::SamplerFormat samplerFormat)
{
    switch (samplerFormat)
    {
        case gl::SamplerFormat::Float:
            switch (intended->getIntendedFormatID())
            {
                case angle::FormatID::R8_UNORM:
                case angle::FormatID::R8G8_UNORM:
                case angle::FormatID::R8G8B8A8_UNORM:
                case angle::FormatID::R16_UNORM:
                case angle::FormatID::R16G16_UNORM:
                case angle::FormatID::R16G16B16A16_UNORM:
                case angle::FormatID::R16_FLOAT:
                case angle::FormatID::R16G16_FLOAT:
                case angle::FormatID::R16G16B16A16_FLOAT:
                case angle::FormatID::R32_FLOAT:
                case angle::FormatID::R32G32_FLOAT:
                case angle::FormatID::R32G32B32_FLOAT:
                case angle::FormatID::R32G32B32A32_FLOAT:
                    return intended;
                case angle::FormatID::R8_SINT:
                case angle::FormatID::R8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8_UNORM);
                case angle::FormatID::R16_SINT:
                case angle::FormatID::R16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16_FLOAT);
                case angle::FormatID::R32_SINT:
                case angle::FormatID::R32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32_FLOAT);
                case angle::FormatID::R8G8_SINT:
                case angle::FormatID::R8G8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8G8_UNORM);
                case angle::FormatID::R16G16_SINT:
                case angle::FormatID::R16G16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16G16_FLOAT);
                case angle::FormatID::R32G32_SINT:
                case angle::FormatID::R32G32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32_FLOAT);
                case angle::FormatID::R32G32B32_SINT:
                case angle::FormatID::R32G32B32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32_FLOAT);
                case angle::FormatID::R8G8B8A8_SINT:
                case angle::FormatID::R8G8B8A8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8G8B8A8_UNORM);
                case angle::FormatID::R16G16B16A16_SINT:
                case angle::FormatID::R16G16B16A16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16G16B16A16_FLOAT);
                case angle::FormatID::R32G32B32A32_SINT:
                case angle::FormatID::R32G32B32A32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32A32_FLOAT);
                default:
                    UNREACHABLE();
                    return intended;
            }
        case gl::SamplerFormat::Unsigned:
            switch (intended->getIntendedFormatID())
            {
                case angle::FormatID::R8_UINT:
                case angle::FormatID::R16_UINT:
                case angle::FormatID::R32_UINT:
                case angle::FormatID::R8G8_UINT:
                case angle::FormatID::R16G16_UINT:
                case angle::FormatID::R32G32_UINT:
                case angle::FormatID::R32G32B32_UINT:
                case angle::FormatID::R8G8B8A8_UINT:
                case angle::FormatID::R16G16B16A16_UINT:
                case angle::FormatID::R32G32B32A32_UINT:
                    return intended;
                case angle::FormatID::R8_UNORM:
                case angle::FormatID::R8_SINT:
                    return &renderer->getFormat(angle::FormatID::R8_UINT);
                case angle::FormatID::R16_FLOAT:
                case angle::FormatID::R16_SINT:
                    return &renderer->getFormat(angle::FormatID::R16_UINT);
                case angle::FormatID::R32_FLOAT:
                case angle::FormatID::R32_SINT:
                    return &renderer->getFormat(angle::FormatID::R32_UINT);
                case angle::FormatID::R8G8_UNORM:
                case angle::FormatID::R8G8_SINT:
                    return &renderer->getFormat(angle::FormatID::R8G8_UINT);
                case angle::FormatID::R16G16_FLOAT:
                case angle::FormatID::R16G16_SINT:
                    return &renderer->getFormat(angle::FormatID::R16G16_UINT);
                case angle::FormatID::R32G32_FLOAT:
                case angle::FormatID::R32G32_SINT:
                    return &renderer->getFormat(angle::FormatID::R32G32_UINT);
                case angle::FormatID::R32G32B32_FLOAT:
                case angle::FormatID::R32G32B32_SINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32_UINT);
                case angle::FormatID::R8G8B8A8_UNORM:
                case angle::FormatID::R8G8B8A8_SINT:
                    return &renderer->getFormat(angle::FormatID::R8G8B8A8_UINT);
                case angle::FormatID::R16G16B16A16_FLOAT:
                case angle::FormatID::R16G16B16A16_SINT:
                    return &renderer->getFormat(angle::FormatID::R16G16B16A16_UINT);
                case angle::FormatID::R32G32B32A32_FLOAT:
                case angle::FormatID::R32G32B32A32_SINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32A32_UINT);
                default:
                    UNREACHABLE();
                    return intended;
            }
        case gl::SamplerFormat::Signed:
            switch (intended->getIntendedFormatID())
            {
                case angle::FormatID::R8_SINT:
                case angle::FormatID::R16_SINT:
                case angle::FormatID::R32_SINT:
                case angle::FormatID::R8G8_SINT:
                case angle::FormatID::R16G16_SINT:
                case angle::FormatID::R32G32_SINT:
                case angle::FormatID::R32G32B32_SINT:
                case angle::FormatID::R8G8B8A8_SINT:
                case angle::FormatID::R16G16B16A16_SINT:
                case angle::FormatID::R32G32B32A32_SINT:
                    return intended;
                case angle::FormatID::R8_UNORM:
                case angle::FormatID::R8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8_SINT);
                case angle::FormatID::R16_FLOAT:
                case angle::FormatID::R16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16_SINT);
                case angle::FormatID::R32_FLOAT:
                case angle::FormatID::R32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32_SINT);
                case angle::FormatID::R8G8_UNORM:
                case angle::FormatID::R8G8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8G8_SINT);
                case angle::FormatID::R16G16_FLOAT:
                case angle::FormatID::R16G16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16G16_SINT);
                case angle::FormatID::R32G32_FLOAT:
                case angle::FormatID::R32G32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32_SINT);
                case angle::FormatID::R32G32B32_FLOAT:
                case angle::FormatID::R32G32B32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32_SINT);
                case angle::FormatID::R8G8B8A8_UNORM:
                case angle::FormatID::R8G8B8A8_UINT:
                    return &renderer->getFormat(angle::FormatID::R8G8B8A8_SINT);
                case angle::FormatID::R16G16B16A16_FLOAT:
                case angle::FormatID::R16G16B16A16_UINT:
                    return &renderer->getFormat(angle::FormatID::R16G16B16A16_SINT);
                case angle::FormatID::R32G32B32A32_FLOAT:
                case angle::FormatID::R32G32B32A32_UINT:
                    return &renderer->getFormat(angle::FormatID::R32G32B32A32_SINT);
                default:
                    UNREACHABLE();
                    return intended;
            }
        default:
            UNREACHABLE();
            return intended;
    }
}

angle::FormatID GetRGBAEmulationDstFormat(angle::FormatID srcFormatID)
{
    switch (srcFormatID)
    {
        case angle::FormatID::R32G32B32_UINT:
            return angle::FormatID::R32G32B32A32_UINT;
        case angle::FormatID::R32G32B32_SINT:
            return angle::FormatID::R32G32B32A32_SINT;
        case angle::FormatID::R32G32B32_FLOAT:
            return angle::FormatID::R32G32B32A32_FLOAT;
        default:
            return angle::FormatID::NONE;
    }
}

bool NeedsRGBAEmulation(vk::Renderer *renderer, angle::FormatID formatID)
{
    if (renderer->hasBufferFormatFeatureBits(formatID, VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT))
    {
        return false;
    }
    // Vulkan driver support is required for all formats except the ones we emulate.
    ASSERT(GetRGBAEmulationDstFormat(formatID) != angle::FormatID::NONE);
    return true;
}

}  // anonymous namespace

// TextureVk implementation.
TextureVk::TextureVk(const gl::TextureState &state, vk::Renderer *renderer)
    : TextureImpl(state),
      mOwnsImage(false),
      mRequiresMutableStorage(false),
      mRequiredImageAccess(vk::ImageAccess::SampleOnly),
      mImmutableSamplerDirty(false),
      mEGLImageNativeType(gl::TextureType::InvalidEnum),
      mEGLImageLayerOffset(0),
      mEGLImageLevelOffset(0),
      mImage(nullptr),
      mImageUsageFlags(0),
      mImageCreateFlags(0),
      mImageObserverBinding(this, kTextureImageSubjectIndex),
      mCurrentBaseLevel(state.getBaseLevel()),
      mCurrentMaxLevel(state.getMaxLevel()),
      mCachedImageViewSubresourceSerialSRGBDecode{},
      mCachedImageViewSubresourceSerialSkipDecode{}
{}

TextureVk::~TextureVk() = default;

void TextureVk::onDestroy(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    releaseAndDeleteImageAndViews(contextVk);
    resetSampler();
}

angle::Result TextureVk::setImage(const gl::Context *context,
                                  const gl::ImageIndex &index,
                                  GLenum internalFormat,
                                  const gl::Extents &size,
                                  GLenum format,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  gl::Buffer *unpackBuffer,
                                  const uint8_t *pixels)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat, type);

    return setImageImpl(context, index, formatInfo, size, type, unpack, unpackBuffer, pixels);
}

angle::Result TextureVk::setSubImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Box &area,
                                     GLenum format,
                                     GLenum type,
                                     const gl::PixelUnpackState &unpack,
                                     gl::Buffer *unpackBuffer,
                                     const uint8_t *pixels)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, type);
    ContextVk *contextVk                 = vk::GetImpl(context);
    const gl::ImageDesc &levelDesc       = mState.getImageDesc(index);
    const vk::Format &vkFormat =
        contextVk->getRenderer()->getFormat(levelDesc.format.info->sizedInternalFormat);

    return setSubImageImpl(context, index, area, formatInfo, type, unpack, unpackBuffer, pixels,
                           vkFormat);
}

bool TextureVk::isCompressedFormatEmulated(const gl::Context *context,
                                           gl::TextureTarget target,
                                           GLint level)
{
    const gl::ImageDesc &levelDesc = mState.getImageDesc(target, level);
    if (!levelDesc.format.info->compressed)
    {
        // If it isn't compressed, the remaining logic won't work
        return false;
    }

    // Check against the list of formats used to emulate compressed textures
    return gl::IsEmulatedCompressedFormat(levelDesc.format.info->sizedInternalFormat);
}

angle::Result TextureVk::setCompressedImage(const gl::Context *context,
                                            const gl::ImageIndex &index,
                                            GLenum internalFormat,
                                            const gl::Extents &size,
                                            const gl::PixelUnpackState &unpack,
                                            size_t imageSize,
                                            const uint8_t *pixels)
{
    const gl::InternalFormat &formatInfo = gl::GetSizedInternalFormatInfo(internalFormat);

    const gl::State &glState = context->getState();
    gl::Buffer *unpackBuffer = glState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    if (unpackBuffer &&
        this->isCompressedFormatEmulated(context, index.getTarget(), index.getLevelIndex()))
    {
        // TODO (anglebug.com/42265933): Can't populate from a buffer using emulated format
        UNIMPLEMENTED();
        return angle::Result::Stop;
    }

    return setImageImpl(context, index, formatInfo, size, GL_UNSIGNED_BYTE, unpack, unpackBuffer,
                        pixels);
}

angle::Result TextureVk::setCompressedSubImage(const gl::Context *context,
                                               const gl::ImageIndex &index,
                                               const gl::Box &area,
                                               GLenum format,
                                               const gl::PixelUnpackState &unpack,
                                               size_t imageSize,
                                               const uint8_t *pixels)
{

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format, GL_UNSIGNED_BYTE);
    ContextVk *contextVk                 = vk::GetImpl(context);
    const gl::ImageDesc &levelDesc       = mState.getImageDesc(index);
    const vk::Format &vkFormat =
        contextVk->getRenderer()->getFormat(levelDesc.format.info->sizedInternalFormat);
    const gl::State &glState = contextVk->getState();
    gl::Buffer *unpackBuffer = glState.getTargetBuffer(gl::BufferBinding::PixelUnpack);

    if (unpackBuffer &&
        this->isCompressedFormatEmulated(context, index.getTarget(), index.getLevelIndex()))
    {
        // TODO (anglebug.com/42265933): Can't populate from a buffer using emulated format
        UNIMPLEMENTED();
        return angle::Result::Stop;
    }

    return setSubImageImpl(context, index, area, formatInfo, GL_UNSIGNED_BYTE, unpack, unpackBuffer,
                           pixels, vkFormat);
}

angle::Result TextureVk::setImageImpl(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      const gl::InternalFormat &formatInfo,
                                      const gl::Extents &size,
                                      GLenum type,
                                      const gl::PixelUnpackState &unpack,
                                      gl::Buffer *unpackBuffer,
                                      const uint8_t *pixels)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    const vk::Format &vkFormat = renderer->getFormat(formatInfo.sizedInternalFormat);

    ANGLE_TRY(redefineLevel(context, index, vkFormat, size));

    // Early-out on empty textures, don't create a zero-sized storage.
    if (size.empty())
    {
        return angle::Result::Continue;
    }

    return setSubImageImpl(context, index, gl::Box(gl::kOffsetZero, size), formatInfo, type, unpack,
                           unpackBuffer, pixels, vkFormat);
}

bool TextureVk::isFastUnpackPossible(const vk::Format &vkFormat,
                                     size_t offset,
                                     const vk::Format &bufferVkFormat) const
{
    // Conditions to determine if fast unpacking is possible
    // 1. Image must be well defined to unpack directly to it
    //    TODO(http://anglebug.com/42262852) Create and stage a temp image instead
    // 2. Can't perform a fast copy for depth/stencil, except from non-emulated depth or stencil
    //    to emulated depth/stencil.  GL requires depth and stencil data to be packed, while Vulkan
    //    requires them to be separate.
    // 3. Can't perform a fast copy for emulated formats, except from non-emulated depth or stencil
    //    to emulated depth/stencil.
    // 4. vkCmdCopyBufferToImage requires byte offset to be a multiple of 4.
    // 5. Actual texture format and intended buffer format must match for color formats
    const angle::Format &bufferFormat = vkFormat.getActualBufferFormat(false);
    const bool isCombinedDepthStencil = bufferFormat.hasDepthAndStencilBits();
    const bool isDepthXorStencil = bufferFormat.hasDepthOrStencilBits() && !isCombinedDepthStencil;
    const bool isCompatibleDepth = vkFormat.getIntendedFormat().depthBits == bufferFormat.depthBits;
    const VkDeviceSize imageCopyAlignment =
        vk::GetImageCopyBufferAlignment(mImage->getActualFormatID());
    const bool formatsMatch = bufferFormat.hasDepthOrStencilBits() ||
                              (vkFormat.getActualImageFormatID(getRequiredImageAccess()) ==
                               bufferVkFormat.getIntendedFormatID());

    return mImage->valid() && !isCombinedDepthStencil &&
           (vkFormat.getIntendedFormatID() ==
                vkFormat.getActualImageFormatID(getRequiredImageAccess()) ||
            (isDepthXorStencil && isCompatibleDepth)) &&
           (offset % imageCopyAlignment) == 0 && formatsMatch;
}

bool TextureVk::isMipImageDescDefined(gl::TextureTarget textureTarget, size_t level)
{
    // A defined image should have defined width, height, and format.
    gl::ImageDesc imageDesc = mState.getImageDesc(textureTarget, level);
    return imageDesc.size.height != 0 && imageDesc.size.width != 0 &&
           imageDesc.format.info->format != GL_NONE;
}

bool TextureVk::isMutableTextureConsistentlySpecifiedForFlush()
{
    // Disable optimization if the base level is not 0.
    if (mState.getBaseLevel() != 0)
    {
        return false;
    }

    // If the texture is a cubemap, we will have to wait until it is complete.
    if (mState.getType() == gl::TextureType::CubeMap && !mState.isCubeComplete())
    {
        return false;
    }

    // Before we initialize the mips, we make sure that the base mip level is properly defined.
    gl::TextureTarget textureTarget = (mState.getType() == gl::TextureType::CubeMap)
                                          ? gl::kCubeMapTextureTargetMin
                                          : gl::TextureTypeToTarget(mState.getType(), 0);
    if (!isMipImageDescDefined(textureTarget, 0))
    {
        return false;
    }

    // We do not flush if the texture has been bound as an attachment.
    if (mState.hasBeenBoundAsAttachment())
    {
        return false;
    }

    // For performance, flushing is skipped if the number of staged updates in a mip level is not
    // one. For a cubemap, this applies to each face of the cube instead.
    size_t maxUpdatesPerMipLevel = (mState.getType() == gl::TextureType::CubeMap) ? 6 : 1;
    if (mImage->getLevelUpdateCount(gl::LevelIndex(0)) != maxUpdatesPerMipLevel)
    {
        return false;
    }

    // The mip levels that are already defined should have attributes compatible with those of the
    // base mip level. For each defined mip level, its size, format, number of samples, and depth
    // are checked before flushing the texture updates. For complete cubemaps, there are 6 images
    // per mip level. Therefore, mState would have 6 times as many images.
    gl::ImageDesc baseImageDesc = mState.getImageDesc(textureTarget, 0);
    size_t maxImageMipLevels    = (mState.getType() == gl::TextureType::CubeMap)
                                      ? (mState.getImageDescs().size() / 6)
                                      : mState.getImageDescs().size();

    for (size_t image = 1; image < maxImageMipLevels; image++)
    {
        gl::ImageDesc mipImageDesc = mState.getImageDesc(textureTarget, image);
        if (!isMipImageDescDefined(textureTarget, image))
        {
            continue;
        }

        // If the texture is 2DArray or 3D, the depths should also be checked according to the mip
        // levels. If the texture type is a cube map array, the depth represents the number of
        // layer-faces and does not change for mipmaps. Otherwise, we skip the depth comparison.
        gl::Extents baseImageDescMipSize;
        baseImageDescMipSize.width  = std::max(baseImageDesc.size.width >> image, 1);
        baseImageDescMipSize.height = std::max(baseImageDesc.size.height >> image, 1);
        baseImageDescMipSize.depth  = std::max(baseImageDesc.size.depth >> image, 1);

        bool isDepthCompatible = (mState.getType() == gl::TextureType::_3D ||
                                  mState.getType() == gl::TextureType::_2DArray)
                                     ? (baseImageDescMipSize.depth == mipImageDesc.size.depth)
                                     : (mState.getType() != gl::TextureType::CubeMapArray ||
                                        baseImageDesc.size.depth == mipImageDesc.size.depth);

        bool isSizeCompatible = (baseImageDescMipSize.width == mipImageDesc.size.width) &&
                                (baseImageDescMipSize.height == mipImageDesc.size.height) &&
                                isDepthCompatible;
        bool isFormatCompatible          = (baseImageDesc.format.info->sizedInternalFormat ==
                                   mipImageDesc.format.info->sizedInternalFormat);
        bool isNumberOfSamplesCompatible = (baseImageDesc.samples == mipImageDesc.samples);

        bool isUpdateCompatible =
            (mImage->getLevelUpdateCount(gl::LevelIndex(static_cast<GLint>(image))) ==
             maxUpdatesPerMipLevel);

        if (!isSizeCompatible || !isFormatCompatible || !isNumberOfSamplesCompatible ||
            !isUpdateCompatible)
        {
            return false;
        }
    }

    return true;
}

bool TextureVk::updateMustBeFlushed(gl::LevelIndex textureLevelIndexGL,
                                    angle::FormatID dstImageFormatID) const
{
    ASSERT(mImage);

    // For EGLImages we should never stage the update since staged update is subject to thread
    // racing bugs when two textures in different share groups are accessed at same time.
    if (!mOwnsImage)
    {
        // EGLImage is always initialized upon creation and format should always renderable so that
        // there is no format upgrade.
        ASSERT(mImage->valid());
        ASSERT(IsTextureLevelInAllocatedImage(*mImage, textureLevelIndexGL));
        ASSERT(!IsTextureLevelRedefined(mRedefinedLevels, mState.getType(), textureLevelIndexGL));
        return true;
    }
    return false;
}

bool TextureVk::updateMustBeStaged(gl::LevelIndex textureLevelIndexGL,
                                   angle::FormatID dstImageFormatID) const
{
    ASSERT(mImage);

    // If we do not have storage yet, there is impossible to immediately do the copy, so just
    // stage it. Note that immutable texture will have a valid storage.
    if (!mImage->valid())
    {
        return true;
    }

    // If update is outside the range of image levels, it must be staged.
    if (!IsTextureLevelInAllocatedImage(*mImage, textureLevelIndexGL))
    {
        return true;
    }

    // During the process of format change, mImage's format may become stale. In that case, we
    // must always stage the update and let caller properly release mImage and initExternal and
    // flush the update.
    if (mImage->getActualFormatID() != dstImageFormatID)
    {
        return true;
    }

    // Otherwise, it can only be directly applied to the image if the level is not previously
    // incompatibly redefined.
    return IsTextureLevelRedefined(mRedefinedLevels, mState.getType(), textureLevelIndexGL);
}

angle::Result TextureVk::clearImage(const gl::Context *context,
                                    GLint level,
                                    GLenum format,
                                    GLenum type,
                                    const uint8_t *data)
{
    // All defined cubemap faces are expected to have equal width and height.
    bool isCubeMap = mState.getType() == gl::TextureType::CubeMap;
    gl::TextureTarget textureTarget =
        isCubeMap ? gl::kCubeMapTextureTargetMin : gl::TextureTypeToTarget(mState.getType(), 0);
    gl::Extents extents = mState.getImageDesc(textureTarget, level).size;

    gl::Box area = gl::Box(gl::kOffsetZero, extents);
    if (isCubeMap)
    {
        // For a cubemap, the depth offset moves between cube faces.
        ASSERT(area.depth == 1);
        area.depth = 6;
    }

    return clearSubImageImpl(context, level, area, vk::ClearTextureMode::FullClear, format, type,
                             data);
}

angle::Result TextureVk::clearSubImage(const gl::Context *context,
                                       GLint level,
                                       const gl::Box &area,
                                       GLenum format,
                                       GLenum type,
                                       const uint8_t *data)
{
    bool isCubeMap = mState.getType() == gl::TextureType::CubeMap;
    gl::TextureTarget textureTarget =
        isCubeMap ? gl::kCubeMapTextureTargetMin : gl::TextureTypeToTarget(mState.getType(), 0);
    gl::Extents extents   = mState.getImageDesc(textureTarget, level).size;
    int depthForFullClear = isCubeMap ? 6 : extents.depth;

    vk::ClearTextureMode clearMode = vk::ClearTextureMode::PartialClear;
    if (extents.width == area.width && extents.height == area.height &&
        depthForFullClear == area.depth)
    {
        clearMode = vk::ClearTextureMode::FullClear;
    }

    return clearSubImageImpl(context, level, area, clearMode, format, type, data);
}

angle::Result TextureVk::clearSubImageImpl(const gl::Context *context,
                                           GLint level,
                                           const gl::Box &clearArea,
                                           vk::ClearTextureMode clearMode,
                                           GLenum format,
                                           GLenum type,
                                           const uint8_t *data)
{
    // There should be no zero extents in the clear area, since such calls should return before
    // entering the backend with no changes to the texture. For 2D textures, depth should be 1.
    //
    // From the spec: For texture types that do not have certain dimensions, this command treats
    // those dimensions as having a size of 1.  For example, to clear a portion of a two-dimensional
    // texture, the application would use <zoffset> equal to zero and <depth> equal to one.
    ASSERT(clearArea.width != 0 && clearArea.height != 0 && clearArea.depth != 0);

    gl::TextureType textureType = mState.getType();
    bool useLayerAsDepth        = textureType == gl::TextureType::CubeMap ||
                           textureType == gl::TextureType::CubeMapArray ||
                           textureType == gl::TextureType::_2DArray ||
                           textureType == gl::TextureType::_2DMultisampleArray;

    // If the texture is renderable (including multisampled), the partial clear can be applied to
    // the image simply by opening/closing a render pass with LOAD_OP_CLEAR. Otherwise, a buffer can
    // be filled with the given pixel data on the host and staged to the image as a buffer update.
    ContextVk *contextVk = vk::GetImpl(context);

    const gl::InternalFormat &inputFormatInfo = gl::GetInternalFormatInfo(format, type);
    const vk::Format &inputVkFormat =
        contextVk->getRenderer()->getFormat(inputFormatInfo.sizedInternalFormat);

    const gl::InternalFormat &outputFormatInfo =
        gl::GetSizedInternalFormatInfo(mImage->getIntendedFormat().glInternalFormat);
    const vk::Format &outputVkFormat =
        contextVk->getRenderer()->getFormat(outputFormatInfo.sizedInternalFormat);
    const angle::FormatID &outputActualFormatID = mImage->getActualFormatID();

    bool usesBufferForClear = false;

    VkFormatFeatureFlags renderableCheckFlag =
        clearMode == vk::ClearTextureMode::FullClear ? VK_FORMAT_FEATURE_TRANSFER_DST_BIT
        : outputFormatInfo.isDepthOrStencil() ? VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
                                              : VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    if (vk::FormatHasNecessaryFeature(contextVk->getRenderer(), outputActualFormatID,
                                      getTilingMode(), renderableCheckFlag))
    {
        uint32_t baseLayer  = useLayerAsDepth ? clearArea.z : 0;
        uint32_t layerCount = useLayerAsDepth ? clearArea.depth : 1;
        ANGLE_TRY(mImage->stagePartialClear(contextVk, clearArea, clearMode, mState.getType(),
                                            level, baseLayer, layerCount, type, inputFormatInfo,
                                            inputVkFormat, getRequiredImageAccess(), data));
    }
    else
    {
        ASSERT(mImage->getSamples() <= 1);
        bool updateAppliedImmediately = false;
        usesBufferForClear            = true;

        auto pixelSize = static_cast<size_t>(inputFormatInfo.pixelBytes);
        std::vector<uint8_t> pixelValue(pixelSize, 0);
        if (data != nullptr)
        {
            memcpy(pixelValue.data(), data, pixelSize);
        }

        // For a cubemap, each face will be updated separately.
        bool isCubeMap = textureType == gl::TextureType::CubeMap;
        size_t clearBufferSize =
            isCubeMap ? clearArea.width * clearArea.height * pixelSize
                      : clearArea.width * clearArea.height * clearArea.depth * pixelSize;

        std::vector<uint8_t> clearBuffer(clearBufferSize, 0);
        ASSERT(clearBufferSize % pixelSize == 0);

        // The pixels in the temporary buffer are tightly packed.
        if (data != nullptr)
        {
            for (GLuint i = 0; i < clearBufferSize; i += pixelSize)
            {
                memcpy(&clearBuffer[i], pixelValue.data(), pixelSize);
            }
        }
        gl::PixelUnpackState pixelUnpackState = {};
        pixelUnpackState.alignment            = 1;

        if (isCubeMap)
        {
            size_t cubeFaceStart = clearArea.z;
            auto cubeFaceEnd     = static_cast<size_t>(clearArea.z + clearArea.depth);

            for (size_t cubeFace = cubeFaceStart; cubeFace < cubeFaceEnd; cubeFace++)
            {
                const gl::ImageIndex index = gl::ImageIndex::MakeFromTarget(
                    gl::CubeFaceIndexToTextureTarget(cubeFace), level, 0);

                ANGLE_TRY(mImage->stageSubresourceUpdate(
                    contextVk, getNativeImageIndex(index),
                    gl::Extents(clearArea.width, clearArea.height, 1),
                    gl::Offset(clearArea.x, clearArea.y, 0), inputFormatInfo, pixelUnpackState,
                    type, clearBuffer.data(), outputVkFormat, getRequiredImageAccess(),
                    vk::ApplyImageUpdate::Defer, &updateAppliedImmediately));
                ASSERT(!updateAppliedImmediately);
            }
        }
        else
        {
            gl::TextureTarget textureTarget = gl::TextureTypeToTarget(textureType, 0);
            uint32_t layerCount             = useLayerAsDepth ? clearArea.depth : 0;
            const gl::ImageIndex index =
                gl::ImageIndex::MakeFromTarget(textureTarget, level, layerCount);

            ANGLE_TRY(mImage->stageSubresourceUpdate(
                contextVk, getNativeImageIndex(index),
                gl::Extents(clearArea.width, clearArea.height, clearArea.depth),
                gl::Offset(clearArea.x, clearArea.y, clearArea.z), inputFormatInfo,
                pixelUnpackState, type, clearBuffer.data(), outputVkFormat,
                getRequiredImageAccess(), vk::ApplyImageUpdate::Defer, &updateAppliedImmediately));
            ASSERT(!updateAppliedImmediately);
        }
    }

    // Flush the staged updates if needed.
    ANGLE_TRY(ensureImageInitializedIfUpdatesNeedStageOrFlush(
        contextVk, gl::LevelIndex(level), outputVkFormat, vk::ApplyImageUpdate::Defer,
        usesBufferForClear));
    return angle::Result::Continue;
}

angle::Result TextureVk::ensureImageInitializedIfUpdatesNeedStageOrFlush(
    ContextVk *contextVk,
    gl::LevelIndex level,
    const vk::Format &vkFormat,
    vk::ApplyImageUpdate applyUpdate,
    bool usesBufferForUpdate)
{
    bool mustFlush =
        updateMustBeFlushed(level, vkFormat.getActualImageFormatID(getRequiredImageAccess()));
    bool mustStage = applyUpdate == vk::ApplyImageUpdate::Defer;

    // If texture has all levels being specified, then do the flush immediately. This tries to avoid
    // issue flush as each level is being provided which may end up flushing out the staged clear
    // that otherwise might able to be removed. It also helps tracking all updates with just one
    // VkEvent instead of one for each level.
    if (mustFlush ||
        (!mustStage && mImage->valid() && mImage->hasBufferSourcedStagedUpdatesInAllLevels()))
    {
        ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

        // If forceSubmitImmutableTextureUpdates is enabled, submit the staged updates as well.
        if (contextVk->getFeatures().forceSubmitImmutableTextureUpdates.enabled)
        {
            ANGLE_TRY(contextVk->submitStagedTextureUpdates());
        }
    }
    else if (usesBufferForUpdate && contextVk->isEligibleForMutableTextureFlush() &&
             !mState.getImmutableFormat())
    {
        // Check if we should flush any mutable textures from before.
        ANGLE_TRY(contextVk->getShareGroup()->onMutableTextureUpload(contextVk, this));
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::setSubImageImpl(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         const gl::Box &area,
                                         const gl::InternalFormat &formatInfo,
                                         GLenum type,
                                         const gl::PixelUnpackState &unpack,
                                         gl::Buffer *unpackBuffer,
                                         const uint8_t *pixels,
                                         const vk::Format &vkFormat)
{
    ContextVk *contextVk = vk::GetImpl(context);

    bool mustStage = updateMustBeStaged(gl::LevelIndex(index.getLevelIndex()),
                                        vkFormat.getActualImageFormatID(getRequiredImageAccess()));

    vk::ApplyImageUpdate applyUpdate;
    if (mustStage)
    {
        applyUpdate = vk::ApplyImageUpdate::Defer;
    }
    else
    {
        // Cannot defer to unlocked tail call if:
        //
        // - The generate mipmap hint is set: This is because on return the Texture class would
        //   attempt to generate mipmaps, which may reallocate the image, or fall back to software
        //   mipmap generation.
        // - The texture is incomplete: This is because unlocked tail call is disabled on draw
        //   calls, but that is when incomplete textures are created and initialized.
        const bool canDeferToUnlockedTailCall =
            mState.getGenerateMipmapHint() != GL_TRUE && !mState.isInternalIncompleteTexture();

        // When possible flush out updates immediately.
        applyUpdate = canDeferToUnlockedTailCall
                          ? vk::ApplyImageUpdate::ImmediatelyInUnlockedTailCall
                          : vk::ApplyImageUpdate::Immediately;
    }
    bool updateAppliedImmediately = false;

    if (unpackBuffer)
    {
        BufferVk *unpackBufferVk       = vk::GetImpl(unpackBuffer);
        vk::BufferHelper &bufferHelper = unpackBufferVk->getBuffer();
        VkDeviceSize bufferOffset      = bufferHelper.getOffset();
        uintptr_t offset               = reinterpret_cast<uintptr_t>(pixels);
        GLuint inputRowPitch           = 0;
        GLuint inputDepthPitch         = 0;
        GLuint inputSkipBytes          = 0;

        ANGLE_TRY(mImage->calculateBufferInfo(
            contextVk, gl::Extents(area.width, area.height, area.depth), formatInfo, unpack, type,
            index.usesTex3D(), &inputRowPitch, &inputDepthPitch, &inputSkipBytes));

        size_t offsetBytes = static_cast<size_t>(bufferOffset + offset + inputSkipBytes);

        // Note: cannot directly copy from a depth/stencil PBO.  GL requires depth and stencil data
        // to be packed, while Vulkan requires them to be separate.
        const VkImageAspectFlags aspectFlags =
            vk::GetFormatAspectFlags(vkFormat.getIntendedFormat());
        const vk::Format &bufferVkFormat =
            contextVk->getRenderer()->getFormat(formatInfo.sizedInternalFormat);

        if (shouldUpdateBeFlushed(gl::LevelIndex(index.getLevelIndex()),
                                  vkFormat.getActualImageFormatID(getRequiredImageAccess())) &&
            isFastUnpackPossible(vkFormat, offsetBytes, bufferVkFormat))
        {
            GLuint pixelSize   = formatInfo.pixelBytes;
            GLuint blockWidth  = formatInfo.compressedBlockWidth;
            GLuint blockHeight = formatInfo.compressedBlockHeight;
            if (!formatInfo.compressed)
            {
                pixelSize   = formatInfo.computePixelBytes(type);
                blockWidth  = 1;
                blockHeight = 1;
            }
            ASSERT(pixelSize != 0 && inputRowPitch != 0 && blockWidth != 0 && blockHeight != 0);

            GLuint rowLengthPixels   = inputRowPitch / pixelSize * blockWidth;
            GLuint imageHeightPixels = inputDepthPitch / inputRowPitch * blockHeight;

            ANGLE_TRY(copyBufferDataToImage(contextVk, &bufferHelper, index, rowLengthPixels,
                                            imageHeightPixels, area, offsetBytes, aspectFlags));
        }
        else
        {
            ANGLE_VK_PERF_WARNING(
                contextVk, GL_DEBUG_SEVERITY_HIGH,
                "TexSubImage with unpack buffer copied on CPU due to store, format "
                "or offset restrictions");

            void *mapPtr = nullptr;

            ANGLE_TRY(unpackBufferVk->mapImpl(contextVk, GL_MAP_READ_BIT, &mapPtr));

            const uint8_t *source =
                static_cast<const uint8_t *>(mapPtr) + reinterpret_cast<ptrdiff_t>(pixels);

            ANGLE_TRY(mImage->stageSubresourceUpdateImpl(
                contextVk, getNativeImageIndex(index),
                gl::Extents(area.width, area.height, area.depth),
                gl::Offset(area.x, area.y, area.z), formatInfo, unpack, type, source, vkFormat,
                getRequiredImageAccess(), inputRowPitch, inputDepthPitch, inputSkipBytes,
                applyUpdate, &updateAppliedImmediately));

            ANGLE_TRY(unpackBufferVk->unmapImpl(contextVk));
        }
    }
    else if (pixels)
    {
        ANGLE_TRY(mImage->stageSubresourceUpdate(
            contextVk, getNativeImageIndex(index), gl::Extents(area.width, area.height, area.depth),
            gl::Offset(area.x, area.y, area.z), formatInfo, unpack, type, pixels, vkFormat,
            getRequiredImageAccess(), applyUpdate, &updateAppliedImmediately));
    }

    if (updateAppliedImmediately)
    {
        // Return if stageSubresourceUpdate already applied the update
        return angle::Result::Continue;
    }

    // Flush the staged updates if needed.
    ANGLE_TRY(ensureImageInitializedIfUpdatesNeedStageOrFlush(
        contextVk, gl::LevelIndex(index.getLevelIndex()), vkFormat, applyUpdate, true));
    return angle::Result::Continue;
}

angle::Result TextureVk::copyImage(const gl::Context *context,
                                   const gl::ImageIndex &index,
                                   const gl::Rectangle &sourceArea,
                                   GLenum internalFormat,
                                   gl::Framebuffer *source)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    gl::Extents newImageSize(sourceArea.width, sourceArea.height, 1);
    const gl::InternalFormat &internalFormatInfo =
        gl::GetInternalFormatInfo(internalFormat, GL_UNSIGNED_BYTE);
    const vk::Format &vkFormat = renderer->getFormat(internalFormatInfo.sizedInternalFormat);

    // Fall back to renderable format if copy cannot be done in transfer.  Must be done before
    // the dst format is accessed anywhere (in |redefineLevel| and |copySubImageImpl|).
    ANGLE_TRY(ensureRenderableIfCopyTexImageCannotTransfer(contextVk, internalFormatInfo, source));

    // The texture level being redefined might be the same as the one bound to the framebuffer.
    // This _could_ be supported by using a temp image before redefining the level (and potentially
    // discarding the image).  However, this is currently unimplemented.
    FramebufferVk *framebufferVk = vk::GetImpl(source);
    RenderTargetVk *colorReadRT  = framebufferVk->getColorReadRenderTarget();
    vk::ImageHelper *srcImage    = &colorReadRT->getImageForCopy();
    const bool isCubeMap         = index.getType() == gl::TextureType::CubeMap;
    gl::LevelIndex levelIndex(getNativeImageIndex(index).getLevelIndex());
    const uint32_t layerIndex    = index.hasLayer() ? index.getLayerIndex() : 0;
    const uint32_t redefinedFace = isCubeMap ? layerIndex : 0;
    const uint32_t sourceFace    = isCubeMap ? colorReadRT->getLayerIndex() : 0;
    const bool isSelfCopy = mImage == srcImage && levelIndex == colorReadRT->getLevelIndex() &&
                            redefinedFace == sourceFace;

    ANGLE_TRY(redefineLevel(context, index, vkFormat, newImageSize));

    if (isSelfCopy)
    {
        UNIMPLEMENTED();
        return angle::Result::Continue;
    }

    return copySubImageImpl(context, index, gl::Offset(0, 0, 0), sourceArea, internalFormatInfo,
                            source);
}

angle::Result TextureVk::copySubImage(const gl::Context *context,
                                      const gl::ImageIndex &index,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      gl::Framebuffer *source)
{
    const gl::InternalFormat &currentFormat = *mState.getImageDesc(index).format.info;

    // Fall back to renderable format if copy cannot be done in transfer.  Must be done before
    // the dst format is accessed anywhere (in |redefineLevel| and |copySubImageImpl|).
    ANGLE_TRY(
        ensureRenderableIfCopyTexImageCannotTransfer(vk::GetImpl(context), currentFormat, source));

    return copySubImageImpl(context, index, destOffset, sourceArea, currentFormat, source);
}

angle::Result TextureVk::copyTexture(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     GLenum internalFormat,
                                     GLenum type,
                                     GLint sourceLevelGL,
                                     bool unpackFlipY,
                                     bool unpackPremultiplyAlpha,
                                     bool unpackUnmultiplyAlpha,
                                     const gl::Texture *source)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    TextureVk *sourceVk = vk::GetImpl(source);
    const gl::ImageDesc &srcImageDesc =
        sourceVk->mState.getImageDesc(NonCubeTextureTypeToTarget(source->getType()), sourceLevelGL);
    gl::Box sourceBox(gl::kOffsetZero, srcImageDesc.size);
    const gl::InternalFormat &dstFormatInfo = gl::GetInternalFormatInfo(internalFormat, type);
    const vk::Format &dstVkFormat = renderer->getFormat(dstFormatInfo.sizedInternalFormat);

    ANGLE_TRY(sourceVk->ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    // Fall back to renderable format if copy cannot be done in transfer.  Must be done before
    // the dst format is accessed anywhere (in |redefineLevel| and |copySubTextureImpl|).
    ANGLE_TRY(ensureRenderableIfCopyTextureCannotTransfer(contextVk, dstFormatInfo, unpackFlipY,
                                                          unpackPremultiplyAlpha,
                                                          unpackUnmultiplyAlpha, sourceVk));

    ANGLE_TRY(redefineLevel(context, index, dstVkFormat, srcImageDesc.size));

    return copySubTextureImpl(contextVk, index, gl::kOffsetZero, dstFormatInfo,
                              gl::LevelIndex(sourceLevelGL), sourceBox, unpackFlipY,
                              unpackPremultiplyAlpha, unpackUnmultiplyAlpha, sourceVk);
}

angle::Result TextureVk::copySubTexture(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Offset &dstOffset,
                                        GLint srcLevelGL,
                                        const gl::Box &sourceBox,
                                        bool unpackFlipY,
                                        bool unpackPremultiplyAlpha,
                                        bool unpackUnmultiplyAlpha,
                                        const gl::Texture *source)
{
    ContextVk *contextVk = vk::GetImpl(context);

    gl::TextureTarget target = index.getTarget();
    gl::LevelIndex dstLevelGL(index.getLevelIndex());
    const gl::InternalFormat &dstFormatInfo =
        *mState.getImageDesc(target, dstLevelGL.get()).format.info;

    ANGLE_TRY(
        vk::GetImpl(source)->ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    // Fall back to renderable format if copy cannot be done in transfer.  Must be done before
    // the dst format is accessed anywhere (in |copySubTextureImpl|).
    ANGLE_TRY(ensureRenderableIfCopyTextureCannotTransfer(
        contextVk, dstFormatInfo, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha,
        vk::GetImpl(source)));

    return copySubTextureImpl(contextVk, index, dstOffset, dstFormatInfo,
                              gl::LevelIndex(srcLevelGL), sourceBox, unpackFlipY,
                              unpackPremultiplyAlpha, unpackUnmultiplyAlpha, vk::GetImpl(source));
}

angle::Result TextureVk::copyRenderbufferSubData(const gl::Context *context,
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
                                                 GLsizei srcDepth)
{
    ContextVk *contextVk     = vk::GetImpl(context);
    RenderbufferVk *sourceVk = vk::GetImpl(srcBuffer);

    // Make sure the source/destination targets are initialized and all staged updates are flushed.
    ANGLE_TRY(sourceVk->ensureImageInitialized(context));
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    return vk::ImageHelper::CopyImageSubData(context, sourceVk->getImage(), srcLevel, srcX, srcY,
                                             srcZ, mImage, dstLevel, dstX, dstY, dstZ, srcWidth,
                                             srcHeight, srcDepth);
}

angle::Result TextureVk::copyTextureSubData(const gl::Context *context,
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
                                            GLsizei srcDepth)
{
    ContextVk *contextVk = vk::GetImpl(context);
    TextureVk *sourceVk  = vk::GetImpl(srcTexture);

    // Make sure the source/destination targets are initialized and all staged updates are flushed.
    ANGLE_TRY(sourceVk->ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    return vk::ImageHelper::CopyImageSubData(context, &sourceVk->getImage(), srcLevel, srcX, srcY,
                                             srcZ, mImage, dstLevel, dstX, dstY, dstZ, srcWidth,
                                             srcHeight, srcDepth);
}

angle::Result TextureVk::copyCompressedTexture(const gl::Context *context,
                                               const gl::Texture *source)
{
    ContextVk *contextVk = vk::GetImpl(context);
    TextureVk *sourceVk  = vk::GetImpl(source);

    gl::TextureTarget sourceTarget = NonCubeTextureTypeToTarget(source->getType());
    constexpr GLint sourceLevelGL  = 0;
    constexpr GLint destLevelGL    = 0;

    const gl::InternalFormat &internalFormat = *source->getFormat(sourceTarget, sourceLevelGL).info;
    const vk::Format &vkFormat =
        contextVk->getRenderer()->getFormat(internalFormat.sizedInternalFormat);
    const gl::Extents size(static_cast<int>(source->getWidth(sourceTarget, sourceLevelGL)),
                           static_cast<int>(source->getHeight(sourceTarget, sourceLevelGL)),
                           static_cast<int>(source->getDepth(sourceTarget, sourceLevelGL)));
    const gl::ImageIndex destIndex = gl::ImageIndex::MakeFromTarget(sourceTarget, destLevelGL, 1);

    ANGLE_TRY(redefineLevel(context, destIndex, vkFormat, size));

    ANGLE_TRY(sourceVk->ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    return copySubImageImplWithTransfer(contextVk, destIndex, gl::kOffsetZero, vkFormat,
                                        gl::LevelIndex(sourceLevelGL), 0,
                                        gl::Box(gl::kOffsetZero, size), &sourceVk->getImage());
}

angle::Result TextureVk::copySubImageImpl(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &destOffset,
                                          const gl::Rectangle &sourceArea,
                                          const gl::InternalFormat &internalFormat,
                                          gl::Framebuffer *source)
{
    gl::Extents fbSize = source->getReadColorAttachment()->getSize();
    gl::Rectangle clippedSourceArea;
    if (!ClipRectangle(sourceArea, gl::Rectangle(0, 0, fbSize.width, fbSize.height),
                       &clippedSourceArea))
    {
        return angle::Result::Continue;
    }

    ContextVk *contextVk         = vk::GetImpl(context);
    vk::Renderer *renderer       = contextVk->getRenderer();
    FramebufferVk *framebufferVk = vk::GetImpl(source);

    const gl::ImageIndex offsetImageIndex = getNativeImageIndex(index);

    // If negative offsets are given, clippedSourceArea ensures we don't read from those offsets.
    // However, that changes the sourceOffset->destOffset mapping.  Here, destOffset is shifted by
    // the same amount as clipped to correct the error.
    VkImageType imageType = gl_vk::GetImageType(mState.getType());
    int zOffset           = (imageType == VK_IMAGE_TYPE_3D) ? destOffset.z : 0;
    const gl::Offset modifiedDestOffset(destOffset.x + clippedSourceArea.x - sourceArea.x,
                                        destOffset.y + clippedSourceArea.y - sourceArea.y, zOffset);

    RenderTargetVk *colorReadRT = framebufferVk->getColorReadRenderTarget();

    angle::FormatID srcIntendedFormatID = colorReadRT->getImageIntendedFormatID();
    angle::FormatID srcActualFormatID   = colorReadRT->getImageActualFormatID();
    VkImageTiling srcTilingMode         = colorReadRT->getImageForCopy().getTilingMode();
    const vk::Format &dstFormat         = renderer->getFormat(internalFormat.sizedInternalFormat);
    angle::FormatID dstIntendedFormatID = dstFormat.getIntendedFormatID();
    angle::FormatID dstActualFormatID = dstFormat.getActualImageFormatID(getRequiredImageAccess());
    VkImageTiling destTilingMode      = getTilingMode();

    bool isViewportFlipY = contextVk->isViewportFlipEnabledForReadFBO();

    gl::Box clippedSourceBox(clippedSourceArea.x, clippedSourceArea.y, colorReadRT->getLayerIndex(),
                             clippedSourceArea.width, clippedSourceArea.height, 1);

    // If it's possible to perform the copy with a transfer, that's the best option.
    if (CanCopyWithTransferForTexImage(renderer, srcIntendedFormatID, srcActualFormatID,
                                       srcTilingMode, dstIntendedFormatID, dstActualFormatID,
                                       destTilingMode, isViewportFlipY))
    {
        return copySubImageImplWithTransfer(contextVk, offsetImageIndex, modifiedDestOffset,
                                            dstFormat, colorReadRT->getLevelIndex(),
                                            colorReadRT->getLayerIndex(), clippedSourceBox,
                                            &colorReadRT->getImageForCopy());
    }

    // If it's possible to perform the copy with a draw call, do that.
    if (CanCopyWithDraw(renderer, srcActualFormatID, srcTilingMode, dstActualFormatID,
                        destTilingMode))
    {
        // Layer count can only be 1 as the source is a framebuffer.
        ASSERT(offsetImageIndex.getLayerCount() == 1);

        // Flush the render pass, which may incur a vkQueueSubmit, before taking any views.
        // Otherwise the view serials would not reflect the render pass they are really used in.
        // http://crbug.com/1272266#c22
        ANGLE_TRY(
            contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::PrepareForImageCopy));

        const vk::ImageView *copyImageView = nullptr;
        ANGLE_TRY(colorReadRT->getCopyImageView(contextVk, &copyImageView));

        return copySubImageImplWithDraw(contextVk, offsetImageIndex, modifiedDestOffset, dstFormat,
                                        colorReadRT->getLevelIndex(), clippedSourceBox,
                                        isViewportFlipY, false, false, false,
                                        &colorReadRT->getImageForCopy(), copyImageView,
                                        contextVk->getRotationReadFramebuffer());
    }

    ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH,
                          "Texture copied on CPU due to format restrictions");

    // Do a CPU readback that does the conversion, and then stage the change to the pixel buffer.
    ANGLE_TRY(mImage->stageSubresourceUpdateFromFramebuffer(
        context, offsetImageIndex, clippedSourceArea, modifiedDestOffset,
        gl::Extents(clippedSourceArea.width, clippedSourceArea.height, 1), internalFormat,
        getRequiredImageAccess(), framebufferVk));

    // Flush out staged update if possible
    if (shouldUpdateBeFlushed(gl::LevelIndex(index.getLevelIndex()), dstActualFormatID))
    {
        ANGLE_TRY(flushImageStagedUpdates(contextVk));
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::copySubTextureImpl(ContextVk *contextVk,
                                            const gl::ImageIndex &index,
                                            const gl::Offset &dstOffset,
                                            const gl::InternalFormat &dstFormat,
                                            gl::LevelIndex sourceLevelGL,
                                            const gl::Box &sourceBox,
                                            bool unpackFlipY,
                                            bool unpackPremultiplyAlpha,
                                            bool unpackUnmultiplyAlpha,
                                            TextureVk *source)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    const angle::Format &srcIntendedFormat = source->getImage().getIntendedFormat();
    angle::FormatID srcFormatID            = source->getImage().getActualFormatID();
    VkImageTiling srcTilingMode            = source->getImage().getTilingMode();
    const vk::Format &dstVkFormat          = renderer->getFormat(dstFormat.sizedInternalFormat);
    angle::FormatID dstFormatID = dstVkFormat.getActualImageFormatID(getRequiredImageAccess());
    VkImageTiling dstTilingMode = getTilingMode();

    const gl::ImageIndex offsetImageIndex = getNativeImageIndex(index);

    // If it's possible to perform the copy with a transfer, that's the best option.
    if (CanCopyWithTransferForCopyTexture(
            renderer, source->getImage(), srcTilingMode, dstVkFormat.getIntendedFormatID(),
            dstFormatID, dstTilingMode, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha))
    {
        return copySubImageImplWithTransfer(contextVk, offsetImageIndex, dstOffset, dstVkFormat,
                                            sourceLevelGL, sourceBox.z, sourceBox,
                                            &source->getImage());
    }

    // If it's possible to perform the copy with a draw call, do that.
    if (CanCopyWithDraw(renderer, srcFormatID, srcTilingMode, dstFormatID, dstTilingMode))
    {
        // Flush the render pass, which may incur a vkQueueSubmit, before taking any views.
        // Otherwise the view serials would not reflect the render pass they are really used in.
        // http://crbug.com/1272266#c22
        ANGLE_TRY(
            contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::PrepareForImageCopy));

        return copySubImageImplWithDraw(
            contextVk, offsetImageIndex, dstOffset, dstVkFormat, sourceLevelGL, sourceBox, false,
            unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha, &source->getImage(),
            &source->getCopyImageView(), SurfaceRotation::Identity);
    }

    ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH,
                          "Texture copied on CPU due to format restrictions");

    // Read back the requested region of the source texture
    vk::RendererScoped<vk::BufferHelper> bufferHelper(renderer);
    uint8_t *sourceData = nullptr;
    ANGLE_TRY(source->copyImageDataToBufferAndGetData(
        contextVk, sourceLevelGL, sourceBox.depth, sourceBox,
        RenderPassClosureReason::CopyTextureOnCPU, &bufferHelper.get(), &sourceData));

    const angle::Format &srcTextureFormat = source->getImage().getActualFormat();
    const angle::Format &dstTextureFormat =
        dstVkFormat.getActualImageFormat(getRequiredImageAccess());
    size_t destinationAllocationSize =
        sourceBox.width * sourceBox.height * sourceBox.depth * dstTextureFormat.pixelBytes;

    // Allocate memory in the destination texture for the copy/conversion
    uint32_t stagingBaseLayer =
        offsetImageIndex.hasLayer() ? offsetImageIndex.getLayerIndex() : dstOffset.z;
    uint32_t stagingLayerCount = sourceBox.depth;
    gl::Offset stagingOffset   = dstOffset;
    gl::Extents stagingExtents(sourceBox.width, sourceBox.height, sourceBox.depth);
    bool is3D = gl_vk::GetImageType(mState.getType()) == VK_IMAGE_TYPE_3D;

    if (is3D)
    {
        stagingBaseLayer  = 0;
        stagingLayerCount = 1;
    }
    else
    {
        stagingOffset.z      = 0;
        stagingExtents.depth = 1;
    }

    const gl::ImageIndex stagingIndex = gl::ImageIndex::Make2DArrayRange(
        offsetImageIndex.getLevelIndex(), stagingBaseLayer, stagingLayerCount);

    uint8_t *destData = nullptr;
    ANGLE_TRY(mImage->stageSubresourceUpdateAndGetData(contextVk, destinationAllocationSize,
                                                       stagingIndex, stagingExtents, stagingOffset,
                                                       &destData, dstFormatID));

    // Source and dst data is tightly packed
    GLuint srcDataRowPitch = sourceBox.width * srcTextureFormat.pixelBytes;
    GLuint dstDataRowPitch = sourceBox.width * dstTextureFormat.pixelBytes;

    GLuint srcDataDepthPitch = srcDataRowPitch * sourceBox.height;
    GLuint dstDataDepthPitch = dstDataRowPitch * sourceBox.height;

    rx::PixelReadFunction pixelReadFunction   = srcTextureFormat.pixelReadFunction;
    rx::PixelWriteFunction pixelWriteFunction = dstTextureFormat.pixelWriteFunction;

    // Fix up the read/write functions for the sake of luminance/alpha that are emulated with
    // formats whose channels don't correspond to the original format (alpha is emulated with red,
    // and luminance/alpha is emulated with red/green).
    if (srcIntendedFormat.isLUMA())
    {
        pixelReadFunction = srcIntendedFormat.pixelReadFunction;
    }
    if (dstVkFormat.getIntendedFormat().isLUMA())
    {
        pixelWriteFunction = dstVkFormat.getIntendedFormat().pixelWriteFunction;
    }

    CopyImageCHROMIUM(sourceData, srcDataRowPitch, srcTextureFormat.pixelBytes, srcDataDepthPitch,
                      pixelReadFunction, destData, dstDataRowPitch, dstTextureFormat.pixelBytes,
                      dstDataDepthPitch, pixelWriteFunction, dstFormat.format,
                      dstFormat.componentType, sourceBox.width, sourceBox.height, sourceBox.depth,
                      unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha);

    if (shouldUpdateBeFlushed(gl::LevelIndex(index.getLevelIndex()), dstFormatID))
    {
        ANGLE_TRY(flushImageStagedUpdates(contextVk));
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::copySubImageImplWithTransfer(ContextVk *contextVk,
                                                      const gl::ImageIndex &index,
                                                      const gl::Offset &dstOffset,
                                                      const vk::Format &dstFormat,
                                                      gl::LevelIndex sourceLevelGL,
                                                      size_t sourceLayer,
                                                      const gl::Box &sourceBox,
                                                      vk::ImageHelper *srcImage)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    gl::LevelIndex level(index.getLevelIndex());
    uint32_t baseLayer  = index.hasLayer() ? index.getLayerIndex() : dstOffset.z;
    uint32_t layerCount = sourceBox.depth;

    gl::Offset srcOffset = {sourceBox.x, sourceBox.y, sourceBox.z};
    gl::Extents extents  = {sourceBox.width, sourceBox.height, sourceBox.depth};

    // Change source layout if necessary
    vk::CommandBufferAccess access;
    access.onImageTransferRead(VK_IMAGE_ASPECT_COLOR_BIT, srcImage);

    VkImageSubresourceLayers srcSubresource = {};
    srcSubresource.aspectMask               = VK_IMAGE_ASPECT_COLOR_BIT;
    srcSubresource.mipLevel                 = srcImage->toVkLevel(sourceLevelGL).get();
    srcSubresource.baseArrayLayer           = static_cast<uint32_t>(sourceLayer);
    srcSubresource.layerCount               = layerCount;

    bool isSrc3D  = srcImage->getExtents().depth > 1;
    bool isDest3D = gl_vk::GetImageType(mState.getType()) == VK_IMAGE_TYPE_3D;

    if (isSrc3D)
    {
        Set3DBaseArrayLayerAndLayerCount(&srcSubresource);
    }
    else
    {
        ASSERT(srcSubresource.baseArrayLayer == static_cast<uint32_t>(srcOffset.z));
        srcOffset.z = 0;
    }

    gl::Offset dstOffsetModified = dstOffset;
    if (!isDest3D)
    {
        // If destination is not 3D, destination offset must be 0.
        dstOffsetModified.z = 0;
    }

    // Perform self-copies through a staging buffer.
    // TODO: optimize to copy directly if possible.  http://anglebug.com/42263319
    bool isSelfCopy = mImage == srcImage;

    // If destination is valid, copy the source directly into it.
    if (shouldUpdateBeFlushed(level, dstFormat.getActualImageFormatID(getRequiredImageAccess())) &&
        !isSelfCopy)
    {
        // Make sure any updates to the image are already flushed.
        ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

        access.onImageTransferWrite(level, 1, baseLayer, layerCount, VK_IMAGE_ASPECT_COLOR_BIT,
                                    mImage);

        vk::OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        VkImageSubresourceLayers destSubresource = srcSubresource;
        destSubresource.mipLevel                 = mImage->toVkLevel(level).get();
        destSubresource.baseArrayLayer           = baseLayer;
        destSubresource.layerCount               = layerCount;

        if (isDest3D)
        {
            Set3DBaseArrayLayerAndLayerCount(&destSubresource);
        }
        else if (!isSrc3D)
        {
            // extents.depth should be set to layer count if any of the source or destination is a
            // 2D Array.  If both are 2D Array, it should be set to 1.
            extents.depth = 1;
        }

        vk::ImageHelper::Copy(renderer, srcImage, mImage, srcOffset, dstOffsetModified, extents,
                              srcSubresource, destSubresource, commandBuffer);
    }
    else
    {
        // Create a temporary image to stage the copy
        std::unique_ptr<vk::RefCounted<vk::ImageHelper>> stagingImage;
        stagingImage = std::make_unique<vk::RefCounted<vk::ImageHelper>>();

        ANGLE_TRY(stagingImage->get().init2DStaging(
            contextVk, mState.hasProtectedContent(), renderer->getMemoryProperties(),
            gl::Extents(sourceBox.width, sourceBox.height, 1), dstFormat.getIntendedFormatID(),
            dstFormat.getActualImageFormatID(getRequiredImageAccess()), kTransferImageFlags,
            layerCount));

        access.onImageTransferWrite(gl::LevelIndex(0), 1, 0, layerCount, VK_IMAGE_ASPECT_COLOR_BIT,
                                    &stagingImage->get());

        vk::OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        VkImageSubresourceLayers destSubresource = srcSubresource;
        destSubresource.mipLevel                 = 0;
        destSubresource.baseArrayLayer           = 0;
        destSubresource.layerCount               = layerCount;

        if (!isSrc3D)
        {
            // extents.depth should be set to layer count if any of the source or destination is a
            // 2D Array.  If both are 2D Array, it should be set to 1.
            extents.depth = 1;
        }

        vk::ImageHelper::Copy(renderer, srcImage, &stagingImage->get(), srcOffset, gl::kOffsetZero,
                              extents, srcSubresource, destSubresource, commandBuffer);

        // Stage the copy for when the image storage is actually created.
        VkImageType imageType = gl_vk::GetImageType(mState.getType());
        const gl::ImageIndex stagingIndex =
            gl::ImageIndex::Make2DArrayRange(level.get(), baseLayer, layerCount);
        mImage->stageSubresourceUpdateFromImage(stagingImage.release(), stagingIndex,
                                                vk::LevelIndex(0), dstOffsetModified, extents,
                                                imageType);
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::copySubImageImplWithDraw(ContextVk *contextVk,
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
                                                  SurfaceRotation srcFramebufferRotation)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    UtilsVk &utilsVk       = contextVk->getUtils();

    // Potentially make adjustments for pre-rotation.
    gl::Box rotatedSourceBox = sourceBox;
    gl::Extents srcExtents   = srcImage->getLevelExtents2D(vk::LevelIndex(0));
    switch (srcFramebufferRotation)
    {
        case SurfaceRotation::Identity:
            // No adjustments needed
            break;
        case SurfaceRotation::Rotated90Degrees:
            // Turn off y-flip for 90 degrees, as we don't want it affecting the
            // shaderParams.srcOffset calculation done in UtilsVk::copyImage().
            ASSERT(isSrcFlipY);
            isSrcFlipY = false;
            std::swap(rotatedSourceBox.x, rotatedSourceBox.y);
            std::swap(rotatedSourceBox.width, rotatedSourceBox.height);
            std::swap(srcExtents.width, srcExtents.height);
            break;
        case SurfaceRotation::Rotated180Degrees:
            ASSERT(isSrcFlipY);
            rotatedSourceBox.x = srcExtents.width - sourceBox.x - sourceBox.width - 1;
            rotatedSourceBox.y = srcExtents.height - sourceBox.y - sourceBox.height - 1;
            break;
        case SurfaceRotation::Rotated270Degrees:
            // Turn off y-flip for 270 degrees, as we don't want it affecting the
            // shaderParams.srcOffset calculation done in UtilsVk::copyImage().  It is needed
            // within the shader (when it will affect how the shader looks-up the source pixel),
            // and so shaderParams.flipY is turned on at the right time within
            // UtilsVk::copyImage().
            ASSERT(isSrcFlipY);
            isSrcFlipY         = false;
            rotatedSourceBox.x = srcExtents.height - sourceBox.y - sourceBox.height - 1;
            rotatedSourceBox.y = srcExtents.width - sourceBox.x - sourceBox.width - 1;
            std::swap(rotatedSourceBox.width, rotatedSourceBox.height);
            std::swap(srcExtents.width, srcExtents.height);
            break;
        default:
            UNREACHABLE();
            break;
    }

    gl::LevelIndex level(index.getLevelIndex());

    UtilsVk::CopyImageParameters params;
    params.srcOffset[0]        = rotatedSourceBox.x;
    params.srcOffset[1]        = rotatedSourceBox.y;
    params.srcExtents[0]       = rotatedSourceBox.width;
    params.srcExtents[1]       = rotatedSourceBox.height;
    params.dstOffset[0]        = dstOffset.x;
    params.dstOffset[1]        = dstOffset.y;
    params.srcMip              = srcImage->toVkLevel(sourceLevelGL).get();
    params.srcSampleCount      = srcImage->getSamples();
    params.srcHeight           = srcExtents.height;
    params.dstMip              = level;
    params.srcPremultiplyAlpha = unpackPremultiplyAlpha && !unpackUnmultiplyAlpha;
    params.srcUnmultiplyAlpha  = unpackUnmultiplyAlpha && !unpackPremultiplyAlpha;
    params.srcFlipY            = isSrcFlipY;
    params.dstFlipY            = unpackFlipY;
    params.srcRotation         = srcFramebufferRotation;

    uint32_t baseLayer  = index.hasLayer() ? index.getLayerIndex() : dstOffset.z;
    uint32_t layerCount = sourceBox.depth;

    gl::Extents extents = {sourceBox.width, sourceBox.height, sourceBox.depth};

    bool isSrc3D  = srcImage->getExtents().depth > 1;
    bool isDest3D = gl_vk::GetImageType(mState.getType()) == VK_IMAGE_TYPE_3D;

    // Perform self-copies through a staging buffer.
    // TODO: optimize to copy directly if possible.  http://anglebug.com/42263319
    bool isSelfCopy = mImage == srcImage;
    params.srcColorEncoding =
        gl::GetSizedInternalFormatInfo(srcImage->getIntendedFormat().glInternalFormat)
            .colorEncoding;
    params.dstColorEncoding =
        gl::GetSizedInternalFormatInfo(dstFormat.getIntendedFormat().glInternalFormat)
            .colorEncoding;

    // If destination is valid, copy the source directly into it.
    if (shouldUpdateBeFlushed(level, dstFormat.getActualImageFormatID(getRequiredImageAccess())) &&
        !isSelfCopy)
    {
        // Make sure any updates to the image are already flushed.
        ANGLE_TRY(flushImageStagedUpdates(contextVk));

        for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
        {
            params.srcLayer = layerIndex + sourceBox.z;
            params.dstLayer = baseLayer + layerIndex;

            const vk::ImageView *destView;
            ANGLE_TRY(getLevelLayerImageView(contextVk, level, baseLayer + layerIndex, &destView));

            ANGLE_TRY(utilsVk.copyImage(contextVk, mImage, destView, srcImage, srcView, params));
        }
    }
    else
    {
        GLint samples                      = srcImage->getSamples();
        gl::TextureType stagingTextureType = vk::Get2DTextureType(layerCount, samples);

        // Create a temporary image to stage the copy
        std::unique_ptr<vk::RefCounted<vk::ImageHelper>> stagingImage;
        stagingImage = std::make_unique<vk::RefCounted<vk::ImageHelper>>();

        ANGLE_TRY(stagingImage->get().init2DStaging(
            contextVk, mState.hasProtectedContent(), renderer->getMemoryProperties(),
            gl::Extents(sourceBox.width, sourceBox.height, 1), dstFormat.getIntendedFormatID(),
            dstFormat.getActualImageFormatID(getRequiredImageAccess()), kDrawStagingImageFlags,
            layerCount));

        params.dstOffset[0] = 0;
        params.dstOffset[1] = 0;

        for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
        {
            params.srcLayer = layerIndex + sourceBox.z;
            params.dstLayer = layerIndex;

            // Create a temporary view for this layer.
            vk::ImageView stagingView;
            ANGLE_TRY(stagingImage->get().initLayerImageView(
                contextVk, stagingTextureType, VK_IMAGE_ASPECT_COLOR_BIT, gl::SwizzleState(),
                &stagingView, vk::LevelIndex(0), 1, layerIndex, 1));

            ANGLE_TRY(utilsVk.copyImage(contextVk, &stagingImage->get(), &stagingView, srcImage,
                                        srcView, params));

            // Queue the resource for cleanup as soon as the copy above is finished.  There's no
            // need to keep it around.
            contextVk->addGarbage(&stagingView);
        }

        if (!isSrc3D)
        {
            // extents.depth should be set to layer count if any of the source or destination is a
            // 2D Array.  If both are 2D Array, it should be set to 1.
            extents.depth = 1;
        }

        gl::Offset dstOffsetModified = dstOffset;
        if (!isDest3D)
        {
            // If destination is not 3D, destination offset must be 0.
            dstOffsetModified.z = 0;
        }

        // Stage the copy for when the image storage is actually created.
        VkImageType imageType = gl_vk::GetImageType(mState.getType());
        const gl::ImageIndex stagingIndex =
            gl::ImageIndex::Make2DArrayRange(level.get(), baseLayer, layerCount);
        mImage->stageSubresourceUpdateFromImage(stagingImage.release(), stagingIndex,
                                                vk::LevelIndex(0), dstOffsetModified, extents,
                                                imageType);
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::setStorageImpl(ContextVk *contextVk,
                                        gl::TextureType type,
                                        const vk::Format &format)
{
    if (!mOwnsImage)
    {
        releaseAndDeleteImageAndViews(contextVk);
    }
    else if (mImage)
    {
        if (!contextVk->hasDisplayTextureShareGroup())
        {
            contextVk->getShareGroup()->onTextureRelease(this);
        }
        mImage->releaseStagedUpdates(contextVk->getRenderer());
    }

    // Assume all multisample texture types must be renderable.
    if (type == gl::TextureType::_2DMultisample || type == gl::TextureType::_2DMultisampleArray)
    {
        ANGLE_TRY(ensureRenderableWithFormat(contextVk, format, nullptr));
    }

    // Fixed rate compression
    if (mState.getSurfaceCompressionFixedRate() != GL_SURFACE_COMPRESSION_FIXED_RATE_NONE_EXT)
    {
        ANGLE_TRY(ensureRenderableWithFormat(contextVk, format, nullptr));
    }

    ANGLE_TRY(ensureImageAllocated(contextVk, format));

    if (mImage->valid())
    {
        releaseImage(contextVk);
    }

    ASSERT(mState.getImmutableFormat());
    ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));
    ANGLE_TRY(initImage(contextVk, format.getIntendedFormatID(),
                        format.getActualImageFormatID(getRequiredImageAccess()),
                        ImageMipLevels::FullMipChainForGenerateMipmap));

    return angle::Result::Continue;
}

angle::Result TextureVk::setStorage(const gl::Context *context,
                                    gl::TextureType type,
                                    size_t levels,
                                    GLenum internalFormat,
                                    const gl::Extents &size)
{
    return setStorageMultisample(context, type, 1, internalFormat, size, true);
}

angle::Result TextureVk::setStorageMultisample(const gl::Context *context,
                                               gl::TextureType type,
                                               GLsizei samples,
                                               GLint internalformat,
                                               const gl::Extents &size,
                                               bool fixedSampleLocations)
{
    ContextVk *contextVk     = GetAs<ContextVk>(context->getImplementation());
    vk::Renderer *renderer   = contextVk->getRenderer();
    const vk::Format &format = renderer->getFormat(internalformat);

    ANGLE_TRY(setStorageImpl(contextVk, type, format));

    return angle::Result::Continue;
}

angle::Result TextureVk::setStorageExternalMemory(const gl::Context *context,
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
    ContextVk *contextVk           = vk::GetImpl(context);
    MemoryObjectVk *memoryObjectVk = vk::GetImpl(memoryObject);
    vk::Renderer *renderer         = contextVk->getRenderer();

    const vk::Format &vkFormat     = renderer->getFormat(internalFormat);
    angle::FormatID actualFormatID = vkFormat.getActualRenderableImageFormatID();

    releaseAndDeleteImageAndViews(contextVk);

    setImageHelper(contextVk, new vk::ImageHelper(), gl::TextureType::InvalidEnum, 0, 0, true, {});

    mImage->setTilingMode(gl_vk::GetTilingMode(mState.getTilingMode()));

    // EXT_external_objects issue 13 says that all supported usage flags must be specified.
    // However, ANGLE_external_objects_flags allows these flags to be masked.  Note that the GL enum
    // values constituting the bits of |usageFlags| are identical to their corresponding Vulkan
    // value.
    usageFlags &= vk::GetMaximalImageUsageFlags(renderer, actualFormatID);

    // Similarly, createFlags is restricted to what is valid.
    createFlags &= vk::GetMinimalImageCreateFlags(renderer, type, usageFlags) |
                   VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    ANGLE_TRY(memoryObjectVk->createImage(contextVk, type, levels, internalFormat, size, offset,
                                          mImage, createFlags, usageFlags, imageCreateInfoPNext));
    mImageUsageFlags  = usageFlags;
    mImageCreateFlags = createFlags;

    constexpr VkImageUsageFlags kRenderableUsageFlags =
        kColorAttachmentImageFlags | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if ((usageFlags & kRenderableUsageFlags) != 0)
    {
        mRequiredImageAccess = vk::ImageAccess::Renderable;
    }

    ANGLE_TRY(initImageViews(contextVk, getImageViewLevelCount()));

    return angle::Result::Continue;
}

angle::Result TextureVk::setStorageAttribs(const gl::Context *context,
                                           gl::TextureType type,
                                           size_t levels,
                                           GLint internalformat,
                                           const gl::Extents &size,
                                           const GLint *attribList)
{
    ContextVk *contextVk     = GetAs<ContextVk>(context->getImplementation());
    vk::Renderer *renderer   = contextVk->getRenderer();
    const vk::Format &format = renderer->getFormat(internalformat);

    ANGLE_TRY(setStorageImpl(contextVk, type, format));

    return angle::Result::Continue;
}

GLint TextureVk::getImageCompressionRate(const gl::Context *context)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    ASSERT(mImage != nullptr && mImage->valid());
    ASSERT(renderer->getFeatures().supportsImageCompressionControl.enabled);

    if (!mOwnsImage)
    {
        return 0;
    }

    VkImageSubresource2EXT imageSubresource2      = {};
    imageSubresource2.sType                       = VK_STRUCTURE_TYPE_IMAGE_SUBRESOURCE_2_EXT;
    imageSubresource2.imageSubresource.aspectMask = mImage->getAspectFlags();

    VkImageCompressionPropertiesEXT compressionProperties = {};
    compressionProperties.sType               = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT;
    VkSubresourceLayout2EXT subresourceLayout = {};
    subresourceLayout.sType                   = VK_STRUCTURE_TYPE_SUBRESOURCE_LAYOUT_2_EXT;
    subresourceLayout.pNext                   = &compressionProperties;

    vkGetImageSubresourceLayout2EXT(renderer->getDevice(), mImage->getImage().getHandle(),
                                    &imageSubresource2, &subresourceLayout);

    GLint compressionRate;
    // For an existing image, should only report one compression rate
    vk_gl::ConvertCompressionFlagsToGLFixedRates(
        compressionProperties.imageCompressionFixedRateFlags, 1, &compressionRate);
    return compressionRate;
}

GLint TextureVk::getFormatSupportedCompressionRatesImpl(vk::Renderer *renderer,
                                                        const vk::Format &format,
                                                        GLsizei bufSize,
                                                        GLint *rates)
{
    if (renderer->getFeatures().supportsImageCompressionControl.enabled)
    {
        VkImageCompressionControlEXT compressionInfo = {};
        compressionInfo.sType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT;
        // Use default compression control flag for query
        compressionInfo.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;

        VkImageCompressionPropertiesEXT compressionProp = {};
        compressionProp.sType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT;

        if (vk::ImageHelper::FormatSupportsUsage(
                renderer,
                vk::GetVkFormatFromFormatID(renderer, format.getActualRenderableImageFormatID()),
                VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                0, &compressionInfo, &compressionProp,
                vk::ImageHelper::FormatSupportCheck::OnlyQuerySuccess))
        {
            if ((compressionProp.imageCompressionFlags &
                 VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT) != 0)
            {
                return vk_gl::ConvertCompressionFlagsToGLFixedRates(
                    compressionProp.imageCompressionFixedRateFlags, bufSize, rates);
            }
        }
    }

    return 0;
}

GLint TextureVk::getFormatSupportedCompressionRates(const gl::Context *context,
                                                    GLenum internalformat,
                                                    GLsizei bufSize,
                                                    GLint *rates)
{
    ContextVk *contextVk     = vk::GetImpl(context);
    vk::Renderer *renderer   = contextVk->getRenderer();
    const vk::Format &format = renderer->getFormat(internalformat);

    return getFormatSupportedCompressionRatesImpl(renderer, format, bufSize, rates);
}

void TextureVk::handleImmutableSamplerTransition(const vk::ImageHelper *previousImage,
                                                 const vk::ImageHelper *nextImage)
{
    // Did the previous image have an immutable sampler
    bool previousImageHadImmutableSampler =
        previousImage && previousImage->valid() && previousImage->hasImmutableSampler();

    // Does the next image require an immutable sampler?
    bool nextImageRequiresImmutableSampler =
        nextImage && nextImage->valid() && nextImage->hasImmutableSampler();

    // Has the external format changed?
    bool externalFormatChanged = false;
    if (previousImageHadImmutableSampler && nextImageRequiresImmutableSampler)
    {
        externalFormatChanged =
            previousImage->getExternalFormat() != nextImage->getExternalFormat();
    }

    // Handle transition of immutable sampler state
    if ((previousImageHadImmutableSampler != nextImageRequiresImmutableSampler) ||
        externalFormatChanged)
    {
        // The immutable sampler state is dirty.
        resetSampler();
        mImmutableSamplerDirty = true;
    }
}

angle::Result TextureVk::setEGLImageTarget(const gl::Context *context,
                                           gl::TextureType type,
                                           egl::Image *image)
{
    ContextVk *contextVk = vk::GetImpl(context);
    ImageVk *imageVk     = vk::GetImpl(image);

    // Early out if we are creating TextureVk with the exact same eglImage and target/face/level to
    // avoid unnecessarily dirty the state and allocating new ImageViews etc.
    if (mImage == imageVk->getImage() && mEGLImageNativeType == imageVk->getImageTextureType() &&
        static_cast<GLint>(mEGLImageLevelOffset) == imageVk->getImageLevel().get() &&
        mEGLImageLayerOffset == imageVk->getImageLayer())
    {
        return angle::Result::Continue;
    }

    ANGLE_TRY(contextVk->getShareGroup()->lockDefaultContextsPriority(contextVk));

    // TODO: Textures other than EGLImage targets can have immutable samplers.
    // http://anglebug.com/42264309
    handleImmutableSamplerTransition(mImage, imageVk ? imageVk->getImage() : nullptr);

    releaseAndDeleteImageAndViews(contextVk);

    UniqueSerial siblingSerial = imageVk->generateSiblingSerial();
    setImageHelper(contextVk, imageVk->getImage(), imageVk->getImageTextureType(),
                   imageVk->getImageLevel().get(), imageVk->getImageLayer(), false, siblingSerial);

    // Update ImageViewHelper's colorspace related state
    EGLenum imageColorspaceAttribute = image->getColorspaceAttribute();
    if (imageColorspaceAttribute != EGL_GL_COLORSPACE_DEFAULT_EXT)
    {
        egl::ImageColorspace imageColorspace =
            (imageColorspaceAttribute == EGL_GL_COLORSPACE_SRGB_KHR) ? egl::ImageColorspace::SRGB
                                                                     : egl::ImageColorspace::Linear;
        ASSERT(mImage != nullptr);
        mImageView.updateEglImageColorspace(*mImage, imageColorspace);
    }

    ANGLE_TRY(initImageViews(contextVk, getImageViewLevelCount()));

    return angle::Result::Continue;
}

angle::Result TextureVk::setImageExternal(const gl::Context *context,
                                          gl::TextureType type,
                                          egl::Stream *stream,
                                          const egl::Stream::GLTextureDescription &desc)
{
    ANGLE_VK_UNREACHABLE(vk::GetImpl(context));
    return angle::Result::Stop;
}

angle::Result TextureVk::setBuffer(const gl::Context *context, GLenum internalFormat)
{
    // No longer an image
    releaseAndDeleteImageAndViews(vk::GetImpl(context));
    resetSampler();

    // There's nothing else to do here.
    return angle::Result::Continue;
}

gl::ImageIndex TextureVk::getNativeImageIndex(const gl::ImageIndex &inputImageIndex) const
{
    if (mEGLImageNativeType == gl::TextureType::InvalidEnum)
    {
        return inputImageIndex;
    }

    // inputImageIndex can point to a specific layer, but only for non-2D textures.
    // mEGLImageNativeType can be a valid type, but only for 2D textures.
    // As such, both of these cannot be true at the same time.
    ASSERT(!inputImageIndex.hasLayer() && inputImageIndex.getLevelIndex() == 0);

    return gl::ImageIndex::MakeFromType(mEGLImageNativeType, mEGLImageLevelOffset,
                                        mEGLImageLayerOffset);
}

gl::LevelIndex TextureVk::getNativeImageLevel(gl::LevelIndex frontendLevel) const
{
    ASSERT(frontendLevel.get() == 0 || mEGLImageLevelOffset == 0);
    return frontendLevel + mEGLImageLevelOffset;
}

uint32_t TextureVk::getNativeImageLayer(uint32_t frontendLayer) const
{
    ASSERT(frontendLayer == 0 || mEGLImageLayerOffset == 0);
    return frontendLayer + mEGLImageLayerOffset;
}

void TextureVk::releaseAndDeleteImageAndViews(ContextVk *contextVk)
{
    if (mImage)
    {
        if (mOwnsImage)
        {
            releaseStagedUpdates(contextVk);
        }
        releaseImage(contextVk);
        mImageObserverBinding.bind(nullptr);
        mRequiresMutableStorage = false;
        mRequiredImageAccess    = vk::ImageAccess::SampleOnly;
        mImageCreateFlags       = 0;
        SafeDelete(mImage);
    }

    if (!contextVk->hasDisplayTextureShareGroup())
    {
        contextVk->getShareGroup()->onTextureRelease(this);
    }

    if (getBuffer().get() != nullptr)
    {
        mBufferContentsObservers->disableForBuffer(getBuffer().get());
    }

    if (mBufferViews.isInitialized())
    {
        mBufferViews.release(contextVk);
        onStateChange(angle::SubjectMessage::SubjectChanged);
    }
    mRedefinedLevels = {};
    mDescriptorSetCacheManager.releaseKeys(contextVk->getRenderer());
}

void TextureVk::initImageUsageFlags(ContextVk *contextVk, angle::FormatID actualFormatID)
{
    ASSERT(actualFormatID != angle::FormatID::NONE);

    mImageUsageFlags = kTransferImageFlags | VK_IMAGE_USAGE_SAMPLED_BIT;

    // If the image has depth/stencil support, add those as possible usage.
    vk::Renderer *renderer = contextVk->getRenderer();
    if (angle::Format::Get(actualFormatID).hasDepthOrStencilBits())
    {
        // Work around a bug in the Mock ICD:
        // https://github.com/KhronosGroup/Vulkan-Tools/issues/445
        if (renderer->hasImageFormatFeatureBits(actualFormatID,
                                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT))
        {
            mImageUsageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            if (renderer->getFeatures().supportsShaderFramebufferFetchDepthStencil.enabled)
            {
                mImageUsageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
            }
        }
    }
    else if (renderer->hasImageFormatFeatureBits(actualFormatID,
                                                 VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT))
    {
        mImageUsageFlags |= kColorAttachmentImageFlags;
    }
}

angle::Result TextureVk::ensureImageAllocated(ContextVk *contextVk, const vk::Format &format)
{
    if (mImage == nullptr)
    {
        setImageHelper(contextVk, new vk::ImageHelper(), gl::TextureType::InvalidEnum, 0, 0, true,
                       {});
    }

    initImageUsageFlags(contextVk, format.getActualImageFormatID(getRequiredImageAccess()));

    return angle::Result::Continue;
}

void TextureVk::setImageHelper(ContextVk *contextVk,
                               vk::ImageHelper *imageHelper,
                               gl::TextureType eglImageNativeType,
                               uint32_t imageLevelOffset,
                               uint32_t imageLayerOffset,
                               bool selfOwned,
                               UniqueSerial siblingSerial)
{
    ASSERT(mImage == nullptr);

    mImageObserverBinding.bind(imageHelper);

    ASSERT(selfOwned == !siblingSerial.valid());
    mOwnsImage          = selfOwned;
    mImageSiblingSerial = siblingSerial;
    // If image is shared between other container objects, force it to renderable format since we
    // don't know if other container object will render or not.
    if (!mOwnsImage && !imageHelper->isBackedByExternalMemory())
    {
        mRequiredImageAccess = vk::ImageAccess::Renderable;
    }
    mEGLImageNativeType  = eglImageNativeType;
    mEGLImageLevelOffset = imageLevelOffset;
    mEGLImageLayerOffset = imageLayerOffset;
    mImage               = imageHelper;

    // All render targets must be already destroyed prior to this call.
    for (auto &renderTargets : mSingleLayerRenderTargets)
    {
        ASSERT(renderTargets.empty());
    }
    ASSERT(mMultiLayerRenderTargets.empty());

    if (!selfOwned)
    {
        // (!selfOwned) implies that the texture is a target sibling.
        // Inherit a few VkImage's create attributes from ImageHelper.
        mImageCreateFlags       = mImage->getCreateFlags();
        mImageUsageFlags        = mImage->getUsage();
        mRequiresMutableStorage = (mImageCreateFlags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0;
    }

    vk::Renderer *renderer = contextVk->getRenderer();

    getImageViews().init(renderer);
}

angle::Result TextureVk::redefineLevel(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const vk::Format &format,
                                       const gl::Extents &size)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (!mOwnsImage)
    {
        releaseAndDeleteImageAndViews(contextVk);
    }

    if (mImage != nullptr)
    {
        // If there are any staged changes for this index, we can remove them since we're going to
        // override them with this call.
        gl::LevelIndex levelIndexGL(index.getLevelIndex());
        const uint32_t layerIndex = index.hasLayer() ? index.getLayerIndex() : 0;
        if (gl::IsArrayTextureType(index.getType()))
        {
            // A multi-layer texture is being redefined, remove all updates to this level; the
            // number of layers may have changed.
            mImage->removeStagedUpdates(contextVk, levelIndexGL, levelIndexGL);
        }
        else
        {
            // Otherwise remove only updates to this layer.  For example, cube map updates can be
            // done through glTexImage2D, one per cube face (i.e. layer) and so should not remove
            // updates to the other layers.
            ASSERT(index.getLayerCount() == 1);
            mImage->removeSingleSubresourceStagedUpdates(contextVk, levelIndexGL, layerIndex,
                                                         index.getLayerCount());
        }

        if (mImage->valid())
        {
            TextureLevelAllocation levelAllocation =
                IsTextureLevelInAllocatedImage(*mImage, levelIndexGL)
                    ? TextureLevelAllocation::WithinAllocatedImage
                    : TextureLevelAllocation::OutsideAllocatedImage;
            TextureLevelDefinition levelDefinition =
                IsTextureLevelDefinitionCompatibleWithImage(
                    *mImage, levelIndexGL, size, format.getIntendedFormatID(),
                    format.getActualImageFormatID(getRequiredImageAccess()))
                    ? TextureLevelDefinition::Compatible
                    : TextureLevelDefinition::Incompatible;
            if (TextureRedefineLevel(levelAllocation, levelDefinition, mState.getImmutableFormat(),
                                     mImage->getLevelCount(), layerIndex, index,
                                     mImage->getFirstAllocatedLevel(), &mRedefinedLevels))
            {
                releaseImage(contextVk);
            }
        }
    }

    // If image is not released due to an out-of-range or incompatible level definition, the image
    // is still valid and we shouldn't redefine it to use the new format.  In that case,
    // ensureImageAllocated will only use the format to update the staging buffer's alignment to
    // support both the previous and the new formats.
    ANGLE_TRY(ensureImageAllocated(contextVk, format));

    return angle::Result::Continue;
}

angle::Result TextureVk::copyImageDataToBufferAndGetData(ContextVk *contextVk,
                                                         gl::LevelIndex sourceLevelGL,
                                                         uint32_t layerCount,
                                                         const gl::Box &sourceArea,
                                                         RenderPassClosureReason reason,
                                                         vk::BufferHelper *copyBuffer,
                                                         uint8_t **outDataPtr)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "TextureVk::copyImageDataToBufferAndGetData");

    // Make sure the source is initialized and it's images are flushed.
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    gl::Box modifiedSourceArea = sourceArea;

    bool is3D = mImage->getExtents().depth > 1;
    if (is3D)
    {
        layerCount = 1;
    }
    else
    {
        modifiedSourceArea.depth = 1;
    }

    ANGLE_TRY(mImage->copyImageDataToBuffer(contextVk, sourceLevelGL, layerCount, 0,
                                            modifiedSourceArea, copyBuffer, outDataPtr));

    // Explicitly finish. If new use cases arise where we don't want to block we can change this.
    ANGLE_TRY(contextVk->finishImpl(reason));
    // invalidate must be called after wait for finish.
    ANGLE_TRY(copyBuffer->invalidate(contextVk->getRenderer()));

    return angle::Result::Continue;
}

angle::Result TextureVk::copyBufferDataToImage(ContextVk *contextVk,
                                               vk::BufferHelper *srcBuffer,
                                               const gl::ImageIndex index,
                                               uint32_t rowLength,
                                               uint32_t imageHeight,
                                               const gl::Box &sourceArea,
                                               size_t offset,
                                               VkImageAspectFlags aspectFlags)
{
    ANGLE_TRACE_EVENT0("gpu.angle", "TextureVk::copyBufferDataToImage");

    // Vulkan Spec requires the bufferOffset to be a multiple of pixel size for
    // vkCmdCopyBufferToImage.
    ASSERT((offset % vk::GetImageCopyBufferAlignment(mImage->getActualFormatID())) == 0);

    gl::LevelIndex level = gl::LevelIndex(index.getLevelIndex());
    GLuint layerCount    = index.getLayerCount();
    GLuint layerIndex    = 0;

    ASSERT((aspectFlags & kDepthStencilAspects) != kDepthStencilAspects);

    VkBufferImageCopy region           = {};
    region.bufferOffset                = offset;
    region.bufferRowLength             = rowLength;
    region.bufferImageHeight           = imageHeight;
    region.imageExtent.width           = sourceArea.width;
    region.imageExtent.height          = sourceArea.height;
    region.imageExtent.depth           = sourceArea.depth;
    region.imageOffset.x               = sourceArea.x;
    region.imageOffset.y               = sourceArea.y;
    region.imageOffset.z               = sourceArea.z;
    region.imageSubresource.aspectMask = aspectFlags;
    region.imageSubresource.layerCount = layerCount;
    region.imageSubresource.mipLevel   = mImage->toVkLevel(level).get();

    if (gl::IsArrayTextureType(index.getType()))
    {
        layerIndex               = sourceArea.z;
        region.imageOffset.z     = 0;
        region.imageExtent.depth = 1;
    }
    else if (index.getType() == gl::TextureType::CubeMap)
    {
        // Copy to the correct cube map face.
        layerIndex = index.getLayerIndex();
    }
    region.imageSubresource.baseArrayLayer = layerIndex;

    // Make sure the source is initialized and its images are flushed.
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    vk::CommandBufferAccess access;
    access.onBufferTransferRead(srcBuffer);
    access.onImageTransferWrite(level, 1, layerIndex, layerCount, mImage->getAspectFlags(), mImage);

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    commandBuffer->copyBufferToImage(srcBuffer->getBuffer().getHandle(), mImage->getImage(),
                                     mImage->getCurrentLayout(contextVk->getRenderer()), 1,
                                     &region);

    return angle::Result::Continue;
}

angle::Result TextureVk::generateMipmapsWithCompute(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // Requires that the image:
    //
    // - is not sRGB
    // - is not integer
    // - is 2D or 2D array
    // - is single sample
    // - is color image
    //
    // Support for the first two can be added easily.  Supporting 3D textures, MSAA and
    // depth/stencil would be more involved.
    ASSERT(!mImage->getActualFormat().isSRGB);
    ASSERT(!mImage->getActualFormat().isInt());
    ASSERT(mImage->getType() == VK_IMAGE_TYPE_2D);
    ASSERT(mImage->getSamples() == 1);
    ASSERT(mImage->getAspectFlags() == VK_IMAGE_ASPECT_COLOR_BIT);

    // Create the appropriate sampler.
    GLenum filter = CalculateGenerateMipmapFilter(contextVk, mImage->getActualFormatID());

    gl::SamplerState samplerState;
    samplerState.setMinFilter(filter);
    samplerState.setMagFilter(filter);
    samplerState.setWrapS(GL_CLAMP_TO_EDGE);
    samplerState.setWrapT(GL_CLAMP_TO_EDGE);
    samplerState.setWrapR(GL_CLAMP_TO_EDGE);

    vk::SharedSamplerPtr sampler;
    vk::SamplerDesc samplerDesc(contextVk, samplerState, false, nullptr,
                                static_cast<angle::FormatID>(0));
    ANGLE_TRY(renderer->getSamplerCache().getSampler(contextVk, samplerDesc, &sampler));

    // If the image has more levels than supported, generate as many mips as possible at a time.
    const vk::LevelIndex maxGenerateLevels(UtilsVk::GetGenerateMipmapMaxLevels(contextVk));
    vk::LevelIndex dstMaxLevelVk = mImage->toVkLevel(gl::LevelIndex(mState.getMipmapMaxLevel()));
    for (vk::LevelIndex dstBaseLevelVk =
             mImage->toVkLevel(gl::LevelIndex(mState.getEffectiveBaseLevel() + 1));
         dstBaseLevelVk <= dstMaxLevelVk; dstBaseLevelVk = dstBaseLevelVk + maxGenerateLevels.get())
    {
        vk::CommandBufferAccess access;

        // For mipmap generation, we should make sure that there is no pending write for the source
        // mip level. If there is, a barrier should be inserted before the source mip being used.
        const vk::LevelIndex srcLevelVk = dstBaseLevelVk - 1;
        uint32_t writeLevelCount =
            std::min(maxGenerateLevels.get(), dstMaxLevelVk.get() + 1 - dstBaseLevelVk.get());

        access.onImageComputeMipmapGenerationRead(mImage->toGLLevel(srcLevelVk), 1, 0,
                                                  mImage->getLayerCount(),
                                                  VK_IMAGE_ASPECT_COLOR_BIT, mImage);
        access.onImageComputeShaderWrite(mImage->toGLLevel(dstBaseLevelVk), writeLevelCount, 0,
                                         mImage->getLayerCount(), VK_IMAGE_ASPECT_COLOR_BIT,
                                         mImage);

        vk::OutsideRenderPassCommandBuffer *commandBuffer;
        ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

        // Generate mipmaps for every layer separately.
        for (uint32_t layer = 0; layer < mImage->getLayerCount(); ++layer)
        {
            // Create the necessary views.
            const vk::ImageView *srcView                         = nullptr;
            UtilsVk::GenerateMipmapDestLevelViews destLevelViews = {};

            ANGLE_TRY(getImageViews().getLevelLayerDrawImageView(contextVk, *mImage, srcLevelVk,
                                                                 layer, &srcView));

            vk::LevelIndex dstLevelCount = maxGenerateLevels;
            for (vk::LevelIndex levelVk(0); levelVk < maxGenerateLevels; ++levelVk)
            {
                vk::LevelIndex dstLevelVk = dstBaseLevelVk + levelVk.get();

                // If fewer levels left than maxGenerateLevels, cut the loop short.
                if (dstLevelVk > dstMaxLevelVk)
                {
                    dstLevelCount = levelVk;
                    break;
                }

                ANGLE_TRY(getImageViews().getLevelLayerDrawImageView(
                    contextVk, *mImage, dstLevelVk, layer, &destLevelViews[levelVk.get()]));
            }

            // If the image has fewer than maximum levels, fill the last views with a unused view.
            ASSERT(dstLevelCount > vk::LevelIndex(0));
            for (vk::LevelIndex levelVk = dstLevelCount;
                 levelVk < vk::LevelIndex(UtilsVk::kGenerateMipmapMaxLevels); ++levelVk)
            {
                destLevelViews[levelVk.get()] = destLevelViews[levelVk.get() - 1];
            }

            // Generate mipmaps.
            UtilsVk::GenerateMipmapParameters params = {};
            params.srcLevel                          = srcLevelVk.get();
            params.dstLevelCount                     = dstLevelCount.get();

            ANGLE_TRY(contextVk->getUtils().generateMipmap(contextVk, mImage, srcView, mImage,
                                                           destLevelViews, sampler->get(), params));
        }
    }

    contextVk->trackImageWithOutsideRenderPassEvent(mImage);

    return angle::Result::Continue;
}

angle::Result TextureVk::generateMipmapsWithCPU(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    gl::LevelIndex baseLevelGL(mState.getEffectiveBaseLevel());
    vk::LevelIndex baseLevelVk         = mImage->toVkLevel(baseLevelGL);
    const gl::Extents baseLevelExtents = mImage->getLevelExtents(baseLevelVk);
    uint32_t imageLayerCount           = mImage->getLayerCount();

    uint8_t *imageData = nullptr;
    gl::Box imageArea(0, 0, 0, baseLevelExtents.width, baseLevelExtents.height,
                      baseLevelExtents.depth);

    vk::RendererScoped<vk::BufferHelper> bufferHelper(contextVk->getRenderer());
    ANGLE_TRY(copyImageDataToBufferAndGetData(contextVk, baseLevelGL, imageLayerCount, imageArea,
                                              RenderPassClosureReason::GenerateMipmapOnCPU,
                                              &bufferHelper.get(), &imageData));

    const angle::Format &angleFormat = mImage->getActualFormat();
    GLuint sourceRowPitch            = baseLevelExtents.width * angleFormat.pixelBytes;
    GLuint sourceDepthPitch          = sourceRowPitch * baseLevelExtents.height;
    size_t baseLevelAllocationSize   = sourceDepthPitch * baseLevelExtents.depth;

    // We now have the base level available to be manipulated in the imageData pointer. Generate all
    // the missing mipmaps with the slow path. For each layer, use the copied data to generate all
    // the mips.
    for (GLuint layer = 0; layer < imageLayerCount; layer++)
    {
        size_t bufferOffset = layer * baseLevelAllocationSize;

        ANGLE_TRY(generateMipmapLevelsWithCPU(contextVk, angleFormat, layer, baseLevelGL + 1,
                                              gl::LevelIndex(mState.getMipmapMaxLevel()),
                                              baseLevelExtents.width, baseLevelExtents.height,
                                              baseLevelExtents.depth, sourceRowPitch,
                                              sourceDepthPitch, imageData + bufferOffset));
    }

    ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));
    return flushImageStagedUpdates(contextVk);
}

angle::Result TextureVk::generateMipmap(const gl::Context *context)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    // The image should already be allocated by a prior syncState.
    ASSERT(mImage->valid());

    // If base level has changed, the front-end should have called syncState already.
    ASSERT(mState.getImmutableFormat() ||
           mImage->getFirstAllocatedLevel() == gl::LevelIndex(mState.getEffectiveBaseLevel()));

    // Only staged update here is the robust resource init if any.
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::FullMipChainForGenerateMipmap));

    vk::LevelIndex baseLevel = mImage->toVkLevel(gl::LevelIndex(mState.getEffectiveBaseLevel()));
    vk::LevelIndex maxLevel  = mImage->toVkLevel(gl::LevelIndex(mState.getMipmapMaxLevel()));
    ASSERT(maxLevel != vk::LevelIndex(0));

    if (getImageViews().hasColorspaceOverrideForWrite(*mImage))
    {
        angle::FormatID actualFormatID =
            getImageViews().getColorspaceOverrideFormatForWrite(mImage->getActualFormatID());
        return contextVk->getUtils().generateMipmapWithDraw(
            contextVk, mImage, actualFormatID,
            gl::IsMipmapFiltered(mState.getSamplerState().getMinFilter()));
    }

    // If it's possible to generate mipmap in compute, that would give the best possible
    // performance on some hardware.
    if (CanGenerateMipmapWithCompute(renderer, mImage->getType(), mImage->getActualFormatID(),
                                     mImage->getSamples(), mOwnsImage))
    {
        ASSERT((mImageUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT) != 0);
        return generateMipmapsWithCompute(contextVk);
    }
    else if (renderer->hasImageFormatFeatureBits(mImage->getActualFormatID(), kBlitFeatureFlags))
    {
        // Otherwise, use blit if possible.
        return mImage->generateMipmapsWithBlit(contextVk, baseLevel, maxLevel);
    }

    ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH,
                          "Mipmap generated on CPU due to format restrictions");

    // If not possible to generate mipmaps on the GPU, do it on the CPU for conformance.
    return generateMipmapsWithCPU(context);
}

angle::Result TextureVk::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    return angle::Result::Continue;
}

angle::Result TextureVk::maybeUpdateBaseMaxLevels(ContextVk *contextVk,
                                                  TextureUpdateResult *updateResultOut)
{
    if (!mImage)
    {
        return angle::Result::Continue;
    }

    bool baseLevelChanged = mCurrentBaseLevel.get() != static_cast<GLint>(mState.getBaseLevel());
    bool maxLevelChanged  = mCurrentMaxLevel.get() != static_cast<GLint>(mState.getMaxLevel());

    if (!maxLevelChanged && !baseLevelChanged)
    {
        return angle::Result::Continue;
    }

    gl::LevelIndex newBaseLevel = gl::LevelIndex(mState.getEffectiveBaseLevel());
    gl::LevelIndex newMaxLevel  = gl::LevelIndex(mState.getEffectiveMaxLevel());
    ASSERT(newBaseLevel <= newMaxLevel);

    if (!mImage->valid())
    {
        // No further work to do, let staged updates handle the new levels
        return angle::Result::Continue;
    }

    if (mState.getImmutableFormat())
    {
        // For immutable texture, baseLevel/maxLevel should be a subset of the texture's actual
        // number of mip levels. We don't need to respecify an image.
        ASSERT(!baseLevelChanged || newBaseLevel >= mImage->getFirstAllocatedLevel());
        ASSERT(!maxLevelChanged || newMaxLevel < gl::LevelIndex(mImage->getLevelCount()));
    }
    else if (!baseLevelChanged && (newMaxLevel <= mImage->getLastAllocatedLevel()))
    {
        // With a valid image, check if only changing the maxLevel to a subset of the texture's
        // actual number of mip levels
        ASSERT(maxLevelChanged);
    }
    else
    {
        *updateResultOut = TextureUpdateResult::ImageRespecified;
        return respecifyImageStorage(contextVk);
    }

    // Don't need to respecify the texture; but do need to update which vkImageView's are served up
    // by ImageViewHelper

    // Update the current max level in ImageViewHelper
    ANGLE_TRY(initImageViews(contextVk, newMaxLevel - newBaseLevel + 1));

    mCurrentBaseLevel = newBaseLevel;
    mCurrentMaxLevel  = newMaxLevel;

    return angle::Result::Continue;
}

angle::Result TextureVk::copyAndStageImageData(ContextVk *contextVk,
                                               gl::LevelIndex previousFirstAllocateLevel,
                                               vk::ImageHelper *srcImage,
                                               vk::ImageHelper *dstImage)
{
    // Preserve the data in the Vulkan image.  GL texture's staged updates that correspond to
    // levels outside the range of the Vulkan image will remain intact.
    vk::Renderer *renderer = contextVk->getRenderer();

    // This path is only called when switching from !owned to owned, in which case if any level was
    // redefined it's already released and deleted by TextureVk::redefineLevel().
    ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));

    // Create a temp copy of srcImage for staging.
    std::unique_ptr<vk::RefCounted<vk::ImageHelper>> stagingImage;
    stagingImage = std::make_unique<vk::RefCounted<vk::ImageHelper>>();

    const uint32_t levelCount = srcImage->getLevelCount();
    const uint32_t layerCount = srcImage->getLayerCount();

    ANGLE_TRY(stagingImage->get().initStaging(
        contextVk, mState.hasProtectedContent(), renderer->getMemoryProperties(),
        srcImage->getType(), srcImage->getExtents(), srcImage->getIntendedFormatID(),
        srcImage->getActualFormatID(), srcImage->getSamples(), kTransferImageFlags, levelCount,
        layerCount));

    // Copy the src image wholly into the staging image
    const VkImageAspectFlags aspectFlags = srcImage->getAspectFlags();

    vk::CommandBufferAccess access;
    access.onImageTransferWrite(gl::LevelIndex(0), levelCount, 0, layerCount, aspectFlags,
                                &stagingImage->get());
    access.onImageTransferRead(aspectFlags, srcImage);

    vk::OutsideRenderPassCommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->getOutsideRenderPassCommandBuffer(access, &commandBuffer));

    VkImageCopy copyRegion               = {};
    copyRegion.srcSubresource.aspectMask = aspectFlags;
    copyRegion.srcSubresource.layerCount = layerCount;
    copyRegion.dstSubresource            = copyRegion.srcSubresource;

    for (vk::LevelIndex levelVk(0); levelVk < vk::LevelIndex(levelCount); ++levelVk)
    {
        gl::Extents levelExtents = srcImage->getLevelExtents(levelVk);

        copyRegion.srcSubresource.mipLevel = levelVk.get();
        copyRegion.dstSubresource.mipLevel = levelVk.get();
        gl_vk::GetExtent(levelExtents, &copyRegion.extent);

        commandBuffer->copyImage(srcImage->getImage(), srcImage->getCurrentLayout(renderer),
                                 stagingImage->get().getImage(),
                                 stagingImage->get().getCurrentLayout(renderer), 1, &copyRegion);
    }

    // Stage the staging image in the destination
    dstImage->stageSubresourceUpdatesFromAllImageLevels(stagingImage.release(),
                                                        previousFirstAllocateLevel);

    return angle::Result::Continue;
}

angle::Result TextureVk::reinitImageAsRenderable(ContextVk *contextVk, const vk::Format &format)
{
    ASSERT(mImage->valid());
    vk::Renderer *renderer = contextVk->getRenderer();

    const uint32_t levelCount = mImage->getLevelCount();
    const uint32_t layerCount = mImage->getLayerCount();

    // Make sure the source is initialized and its staged updates are flushed.
    ANGLE_TRY(flushImageStagedUpdates(contextVk));

    const angle::Format &srcFormat = mImage->getActualFormat();
    const angle::Format &dstFormat = format.getActualImageFormat(getRequiredImageAccess());

    // If layerCount or levelCount is bigger than 1, we go for the slow path for now. The problem
    // with draw path is that in the multiple level/layer case, we have to do copy in a loop.
    // Currently copySubImageImplWithDraw() calls ensureImageInitalized which forces flush out
    // staged updates that we just staged inside the loop which is wrong.
    if (levelCount == 1 && layerCount == 1 &&
        !IsTextureLevelRedefined(mRedefinedLevels, mState.getType(),
                                 mImage->getFirstAllocatedLevel()))
    {
        ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_LOW,
                              "Copying image data due to texture format fallback");

        ASSERT(CanCopyWithDraw(renderer, mImage->getActualFormatID(), mImage->getTilingMode(),
                               format.getActualImageFormatID(getRequiredImageAccess()),
                               getTilingMode()));
        vk::LevelIndex levelVk(0);
        gl::LevelIndex sourceLevelGL = mImage->toGLLevel(levelVk);
        gl::Box sourceBox(gl::kOffsetZero, mImage->getLevelExtents(levelVk));
        const gl::ImageIndex index =
            gl::ImageIndex::MakeFromType(mState.getType(), sourceLevelGL.get());

        // Flush the render pass, which may incur a vkQueueSubmit, before taking any views.
        // Otherwise the view serials would not reflect the render pass they are really used in.
        // http://crbug.com/1272266#c22
        ANGLE_TRY(
            contextVk->flushCommandsAndEndRenderPass(RenderPassClosureReason::PrepareForImageCopy));

        return copySubImageImplWithDraw(contextVk, index, gl::kOffsetZero, format, sourceLevelGL,
                                        sourceBox, false, false, false, false, mImage,
                                        &getCopyImageView(), SurfaceRotation::Identity);
    }

    for (vk::LevelIndex levelVk(0); levelVk < vk::LevelIndex(levelCount); ++levelVk)
    {
        gl::LevelIndex levelGL = mImage->toGLLevel(levelVk);
        if (IsTextureLevelRedefined(mRedefinedLevels, mState.getType(), levelGL))
        {
            continue;
        }

        ANGLE_VK_PERF_WARNING(contextVk, GL_DEBUG_SEVERITY_HIGH,
                              "GPU stall due to texture format fallback");

        gl::Box sourceBox(gl::kOffsetZero, mImage->getLevelExtents(levelVk));
        // copy and stage entire layer
        const gl::ImageIndex index =
            gl::ImageIndex::MakeFromType(mState.getType(), levelGL.get(), 0, layerCount);

        // Read back the requested region of the source texture
        vk::RendererScoped<vk::BufferHelper> bufferHelper(renderer);
        vk::BufferHelper *srcBuffer = &bufferHelper.get();
        uint8_t *srcData            = nullptr;
        ANGLE_TRY(mImage->copyImageDataToBuffer(contextVk, levelGL, layerCount, 0, sourceBox,
                                                srcBuffer, &srcData));

        // Explicitly finish. If new use cases arise where we don't want to block we can change
        // this.
        ANGLE_TRY(contextVk->finishImpl(RenderPassClosureReason::TextureReformatToRenderable));
        // invalidate must be called after wait for finish.
        ANGLE_TRY(srcBuffer->invalidate(renderer));

        size_t dstBufferSize = sourceBox.width * sourceBox.height * sourceBox.depth *
                               dstFormat.pixelBytes * layerCount;

        // Allocate memory in the destination texture for the copy/conversion.
        uint8_t *dstData = nullptr;
        ANGLE_TRY(mImage->stageSubresourceUpdateAndGetData(
            contextVk, dstBufferSize, index, mImage->getLevelExtents(levelVk), gl::kOffsetZero,
            &dstData, dstFormat.id));

        // Source and destination data is tightly packed
        GLuint srcDataRowPitch = sourceBox.width * srcFormat.pixelBytes;
        GLuint dstDataRowPitch = sourceBox.width * dstFormat.pixelBytes;

        GLuint srcDataDepthPitch = srcDataRowPitch * sourceBox.height;
        GLuint dstDataDepthPitch = dstDataRowPitch * sourceBox.height;

        GLuint srcDataLayerPitch = srcDataDepthPitch * sourceBox.depth;
        GLuint dstDataLayerPitch = dstDataDepthPitch * sourceBox.depth;

        rx::PixelReadFunction pixelReadFunction   = srcFormat.pixelReadFunction;
        rx::PixelWriteFunction pixelWriteFunction = dstFormat.pixelWriteFunction;

        const gl::InternalFormat &dstFormatInfo = *mState.getImageDesc(index).format.info;
        for (uint32_t layer = 0; layer < layerCount; layer++)
        {
            CopyImageCHROMIUM(srcData + layer * srcDataLayerPitch, srcDataRowPitch,
                              srcFormat.pixelBytes, srcDataDepthPitch, pixelReadFunction,
                              dstData + layer * dstDataLayerPitch, dstDataRowPitch,
                              dstFormat.pixelBytes, dstDataDepthPitch, pixelWriteFunction,
                              dstFormatInfo.format, dstFormatInfo.componentType, sourceBox.width,
                              sourceBox.height, sourceBox.depth, false, false, false);
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::respecifyImageStorage(ContextVk *contextVk)
{
    if (!mImage->valid())
    {
        ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));
        return angle::Result::Continue;
    }

    // Recreate the image to reflect new base or max levels.
    // First, flush any pending updates so we have good data in the current mImage
    if (mImage->hasStagedUpdatesInAllocatedLevels())
    {
        ANGLE_TRY(flushImageStagedUpdates(contextVk));
    }

    if (!mOwnsImage)
    {
        // Cache values needed for copy and stage operations
        vk::ImageHelper *srcImage = mImage;
        const vk::Format &format  = getBaseLevelFormat(contextVk->getRenderer());

        // If any level was redefined but the image was not owned by the Texture, it's already
        // released and deleted by TextureVk::redefineLevel().
        ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));

        // Save previousFirstAllocateLevel before mImage becomes invalid
        gl::LevelIndex previousFirstAllocateLevel = mImage->getFirstAllocatedLevel();

        // If we didn't own the image, release the current and create a new one
        releaseImage(contextVk);

        // Create the image helper
        ANGLE_TRY(ensureImageAllocated(contextVk, format));
        ANGLE_TRY(initImage(contextVk, format.getIntendedFormatID(),
                            format.getActualImageFormatID(getRequiredImageAccess()),
                            mState.getImmutableFormat()
                                ? ImageMipLevels::FullMipChainForGenerateMipmap
                                : ImageMipLevels::EnabledLevels));

        // Make a copy of the old image (that's being released) and stage that as an update to the
        // new image.
        ANGLE_TRY(copyAndStageImageData(contextVk, previousFirstAllocateLevel, srcImage, mImage));
    }
    else
    {
        const vk::Format &format = getBaseLevelFormat(contextVk->getRenderer());
        if (mImage->getActualFormatID() != format.getActualImageFormatID(getRequiredImageAccess()))
        {
            ANGLE_TRY(reinitImageAsRenderable(contextVk, format));
        }
        else
        {
            stageSelfAsSubresourceUpdates(contextVk);
        }
        // Release the current image so that it will be recreated with the correct number of mip
        // levels, base level, and max level.
        releaseImage(contextVk);
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    ContextVk *contextVk = vk::GetImpl(context);

    releaseAndDeleteImageAndViews(contextVk);

    // eglBindTexImage can only be called with pbuffer (offscreen) surfaces
    OffscreenSurfaceVk *offscreenSurface = GetImplAs<OffscreenSurfaceVk>(surface);
    // Surface can only have single target. Just generate valid serial with throw-away generator.
    UniqueSerial siblingSerial = UniqueSerialFactory().generate();
    setImageHelper(contextVk, offscreenSurface->getColorAttachmentImage(),
                   gl::TextureType::InvalidEnum, 0, 0, false, siblingSerial);

    ASSERT(mImage->getLayerCount() == 1);
    return initImageViews(contextVk, getImageViewLevelCount());
}

angle::Result TextureVk::releaseTexImage(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    releaseImage(contextVk);

    return angle::Result::Continue;
}

angle::Result TextureVk::getAttachmentRenderTarget(const gl::Context *context,
                                                   GLenum binding,
                                                   const gl::ImageIndex &imageIndex,
                                                   GLsizei samples,
                                                   FramebufferAttachmentRenderTarget **rtOut)
{
    GLint requestedLevel = imageIndex.getLevelIndex();
    ASSERT(requestedLevel >= 0);

    ContextVk *contextVk = vk::GetImpl(context);

    // Sync the texture's image.  See comment on this function in the header.
    ANGLE_TRY(respecifyImageStorageIfNecessary(contextVk, gl::Command::Draw));

    // Don't flush staged updates here. We'll handle that in FramebufferVk so we can defer clears.

    if (!mImage->valid())
    {
        const vk::Format &format = getBaseLevelFormat(contextVk->getRenderer());
        ANGLE_TRY(initImage(contextVk, format.getIntendedFormatID(),
                            format.getActualImageFormatID(getRequiredImageAccess()),
                            ImageMipLevels::EnabledLevels));
    }

    const bool hasRenderToTextureEXT =
        contextVk->getFeatures().supportsMultisampledRenderToSingleSampled.enabled;

    // If samples > 1 here, we have a singlesampled texture that's being multisampled rendered to.
    // In this case, create a multisampled image that is otherwise identical to the single sampled
    // image.  That multisampled image is used as color or depth/stencil attachment, while the
    // original image is used as the resolve attachment.
    const gl::RenderToTextureImageIndex renderToTextureIndex =
        hasRenderToTextureEXT
            ? gl::RenderToTextureImageIndex::Default
            : static_cast<gl::RenderToTextureImageIndex>(PackSampleCount(samples));

    if (samples > 1 && !hasRenderToTextureEXT)
    {
        // Initialize mMultisampledImages and mMultisampledImageViews if necessary
        if (mMultisampledImages == nullptr)
        {
            mMultisampledImages     = std::make_unique<MultiSampleImages>();
            mMultisampledImageViews = std::make_unique<MultiSampleImageViews>();
        }

        ASSERT(mState.getBaseLevelDesc().samples <= 1);

        vk::ImageHelper &multisampledImage =
            mMultisampledImages->at(renderToTextureIndex)[requestedLevel];
        if (!multisampledImage.valid())
        {
            // Ensure the view serial is valid.
            vk::Renderer *renderer = contextVk->getRenderer();
            mMultisampledImageViews->at(renderToTextureIndex)[requestedLevel].init(renderer);

            // The MSAA image always comes from the single sampled one, so disable robust init.
            bool useRobustInit = false;

            // Calculate extents for multisample image
            VkExtent3D extents = {};
            gl_vk::GetExtent(
                mImage->getLevelExtents(mImage->toVkLevel(gl::LevelIndex(requestedLevel))),
                &extents);

            // Create the implicit multisampled image.
            ANGLE_TRY(multisampledImage.initImplicitMultisampledRenderToTexture(
                contextVk, mState.hasProtectedContent(), renderer->getMemoryProperties(),
                mState.getType(), samples, *mImage, extents, useRobustInit));
        }
    }

    GLuint layerIndex = 0, layerCount = 0, imageLayerCount = 0;
    GetRenderTargetLayerCountAndIndex(mImage, imageIndex, &layerIndex, &layerCount,
                                      &imageLayerCount);

    if (layerCount == 1)
    {
        initSingleLayerRenderTargets(contextVk, imageLayerCount, gl::LevelIndex(requestedLevel),
                                     renderToTextureIndex);

        std::vector<RenderTargetVector> &levelRenderTargets =
            mSingleLayerRenderTargets[renderToTextureIndex];
        ASSERT(requestedLevel < static_cast<int32_t>(levelRenderTargets.size()));

        RenderTargetVector &layerRenderTargets = levelRenderTargets[requestedLevel];
        ASSERT(imageIndex.getLayerIndex() < static_cast<int32_t>(layerRenderTargets.size()));

        *rtOut = &layerRenderTargets[layerIndex];
    }
    else
    {
        ASSERT(layerCount > 0);
        *rtOut = getMultiLayerRenderTarget(contextVk, gl::LevelIndex(imageIndex.getLevelIndex()),
                                           layerIndex, layerCount);
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::ensureImageInitialized(ContextVk *contextVk, ImageMipLevels mipLevels)
{
    if (mImage->valid() && !mImage->hasStagedUpdatesInAllocatedLevels())
    {
        return angle::Result::Continue;
    }

    if (!mImage->valid())
    {
        ASSERT(!TextureHasAnyRedefinedLevels(mRedefinedLevels));

        const vk::Format &format = getBaseLevelFormat(contextVk->getRenderer());
        ANGLE_TRY(initImage(contextVk, format.getIntendedFormatID(),
                            format.getActualImageFormatID(getRequiredImageAccess()), mipLevels));
        if (mipLevels == ImageMipLevels::FullMipChainForGenerateMipmap)
        {
            // Remove staged updates to non-base mips when generating mipmaps.  These can only be
            // emulated format init clears that are staged in initImage.
            mImage->removeStagedUpdates(contextVk,
                                        gl::LevelIndex(mState.getEffectiveBaseLevel() + 1),
                                        gl::LevelIndex(mState.getMipmapMaxLevel()));
        }
    }

    return flushImageStagedUpdates(contextVk);
}

angle::Result TextureVk::flushImageStagedUpdates(ContextVk *contextVk)
{
    ASSERT(mImage->valid());

    gl::LevelIndex firstLevelGL = getNativeImageLevel(mImage->getFirstAllocatedLevel());
    uint32_t firstLayer         = getNativeImageLayer(0);

    return mImage->flushStagedUpdates(contextVk, firstLevelGL,
                                      firstLevelGL + getImageViewLevelCount(), firstLayer,
                                      firstLayer + getImageViewLayerCount(), mRedefinedLevels);
}

void TextureVk::initSingleLayerRenderTargets(ContextVk *contextVk,
                                             GLuint layerCount,
                                             gl::LevelIndex levelIndex,
                                             gl::RenderToTextureImageIndex renderToTextureIndex)
{
    GLint requestedLevel = levelIndex.get();
    std::vector<RenderTargetVector> &allLevelsRenderTargets =
        mSingleLayerRenderTargets[renderToTextureIndex];

    if (allLevelsRenderTargets.size() <= static_cast<uint32_t>(requestedLevel))
    {
        allLevelsRenderTargets.resize(requestedLevel + 1);
    }

    RenderTargetVector &renderTargets = allLevelsRenderTargets[requestedLevel];

    // Lazy init. Check if already initialized.
    if (!renderTargets.empty())
    {
        return;
    }

    // There are |layerCount| render targets, one for each layer
    renderTargets.resize(layerCount);

    const bool isMultisampledRenderToTexture =
        renderToTextureIndex != gl::RenderToTextureImageIndex::Default;

    vk::ImageHelper *drawImage             = mImage;
    vk::ImageViewHelper *drawImageViews    = &getImageViews();
    vk::ImageHelper *resolveImage          = nullptr;
    vk::ImageViewHelper *resolveImageViews = nullptr;

    RenderTargetTransience transience = RenderTargetTransience::Default;

    // If multisampled render to texture, use the multisampled image as draw image instead, and
    // resolve into the texture's image automatically.
    if (isMultisampledRenderToTexture)
    {
        ASSERT(mMultisampledImages->at(renderToTextureIndex)[requestedLevel].valid());
        ASSERT(!mImage->isYuvResolve());

        resolveImage      = drawImage;
        resolveImageViews = drawImageViews;
        drawImage         = &mMultisampledImages->at(renderToTextureIndex)[requestedLevel];
        drawImageViews    = &mMultisampledImageViews->at(renderToTextureIndex)[requestedLevel];

        // If the texture is depth/stencil, GL_EXT_multisampled_render_to_texture2 explicitly
        // indicates that there is no need for the image to be resolved.  In that case, mark the
        // render target as entirely transient.
        if (mImage->getAspectFlags() != VK_IMAGE_ASPECT_COLOR_BIT)
        {
            transience = RenderTargetTransience::EntirelyTransient;
        }
        else
        {
            transience = RenderTargetTransience::MultisampledTransient;
        }
    }
    else if (mImage->isYuvResolve())
    {
        // If rendering to YUV, similar to multisampled render to texture
        resolveImage      = drawImage;
        resolveImageViews = drawImageViews;

        if (contextVk->getRenderer()->nullColorAttachmentWithExternalFormatResolve())
        {
            // If null color attachment, we still keep drawImage as is (the same as
            // resolveImage) to avoid special treatment in many places where they assume there must
            // be a color attachment if there is a resolve attachment. But when renderPass is
            // created, color attachment will be ignored.
        }
        else
        {
            transience = RenderTargetTransience::YuvResolveTransient;
            // Need to populate drawImage here; either abuse mMultisampledImages etc
            // or build something parallel to it. we don't have a vulkan implementation which
            // wants this path yet, though.
            UNREACHABLE();
        }
    }

    for (uint32_t layerIndex = 0; layerIndex < layerCount; ++layerIndex)
    {
        renderTargets[layerIndex].init(drawImage, drawImageViews, resolveImage, resolveImageViews,
                                       mImageSiblingSerial, getNativeImageLevel(levelIndex),
                                       getNativeImageLayer(layerIndex), 1, transience);
    }
}

RenderTargetVk *TextureVk::getMultiLayerRenderTarget(ContextVk *contextVk,
                                                     gl::LevelIndex level,
                                                     GLuint layerIndex,
                                                     GLuint layerCount)
{
    vk::ImageViewHelper *imageViews = &getImageViews();
    vk::ImageSubresourceRange range = imageViews->getSubresourceDrawRange(
        level, layerIndex, vk::GetLayerMode(*mImage, layerCount));

    auto iter = mMultiLayerRenderTargets.find(range);
    if (iter != mMultiLayerRenderTargets.end())
    {
        return iter->second.get();
    }

    // Create the layered render target.  Note that multisampled render to texture is not
    // allowed with layered render targets; nor is YUV rendering.
    std::unique_ptr<RenderTargetVk> &rt = mMultiLayerRenderTargets[range];
    if (!rt)
    {
        rt = std::make_unique<RenderTargetVk>();
    }

    rt->init(mImage, imageViews, nullptr, nullptr, mImageSiblingSerial, getNativeImageLevel(level),
             getNativeImageLayer(layerIndex), layerCount, RenderTargetTransience::Default);

    return rt.get();
}

void TextureVk::prepareForGenerateMipmap(ContextVk *contextVk)
{
    gl::LevelIndex baseLevel(mState.getEffectiveBaseLevel());
    gl::LevelIndex maxLevel(mState.getMipmapMaxLevel());

    // Remove staged updates to the range that's being respecified (which is all the mips except
    // baseLevel).
    gl::LevelIndex firstGeneratedLevel = baseLevel + 1;
    mImage->removeStagedUpdates(contextVk, firstGeneratedLevel, maxLevel);

    TextureRedefineGenerateMipmapLevels(baseLevel, maxLevel, firstGeneratedLevel,
                                        &mRedefinedLevels);

    // If generating mipmap and base level is incompatibly redefined, the image is going to be
    // recreated.  Don't try to preserve the other mips.
    if (IsTextureLevelRedefined(mRedefinedLevels, mState.getType(), baseLevel))
    {
        ASSERT(!mState.getImmutableFormat());
        releaseImage(contextVk);
    }

    const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
    VkImageType imageType              = gl_vk::GetImageType(mState.getType());
    const vk::Format &format           = getBaseLevelFormat(contextVk->getRenderer());
    const GLint samples                = baseLevelDesc.samples ? baseLevelDesc.samples : 1;

    // If the compute path is to be used to generate mipmaps, add the STORAGE usage.
    if (CanGenerateMipmapWithCompute(contextVk->getRenderer(), imageType,
                                     format.getActualImageFormatID(getRequiredImageAccess()),
                                     samples, mOwnsImage))
    {
        mImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }
}

angle::Result TextureVk::respecifyImageStorageIfNecessary(ContextVk *contextVk, gl::Command source)
{
    ASSERT(mState.getBuffer().get() == nullptr);

    VkImageUsageFlags oldUsageFlags   = mImageUsageFlags;
    VkImageCreateFlags oldCreateFlags = mImageCreateFlags;

    // Create a new image if the storage state is enabled for the first time.
    if (mState.hasBeenBoundAsImage())
    {
        mImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
        mRequiresMutableStorage = true;
    }

    // If we're handling dirty srgb decode/override state, we may have to reallocate the image with
    // VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT. Vulkan requires this bit to be set in order to use
    // imageviews with a format that does not match the texture's internal format.
    if (isSRGBOverrideEnabled())
    {
        mRequiresMutableStorage = true;
    }

    if (mRequiresMutableStorage)
    {
        mImageCreateFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
    }

    // Create a new image if used as attachment for the first time. This must be called before
    // prepareForGenerateMipmap since this changes the format which prepareForGenerateMipmap relies
    // on.
    if (mState.hasBeenBoundAsAttachment())
    {
        TextureUpdateResult updateResult = TextureUpdateResult::ImageUnaffected;
        ANGLE_TRY(ensureRenderable(contextVk, &updateResult));
        if (updateResult == TextureUpdateResult::ImageRespecified)
        {
            oldUsageFlags  = mImageUsageFlags;
            oldCreateFlags = mImageCreateFlags;
        }
    }

    // Before redefining the image for any reason, check to see if it's about to go through mipmap
    // generation.  In that case, drop every staged change for the subsequent mips after base, and
    // make sure the image is created with the complete mip chain.
    const bool isGenerateMipmap = source == gl::Command::GenerateMipmap;
    if (isGenerateMipmap)
    {
        prepareForGenerateMipmap(contextVk);
    }

    // If texture was not originally created using the MSRTSS flag, it should be recreated when it
    // is bound to an MSRTT framebuffer.
    if (contextVk->getFeatures().supportsMultisampledRenderToSingleSampled.enabled &&
        !contextVk->getFeatures().preferMSRTSSFlagByDefault.enabled &&
        mState.hasBeenBoundToMSRTTFramebuffer() &&
        ((mImageCreateFlags & VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT) == 0))
    {
        ANGLE_TRY(respecifyImageStorage(contextVk));
        oldUsageFlags  = mImageUsageFlags;
        oldCreateFlags = mImageCreateFlags;
    }

    // For immutable texture, base level does not affect allocation. Only usage flags are. If usage
    // flag changed, we respecify image storage early on. This makes the code more reliable and also
    // better performance wise. Otherwise, we will try to preserve base level by calling
    // stageSelfAsSubresourceUpdates and then later on find out the mImageUsageFlags changed and the
    // whole thing has to be respecified.
    if (mState.getImmutableFormat() &&
        (oldUsageFlags != mImageUsageFlags || oldCreateFlags != mImageCreateFlags))
    {
        ANGLE_TRY(respecifyImageStorage(contextVk));
        oldUsageFlags  = mImageUsageFlags;
        oldCreateFlags = mImageCreateFlags;
    }

    // Set base and max level before initializing the image
    TextureUpdateResult updateResult = TextureUpdateResult::ImageUnaffected;
    ANGLE_TRY(maybeUpdateBaseMaxLevels(contextVk, &updateResult));

    // Updating levels could have respecified the storage, recapture mImageCreateFlags
    if (updateResult == TextureUpdateResult::ImageRespecified)
    {
        oldUsageFlags  = mImageUsageFlags;
        oldCreateFlags = mImageCreateFlags;
    }

    // It is possible for the image to have a single level (because it doesn't use mipmapping),
    // then have more levels defined in it and mipmapping enabled.  In that case, the image needs
    // to be recreated.
    bool isMipmapEnabledByMinFilter = false;
    if (!isGenerateMipmap && mImage && mImage->valid())
    {
        isMipmapEnabledByMinFilter =
            mImage->getLevelCount() < getMipLevelCount(ImageMipLevels::EnabledLevels);
    }

    // If generating mipmaps and the image needs to be recreated (not full-mip already, or changed
    // usage flags), make sure it's recreated.
    if (isGenerateMipmap && mImage && mImage->valid() &&
        (oldUsageFlags != mImageUsageFlags ||
         (!mState.getImmutableFormat() &&
          mImage->getLevelCount() !=
              getMipLevelCount(ImageMipLevels::FullMipChainForGenerateMipmap))))
    {
        ASSERT(mOwnsImage);
        // Immutable texture is not expected to reach here. The usage flag change should have
        // been handled earlier and level count change should not need to reallocate
        ASSERT(!mState.getImmutableFormat());

        // Flush staged updates to the base level of the image.  Note that updates to the rest of
        // the levels have already been discarded through the |removeStagedUpdates| call above.
        ANGLE_TRY(flushImageStagedUpdates(contextVk));

        stageSelfAsSubresourceUpdates(contextVk);

        // Release the mImage without collecting garbage from image views.
        releaseImage(contextVk);
    }

    // Respecify the image if it's changed in usage, or if any of its levels are redefined and no
    // update to base/max levels were done (otherwise the above call would have already taken care
    // of this).  Note that if both base/max and image usage are changed, the image is recreated
    // twice, which incurs unnecessary copies.  This is not expected to be happening in real
    // applications.
    if (oldUsageFlags != mImageUsageFlags || oldCreateFlags != mImageCreateFlags ||
        TextureHasAnyRedefinedLevels(mRedefinedLevels) || isMipmapEnabledByMinFilter)
    {
        ANGLE_TRY(respecifyImageStorage(contextVk));
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::onLabelUpdate(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);
    return updateTextureLabel(contextVk);
}

angle::Result TextureVk::updateTextureLabel(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();
    std::string label      = mState.getLabel();
    if (!label.empty() && renderer->enableDebugUtils() && imageValid())
    {
        return vk::SetDebugUtilsObjectName(contextVk, VK_OBJECT_TYPE_IMAGE,
                                           (uint64_t)(getImage().getImage().getHandle()),
                                           mState.getLabel());
    }
    return angle::Result::Continue;
}

vk::BufferHelper *TextureVk::getRGBAConversionBufferHelper(vk::Renderer *renderer,
                                                           angle::FormatID formatID) const
{
    BufferVk *bufferVk                                        = vk::GetImpl(getBuffer().get());
    const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = mState.getBuffer();
    const VertexConversionBuffer::CacheKey cacheKey{
        formatID, 16, static_cast<size_t>(bufferBinding.getOffset()), false, true};
    ConversionBuffer *conversion = bufferVk->getVertexConversionBuffer(renderer, cacheKey);
    return conversion->getBuffer();
}

angle::Result TextureVk::convertBufferToRGBA(ContextVk *contextVk, size_t &conversionBufferSize)
{
    vk::Renderer *renderer             = contextVk->getRenderer();
    const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
    const vk::Format *imageUniformFormat =
        &renderer->getFormat(baseLevelDesc.format.info->sizedInternalFormat);
    const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = mState.getBuffer();
    BufferVk *bufferVk                                        = vk::GetImpl(getBuffer().get());
    const size_t bindingOffset                                = bufferBinding.getOffset();
    const VkDeviceSize bufferSize                             = bufferVk->getSize();
    const VkDeviceSize bufferSizeFromOffset                   = bufferSize - bindingOffset;
    conversionBufferSize = roundUpPow2<size_t>(static_cast<size_t>((bufferSizeFromOffset / 3) * 4),
                                               4 * sizeof(uint32_t));

    const VertexConversionBuffer::CacheKey cacheKey{imageUniformFormat->getIntendedFormatID(), 16,
                                                    bindingOffset, false, true};
    ConversionBuffer *conversion = bufferVk->getVertexConversionBuffer(renderer, cacheKey);
    mBufferContentsObservers->enableForBuffer(getBuffer().get());
    if (!conversion->valid())
    {
        ANGLE_TRY(contextVk->initBufferForVertexConversion(conversion, conversionBufferSize,
                                                           vk::MemoryHostVisibility::NonVisible));
    }
    vk::BufferHelper *conversionBufferHelper = conversion->getBuffer();

    if (conversion->dirty())
    {
        vk::BufferHelper &bufferHelper = bufferVk->getBuffer();
        UtilsVk &utilsVk               = contextVk->getUtils();
        const VkDeviceSize pixelSize   = 3 * sizeof(uint32_t);
        const VkDeviceSize pixelCount  = bufferSizeFromOffset / pixelSize;

        ANGLE_TRY(utilsVk.copyRgbToRgba(contextVk, imageUniformFormat->getIntendedFormat(),
                                        &bufferHelper, static_cast<uint32_t>(bindingOffset),
                                        static_cast<uint32_t>(pixelCount), conversionBufferHelper));
        conversion->clearDirty();
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::syncState(const gl::Context *context,
                                   const gl::Texture::DirtyBits &dirtyBits,
                                   gl::Command source)
{
    ContextVk *contextVk   = vk::GetImpl(context);
    vk::Renderer *renderer = contextVk->getRenderer();

    // If this is a texture buffer, release buffer views.  There's nothing else to sync.  The
    // image must already be deleted, and the sampler reset.
    if (mState.getBuffer().get() != nullptr)
    {
        ASSERT(mImage == nullptr);

        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = mState.getBuffer();

        VkDeviceSize offset = bufferBinding.getOffset();
        VkDeviceSize size   = gl::GetBoundBufferAvailableSize(bufferBinding);

        if (NeedsRGBAEmulation(renderer, getBaseLevelFormat(renderer).getIntendedFormatID()))
        {
            size_t conversionBufferSize;
            ANGLE_TRY(convertBufferToRGBA(contextVk, conversionBufferSize));
            offset = 0;
            size   = conversionBufferSize;
        }

        mBufferViews.release(contextVk);
        mBufferViews.init(renderer, offset, size);
        mDescriptorSetCacheManager.releaseKeys(renderer);
        return angle::Result::Continue;
    }

    ANGLE_TRY(respecifyImageStorageIfNecessary(contextVk, source));

    // Initialize the image storage and flush the pixel buffer.
    const bool isGenerateMipmap = source == gl::Command::GenerateMipmap;
    ANGLE_TRY(ensureImageInitialized(contextVk, isGenerateMipmap
                                                    ? ImageMipLevels::FullMipChainForGenerateMipmap
                                                    : ImageMipLevels::EnabledLevels));

    // Mask out the IMPLEMENTATION dirty bit to avoid unnecessary syncs.
    // Keep it set when the border color is used and needs to be resynced.
    gl::Texture::DirtyBits localBits = dirtyBits;
    if (!mState.getSamplerState().usesBorderColor())
    {
        localBits.reset(gl::Texture::DIRTY_BIT_IMPLEMENTATION);
    }
    localBits.reset(gl::Texture::DIRTY_BIT_BASE_LEVEL);
    localBits.reset(gl::Texture::DIRTY_BIT_MAX_LEVEL);

    // For AHBs, the ImageViews are created with VkSamplerYcbcrConversionInfo's chromaFilter
    // matching min/magFilters as part of the eglEGLImageTargetTexture2DOES() call. However, the
    // min/mag filters can change later, requiring the ImageViews to be refreshed.
    if (mImage->valid() && mImage->hasImmutableSampler() &&
        (dirtyBits.test(gl::Texture::DIRTY_BIT_MIN_FILTER) ||
         dirtyBits.test(gl::Texture::DIRTY_BIT_MAG_FILTER)))
    {
        const gl::SamplerState &samplerState = mState.getSamplerState();
        VkFilter chromaFilter = samplerState.getMinFilter() == samplerState.getMagFilter()
                                    ? gl_vk::GetFilter(samplerState.getMinFilter())
                                    : vk::kDefaultYCbCrChromaFilter;
        if (mImage->updateChromaFilter(renderer, chromaFilter))
        {
            resetSampler();
            ANGLE_TRY(refreshImageViews(contextVk));
        }
    }

    if (localBits.none() && mSampler)
    {
        return angle::Result::Continue;
    }

    if (mSampler)
    {
        resetSampler();
    }

    if (localBits.test(gl::Texture::DIRTY_BIT_SWIZZLE_RED) ||
        localBits.test(gl::Texture::DIRTY_BIT_SWIZZLE_GREEN) ||
        localBits.test(gl::Texture::DIRTY_BIT_SWIZZLE_BLUE) ||
        localBits.test(gl::Texture::DIRTY_BIT_SWIZZLE_ALPHA))
    {
        ANGLE_TRY(refreshImageViews(contextVk));
    }

    if (localBits.test(gl::Texture::DIRTY_BIT_SRGB_OVERRIDE) ||
        localBits.test(gl::Texture::DIRTY_BIT_SRGB_DECODE))
    {
        ASSERT(mImage != nullptr);
        gl::SrgbDecode srgbDecode = (mState.getSamplerState().getSRGBDecode() == GL_SKIP_DECODE_EXT)
                                        ? gl::SrgbDecode::Skip
                                        : gl::SrgbDecode::Default;
        mImageView.updateSrgbDecode(*mImage, srgbDecode);
        mImageView.updateSrgbOverride(*mImage, mState.getSRGBOverride());

        if (!renderer->getFeatures().supportsImageFormatList.enabled)
        {
            ANGLE_TRY(refreshImageViews(contextVk));
        }
    }

    vk::SamplerDesc samplerDesc(contextVk, mState.getSamplerState(), mState.isStencilMode(),
                                &mImage->getYcbcrConversionDesc(), mImage->getIntendedFormatID());
    auto y2yConversionDesc = mImage->getY2YConversionDesc();
    vk::SamplerDesc samplerDescSamplerExternal2DY2YEXT(contextVk, mState.getSamplerState(),
                                                       mState.isStencilMode(), &y2yConversionDesc,
                                                       mImage->getIntendedFormatID());
    ANGLE_TRY(renderer->getSamplerCache().getSampler(contextVk, samplerDesc, &mSampler));
    ANGLE_TRY(renderer->getSamplerCache().getSampler(contextVk, samplerDescSamplerExternal2DY2YEXT,
                                                     &mY2YSampler));

    updateCachedImageViewSerials();

    return angle::Result::Continue;
}

angle::Result TextureVk::initializeContents(const gl::Context *context,
                                            GLenum binding,
                                            const gl::ImageIndex &imageIndex)
{
    ContextVk *contextVk      = vk::GetImpl(context);
    const gl::ImageDesc &desc = mState.getImageDesc(imageIndex);
    const vk::Format &format =
        contextVk->getRenderer()->getFormat(desc.format.info->sizedInternalFormat);

    ASSERT(mImage);
    // Note that we cannot ensure the image is initialized because we might be calling subImage
    // on a non-complete cube map.
    return mImage->stageRobustResourceClearWithFormat(
        contextVk, imageIndex, desc.size, format.getIntendedFormat(),
        format.getActualImageFormat(getRequiredImageAccess()));
}

angle::Result TextureVk::initializeContentsWithBlack(const gl::Context *context,
                                                     GLenum binding,
                                                     const gl::ImageIndex &imageIndex)
{
    ContextVk *contextVk      = vk::GetImpl(context);
    const gl::ImageDesc &desc = mState.getImageDesc(imageIndex);
    const vk::Format &format =
        contextVk->getRenderer()->getFormat(desc.format.info->sizedInternalFormat);

    VkClearValue clearValue = {};
    clearValue.color        = {{0, 0, 0, 1.0f}};

    ASSERT(mImage);
    // Note that we cannot ensure the image is initialized because we might be calling subImage
    // on a non-complete cube map.
    return mImage->stageResourceClearWithFormat(
        contextVk, imageIndex, desc.size, format.getIntendedFormat(),
        format.getActualImageFormat(getRequiredImageAccess()), clearValue);
}

void TextureVk::releaseOwnershipOfImage(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    ASSERT(!mImageSiblingSerial.valid());

    mOwnsImage = false;
    releaseAndDeleteImageAndViews(contextVk);
}

const vk::ImageView &TextureVk::getReadImageView(GLenum srgbDecode,
                                                 bool texelFetchStaticUse,
                                                 bool samplerExternal2DY2YEXT) const
{
    ASSERT(mImage->valid());

    const vk::ImageViewHelper &imageViews = getImageViews();

    if (mState.isStencilMode() && imageViews.hasStencilReadImageView())
    {
        return imageViews.getStencilReadImageView();
    }

    if (samplerExternal2DY2YEXT)
    {
        ASSERT(imageViews.getSamplerExternal2DY2YEXTImageView().valid());
        return imageViews.getSamplerExternal2DY2YEXTImageView();
    }

    ASSERT(mImage != nullptr && mImage->valid());
    gl::SrgbDecode decode =
        (srgbDecode == GL_DECODE_EXT) ? gl::SrgbDecode::Default : gl::SrgbDecode::Skip;
    imageViews.updateSrgbDecode(*mImage, decode);
    imageViews.updateStaticTexelFetch(*mImage, texelFetchStaticUse);

    ASSERT(imageViews.getReadImageView().valid());
    return imageViews.getReadImageView();
}

const vk::ImageView &TextureVk::getCopyImageView() const
{
    ASSERT(mImage->valid());

    const vk::ImageViewHelper &imageViews = getImageViews();

    const angle::Format &angleFormat = mImage->getActualFormat();
    ASSERT(angleFormat.isSRGB ==
           (ConvertToLinear(mImage->getActualFormatID()) != angle::FormatID::NONE));
    if (angleFormat.isSRGB)
    {
        return imageViews.getSRGBCopyImageView();
    }
    return imageViews.getLinearCopyImageView();
}

angle::Result TextureVk::getLevelLayerImageView(vk::ErrorContext *context,
                                                gl::LevelIndex level,
                                                size_t layer,
                                                const vk::ImageView **imageViewOut)
{
    ASSERT(mImage && mImage->valid());

    gl::LevelIndex levelGL = getNativeImageLevel(level);
    vk::LevelIndex levelVk = mImage->toVkLevel(levelGL);
    uint32_t nativeLayer   = getNativeImageLayer(static_cast<uint32_t>(layer));

    return getImageViews().getLevelLayerDrawImageView(context, *mImage, levelVk, nativeLayer,
                                                      imageViewOut);
}

angle::Result TextureVk::getStorageImageView(vk::ErrorContext *context,
                                             const gl::ImageUnit &binding,
                                             const vk::ImageView **imageViewOut)
{
    vk::Renderer *renderer = context->getRenderer();

    angle::FormatID formatID = angle::Format::InternalFormatToID(binding.format);
    const vk::Format *format = &renderer->getFormat(formatID);

    format = AdjustStorageViewFormatPerWorkarounds(renderer, format, getRequiredImageAccess());

    gl::LevelIndex nativeLevelGL =
        getNativeImageLevel(gl::LevelIndex(static_cast<uint32_t>(binding.level)));
    vk::LevelIndex nativeLevelVk = mImage->toVkLevel(nativeLevelGL);

    // If the texture does not have multiple layers or faces, the entire texture
    // level is bound, regardless of the values specified by layered and layer.
    if (binding.layered != GL_TRUE && IsLayeredTextureType(mState.getType()))
    {
        uint32_t nativeLayer = getNativeImageLayer(static_cast<uint32_t>(binding.layer));

        return getImageViews().getLevelLayerStorageImageView(
            context, *mImage, nativeLevelVk, nativeLayer,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            format->getActualImageFormatID(getRequiredImageAccess()), imageViewOut);
    }

    uint32_t nativeLayer = getNativeImageLayer(0);

    return getImageViews().getLevelStorageImageView(
        context, mState.getType(), *mImage, nativeLevelVk, nativeLayer,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        format->getActualImageFormatID(getRequiredImageAccess()), imageViewOut);
}

vk::BufferHelper *TextureVk::getPossiblyEmulatedTextureBuffer(vk::ErrorContext *context) const
{
    vk::Renderer *renderer = context->getRenderer();

    angle::FormatID format = getBaseLevelFormat(renderer).getIntendedFormatID();
    if (NeedsRGBAEmulation(renderer, format))
    {
        return getRGBAConversionBufferHelper(renderer, format);
    }

    BufferVk *bufferVk = vk::GetImpl(getBuffer().get());
    return &bufferVk->getBuffer();
}

angle::Result TextureVk::getBufferView(vk::ErrorContext *context,
                                       const vk::Format *imageUniformFormat,
                                       const gl::SamplerBinding *samplerBinding,
                                       bool isImage,
                                       const vk::BufferView **viewOut)
{
    vk::Renderer *renderer = context->getRenderer();

    ASSERT(mState.getBuffer().get() != nullptr);

    // Use the format specified by glTexBuffer if no format specified by the shader.
    if (imageUniformFormat == nullptr)
    {
        imageUniformFormat = &getBaseLevelFormat(renderer);
    }

    if (isImage)
    {
        imageUniformFormat = AdjustStorageViewFormatPerWorkarounds(renderer, imageUniformFormat,
                                                                   getRequiredImageAccess());
    }

    const vk::BufferHelper *buffer = &vk::GetImpl(mState.getBuffer().get())->getBuffer();

    if (NeedsRGBAEmulation(renderer, imageUniformFormat->getIntendedFormatID()))
    {
        buffer = getRGBAConversionBufferHelper(renderer, imageUniformFormat->getIntendedFormatID());
        imageUniformFormat = &renderer->getFormat(
            GetRGBAEmulationDstFormat(imageUniformFormat->getIntendedFormatID()));
    }

    if (samplerBinding)
    {
        imageUniformFormat =
            AdjustViewFormatForSampler(renderer, imageUniformFormat, samplerBinding->format);
    }

    // Create a view for the required format.
    return mBufferViews.getView(context, *buffer, buffer->getOffset(), *imageUniformFormat,
                                viewOut);
}

angle::Result TextureVk::initImage(ContextVk *contextVk,
                                   angle::FormatID intendedImageFormatID,
                                   angle::FormatID actualImageFormatID,
                                   ImageMipLevels mipLevels)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    // Create the image. For immutable texture, we always allocate the full immutable levels
    // specified by texStorage call. Otherwise we only try to allocate from base to max levels.
    const gl::ImageDesc *firstLevelDesc;
    uint32_t firstLevel, levelCount;
    if (mState.getImmutableFormat())
    {
        firstLevelDesc = &mState.getLevelZeroDesc();
        firstLevel     = 0;
        levelCount     = mState.getImmutableLevels();
    }
    else
    {
        firstLevelDesc = &mState.getBaseLevelDesc();
        firstLevel     = mState.getEffectiveBaseLevel();
        levelCount     = getMipLevelCount(mipLevels);
    }
    const gl::Extents &firstLevelExtents = firstLevelDesc->size;

    VkExtent3D vkExtent;
    uint32_t layerCount;
    gl_vk::GetExtentsAndLayerCount(mState.getType(), firstLevelExtents, &vkExtent, &layerCount);
    GLint samples = mState.getBaseLevelDesc().samples ? mState.getBaseLevelDesc().samples : 1;

    if (contextVk->getFeatures().limitSampleCountTo2.enabled)
    {
        samples = std::min(samples, 2);
    }

    if (mState.hasProtectedContent())
    {
        mImageCreateFlags |= VK_IMAGE_CREATE_PROTECTED_BIT;
    }

    if (renderer->getFeatures().supportsComputeTranscodeEtcToBc.enabled &&
        IsETCFormat(intendedImageFormatID) && IsBCFormat(actualImageFormatID))
    {
        mImageCreateFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT |
                             VK_IMAGE_CREATE_EXTENDED_USAGE_BIT |
                             VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT;
        mImageUsageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    mImageCreateFlags |=
        vk::GetMinimalImageCreateFlags(renderer, mState.getType(), mImageUsageFlags);

    const VkFormat actualImageFormat =
        rx::vk::GetVkFormatFromFormatID(renderer, actualImageFormatID);
    const VkImageType imageType     = gl_vk::GetImageType(mState.getType());
    const VkImageTiling imageTiling = mImage->getTilingMode();

    // The MSRTSS bit is included in the create flag for all textures if the feature flag
    // corresponding to its preference is enabled. Otherwise, it is enabled for a texture if it is
    // bound to an MSRTT framebuffer.
    const bool shouldIncludeMSRTSSBit =
        contextVk->getFeatures().supportsMultisampledRenderToSingleSampled.enabled &&
        (contextVk->getFeatures().preferMSRTSSFlagByDefault.enabled ||
         mState.hasBeenBoundToMSRTTFramebuffer());

    if ((mImageUsageFlags & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) != 0 &&
        mOwnsImage && samples == 1 && shouldIncludeMSRTSSBit)
    {
        VkImageCreateFlags createFlagsMultisampled =
            mImageCreateFlags | VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
        bool isActualFormatSRGB             = angle::Format::Get(actualImageFormatID).isSRGB;
        const VkFormat additionalViewFormat = rx::vk::GetVkFormatFromFormatID(
            renderer, isActualFormatSRGB ? ConvertToLinear(actualImageFormatID)
                                         : ConvertToSRGB(actualImageFormatID));
        const bool isAdditionalFormatValid = additionalViewFormat != VK_FORMAT_UNDEFINED;

        // If the texture has already been bound to an MSRTT framebuffer, lack of support should
        // result in failure.
        bool supportsMSRTTUsageActualFormat = vk::ImageHelper::FormatSupportsUsage(
            renderer, actualImageFormat, imageType, imageTiling, mImageUsageFlags,
            createFlagsMultisampled, nullptr, nullptr,
            vk::ImageHelper::FormatSupportCheck::RequireMultisampling);
        bool supportsMSRTTUsageAdditionalFormat =
            !isAdditionalFormatValid ||
            vk::ImageHelper::FormatSupportsUsage(
                renderer, additionalViewFormat, imageType, imageTiling, mImageUsageFlags,
                createFlagsMultisampled, nullptr, nullptr,
                vk::ImageHelper::FormatSupportCheck::RequireMultisampling);

        bool supportsMSRTTUsage =
            supportsMSRTTUsageActualFormat && supportsMSRTTUsageAdditionalFormat;
        if (ANGLE_UNLIKELY(mState.hasBeenBoundToMSRTTFramebuffer() && !supportsMSRTTUsage))
        {
            ERR() << "Texture bound to EXT_multisampled_render_to_texture framebuffer, "
                  << "but this device does not support this format.";
            ANGLE_VK_TRY(contextVk, VK_ERROR_FORMAT_NOT_SUPPORTED);
        }

        // Note: If we ever fail the following check, we should use the emulation path for this
        // texture instead of ignoring MSRTT.
        if (supportsMSRTTUsage)
        {
            // If supported by format add the MSRTSS flag because any texture might end up as an
            // MSRTT attachment.
            mImageCreateFlags |= VK_IMAGE_CREATE_MULTISAMPLED_RENDER_TO_SINGLE_SAMPLED_BIT_EXT;
        }
    }

    // Any format with VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT support is required to also support
    // VK_FORMAT_FEATURE_2_HOST_IMAGE_TRANSFER_BIT_EXT.  So no format feature query is needed.
    // However, it's still necessary to use vkGetPhysicalDeviceImageFormatProperties2 to ensure host
    // image copy is supported for the specific usage and flags.
    //
    // All TextureVk images are expected to have VK_IMAGE_USAGE_SAMPLED_BIT, so that is not checked
    // either.
    ASSERT((mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) != 0);
    if (mOwnsImage && samples == 1 && renderer->getFeatures().supportsHostImageCopy.enabled)
    {
        VkHostImageCopyDevicePerformanceQueryEXT perfQuery = {};
        perfQuery.sType = VK_STRUCTURE_TYPE_HOST_IMAGE_COPY_DEVICE_PERFORMANCE_QUERY_EXT;

        // If host image copy is supported at all ...
        if (vk::ImageHelper::FormatSupportsUsage(
                renderer, actualImageFormat, imageType, imageTiling,
                mImageUsageFlags | VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT, mImageCreateFlags, nullptr,
                &perfQuery, vk::ImageHelper::FormatSupportCheck::OnlyQuerySuccess))
        {
            // Only enable it if it has no performance impact whatsoever (or impact is tiny, given
            // feature).
            if (perfQuery.identicalMemoryLayout ||
                (perfQuery.optimalDeviceAccess &&
                 renderer->getFeatures().allowHostImageCopyDespiteNonIdenticalLayout.enabled))
            {
                mImageUsageFlags |= VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
            }
        }
    }

    // Fixed rate compression
    VkImageCompressionControlEXT *compressionInfo   = nullptr;
    VkImageCompressionControlEXT compressionInfoVar = {};
    compressionInfoVar.sType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_CONTROL_EXT;
    VkImageCompressionFixedRateFlagsEXT compressionRates = VK_IMAGE_COMPRESSION_FIXED_RATE_NONE_EXT;
    if (mOwnsImage && renderer->getFeatures().supportsImageCompressionControl.enabled)
    {
        // Use default compression control flag for query
        compressionInfoVar.flags = VK_IMAGE_COMPRESSION_FIXED_RATE_DEFAULT_EXT;

        VkImageCompressionPropertiesEXT compressionProp = {};
        compressionProp.sType = VK_STRUCTURE_TYPE_IMAGE_COMPRESSION_PROPERTIES_EXT;

        // If fixed rate compression is supported by this type, not support YUV now.
        const vk::Format &format = renderer->getFormat(intendedImageFormatID);
        if (!mImage->isYuvResolve() &&
            (getFormatSupportedCompressionRatesImpl(renderer, format, 0, nullptr) != 0))
        {
            mImage->getCompressionFixedRate(&compressionInfoVar, &compressionRates,
                                            mState.getSurfaceCompressionFixedRate());
            compressionInfo = &compressionInfoVar;
        }
    }

    ANGLE_TRY(mImage->initExternal(
        contextVk, mState.getType(), vkExtent, intendedImageFormatID, actualImageFormatID, samples,
        mImageUsageFlags, mImageCreateFlags, vk::ImageLayout::Undefined, nullptr,
        gl::LevelIndex(firstLevel), levelCount, layerCount,
        contextVk->isRobustResourceInitEnabled(), mState.hasProtectedContent(),
        vk::ImageHelper::deriveConversionDesc(contextVk, actualImageFormatID,
                                              intendedImageFormatID),
        compressionInfo));

    ANGLE_TRY(updateTextureLabel(contextVk));

    // Update create flags with mImage's create flags
    mImageCreateFlags |= mImage->getCreateFlags();
    mRequiresMutableStorage = (mImageCreateFlags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0;

    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (mState.hasProtectedContent())
    {
        flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    }

    ANGLE_TRY(contextVk->initImageAllocation(mImage, mState.hasProtectedContent(),
                                             renderer->getMemoryProperties(), flags,
                                             vk::MemoryAllocationType::TextureImage));

    const uint32_t viewLevelCount =
        mState.getImmutableFormat() ? getMipLevelCount(ImageMipLevels::EnabledLevels) : levelCount;
    ANGLE_TRY(initImageViews(contextVk, viewLevelCount));

    mCurrentBaseLevel = gl::LevelIndex(mState.getBaseLevel());
    mCurrentMaxLevel  = gl::LevelIndex(mState.getMaxLevel());

    return angle::Result::Continue;
}

angle::Result TextureVk::initImageViews(ContextVk *contextVk, uint32_t levelCount)
{
    ASSERT(mImage != nullptr && mImage->valid());

    gl::LevelIndex baseLevelGL =
        getNativeImageLevel(gl::LevelIndex(mState.getEffectiveBaseLevel()));
    vk::LevelIndex baseLevelVk = mImage->toVkLevel(baseLevelGL);
    uint32_t baseLayer         = getNativeImageLayer(0);

    const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
    const bool sized                   = baseLevelDesc.format.info->sized;

    const angle::Format &intendedFormat = mImage->getIntendedFormat();
    gl::SwizzleState formatSwizzle      = GetFormatSwizzle(intendedFormat, sized);
    gl::SwizzleState readSwizzle        = ApplySwizzle(formatSwizzle, mState.getSwizzleState());

    // Use this as a proxy for the SRGB override & skip decode settings.
    bool createExtraSRGBViews = mRequiresMutableStorage;

    const VkImageUsageFlags kDisallowedSwizzledUsage =
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
    ANGLE_TRY(getImageViews().initReadViews(contextVk, mState.getType(), *mImage, formatSwizzle,
                                            readSwizzle, baseLevelVk, levelCount, baseLayer,
                                            getImageViewLayerCount(), createExtraSRGBViews,
                                            getImage().getUsage() & ~kDisallowedSwizzledUsage));

    updateCachedImageViewSerials();

    return angle::Result::Continue;
}

void TextureVk::releaseImage(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    releaseImageViews(contextVk);

    if (mImage)
    {
        if (mOwnsImage)
        {
            mImage->releaseImageFromShareContexts(renderer, contextVk, mImageSiblingSerial);
        }
        else
        {
            mImage->finalizeImageLayoutInShareContexts(renderer, contextVk, mImageSiblingSerial);
            mImageObserverBinding.bind(nullptr);
            mImage = nullptr;
        }
    }

    if (mMultisampledImages)
    {
        for (gl::TexLevelArray<vk::ImageHelper> &images : *mMultisampledImages)
        {
            for (vk::ImageHelper &image : images)
            {
                if (image.valid())
                {
                    image.releaseImageFromShareContexts(renderer, contextVk, mImageSiblingSerial);
                }
            }
        }
        mMultisampledImages.reset();
    }

    onStateChange(angle::SubjectMessage::SubjectChanged);
    mRedefinedLevels = {};
}

void TextureVk::releaseImageViews(ContextVk *contextVk)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    mDescriptorSetCacheManager.releaseKeys(renderer);

    if (mImage == nullptr)
    {
        if (mMultisampledImageViews)
        {
            for (gl::TexLevelArray<vk::ImageViewHelper> &imageViewHelpers :
                 *mMultisampledImageViews)
            {
                for (vk::ImageViewHelper &imageViewHelper : imageViewHelpers)
                {
                    ASSERT(imageViewHelper.isImageViewGarbageEmpty());
                }
            }
            mMultisampledImageViews.reset();
        }
        for (auto &renderTargets : mSingleLayerRenderTargets)
        {
            ASSERT(renderTargets.empty());
        }
        ASSERT(mMultiLayerRenderTargets.empty());
        return;
    }

    mImageView.release(renderer, mImage->getResourceUse());

    if (mMultisampledImageViews)
    {
        for (gl::TexLevelArray<vk::ImageViewHelper> &imageViewHelpers : *mMultisampledImageViews)
        {
            for (vk::ImageViewHelper &imageViewHelper : imageViewHelpers)
            {
                imageViewHelper.release(renderer, mImage->getResourceUse());
            }
        }
        mMultisampledImageViews.reset();
    }

    for (auto &renderTargets : mSingleLayerRenderTargets)
    {
        for (RenderTargetVector &renderTargetLevels : renderTargets)
        {
            for (RenderTargetVk &renderTargetVk : renderTargetLevels)
            {
                renderTargetVk.releaseFramebuffers(contextVk);
            }
            // Clear the layers tracked for each level
            renderTargetLevels.clear();
        }
        // Then clear the levels
        renderTargets.clear();
    }

    for (auto &renderTargetPair : mMultiLayerRenderTargets)
    {
        renderTargetPair.second->releaseFramebuffers(contextVk);
    }
    mMultiLayerRenderTargets.clear();
}

void TextureVk::releaseStagedUpdates(ContextVk *contextVk)
{
    if (mImage)
    {
        mImage->releaseStagedUpdates(contextVk->getRenderer());
    }
}

uint32_t TextureVk::getMipLevelCount(ImageMipLevels mipLevels) const
{
    switch (mipLevels)
    {
        // Returns level count from base to max that has been specified, i.e, enabled.
        case ImageMipLevels::EnabledLevels:
            return mState.getEnabledLevelCount();
        // Returns all mipmap levels from base to max regardless if an image has been specified or
        // not.
        case ImageMipLevels::FullMipChainForGenerateMipmap:
            return getMaxLevelCount() - mState.getEffectiveBaseLevel();

        default:
            UNREACHABLE();
            return 0;
    }
}

uint32_t TextureVk::getMaxLevelCount() const
{
    // getMipmapMaxLevel will be 0 here if mipmaps are not used, so the levelCount is always +1.
    return mState.getMipmapMaxLevel() + 1;
}

angle::Result TextureVk::generateMipmapLevelsWithCPU(ContextVk *contextVk,
                                                     const angle::Format &sourceFormat,
                                                     GLuint layer,
                                                     gl::LevelIndex firstMipLevel,
                                                     gl::LevelIndex maxMipLevel,
                                                     const size_t sourceWidth,
                                                     const size_t sourceHeight,
                                                     const size_t sourceDepth,
                                                     const size_t sourceRowPitch,
                                                     const size_t sourceDepthPitch,
                                                     uint8_t *sourceData)
{
    size_t previousLevelWidth      = sourceWidth;
    size_t previousLevelHeight     = sourceHeight;
    size_t previousLevelDepth      = sourceDepth;
    uint8_t *previousLevelData     = sourceData;
    size_t previousLevelRowPitch   = sourceRowPitch;
    size_t previousLevelDepthPitch = sourceDepthPitch;

    for (gl::LevelIndex currentMipLevel = firstMipLevel; currentMipLevel <= maxMipLevel;
         ++currentMipLevel)
    {
        // Compute next level width and height.
        size_t mipWidth  = std::max<size_t>(1, previousLevelWidth >> 1);
        size_t mipHeight = std::max<size_t>(1, previousLevelHeight >> 1);
        size_t mipDepth  = std::max<size_t>(1, previousLevelDepth >> 1);

        // With the width and height of the next mip, we can allocate the next buffer we need.
        uint8_t *destData     = nullptr;
        size_t destRowPitch   = mipWidth * sourceFormat.pixelBytes;
        size_t destDepthPitch = destRowPitch * mipHeight;

        size_t mipAllocationSize = destDepthPitch * mipDepth;
        gl::Extents mipLevelExtents(static_cast<int>(mipWidth), static_cast<int>(mipHeight),
                                    static_cast<int>(mipDepth));

        ANGLE_TRY(mImage->stageSubresourceUpdateAndGetData(
            contextVk, mipAllocationSize,
            gl::ImageIndex::MakeFromType(mState.getType(), currentMipLevel.get(), layer),
            mipLevelExtents, gl::Offset(), &destData, sourceFormat.id));

        // Generate the mipmap into that new buffer
        sourceFormat.mipGenerationFunction(
            previousLevelWidth, previousLevelHeight, previousLevelDepth, previousLevelData,
            previousLevelRowPitch, previousLevelDepthPitch, destData, destRowPitch, destDepthPitch);

        // Swap for the next iteration
        previousLevelWidth      = mipWidth;
        previousLevelHeight     = mipHeight;
        previousLevelDepth      = mipDepth;
        previousLevelData       = destData;
        previousLevelRowPitch   = destRowPitch;
        previousLevelDepthPitch = destDepthPitch;
    }

    return angle::Result::Continue;
}

const gl::InternalFormat &TextureVk::getImplementationSizedFormat(const gl::Context *context) const
{
    GLenum sizedFormat = GL_NONE;

    if (mImage && mImage->valid())
    {
        sizedFormat = mImage->getActualFormat().glInternalFormat;
    }
    else
    {
        ContextVk *contextVk     = vk::GetImpl(context);
        const vk::Format &format = getBaseLevelFormat(contextVk->getRenderer());
        sizedFormat = format.getActualImageFormat(getRequiredImageAccess()).glInternalFormat;
    }

    return gl::GetSizedInternalFormatInfo(sizedFormat);
}

GLenum TextureVk::getColorReadFormat(const gl::Context *context)
{
    const gl::InternalFormat &sizedFormat = getImplementationSizedFormat(context);
    return sizedFormat.format;
}

GLenum TextureVk::getColorReadType(const gl::Context *context)
{
    const gl::InternalFormat &sizedFormat = getImplementationSizedFormat(context);
    return sizedFormat.type;
}

angle::Result TextureVk::getTexImage(const gl::Context *context,
                                     const gl::PixelPackState &packState,
                                     gl::Buffer *packBuffer,
                                     gl::TextureTarget target,
                                     GLint level,
                                     GLenum format,
                                     GLenum type,
                                     void *pixels)
{
    if (packBuffer && this->isCompressedFormatEmulated(context, target, level))
    {
        // TODO (anglebug.com/42265933): Can't populate from a buffer using emulated format
        UNIMPLEMENTED();
        return angle::Result::Stop;
    }

    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    GLint baseLevel = static_cast<int>(mState.getBaseLevel());
    if (level < baseLevel || level >= baseLevel + static_cast<int>(mState.getEnabledLevelCount()))
    {
        // TODO(http://anglebug.com/42264855): Handle inconsistent textures.
        WARN() << "GetTexImage for inconsistent texture levels is not implemented.";
        UNIMPLEMENTED();
        return angle::Result::Continue;
    }

    gl::MaybeOverrideLuminance(format, type, getColorReadFormat(context),
                               getColorReadType(context));

    uint32_t layer      = 0;
    uint32_t layerCount = 1;

    switch (target)
    {
        case gl::TextureTarget::CubeMapArray:
        case gl::TextureTarget::_2DArray:
            layerCount = mImage->getLayerCount();
            break;
        default:
            if (gl::IsCubeMapFaceTarget(target))
            {
                layer = static_cast<uint32_t>(gl::CubeMapTextureTargetToFaceIndex(target));
            }
            break;
    }

    return mImage->readPixelsForGetImage(contextVk, packState, packBuffer, gl::LevelIndex(level),
                                         layer, layerCount, format, type, pixels);
}

angle::Result TextureVk::getCompressedTexImage(const gl::Context *context,
                                               const gl::PixelPackState &packState,
                                               gl::Buffer *packBuffer,
                                               gl::TextureTarget target,
                                               GLint level,
                                               void *pixels)
{
    ContextVk *contextVk = vk::GetImpl(context);
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    GLint baseLevel = static_cast<int>(mState.getBaseLevel());
    if (level < baseLevel || level >= baseLevel + static_cast<int>(mState.getEnabledLevelCount()))
    {
        // TODO(http://anglebug.com/42264855): Handle inconsistent textures.
        WARN() << "GetCompressedTexImage for inconsistent texture levels is not implemented.";
        UNIMPLEMENTED();
        return angle::Result::Continue;
    }

    uint32_t layer      = 0;
    uint32_t layerCount = 1;

    switch (target)
    {
        case gl::TextureTarget::CubeMapArray:
        case gl::TextureTarget::_2DArray:
            layerCount = mImage->getLayerCount();
            break;
        default:
            if (gl::IsCubeMapFaceTarget(target))
            {
                layer = static_cast<uint32_t>(gl::CubeMapTextureTargetToFaceIndex(target));
            }
            break;
    }

    return mImage->readPixelsForCompressedGetImage(
        contextVk, packState, packBuffer, gl::LevelIndex(level), layer, layerCount, pixels);
}

const vk::Format &TextureVk::getBaseLevelFormat(vk::Renderer *renderer) const
{
    const gl::ImageDesc &baseLevelDesc = mState.getBaseLevelDesc();
    return renderer->getFormat(baseLevelDesc.format.info->sizedInternalFormat);
}

void TextureVk::onSubjectStateChange(angle::SubjectIndex index, angle::SubjectMessage message)
{
    ASSERT(index == kTextureImageSubjectIndex &&
           (message == angle::SubjectMessage::SubjectChanged ||
            message == angle::SubjectMessage::InitializationComplete));

    // Forward the notification to the parent that the staging buffer changed.
    onStateChange(message);
}

vk::ImageOrBufferViewSubresourceSerial TextureVk::getImageViewSubresourceSerialImpl(
    vk::ImageViewColorspace colorspace) const
{
    gl::LevelIndex baseLevel(mState.getEffectiveBaseLevel());
    // getMipmapMaxLevel will clamp to the max level if it is smaller than the number of mips.
    uint32_t levelCount = gl::LevelIndex(mState.getMipmapMaxLevel()) - baseLevel + 1;

    return getImageViews().getSubresourceSerialForColorspace(baseLevel, levelCount, 0,
                                                             vk::LayerMode::All, colorspace);
}

vk::ImageOrBufferViewSubresourceSerial TextureVk::getBufferViewSerial() const
{
    return mBufferViews.getSerial();
}

vk::ImageOrBufferViewSubresourceSerial TextureVk::getStorageImageViewSerial(
    const gl::ImageUnit &binding) const
{
    vk::LayerMode layerMode = binding.layered == GL_TRUE ? vk::LayerMode::All : vk::LayerMode::_1;
    uint32_t frontendLayer  = binding.layered == GL_TRUE ? 0 : static_cast<uint32_t>(binding.layer);
    uint32_t nativeLayer    = getNativeImageLayer(frontendLayer);

    gl::LevelIndex baseLevel(
        getNativeImageLevel(gl::LevelIndex(static_cast<uint32_t>(binding.level))));

    return getImageViews().getSubresourceSerial(baseLevel, 1, nativeLayer, layerMode);
}

uint32_t TextureVk::getImageViewLayerCount() const
{
    // We use a special layer count here to handle EGLImages. They might only be
    // looking at one layer of a cube or 2D array texture.
    return mEGLImageNativeType == gl::TextureType::InvalidEnum ? mImage->getLayerCount() : 1;
}

uint32_t TextureVk::getImageViewLevelCount() const
{
    // We use a special level count here to handle EGLImages. They might only be
    // looking at one level of the texture's mipmap chain.
    return mEGLImageNativeType == gl::TextureType::InvalidEnum ? mImage->getLevelCount() : 1;
}

angle::Result TextureVk::refreshImageViews(ContextVk *contextVk)
{
    vk::ImageViewHelper &imageView = getImageViews();
    if (mImage == nullptr)
    {
        ASSERT(imageView.isImageViewGarbageEmpty());
    }
    else
    {
        vk::Renderer *renderer = contextVk->getRenderer();
        imageView.release(renderer, mImage->getResourceUse());

        // Since view has changed, some descriptorSet cache maybe obsolete. SO proactively release
        // cache.
        mDescriptorSetCacheManager.releaseKeys(renderer);

        for (auto &renderTargets : mSingleLayerRenderTargets)
        {
            for (RenderTargetVector &renderTargetLevels : renderTargets)
            {
                for (RenderTargetVk &renderTargetVk : renderTargetLevels)
                {
                    renderTargetVk.releaseFramebuffers(contextVk);
                }
            }
        }
        for (auto &renderTargetPair : mMultiLayerRenderTargets)
        {
            renderTargetPair.second->releaseFramebuffers(contextVk);
        }
    }

    ANGLE_TRY(initImageViews(contextVk, getImageViewLevelCount()));

    // Let any Framebuffers know we need to refresh the RenderTarget cache.
    onStateChange(angle::SubjectMessage::SubjectChanged);

    return angle::Result::Continue;
}

angle::Result TextureVk::ensureMutable(ContextVk *contextVk)
{
    if (mRequiresMutableStorage)
    {
        return angle::Result::Continue;
    }

    mRequiresMutableStorage = true;
    mImageCreateFlags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;

    ANGLE_TRY(respecifyImageStorage(contextVk));
    ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));

    return refreshImageViews(contextVk);
}

angle::Result TextureVk::ensureRenderable(ContextVk *contextVk,
                                          TextureUpdateResult *updateResultOut)
{
    return ensureRenderableWithFormat(contextVk, getBaseLevelFormat(contextVk->getRenderer()),
                                      updateResultOut);
}

angle::Result TextureVk::ensureRenderableWithFormat(ContextVk *contextVk,
                                                    const vk::Format &format,
                                                    TextureUpdateResult *updateResultOut)
{
    if (mRequiredImageAccess == vk::ImageAccess::Renderable)
    {
        return angle::Result::Continue;
    }

    mRequiredImageAccess = vk::ImageAccess::Renderable;
    if (!mImage)
    {
        // Later on when ensureImageAllocated() is called, it will ensure a renderable format is
        // used.
        return angle::Result::Continue;
    }

    if (!format.hasRenderableImageFallbackFormat())
    {
        // If there is no fallback format for renderable, then nothing to do.
        return angle::Result::Continue;
    }

    // luminance/alpha format never fallback for rendering and if we ever do fallback, the
    // following code may not handle it properly.
    ASSERT(!format.getIntendedFormat().isLUMA());

    angle::FormatID previousActualFormatID =
        format.getActualImageFormatID(vk::ImageAccess::SampleOnly);
    angle::FormatID actualFormatID = format.getActualImageFormatID(vk::ImageAccess::Renderable);

    if (!mImage->valid())
    {
        // Immutable texture must already have a valid image
        ASSERT(!mState.getImmutableFormat());
        // If we have staged updates and they were encoded with different format, we need to flush
        // out these staged updates. The respecifyImageStorage should handle reading back the
        // flushed data and re-stage it with the new format.
        angle::FormatID intendedFormatID = format.getIntendedFormatID();

        gl::LevelIndex levelGLStart, levelGLEnd;
        ImageMipLevels mipLevels;
        if (mState.getImmutableFormat())
        {
            levelGLStart = gl::LevelIndex(0);
            levelGLEnd   = gl::LevelIndex(mState.getImmutableLevels());
            mipLevels    = ImageMipLevels::FullMipChainForGenerateMipmap;
        }
        else
        {
            levelGLStart = gl::LevelIndex(mState.getEffectiveBaseLevel());
            levelGLEnd =
                gl::LevelIndex(levelGLStart + getMipLevelCount(ImageMipLevels::EnabledLevels));
            mipLevels = ImageMipLevels::EnabledLevels;
        }

        if (mImage->hasStagedImageUpdatesWithMismatchedFormat(levelGLStart, levelGLEnd,
                                                              actualFormatID))
        {
            angle::FormatID sampleOnlyFormatID =
                format.getActualImageFormatID(vk::ImageAccess::SampleOnly);

            ANGLE_TRY(initImage(contextVk, intendedFormatID, sampleOnlyFormatID, mipLevels));
        }
        else
        {
            // First try to convert any staged buffer updates from old format to new format using
            // CPU.
            ANGLE_TRY(mImage->reformatStagedBufferUpdates(contextVk, previousActualFormatID,
                                                          actualFormatID));
        }
    }

    // Make sure we update mImageUsage bits
    const bool imageWasInitialized = mImage->valid();
    ANGLE_TRY(ensureImageAllocated(contextVk, format));
    ANGLE_TRY(respecifyImageStorage(contextVk));
    if (imageWasInitialized)
    {
        ANGLE_TRY(ensureImageInitialized(contextVk, ImageMipLevels::EnabledLevels));
        ANGLE_TRY(refreshImageViews(contextVk));
    }

    if (updateResultOut != nullptr)
    {
        *updateResultOut = TextureUpdateResult::ImageRespecified;
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::ensureRenderableIfCopyTextureCannotTransfer(
    ContextVk *contextVk,
    const gl::InternalFormat &dstFormat,
    bool unpackFlipY,
    bool unpackPremultiplyAlpha,
    bool unpackUnmultiplyAlpha,
    TextureVk *source)
{
    vk::Renderer *renderer = contextVk->getRenderer();

    VkImageTiling srcTilingMode   = source->getImage().getTilingMode();
    const vk::Format &dstVkFormat = renderer->getFormat(dstFormat.sizedInternalFormat);
    angle::FormatID dstFormatID   = dstVkFormat.getActualImageFormatID(getRequiredImageAccess());
    VkImageTiling dstTilingMode   = getTilingMode();

    if (!CanCopyWithTransferForCopyTexture(
            renderer, source->getImage(), srcTilingMode, dstVkFormat.getIntendedFormatID(),
            dstFormatID, dstTilingMode, unpackFlipY, unpackPremultiplyAlpha, unpackUnmultiplyAlpha))
    {
        ANGLE_TRY(ensureRenderableWithFormat(contextVk, dstVkFormat, nullptr));
    }

    return angle::Result::Continue;
}

angle::Result TextureVk::ensureRenderableIfCopyTexImageCannotTransfer(
    ContextVk *contextVk,
    const gl::InternalFormat &dstFormat,
    gl::Framebuffer *source)
{
    vk::Renderer *renderer       = contextVk->getRenderer();
    FramebufferVk *framebufferVk = vk::GetImpl(source);

    RenderTargetVk *colorReadRT = framebufferVk->getColorReadRenderTarget();

    angle::FormatID srcIntendedFormatID = colorReadRT->getImageIntendedFormatID();
    angle::FormatID srcActualFormatID   = colorReadRT->getImageActualFormatID();
    VkImageTiling srcTilingMode         = colorReadRT->getImageForCopy().getTilingMode();
    const vk::Format &dstVkFormat       = renderer->getFormat(dstFormat.sizedInternalFormat);
    angle::FormatID dstIntendedFormatID = dstVkFormat.getIntendedFormatID();
    angle::FormatID dstActualFormatID =
        dstVkFormat.getActualImageFormatID(getRequiredImageAccess());
    VkImageTiling destTilingMode = getTilingMode();

    bool isViewportFlipY = contextVk->isViewportFlipEnabledForReadFBO();

    if (!CanCopyWithTransferForTexImage(renderer, srcIntendedFormatID, srcActualFormatID,
                                        srcTilingMode, dstIntendedFormatID, dstActualFormatID,
                                        destTilingMode, isViewportFlipY))
    {
        ANGLE_TRY(ensureRenderableWithFormat(contextVk, dstVkFormat, nullptr));
    }

    return angle::Result::Continue;
}

void TextureVk::stageSelfAsSubresourceUpdates(ContextVk *contextVk)
{
    // If we are calling stageSelfAsSubresourceUpdates(), the current image will be swapped
    // to prevImage in stageSelfAsSubresourceUpdates(), therefore we need to release the
    // imageViews first as we want to use current image.mUse to keep track of imageViews' resource
    // lifetime.
    releaseImageViews(contextVk);
    // Make the image stage itself as updates to its levels.
    ASSERT(!mImageSiblingSerial.valid());
    mImage->stageSelfAsSubresourceUpdates(contextVk, mImage->getLevelCount(), mState.getType(),
                                          mRedefinedLevels);
}

void TextureVk::updateCachedImageViewSerials()
{
    mCachedImageViewSubresourceSerialSRGBDecode =
        getImageViewSubresourceSerialImpl(vk::ImageViewColorspace::SRGB);
    mCachedImageViewSubresourceSerialSkipDecode =
        getImageViewSubresourceSerialImpl(vk::ImageViewColorspace::Linear);
}
}  // namespace rx
