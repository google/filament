//
// Copyright 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureStorage11.cpp: Implements the abstract rx::TextureStorage11 class and its concrete derived
// classes TextureStorage11_2D and TextureStorage11_Cube, which act as the interface to the D3D11
// texture.

#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"

#include <tuple>

#include "common/MemoryBuffer.h"
#include "common/utilities.h"
#include "image_util/loadimage.h"
#include "libANGLE/Context.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/renderer/d3d/d3d11/Blit11.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/Context11.h"
#include "libANGLE/renderer/d3d/d3d11/Image11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/StreamProducerD3DTexture.h"
#include "libANGLE/renderer/d3d/d3d11/SwapChain11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"

namespace rx
{
TextureStorage11::SamplerKey::SamplerKey() : SamplerKey(0, 0, false, false, false) {}

TextureStorage11::SamplerKey::SamplerKey(int baseLevel,
                                         int mipLevels,
                                         bool swizzle,
                                         bool dropStencil,
                                         bool forceLinearSampler)
    : baseLevel(baseLevel),
      mipLevels(mipLevels),
      swizzle(swizzle),
      dropStencil(dropStencil),
      forceLinearSampler(forceLinearSampler)
{}

bool TextureStorage11::SamplerKey::operator<(const SamplerKey &rhs) const
{
    return std::tie(baseLevel, mipLevels, swizzle, dropStencil) <
           std::tie(rhs.baseLevel, rhs.mipLevels, rhs.swizzle, rhs.dropStencil);
}

TextureStorage11::ImageKey::ImageKey()
    : level(0), layered(false), layer(0), access(GL_READ_ONLY), format(GL_R32UI)
{}

TextureStorage11::ImageKey::ImageKey(int level,
                                     bool layered,
                                     int layer,
                                     GLenum access,
                                     GLenum format)
    : level(level), layered(layered), layer(layer), access(access), format(format)
{}

bool TextureStorage11::ImageKey::operator<(const ImageKey &rhs) const
{
    return std::tie(level, layered, layer, access, format) <
           std::tie(rhs.level, rhs.layered, rhs.layer, rhs.access, rhs.format);
}

MultisampledRenderToTextureInfo::MultisampledRenderToTextureInfo(const GLsizei samples,
                                                                 const gl::ImageIndex &indexSS,
                                                                 const gl::ImageIndex &indexMS)
    : samples(samples), indexSS(indexSS), indexMS(indexMS), msTextureNeedsResolve(false)
{}

MultisampledRenderToTextureInfo::~MultisampledRenderToTextureInfo() {}

TextureStorage11::TextureStorage11(Renderer11 *renderer,
                                   UINT bindFlags,
                                   UINT miscFlags,
                                   GLenum internalFormat,
                                   const std::string &label)
    : TextureStorage(label),
      mRenderer(renderer),
      mTopLevel(0),
      mMipLevels(0),
      mFormatInfo(d3d11::Format::Get(internalFormat, mRenderer->getRenderer11DeviceCaps())),
      mTextureWidth(0),
      mTextureHeight(0),
      mTextureDepth(0),
      mDropStencilTexture(),
      mBindFlags(bindFlags),
      mMiscFlags(miscFlags)
{}

TextureStorage11::~TextureStorage11()
{
    mSrvCacheForSampler.clear();
}

DWORD TextureStorage11::GetTextureBindFlags(GLenum internalFormat,
                                            const Renderer11DeviceCaps &renderer11DeviceCaps,
                                            BindFlags flags)
{
    UINT bindFlags = 0;

    const d3d11::Format &formatInfo = d3d11::Format::Get(internalFormat, renderer11DeviceCaps);
    if (formatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (formatInfo.uavFormat != DXGI_FORMAT_UNKNOWN && flags.unorderedAccess)
    {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
    if (formatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }
    if (formatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN && flags.renderTarget)
    {
        bindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    return bindFlags;
}

DWORD TextureStorage11::GetTextureMiscFlags(GLenum internalFormat,
                                            const Renderer11DeviceCaps &renderer11DeviceCaps,
                                            BindFlags bindFlags,
                                            int levels)
{
    UINT miscFlags = 0;

    const d3d11::Format &formatInfo = d3d11::Format::Get(internalFormat, renderer11DeviceCaps);
    if (bindFlags.renderTarget)
    {
        if (d3d11::SupportsMipGen(formatInfo.texFormat, renderer11DeviceCaps.featureLevel))
        {
            miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
    }

    return miscFlags;
}

UINT TextureStorage11::getBindFlags() const
{
    return mBindFlags;
}

UINT TextureStorage11::getMiscFlags() const
{
    return mMiscFlags;
}

int TextureStorage11::getTopLevel() const
{
    // Applying top level is meant to be encapsulated inside TextureStorage11.
    UNREACHABLE();
    return mTopLevel;
}

bool TextureStorage11::isRenderTarget() const
{
    return (mBindFlags & (D3D11_BIND_RENDER_TARGET | D3D11_BIND_DEPTH_STENCIL)) != 0;
}

bool TextureStorage11::isManaged() const
{
    return false;
}

bool TextureStorage11::supportsNativeMipmapFunction() const
{
    return (mMiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS) != 0;
}

int TextureStorage11::getLevelCount() const
{
    return mMipLevels - mTopLevel;
}

int TextureStorage11::getLevelWidth(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureWidth) >> mipLevel, 1);
}

int TextureStorage11::getLevelHeight(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureHeight) >> mipLevel, 1);
}

int TextureStorage11::getLevelDepth(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureDepth) >> mipLevel, 1);
}

bool TextureStorage11::isMultiplanar(const gl::Context *context)
{
    const TextureHelper11 *dstTexture = nullptr;
    if (getResource(context, &dstTexture) == angle::Result::Continue)
    {
        DXGI_FORMAT format = dstTexture->getFormat();
        return format == DXGI_FORMAT_NV12 || format == DXGI_FORMAT_P010 ||
               format == DXGI_FORMAT_P016;
    }
    return false;
}

angle::Result TextureStorage11::getMippedResource(const gl::Context *context,
                                                  const TextureHelper11 **outResource)
{
    return getResource(context, outResource);
}

angle::Result TextureStorage11::getSubresourceIndex(const gl::Context *context,
                                                    const gl::ImageIndex &index,
                                                    UINT *outSubresourceIndex) const
{
    UINT mipSlice = static_cast<UINT>(index.getLevelIndex() + mTopLevel);
    // D3D11CalcSubresource reference: always use 0 for volume (3D) textures
    UINT arraySlice = static_cast<UINT>(
        (index.hasLayer() && index.getType() != gl::TextureType::_3D) ? index.getLayerIndex() : 0);
    UINT subresource = D3D11CalcSubresource(mipSlice, arraySlice, mMipLevels);
    ASSERT(subresource != std::numeric_limits<UINT>::max());
    *outSubresourceIndex = subresource;
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getSRVForSampler(const gl::Context *context,
                                                 const gl::TextureState &textureState,
                                                 const gl::SamplerState &sampler,
                                                 const d3d11::SharedSRV **outSRV)
{
    ANGLE_TRY(resolveTexture(context));
    // Make sure to add the level offset for our tiny compressed texture workaround
    const GLuint effectiveBaseLevel = textureState.getEffectiveBaseLevel();
    const bool swizzleRequired      = SwizzleRequired(textureState);
    const bool mipmapping           = gl::IsMipmapFiltered(sampler.getMinFilter());
    unsigned int mipLevels =
        mipmapping ? (textureState.getEffectiveMaxLevel() - effectiveBaseLevel + 1) : 1;

    // Make sure there's 'mipLevels' mipmap levels below the base level (offset by the top level,
    // which corresponds to GL level 0)
    mipLevels = std::min(mipLevels, mMipLevels - mTopLevel - effectiveBaseLevel);

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        ASSERT(!swizzleRequired);
        ASSERT(mipLevels == 1 || mipLevels == mMipLevels);
    }

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        // We must ensure that the level zero texture is in sync with mipped texture.
        ANGLE_TRY(useLevelZeroWorkaroundTexture(context, mipLevels == 1));
    }

    if (swizzleRequired)
    {
        verifySwizzleExists(GetEffectiveSwizzle(textureState));
    }

    // We drop the stencil when sampling from the SRV if three conditions hold:
    // 1. the drop stencil workaround is enabled.
    const bool emulateTinyStencilTextures =
        mRenderer->getFeatures().emulateTinyStencilTextures.enabled;
    // 2. this is a stencil texture.
    const bool hasStencil = (mFormatInfo.format().stencilBits > 0);
    // 3. the texture has a 1x1 or 2x2 mip.
    const int effectiveTopLevel = effectiveBaseLevel + mipLevels - 1;
    const bool hasSmallMips =
        (getLevelWidth(effectiveTopLevel) <= 2 || getLevelHeight(effectiveTopLevel) <= 2);

    const bool useDropStencil = (emulateTinyStencilTextures && hasStencil && hasSmallMips);
    const bool forceLinearSampler =
        false;  // If supporting non-default sRGB Decode, this is where it will be passed.
    const SamplerKey key(effectiveBaseLevel, mipLevels, swizzleRequired, useDropStencil,
                         forceLinearSampler);
    if (useDropStencil)
    {
        // Ensure drop texture gets created.
        DropStencil result = DropStencil::CREATED;
        ANGLE_TRY(ensureDropStencilTexture(context, &result));

        // Clear the SRV cache if necessary.
        // TODO(jmadill): Re-use find query result.
        const auto srvEntry = mSrvCacheForSampler.find(key);
        if (result == DropStencil::CREATED && srvEntry != mSrvCacheForSampler.end())
        {
            mSrvCacheForSampler.erase(key);
        }
    }

    ANGLE_TRY(getCachedOrCreateSRVForSampler(context, key, outSRV));

    return angle::Result::Continue;
}

angle::Result TextureStorage11::getCachedOrCreateSRVForSampler(const gl::Context *context,
                                                               const SamplerKey &key,
                                                               const d3d11::SharedSRV **outSRV)
{
    auto iter = mSrvCacheForSampler.find(key);
    if (iter != mSrvCacheForSampler.end())
    {
        *outSRV = &iter->second;
        return angle::Result::Continue;
    }

    const TextureHelper11 *texture = nullptr;
    DXGI_FORMAT format             = DXGI_FORMAT_UNKNOWN;

    if (key.swizzle)
    {
        const auto &swizzleFormat =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());
        ASSERT(!key.dropStencil || swizzleFormat.format().stencilBits == 0);
        ANGLE_TRY(getSwizzleTexture(context, &texture));
        format = swizzleFormat.srvFormat;
    }
    else if (key.dropStencil)
    {
        ASSERT(mDropStencilTexture.valid());
        texture = &mDropStencilTexture;
        format  = DXGI_FORMAT_R32_FLOAT;
    }
    else if (key.forceLinearSampler)
    {
        ANGLE_TRY(getResource(context, &texture));
        if (mFormatInfo.linearSRVFormat != DXGI_FORMAT_UNKNOWN)
        {
            ASSERT(requiresTypelessTextureFormat());
            format = mFormatInfo.linearSRVFormat;
        }
        else
        {
            format = mFormatInfo.srvFormat;
        }
    }
    else
    {
        ANGLE_TRY(getResource(context, &texture));
        format = mFormatInfo.srvFormat;
    }

    d3d11::SharedSRV srv;

    ANGLE_TRY(createSRVForSampler(context, key.baseLevel, key.mipLevels, format, *texture, &srv));

    const auto &insertIt = mSrvCacheForSampler.insert(std::make_pair(key, std::move(srv)));
    *outSRV              = &insertIt.first->second;

    return angle::Result::Continue;
}

angle::Result TextureStorage11::getSRVLevel(const gl::Context *context,
                                            int mipLevel,
                                            SRVType srvType,
                                            const d3d11::SharedSRV **outSRV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    ANGLE_TRY(resolveTexture(context));
    if (srvType == SRVType::Stencil)
    {
        if (!mLevelStencilSRVs[mipLevel].valid())
        {
            const TextureHelper11 *resource = nullptr;
            ANGLE_TRY(getResource(context, &resource));

            ANGLE_TRY(createSRVForSampler(context, mipLevel, 1, mFormatInfo.stencilSRVFormat,
                                          *resource, &mLevelStencilSRVs[mipLevel]));
        }
        *outSRV = &mLevelStencilSRVs[mipLevel];
        return angle::Result::Continue;
    }

    auto &levelSRVs      = srvType == SRVType::Blit ? mLevelBlitSRVs : mLevelSRVs;
    auto &otherLevelSRVs = srvType == SRVType::Blit ? mLevelSRVs : mLevelBlitSRVs;

    if (!levelSRVs[mipLevel].valid())
    {
        // Only create a different SRV for blit if blit format is different from regular srv format
        if (otherLevelSRVs[mipLevel].valid() && mFormatInfo.srvFormat == mFormatInfo.blitSRVFormat)
        {
            levelSRVs[mipLevel] = otherLevelSRVs[mipLevel].makeCopy();
        }
        else
        {
            const TextureHelper11 *resource = nullptr;
            ANGLE_TRY(getResource(context, &resource));

            DXGI_FORMAT resourceFormat =
                srvType == SRVType::Blit ? mFormatInfo.blitSRVFormat : mFormatInfo.srvFormat;
            ANGLE_TRY(createSRVForSampler(context, mipLevel, 1, resourceFormat, *resource,
                                          &levelSRVs[mipLevel]));
        }
    }

    *outSRV = &levelSRVs[mipLevel];
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getSRVLevels(const gl::Context *context,
                                             GLint baseLevel,
                                             GLint maxLevel,
                                             bool forceLinearSampler,
                                             const d3d11::SharedSRV **outSRV)
{
    ANGLE_TRY(resolveTexture(context));
    unsigned int mipLevels = maxLevel - baseLevel + 1;

    // Make sure there's 'mipLevels' mipmap levels below the base level (offset by the top level,
    // which corresponds to GL level 0)
    mipLevels = std::min(mipLevels, mMipLevels - mTopLevel - baseLevel);

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        ASSERT(mipLevels == 1 || mipLevels == mMipLevels);
    }

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        // We must ensure that the level zero texture is in sync with mipped texture.
        ANGLE_TRY(useLevelZeroWorkaroundTexture(context, mipLevels == 1));
    }

    // TODO(jmadill): Assert we don't need to drop stencil.

    SamplerKey key(baseLevel, mipLevels, false, false, forceLinearSampler);
    ANGLE_TRY(getCachedOrCreateSRVForSampler(context, key, outSRV));

    return angle::Result::Continue;
}

angle::Result TextureStorage11::getSRVForImage(const gl::Context *context,
                                               const gl::ImageUnit &imageUnit,
                                               const d3d11::SharedSRV **outSRV)
{
    ANGLE_TRY(resolveTexture(context));
    // TODO(Xinghua.cao@intel.com): Add solution to handle swizzle required.
    ImageKey key(imageUnit.level, (imageUnit.layered == GL_TRUE), imageUnit.layer, imageUnit.access,
                 imageUnit.format);
    ANGLE_TRY(getCachedOrCreateSRVForImage(context, key, outSRV));
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getCachedOrCreateSRVForImage(const gl::Context *context,
                                                             const ImageKey &key,
                                                             const d3d11::SharedSRV **outSRV)
{
    auto iter = mSrvCacheForImage.find(key);
    if (iter != mSrvCacheForImage.end())
    {
        *outSRV = &iter->second;
        return angle::Result::Continue;
    }
    const TextureHelper11 *texture = nullptr;
    ANGLE_TRY(getResource(context, &texture));
    DXGI_FORMAT format =
        d3d11::Format::Get(key.format, mRenderer->getRenderer11DeviceCaps()).srvFormat;
    d3d11::SharedSRV srv;
    ANGLE_TRY(createSRVForImage(context, key.level, format, *texture, &srv));
    const auto &insertIt = mSrvCacheForImage.insert(std::make_pair(key, std::move(srv)));
    *outSRV              = &insertIt.first->second;
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getUAVForImage(const gl::Context *context,
                                               const gl::ImageUnit &imageUnit,
                                               const d3d11::SharedUAV **outUAV)
{
    ANGLE_TRY(resolveTexture(context));
    // TODO(Xinghua.cao@intel.com): Add solution to handle swizzle required.
    ImageKey key(imageUnit.level, (imageUnit.layered == GL_TRUE), imageUnit.layer, imageUnit.access,
                 imageUnit.format);
    ANGLE_TRY(getCachedOrCreateUAVForImage(context, key, outUAV));
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getCachedOrCreateUAVForImage(const gl::Context *context,
                                                             const ImageKey &key,
                                                             const d3d11::SharedUAV **outUAV)
{
    auto iter = mUavCacheForImage.find(key);
    if (iter != mUavCacheForImage.end())
    {
        *outUAV = &iter->second;
        return angle::Result::Continue;
    }
    const TextureHelper11 *texture = nullptr;
    ANGLE_TRY(getResource(context, &texture));
    DXGI_FORMAT format =
        d3d11::Format::Get(key.format, mRenderer->getRenderer11DeviceCaps()).uavFormat;
    ASSERT(format != DXGI_FORMAT_UNKNOWN);
    d3d11::SharedUAV uav;
    ANGLE_TRY(createUAVForImage(context, key.level, format, *texture, &uav));
    const auto &insertIt = mUavCacheForImage.insert(std::make_pair(key, std::move(uav)));
    *outUAV              = &insertIt.first->second;
    return angle::Result::Continue;
}

const d3d11::Format &TextureStorage11::getFormatSet() const
{
    return mFormatInfo;
}

angle::Result TextureStorage11::generateSwizzles(const gl::Context *context,
                                                 const gl::TextureState &textureState)
{
    ANGLE_TRY(resolveTexture(context));
    gl::SwizzleState swizzleTarget = GetEffectiveSwizzle(textureState);
    for (int level = 0; level < getLevelCount(); level++)
    {
        // Check if the swizzle for this level is out of date
        if (mSwizzleCache[level] != swizzleTarget)
        {
            // Need to re-render the swizzle for this level
            const d3d11::SharedSRV *sourceSRV = nullptr;
            ANGLE_TRY(getSRVLevel(context, level,
                                  textureState.isStencilMode() ? SRVType::Stencil : SRVType::Blit,
                                  &sourceSRV));

            const d3d11::RenderTargetView *destRTV;
            ANGLE_TRY(getSwizzleRenderTarget(context, level, &destRTV));

            gl::Extents size(getLevelWidth(level), getLevelHeight(level), getLevelDepth(level));

            Blit11 *blitter = mRenderer->getBlitter();

            ANGLE_TRY(blitter->swizzleTexture(context, *sourceSRV, *destRTV, size, swizzleTarget));

            mSwizzleCache[level] = swizzleTarget;
        }
    }

    return angle::Result::Continue;
}

void TextureStorage11::markLevelDirty(int mipLevel)
{
    if (mipLevel >= 0 && static_cast<size_t>(mipLevel) < mSwizzleCache.size())
    {
        // The default constructor of SwizzleState has GL_INVALID_INDEX for all channels which is
        // not a valid swizzle combination
        if (mSwizzleCache[mipLevel] != gl::SwizzleState())
        {
            // TODO(jmadill): Invalidate specific swizzle.
            mRenderer->getStateManager()->invalidateSwizzles();
            mSwizzleCache[mipLevel] = gl::SwizzleState();
        }
    }

    if (mDropStencilTexture.valid())
    {
        mDropStencilTexture.reset();
    }
}

void TextureStorage11::markDirty()
{
    for (size_t mipLevel = 0; mipLevel < mSwizzleCache.size(); ++mipLevel)
    {
        markLevelDirty(static_cast<int>(mipLevel));
    }
}

angle::Result TextureStorage11::updateSubresourceLevel(const gl::Context *context,
                                                       const TextureHelper11 &srcTexture,
                                                       unsigned int sourceSubresource,
                                                       const gl::ImageIndex &index,
                                                       const gl::Box &copyArea)
{
    ASSERT(srcTexture.valid());
    ANGLE_TRY(resolveTexture(context));
    const GLint level = index.getLevelIndex();

    markLevelDirty(level);

    gl::Extents texSize(getLevelWidth(level), getLevelHeight(level), getLevelDepth(level));

    bool fullCopy = copyArea.coversSameExtent(texSize);

    const TextureHelper11 *dstTexture = nullptr;

    // If the zero-LOD workaround is active and we want to update a level greater than zero,
    // then we should update the mipmapped texture, even if mapmaps are currently disabled.
    if (level > 0 && mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ANGLE_TRY(getMippedResource(context, &dstTexture));
    }
    else
    {
        ANGLE_TRY(getResource(context, &dstTexture));
    }

    unsigned int dstSubresource = 0;
    ANGLE_TRY(getSubresourceIndex(context, index, &dstSubresource));

    ASSERT(dstTexture->valid());

    const d3d11::DXGIFormatSize &dxgiFormatSizeInfo =
        d3d11::GetDXGIFormatSizeInfo(mFormatInfo.texFormat);
    if (!fullCopy && mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        // CopySubresourceRegion cannot copy partial depth stencils, use the blitter instead
        Blit11 *blitter = mRenderer->getBlitter();
        return blitter->copyDepthStencil(context, srcTexture, sourceSubresource, copyArea, texSize,
                                         *dstTexture, dstSubresource, copyArea, texSize, nullptr);
    }

    D3D11_BOX srcBox;
    srcBox.left = copyArea.x;
    srcBox.top  = copyArea.y;
    srcBox.right =
        copyArea.x + roundUp(static_cast<UINT>(copyArea.width), dxgiFormatSizeInfo.blockWidth);
    srcBox.bottom =
        copyArea.y + roundUp(static_cast<UINT>(copyArea.height), dxgiFormatSizeInfo.blockHeight);
    srcBox.front = copyArea.z;
    srcBox.back  = copyArea.z + copyArea.depth;

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    if (d3d11::IsSupportedMultiplanarFormat(dstTexture->getFormat()))
    {
        ASSERT(dstSubresource == 0);
        if (dstSubresource != 0)
        {
            return angle::Result::Stop;
        }
        // Intermediate texture used for copy for multiplanar formats.
        TextureHelper11 intermediateTextureHelper;

        D3D11_TEXTURE2D_DESC planeDesc;
        planeDesc.Width              = static_cast<UINT>(copyArea.width);
        planeDesc.Height             = static_cast<UINT>(copyArea.height);
        planeDesc.MipLevels          = 1;
        planeDesc.ArraySize          = 1;
        planeDesc.Format             = srcTexture.getFormatSet().srvFormat;
        planeDesc.SampleDesc.Count   = 1;
        planeDesc.SampleDesc.Quality = 0;
        planeDesc.Usage              = D3D11_USAGE_DEFAULT;
        planeDesc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        planeDesc.CPUAccessFlags     = 0;
        planeDesc.MiscFlags          = 0;

        GLenum internalFormat = srcTexture.getFormatSet().internalFormat;

        // Allocate intermediate texture and copy srcTexture into it.
        ANGLE_TRY(mRenderer->allocateTexture(
            GetImplAs<Context11>(context), planeDesc,
            d3d11::Format::Get(internalFormat, mRenderer->getRenderer11DeviceCaps()),
            &intermediateTextureHelper));
        intermediateTextureHelper.setInternalName(
            "updateSubresourceLevel::intermediateTextureHelper");

        // Intermediate texture has offsets 0.
        deviceContext->CopySubresourceRegion(intermediateTextureHelper.get(), 0, 0, 0, 0,
                                             srcTexture.get(), sourceSubresource,
                                             fullCopy ? nullptr : &srcBox);

        Context11 *context11 = GetImplAs<Context11>(context);
        d3d11::RenderTargetView rtv;
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format             = srcTexture.getFormatSet().rtvFormat;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;

        ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, dstTexture->get(), &rtv));
        rtv.setInternalName("updateSubresourceLevel.RTV");

        d3d11::SharedSRV srv;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format                    = srcTexture.getFormatSet().srvFormat;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels       = 1;

        ANGLE_TRY(
            mRenderer->allocateResource(context11, srvDesc, intermediateTextureHelper.get(), &srv));
        srv.setInternalName("updateSubresourceLevel.SRV");

        // Intermediate texture has 0 offsets.
        gl::Box intermediateGlBox(0, 0, 0, copyArea.width, copyArea.height, 1);
        // Destination texture has offsets similar to that of source texture.
        gl::Box destGlBox(copyArea.x, copyArea.y, copyArea.z, copyArea.width, copyArea.height, 1);
        gl::Extents srcSize(copyArea.width, copyArea.height, 1);
        gl::Extents dstSize(texSize.width, texSize.height, 1);

        // Perform a copy to dstTexture from intermediate as we cannot copy directly to NV12 d3d11
        // textures.
        Blit11 *blitter = mRenderer->getBlitter();
        ANGLE_TRY(blitter->copyTexture(context, srv, intermediateGlBox, srcSize, internalFormat,
                                       rtv, destGlBox, dstSize, nullptr,
                                       gl::GetUnsizedFormat(internalFormat), GL_NONE, GL_NEAREST,
                                       false, false, false));
    }
    else
    {
        deviceContext->CopySubresourceRegion(dstTexture->get(), dstSubresource, copyArea.x,
                                             copyArea.y, copyArea.z, srcTexture.get(),
                                             sourceSubresource, fullCopy ? nullptr : &srcBox);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11::copySubresourceLevel(const gl::Context *context,
                                                     const TextureHelper11 &dstTexture,
                                                     unsigned int dstSubresource,
                                                     const gl::ImageIndex &index,
                                                     const gl::Box &region)
{
    ASSERT(dstTexture.valid());

    ANGLE_TRY(resolveTexture(context));
    const TextureHelper11 *srcTexture = nullptr;

    // If the zero-LOD workaround is active and we want to update a level greater than zero, then we
    // should update the mipmapped texture, even if mapmaps are currently disabled.
    if (index.getLevelIndex() > 0 && mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ANGLE_TRY(getMippedResource(context, &srcTexture));
    }
    else
    {
        ANGLE_TRY(getResource(context, &srcTexture));
    }

    ASSERT(srcTexture->valid());

    unsigned int srcSubresource = 0;
    ANGLE_TRY(getSubresourceIndex(context, index, &srcSubresource));

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // D3D11 can't perform partial CopySubresourceRegion on depth/stencil textures, so pSrcBox
    // should be nullptr.
    D3D11_BOX srcBox;
    D3D11_BOX *pSrcBox = nullptr;
    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        GLsizei width  = region.width;
        GLsizei height = region.height;
        d3d11::MakeValidSize(false, mFormatInfo.texFormat, &width, &height, nullptr);

        // Keep srcbox as nullptr if we're dealing with tiny mips of compressed textures.
        if (width == region.width && height == region.height)
        {
            // However, D3D10Level9 doesn't always perform CopySubresourceRegion correctly unless
            // the source box is specified. This is okay, since we don't perform
            // CopySubresourceRegion on depth/stencil textures on 9_3.
            ASSERT(mFormatInfo.dsvFormat == DXGI_FORMAT_UNKNOWN);
            srcBox.left   = region.x;
            srcBox.right  = region.x + region.width;
            srcBox.top    = region.y;
            srcBox.bottom = region.y + region.height;
            srcBox.front  = region.z;
            srcBox.back   = region.z + region.depth;
            pSrcBox       = &srcBox;
        }
    }

    deviceContext->CopySubresourceRegion(dstTexture.get(), dstSubresource, region.x, region.y,
                                         region.z, srcTexture->get(), srcSubresource, pSrcBox);

    return angle::Result::Continue;
}

bool TextureStorage11::requiresTypelessTextureFormat() const
{
    return isUnorderedAccess() || (mFormatInfo.typelessFormat != DXGI_FORMAT_UNKNOWN &&
                                   mFormatInfo.linearSRVFormat != DXGI_FORMAT_UNKNOWN);
}

angle::Result TextureStorage11::generateMipmap(const gl::Context *context,
                                               const gl::ImageIndex &sourceIndex,
                                               const gl::ImageIndex &destIndex)
{
    ASSERT(sourceIndex.getLayerIndex() == destIndex.getLayerIndex());

    ANGLE_TRY(resolveTexture(context));
    markLevelDirty(destIndex.getLevelIndex());

    RenderTargetD3D *source = nullptr;
    ANGLE_TRY(getRenderTarget(context, sourceIndex, 0, &source));

    // dest will always have 0 since, we have just released the MS Texture struct
    RenderTargetD3D *dest = nullptr;
    ANGLE_TRY(getRenderTarget(context, destIndex, 0, &dest));

    RenderTarget11 *srcRT11                = GetAs<RenderTarget11>(source);
    RenderTarget11 *dstRT11                = GetAs<RenderTarget11>(dest);
    const d3d11::RenderTargetView &destRTV = dstRT11->getRenderTargetView();
    const d3d11::SharedSRV *sourceSRV;
    ANGLE_TRY(srcRT11->getBlitShaderResourceView(context, &sourceSRV));

    gl::Box sourceArea(0, 0, 0, source->getWidth(), source->getHeight(), source->getDepth());
    gl::Extents sourceSize(source->getWidth(), source->getHeight(), source->getDepth());

    gl::Box destArea(0, 0, 0, dest->getWidth(), dest->getHeight(), dest->getDepth());
    gl::Extents destSize(dest->getWidth(), dest->getHeight(), dest->getDepth());

    Blit11 *blitter = mRenderer->getBlitter();
    const gl::InternalFormat &sourceInternalFormat =
        gl::GetSizedInternalFormatInfo(source->getInternalFormat());
    GLenum format = sourceInternalFormat.format;
    GLenum type   = sourceInternalFormat.type;
    return blitter->copyTexture(context, *sourceSRV, sourceArea, sourceSize, format, destRTV,
                                destArea, destSize, nullptr, format, type, GL_LINEAR, false, false,
                                false);
}

void TextureStorage11::verifySwizzleExists(const gl::SwizzleState &swizzleState)
{
    for (unsigned int level = 0; level < mMipLevels; level++)
    {
        ASSERT(mSwizzleCache[level] == swizzleState);
    }
}

void TextureStorage11::clearSRVCache()
{
    markDirty();
    mSrvCacheForSampler.clear();

    for (size_t level = 0; level < mLevelSRVs.size(); level++)
    {
        mLevelSRVs[level].reset();
        mLevelBlitSRVs[level].reset();
    }
}

angle::Result TextureStorage11::copyToStorage(const gl::Context *context,
                                              TextureStorage *destStorage)
{
    ASSERT(destStorage);

    ANGLE_TRY(resolveTexture(context));
    const TextureHelper11 *sourceResouce = nullptr;
    ANGLE_TRY(getResource(context, &sourceResouce));

    TextureStorage11 *dest11            = GetAs<TextureStorage11>(destStorage);
    const TextureHelper11 *destResource = nullptr;
    ANGLE_TRY(dest11->getResource(context, &destResource));

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
    immediateContext->CopyResource(destResource->get(), sourceResouce->get());

    dest11->markDirty();

    return angle::Result::Continue;
}

void TextureStorage11::invalidateTextures()
{
    mRenderer->getStateManager()->invalidateTexturesAndSamplers();
}

angle::Result TextureStorage11::setData(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        ImageD3D *image,
                                        const gl::Box *destBox,
                                        GLenum type,
                                        const gl::PixelUnpackState &unpack,
                                        const uint8_t *pixelData)
{
    ASSERT(!image->isDirty());

    ANGLE_TRY(resolveTexture(context));
    markLevelDirty(index.getLevelIndex());

    const TextureHelper11 *resource = nullptr;
    ANGLE_TRY(getResource(context, &resource));
    ASSERT(resource && resource->valid());

    UINT destSubresource = 0;
    ANGLE_TRY(getSubresourceIndex(context, index, &destSubresource));

    const gl::InternalFormat &internalFormatInfo =
        gl::GetInternalFormatInfo(image->getInternalFormat(), type);

    gl::Box levelBox(0, 0, 0, getLevelWidth(index.getLevelIndex()),
                     getLevelHeight(index.getLevelIndex()), getLevelDepth(index.getLevelIndex()));
    bool fullUpdate = (destBox == nullptr || *destBox == levelBox);
    ASSERT(internalFormatInfo.depthBits == 0 || fullUpdate);

    // TODO(jmadill): Handle compressed formats
    // Compressed formats have different load syntax, so we'll have to handle them with slightly
    // different logic. Will implemnent this in a follow-up patch, and ensure we do not use SetData
    // with compressed formats in the calling logic.
    ASSERT(!internalFormatInfo.compressed);

    Context11 *context11 = GetImplAs<Context11>(context);

    const int width    = destBox ? destBox->width : static_cast<int>(image->getWidth());
    const int height   = destBox ? destBox->height : static_cast<int>(image->getHeight());
    const int depth    = destBox ? destBox->depth : static_cast<int>(image->getDepth());
    GLuint srcRowPitch = 0;
    ANGLE_CHECK_GL_MATH(context11,
                        internalFormatInfo.computeRowPitch(type, width, unpack.alignment,
                                                           unpack.rowLength, &srcRowPitch));
    GLuint srcDepthPitch = 0;
    ANGLE_CHECK_GL_MATH(context11, internalFormatInfo.computeDepthPitch(
                                       height, unpack.imageHeight, srcRowPitch, &srcDepthPitch));
    GLuint srcSkipBytes = 0;
    ANGLE_CHECK_GL_MATH(
        context11, internalFormatInfo.computeSkipBytes(type, srcRowPitch, srcDepthPitch, unpack,
                                                       index.usesTex3D(), &srcSkipBytes));

    const d3d11::Format &d3d11Format =
        d3d11::Format::Get(image->getInternalFormat(), mRenderer->getRenderer11DeviceCaps());
    const d3d11::DXGIFormatSize &dxgiFormatInfo =
        d3d11::GetDXGIFormatSizeInfo(d3d11Format.texFormat);

    const size_t outputPixelSize = dxgiFormatInfo.pixelBytes;

    UINT bufferRowPitch   = static_cast<unsigned int>(outputPixelSize) * width;
    UINT bufferDepthPitch = bufferRowPitch * height;

    const size_t neededSize               = bufferDepthPitch * depth;
    angle::MemoryBuffer *conversionBuffer = nullptr;
    const uint8_t *data                   = nullptr;

    LoadImageFunctionInfo loadFunctionInfo = d3d11Format.getLoadFunctions()(type);
    if (loadFunctionInfo.requiresConversion)
    {
        ANGLE_TRY(mRenderer->getScratchMemoryBuffer(context11, neededSize, &conversionBuffer));
        loadFunctionInfo.loadFunction(mRenderer->getDisplay()->getImageLoadContext(), width, height,
                                      depth, pixelData + srcSkipBytes, srcRowPitch, srcDepthPitch,
                                      conversionBuffer->data(), bufferRowPitch, bufferDepthPitch);
        data = conversionBuffer->data();
    }
    else
    {
        data             = pixelData + srcSkipBytes;
        bufferRowPitch   = srcRowPitch;
        bufferDepthPitch = srcDepthPitch;
    }

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

    if (!fullUpdate)
    {
        ASSERT(destBox);

        D3D11_BOX destD3DBox;
        destD3DBox.left   = destBox->x;
        destD3DBox.right  = destBox->x + destBox->width;
        destD3DBox.top    = destBox->y;
        destD3DBox.bottom = destBox->y + destBox->height;
        destD3DBox.front  = destBox->z;
        destD3DBox.back   = destBox->z + destBox->depth;

        immediateContext->UpdateSubresource(resource->get(), destSubresource, &destD3DBox, data,
                                            bufferRowPitch, bufferDepthPitch);
    }
    else
    {
        immediateContext->UpdateSubresource(resource->get(), destSubresource, nullptr, data,
                                            bufferRowPitch, bufferDepthPitch);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11::ensureDropStencilTexture(
    const gl::Context *context,
    TextureStorage11::DropStencil *dropStencilOut)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11::initDropStencilTexture(const gl::Context *context,
                                                       const gl::ImageIndexIterator &it)
{
    const TextureHelper11 *sourceTexture = nullptr;
    ANGLE_TRY(getResource(context, &sourceTexture));

    gl::ImageIndexIterator itCopy = it;

    while (itCopy.hasNext())
    {
        gl::ImageIndex index = itCopy.next();
        gl::Box wholeArea(0, 0, 0, getLevelWidth(index.getLevelIndex()),
                          getLevelHeight(index.getLevelIndex()), 1);
        gl::Extents wholeSize(wholeArea.width, wholeArea.height, 1);

        UINT subresource = 0;
        ANGLE_TRY(getSubresourceIndex(context, index, &subresource));

        ANGLE_TRY(mRenderer->getBlitter()->copyDepthStencil(
            context, *sourceTexture, subresource, wholeArea, wholeSize, mDropStencilTexture,
            subresource, wholeArea, wholeSize, nullptr));
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11::resolveTextureHelper(const gl::Context *context,
                                                     const TextureHelper11 &texture)
{
    UINT subresourceIndexSS;
    ANGLE_TRY(getSubresourceIndex(context, mMSTexInfo->indexSS, &subresourceIndexSS));
    UINT subresourceIndexMS;
    ANGLE_TRY(getSubresourceIndex(context, mMSTexInfo->indexMS, &subresourceIndexMS));
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    const TextureHelper11 *resource    = nullptr;
    ANGLE_TRY(mMSTexInfo->msTex->getResource(context, &resource));
    deviceContext->ResolveSubresource(texture.get(), subresourceIndexSS, resource->get(),
                                      subresourceIndexMS, texture.getFormat());
    mMSTexInfo->msTextureNeedsResolve = false;
    return angle::Result::Continue;
}

angle::Result TextureStorage11::releaseMultisampledTexStorageForLevel(size_t level)
{
    if (mMSTexInfo && mMSTexInfo->indexSS.getLevelIndex() == static_cast<int>(level))
    {
        mMSTexInfo->msTex.reset();
        onStateChange(angle::SubjectMessage::ContentsChanged);
    }
    return angle::Result::Continue;
}

GLsizei TextureStorage11::getRenderToTextureSamples() const
{
    if (mMSTexInfo)
    {
        return mMSTexInfo->samples;
    }
    return 0;
}

angle::Result TextureStorage11::findMultisampledRenderTarget(const gl::Context *context,
                                                             const gl::ImageIndex &index,
                                                             GLsizei samples,
                                                             RenderTargetD3D **outRT) const
{
    const int level = index.getLevelIndex();
    if (!mMSTexInfo || level != mMSTexInfo->indexSS.getLevelIndex() ||
        samples != mMSTexInfo->samples || !mMSTexInfo->msTex)
    {
        *outRT = nullptr;
        return angle::Result::Continue;
    }
    RenderTargetD3D *rt;
    ANGLE_TRY(mMSTexInfo->msTex->findRenderTarget(context, mMSTexInfo->indexMS, samples, &rt));
    *outRT = rt;
    return angle::Result::Continue;
}

angle::Result TextureStorage11::getMultisampledRenderTarget(const gl::Context *context,
                                                            const gl::ImageIndex &index,
                                                            GLsizei samples,
                                                            RenderTargetD3D **outRT)
{
    const int level = index.getLevelIndex();
    if (!mMSTexInfo || level != mMSTexInfo->indexSS.getLevelIndex() ||
        samples != mMSTexInfo->samples || !mMSTexInfo->msTex)
    {
        // if mMSTexInfo already exists, then we want to resolve and release it
        // since the mMSTexInfo must be for a different sample count or level
        ANGLE_TRY(resolveTexture(context));

        // Now we can create a new object for the correct sample and level
        GLsizei width         = getLevelWidth(level);
        GLsizei height        = getLevelHeight(level);
        GLenum internalFormat = mFormatInfo.internalFormat;
        std::unique_ptr<TextureStorage11_2DMultisample> texMS(
            GetAs<TextureStorage11_2DMultisample>(mRenderer->createTextureStorage2DMultisample(
                internalFormat, width, height, level, samples, true, mKHRDebugLabel)));

        // make sure multisample object has the blitted information.
        gl::Rectangle area(0, 0, width, height);
        RenderTargetD3D *readRenderTarget = nullptr;
        // use incoming index here since the index will correspond to the single sampled texture
        ANGLE_TRY(getRenderTarget(context, index, 0, &readRenderTarget));
        gl::ImageIndex indexMS            = gl::ImageIndex::Make2DMultisample();
        RenderTargetD3D *drawRenderTarget = nullptr;
        ANGLE_TRY(texMS->getRenderTarget(context, indexMS, samples, &drawRenderTarget));

        // blit SS -> MS
        // mask: GL_COLOR_BUFFER_BIT, filter: GL_NEAREST
        ANGLE_TRY(mRenderer->blitRenderbufferRect(context, area, area, 0, 0, readRenderTarget,
                                                  drawRenderTarget, GL_NEAREST, nullptr, true,
                                                  false, false));
        mMSTexInfo = std::make_unique<MultisampledRenderToTextureInfo>(samples, index, indexMS);
        mMSTexInfo->msTex = std::move(texMS);
    }
    RenderTargetD3D *rt;
    ANGLE_TRY(mMSTexInfo->msTex->getRenderTarget(context, mMSTexInfo->indexMS, samples, &rt));
    // By returning the multisampled render target to the caller, the render target
    // is expected to be changed so we need to resolve to a single sampled texture
    // next time resolveTexture is called.
    mMSTexInfo->msTextureNeedsResolve = true;
    *outRT                            = rt;
    return angle::Result::Continue;
}

TextureStorage11_2D::TextureStorage11_2D(Renderer11 *renderer,
                                         SwapChain11 *swapchain,
                                         const std::string &label)
    : TextureStorage11(renderer,
                       D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                       0,
                       swapchain->getRenderTargetInternalFormat(),
                       label),
      mTexture(swapchain->getOffscreenTexture()),
      mLevelZeroTexture(),
      mLevelZeroRenderTarget(nullptr),
      mUseLevelZeroTexture(false),
      mSwizzleTexture()
{
    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i] = nullptr;
        mRenderTarget[i]     = nullptr;
    }

    D3D11_TEXTURE2D_DESC texDesc;
    mTexture.getDesc(&texDesc);
    mMipLevels     = texDesc.MipLevels;
    mTextureWidth  = texDesc.Width;
    mTextureHeight = texDesc.Height;
    mTextureDepth  = 1;
    mHasKeyedMutex = (texDesc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX) != 0;
}

TextureStorage11_2D::TextureStorage11_2D(Renderer11 *renderer,
                                         GLenum internalformat,
                                         BindFlags bindFlags,
                                         GLsizei width,
                                         GLsizei height,
                                         int levels,
                                         const std::string &label,
                                         bool hintLevelZeroOnly)
    : TextureStorage11(
          renderer,
          GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), bindFlags),
          GetTextureMiscFlags(internalformat,
                              renderer->getRenderer11DeviceCaps(),
                              bindFlags,
                              levels),
          internalformat,
          label),
      mTexture(),
      mHasKeyedMutex(false),
      mLevelZeroTexture(),
      mLevelZeroRenderTarget(nullptr),
      mUseLevelZeroTexture(hintLevelZeroOnly && levels > 1),
      mSwizzleTexture()
{
    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i] = nullptr;
        mRenderTarget[i]     = nullptr;
    }

    d3d11::MakeValidSize(false, mFormatInfo.texFormat, &width, &height, &mTopLevel);
    mMipLevels     = mTopLevel + levels;
    mTextureWidth  = width;
    mTextureHeight = height;
    mTextureDepth  = 1;

    // The LevelZeroOnly hint should only be true if the zero max LOD workaround is active.
    ASSERT(!mUseLevelZeroTexture || mRenderer->getFeatures().zeroMaxLodWorkaround.enabled);
}

void TextureStorage11_2D::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mLevelZeroTexture.valid())
    {
        mLevelZeroTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mSwizzleTexture.valid())
    {
        mSwizzleTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

angle::Result TextureStorage11_2D::onDestroy(const gl::Context *context)
{
    for (unsigned i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        if (mAssociatedImages[i] != nullptr)
        {
            mAssociatedImages[i]->verifyAssociatedStorageValid(this);

            // We must let the Images recover their data before we delete it from the
            // TextureStorage.
            ANGLE_TRY(mAssociatedImages[i]->recoverFromAssociatedStorage(context));
        }
    }

    if (mHasKeyedMutex)
    {
        // If the keyed mutex is released that will unbind it and cause the state cache to become
        // desynchronized.
        mRenderer->getStateManager()->invalidateBoundViews();
    }

    return angle::Result::Continue;
}

TextureStorage11_2D::~TextureStorage11_2D() {}

angle::Result TextureStorage11_2D::copyToStorage(const gl::Context *context,
                                                 TextureStorage *destStorage)
{
    ASSERT(destStorage);

    TextureStorage11_2D *dest11           = GetAs<TextureStorage11_2D>(destStorage);
    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        // If either mTexture or mLevelZeroTexture exist, then we need to copy them into the
        // corresponding textures in destStorage.
        if (mTexture.valid())
        {
            ANGLE_TRY(dest11->useLevelZeroWorkaroundTexture(context, false));

            const TextureHelper11 *destResource = nullptr;
            ANGLE_TRY(dest11->getResource(context, &destResource));

            immediateContext->CopyResource(destResource->get(), mTexture.get());
        }

        if (mLevelZeroTexture.valid())
        {
            ANGLE_TRY(dest11->useLevelZeroWorkaroundTexture(context, true));

            const TextureHelper11 *destResource = nullptr;
            ANGLE_TRY(dest11->getResource(context, &destResource));

            immediateContext->CopyResource(destResource->get(), mLevelZeroTexture.get());
        }

        return angle::Result::Continue;
    }

    const TextureHelper11 *sourceResouce = nullptr;
    ANGLE_TRY(getResource(context, &sourceResouce));

    const TextureHelper11 *destResource = nullptr;
    ANGLE_TRY(dest11->getResource(context, &destResource));

    immediateContext->CopyResource(destResource->get(), sourceResouce->get());
    dest11->markDirty();

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                                 bool useLevelZeroTexture)
{
    if (useLevelZeroTexture && mMipLevels > 1)
    {
        if (!mUseLevelZeroTexture && mTexture.valid())
        {
            ANGLE_TRY(ensureTextureExists(context, 1));

            // Pull data back from the mipped texture if necessary.
            ASSERT(mLevelZeroTexture.valid());
            ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
            deviceContext->CopySubresourceRegion(mLevelZeroTexture.get(), 0, 0, 0, 0,
                                                 mTexture.get(), 0, nullptr);
        }

        mUseLevelZeroTexture = true;
    }
    else
    {
        if (mUseLevelZeroTexture && mLevelZeroTexture.valid())
        {
            ANGLE_TRY(ensureTextureExists(context, mMipLevels));

            // Pull data back from the level zero texture if necessary.
            ASSERT(mTexture.valid());
            ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
            deviceContext->CopySubresourceRegion(mTexture.get(), 0, 0, 0, 0,
                                                 mLevelZeroTexture.get(), 0, nullptr);
        }

        mUseLevelZeroTexture = false;
    }

    return angle::Result::Continue;
}

void TextureStorage11_2D::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        mAssociatedImages[level] = image;
    }
}

void TextureStorage11_2D::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                     Image11 *expectedImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    // This validation check should never return false. It means the Image/TextureStorage
    // association is broken.
    ASSERT(mAssociatedImages[level] == expectedImage);
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_2D::disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(mAssociatedImages[level] == expectedImage);
    mAssociatedImages[level] = nullptr;
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image
// recover its data before ending the association.
angle::Result TextureStorage11_2D::releaseAssociatedImage(const gl::Context *context,
                                                          const gl::ImageIndex &index,
                                                          Image11 *incomingImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // No need to let the old Image recover its data, if it is also the incoming Image.
        if (mAssociatedImages[level] != nullptr && mAssociatedImages[level] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage.
            mAssociatedImages[level]->verifyAssociatedStorageValid(this);

            // Force the image to recover from storage before its data is overwritten.
            // This will reset mAssociatedImages[level] to nullptr too.
            ANGLE_TRY(mAssociatedImages[level]->recoverFromAssociatedStorage(context));
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::getResource(const gl::Context *context,
                                               const TextureHelper11 **outResource)
{
    if (mUseLevelZeroTexture && mMipLevels > 1)
    {
        ANGLE_TRY(ensureTextureExists(context, 1));

        *outResource = &mLevelZeroTexture;
        return angle::Result::Continue;
    }

    ANGLE_TRY(ensureTextureExists(context, mMipLevels));

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::getMippedResource(const gl::Context *context,
                                                     const TextureHelper11 **outResource)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    ASSERT(mRenderer->getFeatures().zeroMaxLodWorkaround.enabled);

    ANGLE_TRY(ensureTextureExists(context, mMipLevels));

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::ensureTextureExists(const gl::Context *context, int mipLevels)
{
    // If mMipLevels = 1 then always use mTexture rather than mLevelZeroTexture.
    ANGLE_TRY(resolveTexture(context));
    bool useLevelZeroTexture       = mRenderer->getFeatures().zeroMaxLodWorkaround.enabled
                                         ? (mipLevels == 1) && (mMipLevels > 1)
                                         : false;
    TextureHelper11 *outputTexture = useLevelZeroTexture ? &mLevelZeroTexture : &mTexture;

    // if the width or height is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (!outputTexture->valid() && mTextureWidth > 0 && mTextureHeight > 0)
    {
        ASSERT(mipLevels > 0);

        D3D11_TEXTURE2D_DESC desc;
        desc.Width     = mTextureWidth;  // Compressed texture size constraints?
        desc.Height    = mTextureHeight;
        desc.MipLevels = mipLevels;
        desc.ArraySize = 1;
        desc.Format =
            requiresTypelessTextureFormat() ? mFormatInfo.typelessFormat : mFormatInfo.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = getBindFlags();
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = getMiscFlags();

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             outputTexture));

        if (useLevelZeroTexture)
        {
            outputTexture->setLabels("TexStorage2D.Level0", &mKHRDebugLabel);
        }
        else
        {
            outputTexture->setLabels("TexStorage2D", &mKHRDebugLabel);
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::findRenderTarget(const gl::Context *context,
                                                    const gl::ImageIndex &index,
                                                    GLsizei samples,
                                                    RenderTargetD3D **outRT) const
{
    ASSERT(!index.hasLayer());

    const int level = index.getLevelIndex();
    ASSERT(level >= 0 && level < getLevelCount());

    bool needMS = samples > 0;
    if (needMS)
    {
        return findMultisampledRenderTarget(context, index, samples, outRT);
    }

    ASSERT(outRT);
    if (mRenderTarget[level])
    {
        *outRT = mRenderTarget[level].get();
        return angle::Result::Continue;
    }

    if (mUseLevelZeroTexture)
    {
        ASSERT(level == 0);
        *outRT = mLevelZeroRenderTarget.get();
        return angle::Result::Continue;
    }

    *outRT = nullptr;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::getRenderTarget(const gl::Context *context,
                                                   const gl::ImageIndex &index,
                                                   GLsizei samples,
                                                   RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());

    const int level = index.getLevelIndex();
    ASSERT(level >= 0 && level < getLevelCount());

    bool needMS = samples > 0;
    if (needMS)
    {
        return getMultisampledRenderTarget(context, index, samples, outRT);
    }
    else
    {
        ANGLE_TRY(resolveTexture(context));
    }

    // In GL ES 2.0, the application can only render to level zero of the texture (Section 4.4.3 of
    // the GLES 2.0 spec, page 113 of version 2.0.25). Other parts of TextureStorage11_2D could
    // create RTVs on non-zero levels of the texture (e.g. generateMipmap).
    // On Feature Level 9_3, this is unlikely to be useful. The renderer can't create SRVs on the
    // individual levels of the texture, so methods like generateMipmap can't do anything useful
    // with non-zero-level RTVs. Therefore if level > 0 on 9_3 then there's almost certainly
    // something wrong.
    ASSERT(
        !(mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3 && level > 0));
    ASSERT(outRT);
    if (mRenderTarget[level])
    {
        *outRT = mRenderTarget[level].get();
        return angle::Result::Continue;
    }

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ASSERT(level == 0);
        ANGLE_TRY(useLevelZeroWorkaroundTexture(context, true));
    }

    const TextureHelper11 *texture = nullptr;
    ANGLE_TRY(getResource(context, &texture));

    const d3d11::SharedSRV *srv = nullptr;
    ANGLE_TRY(getSRVLevel(context, level, SRVType::Sample, &srv));

    const d3d11::SharedSRV *blitSRV = nullptr;
    ANGLE_TRY(getSRVLevel(context, level, SRVType::Blit, &blitSRV));

    Context11 *context11 = GetImplAs<Context11>(context);

    if (mUseLevelZeroTexture)
    {
        if (!mLevelZeroRenderTarget)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format             = mFormatInfo.rtvFormat;
            rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = mTopLevel + level;

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(
                mRenderer->allocateResource(context11, rtvDesc, mLevelZeroTexture.get(), &rtv));
            rtv.setLabels("TexStorage2D.Level0RTV", &mKHRDebugLabel);

            mLevelZeroRenderTarget.reset(new TextureRenderTarget11(
                std::move(rtv), mLevelZeroTexture, d3d11::SharedSRV(), d3d11::SharedSRV(),
                mFormatInfo.internalFormat, getFormatSet(), getLevelWidth(level),
                getLevelHeight(level), 1, 0));
        }

        *outRT = mLevelZeroRenderTarget.get();
        return angle::Result::Continue;
    }

    if (mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format             = mFormatInfo.rtvFormat;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mTopLevel + level;

        d3d11::RenderTargetView rtv;
        ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, texture->get(), &rtv));
        rtv.setLabels("TexStorage2D.RTV", &mKHRDebugLabel);

        mRenderTarget[level].reset(new TextureRenderTarget11(
            std::move(rtv), *texture, *srv, *blitSRV, mFormatInfo.internalFormat, getFormatSet(),
            getLevelWidth(level), getLevelHeight(level), 1, 0));

        *outRT = mRenderTarget[level].get();
        return angle::Result::Continue;
    }

    ASSERT(mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format             = mFormatInfo.dsvFormat;
    dsvDesc.ViewDimension      = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = mTopLevel + level;
    dsvDesc.Flags              = 0;

    d3d11::DepthStencilView dsv;
    ANGLE_TRY(mRenderer->allocateResource(context11, dsvDesc, texture->get(), &dsv));
    dsv.setLabels("TexStorage2D.DSV", &mKHRDebugLabel);

    mRenderTarget[level].reset(new TextureRenderTarget11(
        std::move(dsv), *texture, *srv, mFormatInfo.internalFormat, getFormatSet(),
        getLevelWidth(level), getLevelHeight(level), 1, 0));

    *outRT = mRenderTarget[level].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::createSRVForSampler(const gl::Context *context,
                                                       int baseLevel,
                                                       int mipLevels,
                                                       DXGI_FORMAT format,
                                                       const TextureHelper11 &texture,
                                                       d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = mTopLevel + baseLevel;
    srvDesc.Texture2D.MipLevels       = mipLevels;

    const TextureHelper11 *srvTexture = &texture;

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ASSERT(mTopLevel == 0);
        ASSERT(baseLevel == 0);
        // This code also assumes that the incoming texture equals either mLevelZeroTexture or
        // mTexture.

        if (mipLevels == 1 && mMipLevels > 1)
        {
            // We must use a SRV on the level-zero-only texture.
            ANGLE_TRY(ensureTextureExists(context, 1));
            srvTexture = &mLevelZeroTexture;
        }
        else
        {
            ASSERT(mipLevels == static_cast<int>(mMipLevels));
            ASSERT(mTexture.valid() && texture == mTexture);
            srvTexture = &mTexture;
        }
    }

    ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, srvTexture->get(),
                                          outSRV));
    outSRV->setLabels("TexStorage2D.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::createSRVForImage(const gl::Context *context,
                                                     int level,
                                                     DXGI_FORMAT format,
                                                     const TextureHelper11 &texture,
                                                     d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = mTopLevel + level;
    srvDesc.Texture2D.MipLevels       = 1;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2D.SRVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::createUAVForImage(const gl::Context *context,
                                                     int level,
                                                     DXGI_FORMAT format,
                                                     const TextureHelper11 &texture,
                                                     d3d11::SharedUAV *outUAV)
{
    ASSERT(outUAV);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format             = format;
    uavDesc.ViewDimension      = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = mTopLevel + level;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), uavDesc, texture.get(), outUAV));
    outUAV->setLabels("TexStorage2D.UAVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::getSwizzleTexture(const gl::Context *context,
                                                     const TextureHelper11 **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture.valid())
    {
        const auto &format = mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = mTextureWidth;
        desc.Height             = mTextureHeight;
        desc.MipLevels          = mMipLevels;
        desc.ArraySize          = 1;
        desc.Format             = format.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, format,
                                             &mSwizzleTexture));
        mSwizzleTexture.setLabels("TexStorage2D.Swizzle", &mKHRDebugLabel);
    }

    *outTexture = &mSwizzleTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::getSwizzleRenderTarget(const gl::Context *context,
                                                          int mipLevel,
                                                          const d3d11::RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel].valid())
    {
        const TextureHelper11 *swizzleTexture = nullptr;
        ANGLE_TRY(getSwizzleTexture(context, &swizzleTexture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps()).rtvFormat;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mTopLevel + mipLevel;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                              mSwizzleTexture.get(),
                                              &mSwizzleRenderTargets[mipLevel]));
    }

    *outRTV = &mSwizzleRenderTargets[mipLevel];
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::ensureDropStencilTexture(const gl::Context *context,
                                                            DropStencil *dropStencilOut)
{
    if (mDropStencilTexture.valid())
    {
        *dropStencilOut = DropStencil::ALREADY_EXISTS;
        return angle::Result::Continue;
    }

    D3D11_TEXTURE2D_DESC dropDesc = {};
    dropDesc.ArraySize            = 1;
    dropDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    dropDesc.CPUAccessFlags       = 0;
    dropDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
    dropDesc.Height               = mTextureHeight;
    dropDesc.MipLevels            = mMipLevels;
    dropDesc.MiscFlags            = 0;
    dropDesc.SampleDesc.Count     = 1;
    dropDesc.SampleDesc.Quality   = 0;
    dropDesc.Usage                = D3D11_USAGE_DEFAULT;
    dropDesc.Width                = mTextureWidth;

    const auto &format =
        d3d11::Format::Get(GL_DEPTH_COMPONENT32F, mRenderer->getRenderer11DeviceCaps());
    ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), dropDesc, format,
                                         &mDropStencilTexture));
    mDropStencilTexture.setLabels("TexStorage2D.DropStencil", &mKHRDebugLabel);

    ANGLE_TRY(initDropStencilTexture(context, gl::ImageIndexIterator::Make2D(0, mMipLevels)));

    *dropStencilOut = DropStencil::CREATED;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2D::resolveTexture(const gl::Context *context)
{
    if (mMSTexInfo && mMSTexInfo->msTex && mMSTexInfo->msTextureNeedsResolve)
    {
        ANGLE_TRY(resolveTextureHelper(context, mTexture));
        onStateChange(angle::SubjectMessage::ContentsChanged);
    }
    return angle::Result::Continue;
}

TextureStorage11_External::TextureStorage11_External(
    Renderer11 *renderer,
    egl::Stream *stream,
    const egl::Stream::GLTextureDescription &glDesc,
    const std::string &label)
    : TextureStorage11(renderer, D3D11_BIND_SHADER_RESOURCE, 0, glDesc.internalFormat, label),
      mAssociatedImage(nullptr)
{
    ASSERT(stream->getProducerType() == egl::Stream::ProducerType::D3D11Texture);
    auto *producer = static_cast<StreamProducerD3DTexture *>(stream->getImplementation());
    mTexture.set(producer->getD3DTexture(), mFormatInfo);
    mSubresourceIndex = producer->getArraySlice();
    mTexture.get()->AddRef();
    mMipLevels = 1;

    D3D11_TEXTURE2D_DESC desc;
    mTexture.getDesc(&desc);
    mTextureWidth  = desc.Width;
    mTextureHeight = desc.Height;
    mTextureDepth  = 1;
    mHasKeyedMutex = (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX) != 0;
}

angle::Result TextureStorage11_External::onDestroy(const gl::Context *context)
{
    if (mHasKeyedMutex)
    {
        // If the keyed mutex is released that will unbind it and cause the state cache to become
        // desynchronized.
        mRenderer->getStateManager()->invalidateBoundViews();
    }

    if (mAssociatedImage != nullptr)
    {
        mAssociatedImage->verifyAssociatedStorageValid(this);

        // We must let the Images recover their data before we delete it from the
        // TextureStorage.
        ANGLE_TRY(mAssociatedImage->recoverFromAssociatedStorage(context));
    }

    return angle::Result::Continue;
}

TextureStorage11_External::~TextureStorage11_External() {}

angle::Result TextureStorage11_External::copyToStorage(const gl::Context *context,
                                                       TextureStorage *destStorage)
{
    UNIMPLEMENTED();
    return angle::Result::Continue;
}

void TextureStorage11_External::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    ASSERT(index.getLevelIndex() == 0);
    mAssociatedImage = image;
}

void TextureStorage11_External::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                           Image11 *expectedImage)
{
    ASSERT(index.getLevelIndex() == 0 && mAssociatedImage == expectedImage);
}

void TextureStorage11_External::disassociateImage(const gl::ImageIndex &index,
                                                  Image11 *expectedImage)
{
    ASSERT(index.getLevelIndex() == 0);
    ASSERT(mAssociatedImage == expectedImage);
    mAssociatedImage = nullptr;
}

angle::Result TextureStorage11_External::releaseAssociatedImage(const gl::Context *context,
                                                                const gl::ImageIndex &index,
                                                                Image11 *incomingImage)
{
    ASSERT(index.getLevelIndex() == 0);

    if (mAssociatedImage != nullptr && mAssociatedImage != incomingImage)
    {
        mAssociatedImage->verifyAssociatedStorageValid(this);

        ANGLE_TRY(mAssociatedImage->recoverFromAssociatedStorage(context));
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_External::getResource(const gl::Context *context,
                                                     const TextureHelper11 **outResource)
{
    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_External::getMippedResource(const gl::Context *context,
                                                           const TextureHelper11 **outResource)
{
    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_External::findRenderTarget(const gl::Context *context,
                                                          const gl::ImageIndex &index,
                                                          GLsizei samples,
                                                          RenderTargetD3D **outRT) const
{
    // Render targets are not supported for external textures
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_External::getRenderTarget(const gl::Context *context,
                                                         const gl::ImageIndex &index,
                                                         GLsizei samples,
                                                         RenderTargetD3D **outRT)
{
    // Render targets are not supported for external textures
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_External::createSRVForSampler(const gl::Context *context,
                                                             int baseLevel,
                                                             int mipLevels,
                                                             DXGI_FORMAT format,
                                                             const TextureHelper11 &texture,
                                                             d3d11::SharedSRV *outSRV)
{
    // Since external textures are treates as non-mipmapped textures, we ignore mipmap levels and
    // use the specified subresource ID the storage was created with.
    ASSERT(mipLevels == 1);
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format        = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    // subresource index is equal to the mip level for 2D textures
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = mSubresourceIndex;
    srvDesc.Texture2DArray.ArraySize       = 1;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2D.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_External::createSRVForImage(const gl::Context *context,
                                                           int level,
                                                           DXGI_FORMAT format,
                                                           const TextureHelper11 &texture,
                                                           d3d11::SharedSRV *outSRV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_External::createUAVForImage(const gl::Context *context,
                                                           int level,
                                                           DXGI_FORMAT format,
                                                           const TextureHelper11 &texture,
                                                           d3d11::SharedUAV *outUAV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_External::getSwizzleTexture(const gl::Context *context,
                                                           const TextureHelper11 **outTexture)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_External::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

void TextureStorage11_External::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11ImmutableBase::TextureStorage11ImmutableBase(Renderer11 *renderer,
                                                             UINT bindFlags,
                                                             UINT miscFlags,
                                                             GLenum internalFormat,
                                                             const std::string &label)
    : TextureStorage11(renderer, bindFlags, miscFlags, internalFormat, label)
{}

void TextureStorage11ImmutableBase::associateImage(Image11 *, const gl::ImageIndex &) {}

void TextureStorage11ImmutableBase::disassociateImage(const gl::ImageIndex &, Image11 *) {}

void TextureStorage11ImmutableBase::verifyAssociatedImageValid(const gl::ImageIndex &, Image11 *) {}

angle::Result TextureStorage11ImmutableBase::releaseAssociatedImage(const gl::Context *context,
                                                                    const gl::ImageIndex &,
                                                                    Image11 *)
{
    return angle::Result::Continue;
}

angle::Result TextureStorage11ImmutableBase::createSRVForImage(const gl::Context *context,
                                                               int level,
                                                               DXGI_FORMAT format,
                                                               const TextureHelper11 &texture,
                                                               d3d11::SharedSRV *outSRV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11ImmutableBase::createUAVForImage(const gl::Context *context,
                                                               int level,
                                                               DXGI_FORMAT format,
                                                               const TextureHelper11 &texture,
                                                               d3d11::SharedUAV *outUAV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

TextureStorage11_EGLImage::TextureStorage11_EGLImage(Renderer11 *renderer,
                                                     EGLImageD3D *eglImage,
                                                     RenderTarget11 *renderTarget11,
                                                     const std::string &label)
    : TextureStorage11ImmutableBase(renderer,
                                    D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                                    0,
                                    renderTarget11->getInternalFormat(),
                                    label),
      mImage(eglImage),
      mCurrentRenderTarget(0),
      mSwizzleTexture(),
      mSwizzleRenderTargets(gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS),
      mAssociatedImage(nullptr)
{
    mCurrentRenderTarget = reinterpret_cast<uintptr_t>(renderTarget11);

    mMipLevels     = 1;
    mTextureWidth  = renderTarget11->getWidth();
    mTextureHeight = renderTarget11->getHeight();
    mTextureDepth  = 1;
}

TextureStorage11_EGLImage::~TextureStorage11_EGLImage() {}

angle::Result TextureStorage11_EGLImage::onDestroy(const gl::Context *context)
{
    if (mAssociatedImage != nullptr)
    {
        mAssociatedImage->verifyAssociatedStorageValid(this);

        // We must let the Images recover their data before we delete it from the
        // TextureStorage.
        ANGLE_TRY(mAssociatedImage->recoverFromAssociatedStorage(context));
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::getSubresourceIndex(const gl::Context *context,
                                                             const gl::ImageIndex &index,
                                                             UINT *outSubresourceIndex) const
{
    ASSERT(index.getType() == gl::TextureType::_2D);
    ASSERT(index.getLevelIndex() == 0);

    RenderTarget11 *renderTarget11 = nullptr;
    ANGLE_TRY(getImageRenderTarget(context, &renderTarget11));
    *outSubresourceIndex = renderTarget11->getSubresourceIndex();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::getResource(const gl::Context *context,
                                                     const TextureHelper11 **outResource)
{
    ANGLE_TRY(checkForUpdatedRenderTarget(context));

    RenderTarget11 *renderTarget11 = nullptr;
    ANGLE_TRY(getImageRenderTarget(context, &renderTarget11));
    *outResource = &renderTarget11->getTexture();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::getSRVForSampler(const gl::Context *context,
                                                          const gl::TextureState &textureState,
                                                          const gl::SamplerState &sampler,
                                                          const d3d11::SharedSRV **outSRV)
{
    ANGLE_TRY(checkForUpdatedRenderTarget(context));
    return TextureStorage11::getSRVForSampler(context, textureState, sampler, outSRV);
}

angle::Result TextureStorage11_EGLImage::getMippedResource(const gl::Context *context,
                                                           const TextureHelper11 **)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    // EGL images are unavailable in this configuration.
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_EGLImage::findRenderTarget(const gl::Context *context,
                                                          const gl::ImageIndex &index,
                                                          GLsizei samples,
                                                          RenderTargetD3D **outRT) const
{
    // Since the render target of an EGL image will be updated when orphaning, trying to find a
    // cache of it can be rarely useful.
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_EGLImage::getRenderTarget(const gl::Context *context,
                                                         const gl::ImageIndex &index,
                                                         GLsizei samples,
                                                         RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());
    ASSERT(index.getLevelIndex() == 0);

    ANGLE_TRY(checkForUpdatedRenderTarget(context));

    return mImage->getRenderTarget(context, outRT);
}

angle::Result TextureStorage11_EGLImage::copyToStorage(const gl::Context *context,
                                                       TextureStorage *destStorage)
{
    const TextureHelper11 *sourceResouce = nullptr;
    ANGLE_TRY(getResource(context, &sourceResouce));

    ASSERT(destStorage);
    TextureStorage11_2D *dest11         = GetAs<TextureStorage11_2D>(destStorage);
    const TextureHelper11 *destResource = nullptr;
    ANGLE_TRY(dest11->getResource(context, &destResource));

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
    immediateContext->CopyResource(destResource->get(), sourceResouce->get());

    dest11->markDirty();

    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                                       bool)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_EGLImage::getSwizzleTexture(const gl::Context *context,
                                                           const TextureHelper11 **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture.valid())
    {
        const auto &format = mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = mTextureWidth;
        desc.Height             = mTextureHeight;
        desc.MipLevels          = mMipLevels;
        desc.ArraySize          = 1;
        desc.Format             = format.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, format,
                                             &mSwizzleTexture));
        mSwizzleTexture.setLabels("TexStorageEGLImage.Swizzle", &mKHRDebugLabel);
    }

    *outTexture = &mSwizzleTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel].valid())
    {
        const TextureHelper11 *swizzleTexture = nullptr;
        ANGLE_TRY(getSwizzleTexture(context, &swizzleTexture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps()).rtvFormat;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mTopLevel + mipLevel;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                              mSwizzleTexture.get(),
                                              &mSwizzleRenderTargets[mipLevel]));
    }

    *outRTV = &mSwizzleRenderTargets[mipLevel];
    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::checkForUpdatedRenderTarget(const gl::Context *context)
{
    RenderTarget11 *renderTarget11 = nullptr;
    ANGLE_TRY(getImageRenderTarget(context, &renderTarget11));

    if (mCurrentRenderTarget != reinterpret_cast<uintptr_t>(renderTarget11))
    {
        clearSRVCache();
        mCurrentRenderTarget = reinterpret_cast<uintptr_t>(renderTarget11);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::createSRVForSampler(const gl::Context *context,
                                                             int baseLevel,
                                                             int mipLevels,
                                                             DXGI_FORMAT format,
                                                             const TextureHelper11 &texture,
                                                             d3d11::SharedSRV *outSRV)
{
    ASSERT(baseLevel == 0);
    ASSERT(mipLevels == 1);
    ASSERT(outSRV);

    // Create a new SRV only for the swizzle texture.  Otherwise just return the Image's
    // RenderTarget's SRV.
    if (texture == mSwizzleTexture)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format                    = format;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = mTopLevel + baseLevel;
        srvDesc.Texture2D.MipLevels       = mipLevels;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(),
                                              outSRV));
        outSRV->setLabels("TexStorageEGLImage.SRV", &mKHRDebugLabel);
    }
    else
    {
        RenderTarget11 *renderTarget = nullptr;
        ANGLE_TRY(getImageRenderTarget(context, &renderTarget));

        ASSERT(texture == renderTarget->getTexture());

        const d3d11::SharedSRV *srv;
        ANGLE_TRY(renderTarget->getShaderResourceView(context, &srv));

        *outSRV = srv->makeCopy();
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_EGLImage::getImageRenderTarget(const gl::Context *context,
                                                              RenderTarget11 **outRT) const
{
    RenderTargetD3D *renderTargetD3D = nullptr;
    ANGLE_TRY(mImage->getRenderTarget(context, &renderTargetD3D));
    *outRT = GetAs<RenderTarget11>(renderTargetD3D);
    return angle::Result::Continue;
}

void TextureStorage11_EGLImage::onLabelUpdate()
{
    if (mSwizzleTexture.valid())
    {
        mSwizzleTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

void TextureStorage11_EGLImage::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    ASSERT(index.getLevelIndex() == 0);
    mAssociatedImage = image;
}

void TextureStorage11_EGLImage::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                           Image11 *expectedImage)
{
    ASSERT(index.getLevelIndex() == 0 && mAssociatedImage == expectedImage);
}

void TextureStorage11_EGLImage::disassociateImage(const gl::ImageIndex &index,
                                                  Image11 *expectedImage)
{
    ASSERT(index.getLevelIndex() == 0);
    ASSERT(mAssociatedImage == expectedImage);
    mAssociatedImage = nullptr;
}

angle::Result TextureStorage11_EGLImage::releaseAssociatedImage(const gl::Context *context,
                                                                const gl::ImageIndex &index,
                                                                Image11 *incomingImage)
{
    ASSERT(index.getLevelIndex() == 0);

    if (mAssociatedImage != nullptr && mAssociatedImage != incomingImage)
    {
        mAssociatedImage->verifyAssociatedStorageValid(this);

        ANGLE_TRY(mAssociatedImage->recoverFromAssociatedStorage(context));
    }

    return angle::Result::Continue;
}

TextureStorage11_Cube::TextureStorage11_Cube(Renderer11 *renderer,
                                             GLenum internalformat,
                                             BindFlags bindFlags,
                                             int size,
                                             int levels,
                                             bool hintLevelZeroOnly,
                                             const std::string &label)
    : TextureStorage11(
          renderer,
          GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), bindFlags),
          GetTextureMiscFlags(internalformat,
                              renderer->getRenderer11DeviceCaps(),
                              bindFlags,
                              levels),
          internalformat,
          label),
      mTexture(),
      mLevelZeroTexture(),
      mUseLevelZeroTexture(hintLevelZeroOnly && levels > 1),
      mSwizzleTexture()
{
    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        for (unsigned int face = 0; face < gl::kCubeFaceCount; face++)
        {
            mAssociatedImages[face][level] = nullptr;
            mRenderTarget[face][level]     = nullptr;
        }
    }

    for (unsigned int face = 0; face < gl::kCubeFaceCount; face++)
    {
        mLevelZeroRenderTarget[face] = nullptr;
    }

    // adjust size if needed for compressed textures
    int height = size;
    d3d11::MakeValidSize(false, mFormatInfo.texFormat, &size, &height, &mTopLevel);

    mMipLevels     = mTopLevel + levels;
    mTextureWidth  = size;
    mTextureHeight = size;
    mTextureDepth  = 1;

    // The LevelZeroOnly hint should only be true if the zero max LOD workaround is active.
    ASSERT(!mUseLevelZeroTexture || mRenderer->getFeatures().zeroMaxLodWorkaround.enabled);
}

angle::Result TextureStorage11_Cube::onDestroy(const gl::Context *context)
{
    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        for (unsigned int face = 0; face < gl::kCubeFaceCount; face++)
        {
            if (mAssociatedImages[face][level] != nullptr)
            {
                mAssociatedImages[face][level]->verifyAssociatedStorageValid(this);

                // We must let the Images recover their data before we delete it from the
                // TextureStorage.
                ANGLE_TRY(mAssociatedImages[face][level]->recoverFromAssociatedStorage(context));
            }
        }
    }

    return angle::Result::Continue;
}

TextureStorage11_Cube::~TextureStorage11_Cube() {}

angle::Result TextureStorage11_Cube::getSubresourceIndex(const gl::Context *context,
                                                         const gl::ImageIndex &index,
                                                         UINT *outSubresourceIndex) const
{
    UINT arraySlice = index.cubeMapFaceIndex();
    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled && mUseLevelZeroTexture &&
        index.getLevelIndex() == 0)
    {
        UINT subresource = D3D11CalcSubresource(0, arraySlice, 1);
        ASSERT(subresource != std::numeric_limits<UINT>::max());
        *outSubresourceIndex = subresource;
    }
    else
    {
        UINT mipSlice    = static_cast<UINT>(index.getLevelIndex() + mTopLevel);
        UINT subresource = D3D11CalcSubresource(mipSlice, arraySlice, mMipLevels);
        ASSERT(subresource != std::numeric_limits<UINT>::max());
        *outSubresourceIndex = subresource;
    }
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::copyToStorage(const gl::Context *context,
                                                   TextureStorage *destStorage)
{
    ASSERT(destStorage);

    TextureStorage11_Cube *dest11 = GetAs<TextureStorage11_Cube>(destStorage);

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

        // If either mTexture or mLevelZeroTexture exist, then we need to copy them into the
        // corresponding textures in destStorage.
        if (mTexture.valid())
        {
            ANGLE_TRY(dest11->useLevelZeroWorkaroundTexture(context, false));

            const TextureHelper11 *destResource = nullptr;
            ANGLE_TRY(dest11->getResource(context, &destResource));

            immediateContext->CopyResource(destResource->get(), mTexture.get());
        }

        if (mLevelZeroTexture.valid())
        {
            ANGLE_TRY(dest11->useLevelZeroWorkaroundTexture(context, true));

            const TextureHelper11 *destResource = nullptr;
            ANGLE_TRY(dest11->getResource(context, &destResource));

            immediateContext->CopyResource(destResource->get(), mLevelZeroTexture.get());
        }
    }
    else
    {
        const TextureHelper11 *sourceResouce = nullptr;
        ANGLE_TRY(getResource(context, &sourceResouce));

        const TextureHelper11 *destResource = nullptr;
        ANGLE_TRY(dest11->getResource(context, &destResource));

        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
        immediateContext->CopyResource(destResource->get(), sourceResouce->get());
    }

    dest11->markDirty();

    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::useLevelZeroWorkaroundTexture(const gl::Context *context,
                                                                   bool useLevelZeroTexture)
{
    if (useLevelZeroTexture && mMipLevels > 1)
    {
        if (!mUseLevelZeroTexture && mTexture.valid())
        {
            ANGLE_TRY(ensureTextureExists(context, 1));

            // Pull data back from the mipped texture if necessary.
            ASSERT(mLevelZeroTexture.valid());
            ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

            for (int face = 0; face < 6; face++)
            {
                deviceContext->CopySubresourceRegion(mLevelZeroTexture.get(),
                                                     D3D11CalcSubresource(0, face, 1), 0, 0, 0,
                                                     mTexture.get(), face * mMipLevels, nullptr);
            }
        }

        mUseLevelZeroTexture = true;
    }
    else
    {
        if (mUseLevelZeroTexture && mLevelZeroTexture.valid())
        {
            ANGLE_TRY(ensureTextureExists(context, mMipLevels));

            // Pull data back from the level zero texture if necessary.
            ASSERT(mTexture.valid());
            ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

            for (int face = 0; face < 6; face++)
            {
                deviceContext->CopySubresourceRegion(mTexture.get(),
                                                     D3D11CalcSubresource(0, face, mMipLevels), 0,
                                                     0, 0, mLevelZeroTexture.get(), face, nullptr);
            }
        }

        mUseLevelZeroTexture = false;
    }

    return angle::Result::Continue;
}

void TextureStorage11_Cube::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.cubeMapFaceIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount));

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        if (0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount))
        {
            mAssociatedImages[layerTarget][level] = image;
        }
    }
}

void TextureStorage11_Cube::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                       Image11 *expectedImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.cubeMapFaceIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount));
    // This validation check should never return false. It means the Image/TextureStorage
    // association is broken.
    ASSERT(mAssociatedImages[layerTarget][level] == expectedImage);
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_Cube::disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.cubeMapFaceIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount));
    ASSERT(mAssociatedImages[layerTarget][level] == expectedImage);
    mAssociatedImages[layerTarget][level] = nullptr;
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image
// recover its data before ending the association.
angle::Result TextureStorage11_Cube::releaseAssociatedImage(const gl::Context *context,
                                                            const gl::ImageIndex &index,
                                                            Image11 *incomingImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.cubeMapFaceIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount));

    if ((0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS))
    {
        if (0 <= layerTarget && layerTarget < static_cast<GLint>(gl::kCubeFaceCount))
        {
            // No need to let the old Image recover its data, if it is also the incoming Image.
            if (mAssociatedImages[layerTarget][level] != nullptr &&
                mAssociatedImages[layerTarget][level] != incomingImage)
            {
                // Ensure that the Image is still associated with this TextureStorage.
                mAssociatedImages[layerTarget][level]->verifyAssociatedStorageValid(this);

                // Force the image to recover from storage before its data is overwritten.
                // This will reset mAssociatedImages[level] to nullptr too.
                ANGLE_TRY(
                    mAssociatedImages[layerTarget][level]->recoverFromAssociatedStorage(context));
            }
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::getResource(const gl::Context *context,
                                                 const TextureHelper11 **outResource)
{
    if (mUseLevelZeroTexture && mMipLevels > 1)
    {
        ANGLE_TRY(ensureTextureExists(context, 1));
        *outResource = &mLevelZeroTexture;
    }
    else
    {
        ANGLE_TRY(ensureTextureExists(context, mMipLevels));
        *outResource = &mTexture;
    }
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::getMippedResource(const gl::Context *context,
                                                       const TextureHelper11 **outResource)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    ASSERT(mRenderer->getFeatures().zeroMaxLodWorkaround.enabled);

    ANGLE_TRY(ensureTextureExists(context, mMipLevels));
    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::ensureTextureExists(const gl::Context *context, int mipLevels)
{
    // If mMipLevels = 1 then always use mTexture rather than mLevelZeroTexture.
    ANGLE_TRY(resolveTexture(context));
    bool useLevelZeroTexture       = mRenderer->getFeatures().zeroMaxLodWorkaround.enabled
                                         ? (mipLevels == 1) && (mMipLevels > 1)
                                         : false;
    TextureHelper11 *outputTexture = useLevelZeroTexture ? &mLevelZeroTexture : &mTexture;

    // if the size is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (!outputTexture->valid() && mTextureWidth > 0 && mTextureHeight > 0)
    {
        ASSERT(mMipLevels > 0);

        D3D11_TEXTURE2D_DESC desc;
        desc.Width     = mTextureWidth;
        desc.Height    = mTextureHeight;
        desc.MipLevels = mipLevels;
        desc.ArraySize = gl::kCubeFaceCount;
        desc.Format =
            requiresTypelessTextureFormat() ? mFormatInfo.typelessFormat : mFormatInfo.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = getBindFlags();
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = D3D11_RESOURCE_MISC_TEXTURECUBE | getMiscFlags();

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             outputTexture));
        outputTexture->setLabels("TexStorageCube", &mKHRDebugLabel);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::findRenderTarget(const gl::Context *context,
                                                      const gl::ImageIndex &index,
                                                      GLsizei samples,
                                                      RenderTargetD3D **outRT) const
{
    const int faceIndex = index.cubeMapFaceIndex();
    const int level     = index.getLevelIndex();

    ASSERT(level >= 0 && level < getLevelCount());
    ASSERT(faceIndex >= 0 && faceIndex < static_cast<GLint>(gl::kCubeFaceCount));

    bool needMS = samples > 0;
    if (needMS)
    {
        return findMultisampledRenderTarget(context, index, samples, outRT);
    }

    if (!mRenderTarget[faceIndex][level])
    {
        if (mUseLevelZeroTexture)
        {
            ASSERT(index.getLevelIndex() == 0);
            ASSERT(outRT);
            *outRT = mLevelZeroRenderTarget[faceIndex].get();
            return angle::Result::Continue;
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTarget[faceIndex][level].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::createRenderTargetSRV(const gl::Context *context,
                                                           const TextureHelper11 &texture,
                                                           const gl::ImageIndex &index,
                                                           DXGI_FORMAT resourceFormat,
                                                           d3d11::SharedSRV *srv) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = resourceFormat;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + index.getLevelIndex();
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = index.cubeMapFaceIndex();
    srvDesc.Texture2DArray.ArraySize       = 1;

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_10_0)
    {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    }
    else
    {
        // Will be used with Texture2D sampler, not TextureCube
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    }

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), srv));
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::getRenderTarget(const gl::Context *context,
                                                     const gl::ImageIndex &index,
                                                     GLsizei samples,
                                                     RenderTargetD3D **outRT)
{
    const int faceIndex = index.cubeMapFaceIndex();
    const int level     = index.getLevelIndex();

    ASSERT(level >= 0 && level < getLevelCount());
    ASSERT(faceIndex >= 0 && faceIndex < static_cast<GLint>(gl::kCubeFaceCount));

    bool needMS = samples > 0;
    if (needMS)
    {
        return getMultisampledRenderTarget(context, index, samples, outRT);
    }
    else
    {
        ANGLE_TRY(resolveTexture(context));
    }

    Context11 *context11 = GetImplAs<Context11>(context);

    if (!mRenderTarget[faceIndex][level])
    {
        if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
        {
            ASSERT(index.getLevelIndex() == 0);
            ANGLE_TRY(useLevelZeroWorkaroundTexture(context, true));
        }

        const TextureHelper11 *texture = nullptr;
        ANGLE_TRY(getResource(context, &texture));

        if (mUseLevelZeroTexture)
        {
            if (!mLevelZeroRenderTarget[faceIndex])
            {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format                         = mFormatInfo.rtvFormat;
                rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice        = mTopLevel + level;
                rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;
                rtvDesc.Texture2DArray.ArraySize       = 1;

                d3d11::RenderTargetView rtv;
                ANGLE_TRY(
                    mRenderer->allocateResource(context11, rtvDesc, mLevelZeroTexture.get(), &rtv));

                mLevelZeroRenderTarget[faceIndex].reset(new TextureRenderTarget11(
                    std::move(rtv), mLevelZeroTexture, d3d11::SharedSRV(), d3d11::SharedSRV(),
                    mFormatInfo.internalFormat, getFormatSet(), getLevelWidth(level),
                    getLevelHeight(level), 1, 0));
            }

            ASSERT(outRT);
            *outRT = mLevelZeroRenderTarget[faceIndex].get();
            return angle::Result::Continue;
        }

        d3d11::SharedSRV srv;
        ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.srvFormat, &srv));
        d3d11::SharedSRV blitSRV;
        if (mFormatInfo.blitSRVFormat != mFormatInfo.srvFormat)
        {
            ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.blitSRVFormat,
                                            &blitSRV));
        }
        else
        {
            blitSRV = srv.makeCopy();
        }

        srv.setLabels("TexStorageCube.RenderTargetSRV", &mKHRDebugLabel);

        if (mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format                         = mFormatInfo.rtvFormat;
            rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice        = mTopLevel + level;
            rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;
            rtvDesc.Texture2DArray.ArraySize       = 1;

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, texture->get(), &rtv));
            rtv.setLabels("TexStorageCube.RenderTargetRTV", &mKHRDebugLabel);

            mRenderTarget[faceIndex][level].reset(new TextureRenderTarget11(
                std::move(rtv), *texture, srv, blitSRV, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(level), getLevelHeight(level), 1, 0));
        }
        else if (mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format                         = mFormatInfo.dsvFormat;
            dsvDesc.ViewDimension                  = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Flags                          = 0;
            dsvDesc.Texture2DArray.MipSlice        = mTopLevel + level;
            dsvDesc.Texture2DArray.FirstArraySlice = faceIndex;
            dsvDesc.Texture2DArray.ArraySize       = 1;

            d3d11::DepthStencilView dsv;
            ANGLE_TRY(mRenderer->allocateResource(context11, dsvDesc, texture->get(), &dsv));
            dsv.setLabels("TexStorageCube.RenderTargetDSV", &mKHRDebugLabel);

            mRenderTarget[faceIndex][level].reset(new TextureRenderTarget11(
                std::move(dsv), *texture, srv, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(level), getLevelHeight(level), 1, 0));
        }
        else
        {
            UNREACHABLE();
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTarget[faceIndex][level].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::createSRVForSampler(const gl::Context *context,
                                                         int baseLevel,
                                                         int mipLevels,
                                                         DXGI_FORMAT format,
                                                         const TextureHelper11 &texture,
                                                         d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;

    // Unnormalized integer cube maps are not supported by DX11; we emulate them as an array of six
    // 2D textures
    const GLenum componentType = d3d11::GetComponentType(format);
    if (componentType == GL_INT || componentType == GL_UNSIGNED_INT)
    {
        srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + baseLevel;
        srvDesc.Texture2DArray.MipLevels       = mipLevels;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize       = gl::kCubeFaceCount;
    }
    else
    {
        srvDesc.ViewDimension               = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels       = mipLevels;
        srvDesc.TextureCube.MostDetailedMip = mTopLevel + baseLevel;
    }

    const TextureHelper11 *srvTexture = &texture;

    if (mRenderer->getFeatures().zeroMaxLodWorkaround.enabled)
    {
        ASSERT(mTopLevel == 0);
        ASSERT(baseLevel == 0);
        // This code also assumes that the incoming texture equals either mLevelZeroTexture or
        // mTexture.

        if (mipLevels == 1 && mMipLevels > 1)
        {
            // We must use a SRV on the level-zero-only texture.
            ANGLE_TRY(ensureTextureExists(context, 1));
            srvTexture = &mLevelZeroTexture;
        }
        else
        {
            ASSERT(mipLevels == static_cast<int>(mMipLevels));
            ASSERT(mTexture.valid() && texture == mTexture);
            srvTexture = &mTexture;
        }
    }

    ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, srvTexture->get(),
                                          outSRV));
    outSRV->setLabels("TexStorageCube.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::createSRVForImage(const gl::Context *context,
                                                       int level,
                                                       DXGI_FORMAT format,
                                                       const TextureHelper11 &texture,
                                                       d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = format;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + level;
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = gl::kCubeFaceCount;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorageCube.SRVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::createUAVForImage(const gl::Context *context,
                                                       int level,
                                                       DXGI_FORMAT format,
                                                       const TextureHelper11 &texture,
                                                       d3d11::SharedUAV *outUAV)
{
    ASSERT(outUAV);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format                         = format;
    uavDesc.ViewDimension                  = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.MipSlice        = mTopLevel + level;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize       = gl::kCubeFaceCount;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), uavDesc, texture.get(), outUAV));
    outUAV->setLabels("TexStorageCube.UAVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::getSwizzleTexture(const gl::Context *context,
                                                       const TextureHelper11 **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture.valid())
    {
        const auto &format = mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = mTextureWidth;
        desc.Height             = mTextureHeight;
        desc.MipLevels          = mMipLevels;
        desc.ArraySize          = gl::kCubeFaceCount;
        desc.Format             = format.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = D3D11_RESOURCE_MISC_TEXTURECUBE;

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, format,
                                             &mSwizzleTexture));
        mSwizzleTexture.setLabels("TexStorageCube.Swizzle", &mKHRDebugLabel);
    }

    *outTexture = &mSwizzleTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::getSwizzleRenderTarget(const gl::Context *context,
                                                            int mipLevel,
                                                            const d3d11::RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel].valid())
    {
        const TextureHelper11 *swizzleTexture = nullptr;
        ANGLE_TRY(getSwizzleTexture(context, &swizzleTexture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps()).rtvFormat;
        rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice        = mTopLevel + mipLevel;
        rtvDesc.Texture2DArray.FirstArraySlice = 0;
        rtvDesc.Texture2DArray.ArraySize       = gl::kCubeFaceCount;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                              mSwizzleTexture.get(),
                                              &mSwizzleRenderTargets[mipLevel]));
    }

    *outRTV = &mSwizzleRenderTargets[mipLevel];
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::ensureDropStencilTexture(const gl::Context *context,
                                                              DropStencil *dropStencilOut)
{
    if (mDropStencilTexture.valid())
    {
        *dropStencilOut = DropStencil::ALREADY_EXISTS;
        return angle::Result::Continue;
    }

    D3D11_TEXTURE2D_DESC dropDesc = {};
    dropDesc.ArraySize            = 6;
    dropDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    dropDesc.CPUAccessFlags       = 0;
    dropDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
    dropDesc.Height               = mTextureHeight;
    dropDesc.MipLevels            = mMipLevels;
    dropDesc.MiscFlags            = D3D11_RESOURCE_MISC_TEXTURECUBE;
    dropDesc.SampleDesc.Count     = 1;
    dropDesc.SampleDesc.Quality   = 0;
    dropDesc.Usage                = D3D11_USAGE_DEFAULT;
    dropDesc.Width                = mTextureWidth;

    const auto &format =
        d3d11::Format::Get(GL_DEPTH_COMPONENT32F, mRenderer->getRenderer11DeviceCaps());
    ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), dropDesc, format,
                                         &mDropStencilTexture));
    mDropStencilTexture.setLabels("TexStorageCube.DropStencil", &mKHRDebugLabel);

    ANGLE_TRY(initDropStencilTexture(context, gl::ImageIndexIterator::MakeCube(0, mMipLevels)));

    *dropStencilOut = DropStencil::CREATED;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Cube::resolveTexture(const gl::Context *context)
{
    if (mMSTexInfo && mMSTexInfo->msTex && mMSTexInfo->msTextureNeedsResolve)
    {
        ANGLE_TRY(resolveTextureHelper(context, mTexture));
        onStateChange(angle::SubjectMessage::ContentsChanged);
    }
    return angle::Result::Continue;
}

void TextureStorage11_Cube::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mLevelZeroTexture.valid())
    {
        mLevelZeroTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mSwizzleTexture.valid())
    {
        mSwizzleTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11_3D::TextureStorage11_3D(Renderer11 *renderer,
                                         GLenum internalformat,
                                         BindFlags bindFlags,
                                         GLsizei width,
                                         GLsizei height,
                                         GLsizei depth,
                                         int levels,
                                         const std::string &label)
    : TextureStorage11(
          renderer,
          GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), bindFlags),
          GetTextureMiscFlags(internalformat,
                              renderer->getRenderer11DeviceCaps(),
                              bindFlags,
                              levels),
          internalformat,
          label)
{
    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i]   = nullptr;
        mLevelRenderTargets[i] = nullptr;
    }

    // adjust size if needed for compressed textures
    d3d11::MakeValidSize(false, mFormatInfo.texFormat, &width, &height, &mTopLevel);

    mMipLevels     = mTopLevel + levels;
    mTextureWidth  = width;
    mTextureHeight = height;
    mTextureDepth  = depth;
}

angle::Result TextureStorage11_3D::onDestroy(const gl::Context *context)
{
    for (unsigned i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        if (mAssociatedImages[i] != nullptr)
        {
            mAssociatedImages[i]->verifyAssociatedStorageValid(this);

            // We must let the Images recover their data before we delete it from the
            // TextureStorage.
            ANGLE_TRY(mAssociatedImages[i]->recoverFromAssociatedStorage(context));
        }
    }

    return angle::Result::Continue;
}

TextureStorage11_3D::~TextureStorage11_3D() {}

void TextureStorage11_3D::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        mAssociatedImages[level] = image;
    }
}

void TextureStorage11_3D::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                     Image11 *expectedImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    // This validation check should never return false. It means the Image/TextureStorage
    // association is broken.
    ASSERT(mAssociatedImages[level] == expectedImage);
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_3D::disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(mAssociatedImages[level] == expectedImage);
    mAssociatedImages[level] = nullptr;
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image
// recover its data before ending the association.
angle::Result TextureStorage11_3D::releaseAssociatedImage(const gl::Context *context,
                                                          const gl::ImageIndex &index,
                                                          Image11 *incomingImage)
{
    const GLint level = index.getLevelIndex();

    ASSERT((0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS));

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // No need to let the old Image recover its data, if it is also the incoming Image.
        if (mAssociatedImages[level] != nullptr && mAssociatedImages[level] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage.
            mAssociatedImages[level]->verifyAssociatedStorageValid(this);

            // Force the image to recover from storage before its data is overwritten.
            // This will reset mAssociatedImages[level] to nullptr too.
            ANGLE_TRY(mAssociatedImages[level]->recoverFromAssociatedStorage(context));
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::getResource(const gl::Context *context,
                                               const TextureHelper11 **outResource)
{
    // If the width, height or depth are not positive this should be treated as an incomplete
    // texture. We handle that here by skipping the d3d texture creation.
    if (!mTexture.valid() && mTextureWidth > 0 && mTextureHeight > 0 && mTextureDepth > 0)
    {
        ASSERT(mMipLevels > 0);

        D3D11_TEXTURE3D_DESC desc;
        desc.Width     = mTextureWidth;
        desc.Height    = mTextureHeight;
        desc.Depth     = mTextureDepth;
        desc.MipLevels = mMipLevels;
        desc.Format =
            requiresTypelessTextureFormat() ? mFormatInfo.typelessFormat : mFormatInfo.texFormat;
        desc.Usage          = D3D11_USAGE_DEFAULT;
        desc.BindFlags      = getBindFlags();
        desc.CPUAccessFlags = 0;
        desc.MiscFlags      = getMiscFlags();

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             &mTexture));
        mTexture.setLabels("TexStorage3D", &mKHRDebugLabel);
    }

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::createSRVForSampler(const gl::Context *context,
                                                       int baseLevel,
                                                       int mipLevels,
                                                       DXGI_FORMAT format,
                                                       const TextureHelper11 &texture,
                                                       d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MostDetailedMip = baseLevel;
    srvDesc.Texture3D.MipLevels       = mipLevels;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage3D.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::createSRVForImage(const gl::Context *context,
                                                     int level,
                                                     DXGI_FORMAT format,
                                                     const TextureHelper11 &texture,
                                                     d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                    = format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MostDetailedMip = mTopLevel + level;
    srvDesc.Texture3D.MipLevels       = 1;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage3D.SRVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::createUAVForImage(const gl::Context *context,
                                                     int level,
                                                     DXGI_FORMAT format,
                                                     const TextureHelper11 &texture,
                                                     d3d11::SharedUAV *outUAV)
{
    ASSERT(outUAV);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format                = format;
    uavDesc.ViewDimension         = D3D11_UAV_DIMENSION_TEXTURE3D;
    uavDesc.Texture3D.MipSlice    = mTopLevel + level;
    uavDesc.Texture3D.FirstWSlice = 0;
    uavDesc.Texture3D.WSize       = mTextureDepth;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), uavDesc, texture.get(), outUAV));
    outUAV->setLabels("TexStorage3D.UAVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::findRenderTarget(const gl::Context *context,
                                                    const gl::ImageIndex &index,
                                                    GLsizei samples,
                                                    RenderTargetD3D **outRT) const
{
    const int mipLevel = index.getLevelIndex();
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    if (!index.hasLayer())
    {
        ASSERT(outRT);
        *outRT = mLevelRenderTargets[mipLevel].get();
        return angle::Result::Continue;
    }

    const int layer = index.getLayerIndex();

    LevelLayerKey key(mipLevel, layer);
    if (mLevelLayerRenderTargets.find(key) == mLevelLayerRenderTargets.end())
    {
        ASSERT(outRT);
        *outRT = nullptr;
        return angle::Result::Continue;
    }

    ASSERT(outRT);
    *outRT = mLevelLayerRenderTargets.at(key).get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::getRenderTarget(const gl::Context *context,
                                                   const gl::ImageIndex &index,
                                                   GLsizei samples,
                                                   RenderTargetD3D **outRT)
{
    const int mipLevel = index.getLevelIndex();
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    ASSERT(mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN);

    Context11 *context11 = GetImplAs<Context11>(context);

    if (!index.hasLayer())
    {
        if (!mLevelRenderTargets[mipLevel])
        {
            const TextureHelper11 *texture = nullptr;
            ANGLE_TRY(getResource(context, &texture));

            const d3d11::SharedSRV *srv = nullptr;
            ANGLE_TRY(getSRVLevel(context, mipLevel, SRVType::Sample, &srv));

            const d3d11::SharedSRV *blitSRV = nullptr;
            ANGLE_TRY(getSRVLevel(context, mipLevel, SRVType::Blit, &blitSRV));

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format                = mFormatInfo.rtvFormat;
            rtvDesc.ViewDimension         = D3D11_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice    = mTopLevel + mipLevel;
            rtvDesc.Texture3D.FirstWSlice = 0;
            rtvDesc.Texture3D.WSize       = static_cast<UINT>(-1);

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, texture->get(), &rtv));
            rtv.setLabels("TexStorage3D.RTV", &mKHRDebugLabel);

            mLevelRenderTargets[mipLevel].reset(new TextureRenderTarget11(
                std::move(rtv), *texture, *srv, *blitSRV, mFormatInfo.internalFormat,
                getFormatSet(), getLevelWidth(mipLevel), getLevelHeight(mipLevel),
                getLevelDepth(mipLevel), 0));
        }

        ASSERT(outRT);
        *outRT = mLevelRenderTargets[mipLevel].get();
        return angle::Result::Continue;
    }

    const int layer = index.getLayerIndex();

    LevelLayerKey key(mipLevel, layer);
    if (mLevelLayerRenderTargets.find(key) == mLevelLayerRenderTargets.end())
    {
        const TextureHelper11 *texture = nullptr;
        ANGLE_TRY(getResource(context, &texture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format                = mFormatInfo.rtvFormat;
        rtvDesc.ViewDimension         = D3D11_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice    = mTopLevel + mipLevel;
        rtvDesc.Texture3D.FirstWSlice = layer;
        rtvDesc.Texture3D.WSize       = 1;

        const d3d11::SharedSRV *srv = nullptr;
        ANGLE_TRY(getSRVLevel(context, mipLevel, SRVType::Sample, &srv));

        const d3d11::SharedSRV *blitSRV = nullptr;
        ANGLE_TRY(getSRVLevel(context, mipLevel, SRVType::Blit, &blitSRV));

        d3d11::RenderTargetView rtv;
        ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, texture->get(), &rtv));
        rtv.setLabels("TexStorage3D.LayerRTV", &mKHRDebugLabel);

        mLevelLayerRenderTargets[key].reset(new TextureRenderTarget11(
            std::move(rtv), *texture, *srv, *blitSRV, mFormatInfo.internalFormat, getFormatSet(),
            getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, 0));
    }

    ASSERT(outRT);
    *outRT = mLevelLayerRenderTargets[key].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::getSwizzleTexture(const gl::Context *context,
                                                     const TextureHelper11 **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture.valid())
    {
        const auto &format = mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE3D_DESC desc;
        desc.Width          = mTextureWidth;
        desc.Height         = mTextureHeight;
        desc.Depth          = mTextureDepth;
        desc.MipLevels      = mMipLevels;
        desc.Format         = format.texFormat;
        desc.Usage          = D3D11_USAGE_DEFAULT;
        desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags      = 0;

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, format,
                                             &mSwizzleTexture));
        mSwizzleTexture.setLabels("TexStorage3D.Swizzle", &mKHRDebugLabel);
    }

    *outTexture = &mSwizzleTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_3D::getSwizzleRenderTarget(const gl::Context *context,
                                                          int mipLevel,
                                                          const d3d11::RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel].valid())
    {
        const TextureHelper11 *swizzleTexture = nullptr;
        ANGLE_TRY(getSwizzleTexture(context, &swizzleTexture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps()).rtvFormat;
        rtvDesc.ViewDimension         = D3D11_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice    = mTopLevel + mipLevel;
        rtvDesc.Texture3D.FirstWSlice = 0;
        rtvDesc.Texture3D.WSize       = static_cast<UINT>(-1);

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                              mSwizzleTexture.get(),
                                              &mSwizzleRenderTargets[mipLevel]));
        mSwizzleRenderTargets[mipLevel].setLabels("TexStorage3D.SwizzleRTV", &mKHRDebugLabel);
    }

    *outRTV = &mSwizzleRenderTargets[mipLevel];
    return angle::Result::Continue;
}

void TextureStorage11_3D::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mSwizzleTexture.valid())
    {
        mSwizzleTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11_2DArray::TextureStorage11_2DArray(Renderer11 *renderer,
                                                   GLenum internalformat,
                                                   BindFlags bindFlags,
                                                   GLsizei width,
                                                   GLsizei height,
                                                   GLsizei depth,
                                                   int levels,
                                                   const std::string &label)
    : TextureStorage11(
          renderer,
          GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), bindFlags),
          GetTextureMiscFlags(internalformat,
                              renderer->getRenderer11DeviceCaps(),
                              bindFlags,
                              levels),
          internalformat,
          label)
{
    // adjust size if needed for compressed textures
    d3d11::MakeValidSize(false, mFormatInfo.texFormat, &width, &height, &mTopLevel);

    mMipLevels     = mTopLevel + levels;
    mTextureWidth  = width;
    mTextureHeight = height;
    mTextureDepth  = depth;
}

angle::Result TextureStorage11_2DArray::onDestroy(const gl::Context *context)
{
    for (auto iter : mAssociatedImages)
    {
        if (iter.second)
        {
            iter.second->verifyAssociatedStorageValid(this);

            // We must let the Images recover their data before we delete it from the
            // TextureStorage.
            ANGLE_TRY(iter.second->recoverFromAssociatedStorage(context));
        }
    }
    mAssociatedImages.clear();

    return angle::Result::Continue;
}

TextureStorage11_2DArray::~TextureStorage11_2DArray() {}

void TextureStorage11_2DArray::associateImage(Image11 *image, const gl::ImageIndex &index)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.getLayerIndex();
    const GLint numLayers   = index.getLayerCount();

    ASSERT(0 <= level && level < getLevelCount());

    if (0 <= level && level < getLevelCount())
    {
        LevelLayerRangeKey key(level, layerTarget, numLayers);
        mAssociatedImages[key] = image;
    }
}

void TextureStorage11_2DArray::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                          Image11 *expectedImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.getLayerIndex();
    const GLint numLayers   = index.getLayerCount();

    LevelLayerRangeKey key(level, layerTarget, numLayers);

    // This validation check should never return false. It means the Image/TextureStorage
    // association is broken.
    bool retValue = (mAssociatedImages.find(key) != mAssociatedImages.end() &&
                     (mAssociatedImages[key] == expectedImage));
    ASSERT(retValue);
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_2DArray::disassociateImage(const gl::ImageIndex &index,
                                                 Image11 *expectedImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.getLayerIndex();
    const GLint numLayers   = index.getLayerCount();

    LevelLayerRangeKey key(level, layerTarget, numLayers);

    bool imageAssociationCorrect = (mAssociatedImages.find(key) != mAssociatedImages.end() &&
                                    (mAssociatedImages[key] == expectedImage));
    ASSERT(imageAssociationCorrect);
    mAssociatedImages[key] = nullptr;
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image
// recover its data before ending the association.
angle::Result TextureStorage11_2DArray::releaseAssociatedImage(const gl::Context *context,
                                                               const gl::ImageIndex &index,
                                                               Image11 *incomingImage)
{
    const GLint level       = index.getLevelIndex();
    const GLint layerTarget = index.getLayerIndex();
    const GLint numLayers   = index.getLayerCount();

    LevelLayerRangeKey key(level, layerTarget, numLayers);

    if (mAssociatedImages.find(key) != mAssociatedImages.end())
    {
        if (mAssociatedImages[key] != nullptr && mAssociatedImages[key] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage.
            mAssociatedImages[key]->verifyAssociatedStorageValid(this);

            // Force the image to recover from storage before its data is overwritten.
            // This will reset mAssociatedImages[level] to nullptr too.
            ANGLE_TRY(mAssociatedImages[key]->recoverFromAssociatedStorage(context));
        }
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::getResource(const gl::Context *context,
                                                    const TextureHelper11 **outResource)
{
    // if the width, height or depth is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (!mTexture.valid() && mTextureWidth > 0 && mTextureHeight > 0 && mTextureDepth > 0)
    {
        ASSERT(mMipLevels > 0);

        D3D11_TEXTURE2D_DESC desc;
        desc.Width     = mTextureWidth;
        desc.Height    = mTextureHeight;
        desc.MipLevels = mMipLevels;
        desc.ArraySize = mTextureDepth;
        desc.Format =
            requiresTypelessTextureFormat() ? mFormatInfo.typelessFormat : mFormatInfo.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = getBindFlags();
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = getMiscFlags();

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             &mTexture));
        mTexture.setLabels("TexStorage2DArray", &mKHRDebugLabel);
    }

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::createSRVForSampler(const gl::Context *context,
                                                            int baseLevel,
                                                            int mipLevels,
                                                            DXGI_FORMAT format,
                                                            const TextureHelper11 &texture,
                                                            d3d11::SharedSRV *outSRV)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = format;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + baseLevel;
    srvDesc.Texture2DArray.MipLevels       = mipLevels;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = mTextureDepth;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2DArray.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::createSRVForImage(const gl::Context *context,
                                                          int level,
                                                          DXGI_FORMAT format,
                                                          const TextureHelper11 &texture,
                                                          d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = format;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + level;
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize       = mTextureDepth;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2DArray.SRVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::createUAVForImage(const gl::Context *context,
                                                          int level,
                                                          DXGI_FORMAT format,
                                                          const TextureHelper11 &texture,
                                                          d3d11::SharedUAV *outUAV)
{
    ASSERT(outUAV);
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format                         = format;
    uavDesc.ViewDimension                  = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.MipSlice        = mTopLevel + level;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize       = mTextureDepth;
    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), uavDesc, texture.get(), outUAV));
    outUAV->setLabels("TexStorage2DArray.UAVForImage", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::findRenderTarget(const gl::Context *context,
                                                         const gl::ImageIndex &index,
                                                         GLsizei samples,
                                                         RenderTargetD3D **outRT) const
{
    ASSERT(index.hasLayer());

    const int mipLevel  = index.getLevelIndex();
    const int layer     = index.getLayerIndex();
    const int numLayers = index.getLayerCount();

    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    LevelLayerRangeKey key(mipLevel, layer, numLayers);
    if (mRenderTargets.find(key) == mRenderTargets.end())
    {
        ASSERT(outRT);
        *outRT = nullptr;
        return angle::Result::Continue;
    }

    ASSERT(outRT);
    *outRT = mRenderTargets.at(key).get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::createRenderTargetSRV(const gl::Context *context,
                                                              const TextureHelper11 &texture,
                                                              const gl::ImageIndex &index,
                                                              DXGI_FORMAT resourceFormat,
                                                              d3d11::SharedSRV *srv) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                         = resourceFormat;
    srvDesc.ViewDimension                  = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + index.getLevelIndex();
    srvDesc.Texture2DArray.MipLevels       = 1;
    srvDesc.Texture2DArray.FirstArraySlice = index.getLayerIndex();
    srvDesc.Texture2DArray.ArraySize       = index.getLayerCount();

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), srv));

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::getRenderTarget(const gl::Context *context,
                                                        const gl::ImageIndex &index,
                                                        GLsizei samples,
                                                        RenderTargetD3D **outRT)
{
    ASSERT(index.hasLayer());

    const int mipLevel  = index.getLevelIndex();
    const int layer     = index.getLayerIndex();
    const int numLayers = index.getLayerCount();

    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    LevelLayerRangeKey key(mipLevel, layer, numLayers);
    if (mRenderTargets.find(key) == mRenderTargets.end())
    {
        const TextureHelper11 *texture = nullptr;
        ANGLE_TRY(getResource(context, &texture));
        d3d11::SharedSRV srv;
        ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.srvFormat, &srv));
        d3d11::SharedSRV blitSRV;
        if (mFormatInfo.blitSRVFormat != mFormatInfo.srvFormat)
        {
            ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.blitSRVFormat,
                                            &blitSRV));
        }
        else
        {
            blitSRV = srv.makeCopy();
        }

        srv.setLabels("TexStorage2DArray.RenderTargetSRV", &mKHRDebugLabel);

        if (mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format                         = mFormatInfo.rtvFormat;
            rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice        = mTopLevel + mipLevel;
            rtvDesc.Texture2DArray.FirstArraySlice = layer;
            rtvDesc.Texture2DArray.ArraySize       = numLayers;

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                                  texture->get(), &rtv));
            rtv.setLabels("TexStorage2DArray.RenderTargetRTV", &mKHRDebugLabel);

            mRenderTargets[key].reset(new TextureRenderTarget11(
                std::move(rtv), *texture, srv, blitSRV, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, 0));
        }
        else
        {
            ASSERT(mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format                         = mFormatInfo.dsvFormat;
            dsvDesc.ViewDimension                  = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.MipSlice        = mTopLevel + mipLevel;
            dsvDesc.Texture2DArray.FirstArraySlice = layer;
            dsvDesc.Texture2DArray.ArraySize       = numLayers;
            dsvDesc.Flags                          = 0;

            d3d11::DepthStencilView dsv;
            ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), dsvDesc,
                                                  texture->get(), &dsv));
            dsv.setLabels("TexStorage2DArray.RenderTargetDSV", &mKHRDebugLabel);

            mRenderTargets[key].reset(new TextureRenderTarget11(
                std::move(dsv), *texture, srv, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, 0));
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTargets[key].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::getSwizzleTexture(const gl::Context *context,
                                                          const TextureHelper11 **outTexture)
{
    if (!mSwizzleTexture.valid())
    {
        const auto &format = mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = mTextureWidth;
        desc.Height             = mTextureHeight;
        desc.MipLevels          = mMipLevels;
        desc.ArraySize          = mTextureDepth;
        desc.Format             = format.texFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, format,
                                             &mSwizzleTexture));
        mSwizzleTexture.setLabels("TexStorage2DArray.Swizzle", &mKHRDebugLabel);
    }

    *outTexture = &mSwizzleTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel].valid())
    {
        const TextureHelper11 *swizzleTexture = nullptr;
        ANGLE_TRY(getSwizzleTexture(context, &swizzleTexture));

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format =
            mFormatInfo.getSwizzleFormat(mRenderer->getRenderer11DeviceCaps()).rtvFormat;
        rtvDesc.ViewDimension                  = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice        = mTopLevel + mipLevel;
        rtvDesc.Texture2DArray.FirstArraySlice = 0;
        rtvDesc.Texture2DArray.ArraySize       = mTextureDepth;

        ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                              mSwizzleTexture.get(),
                                              &mSwizzleRenderTargets[mipLevel]));
    }

    *outRTV = &mSwizzleRenderTargets[mipLevel];
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DArray::ensureDropStencilTexture(const gl::Context *context,
                                                                 DropStencil *dropStencilOut)
{
    if (mDropStencilTexture.valid())
    {
        *dropStencilOut = DropStencil::ALREADY_EXISTS;
        return angle::Result::Continue;
    }

    D3D11_TEXTURE2D_DESC dropDesc = {};
    dropDesc.ArraySize            = mTextureDepth;
    dropDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
    dropDesc.CPUAccessFlags       = 0;
    dropDesc.Format               = DXGI_FORMAT_R32_TYPELESS;
    dropDesc.Height               = mTextureHeight;
    dropDesc.MipLevels            = mMipLevels;
    dropDesc.MiscFlags            = 0;
    dropDesc.SampleDesc.Count     = 1;
    dropDesc.SampleDesc.Quality   = 0;
    dropDesc.Usage                = D3D11_USAGE_DEFAULT;
    dropDesc.Width                = mTextureWidth;

    const auto &format =
        d3d11::Format::Get(GL_DEPTH_COMPONENT32F, mRenderer->getRenderer11DeviceCaps());
    ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), dropDesc, format,
                                         &mDropStencilTexture));
    mDropStencilTexture.setLabels("TexStorage2DArray.DropStencil", &mKHRDebugLabel);

    std::vector<GLsizei> layerCounts(mMipLevels, mTextureDepth);

    ANGLE_TRY(initDropStencilTexture(
        context, gl::ImageIndexIterator::Make2DArray(0, mMipLevels, layerCounts.data())));

    *dropStencilOut = DropStencil::CREATED;
    return angle::Result::Continue;
}

void TextureStorage11_2DArray::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
    if (mSwizzleTexture.valid())
    {
        mSwizzleTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11_2DMultisample::TextureStorage11_2DMultisample(Renderer11 *renderer,
                                                               GLenum internalformat,
                                                               GLsizei width,
                                                               GLsizei height,
                                                               int levels,
                                                               int samples,
                                                               bool fixedSampleLocations,
                                                               const std::string &label)
    : TextureStorage11ImmutableBase(renderer,
                                    GetTextureBindFlags(internalformat,
                                                        renderer->getRenderer11DeviceCaps(),
                                                        BindFlags::RenderTarget()),
                                    GetTextureMiscFlags(internalformat,
                                                        renderer->getRenderer11DeviceCaps(),
                                                        BindFlags::RenderTarget(),
                                                        levels),
                                    internalformat,
                                    label),
      mTexture(),
      mRenderTarget(nullptr)
{
    // There are no multisampled compressed formats, so there's no need to adjust texture size
    // according to block size.
    ASSERT(d3d11::GetDXGIFormatSizeInfo(mFormatInfo.texFormat).blockWidth <= 1);
    ASSERT(d3d11::GetDXGIFormatSizeInfo(mFormatInfo.texFormat).blockHeight <= 1);

    mMipLevels            = 1;
    mTextureWidth         = width;
    mTextureHeight        = height;
    mTextureDepth         = 1;
    mSamples              = samples;
    mFixedSampleLocations = fixedSampleLocations;
}

angle::Result TextureStorage11_2DMultisample::onDestroy(const gl::Context *context)
{
    mRenderTarget.reset();
    return angle::Result::Continue;
}

TextureStorage11_2DMultisample::~TextureStorage11_2DMultisample() {}

angle::Result TextureStorage11_2DMultisample::copyToStorage(const gl::Context *context,
                                                            TextureStorage *destStorage)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisample::getResource(const gl::Context *context,
                                                          const TextureHelper11 **outResource)
{
    ANGLE_TRY(ensureTextureExists(context, 1));

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisample::ensureTextureExists(const gl::Context *context,
                                                                  int mipLevels)
{
    // For Multisampled textures, mipLevels always equals 1.
    ASSERT(mipLevels == 1);

    // if the width or height is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (!mTexture.valid() && mTextureWidth > 0 && mTextureHeight > 0)
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width          = mTextureWidth;  // Compressed texture size constraints?
        desc.Height         = mTextureHeight;
        desc.MipLevels      = mipLevels;
        desc.ArraySize      = 1;
        desc.Format         = mFormatInfo.texFormat;
        desc.Usage          = D3D11_USAGE_DEFAULT;
        desc.BindFlags      = getBindFlags() & ~D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags      = getMiscFlags();

        const gl::TextureCaps &textureCaps =
            mRenderer->getNativeTextureCaps().get(mFormatInfo.internalFormat);
        GLuint supportedSamples = textureCaps.getNearestSamples(mSamples);
        desc.SampleDesc.Count   = (supportedSamples == 0) ? 1 : supportedSamples;
        desc.SampleDesc.Quality = mRenderer->getSampleDescQuality(supportedSamples);

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             &mTexture));
        mTexture.setLabels("TexStorage2DMS", &mKHRDebugLabel);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisample::findRenderTarget(const gl::Context *context,
                                                               const gl::ImageIndex &index,
                                                               GLsizei samples,
                                                               RenderTargetD3D **outRT) const
{
    ASSERT(!index.hasLayer());

    const int level = index.getLevelIndex();
    ASSERT(level == 0);

    ASSERT(outRT);
    *outRT = mRenderTarget.get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisample::getRenderTarget(const gl::Context *context,
                                                              const gl::ImageIndex &index,
                                                              GLsizei samples,
                                                              RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());

    const int level = index.getLevelIndex();
    ASSERT(level == 0);

    ASSERT(outRT);
    if (mRenderTarget)
    {
        *outRT = mRenderTarget.get();
        return angle::Result::Continue;
    }

    const TextureHelper11 *texture = nullptr;
    ANGLE_TRY(getResource(context, &texture));

    const d3d11::SharedSRV *srv = nullptr;
    ANGLE_TRY(getSRVLevel(context, level, SRVType::Sample, &srv));

    const d3d11::SharedSRV *blitSRV = nullptr;
    ANGLE_TRY(getSRVLevel(context, level, SRVType::Blit, &blitSRV));

    Context11 *context11 = GetImplAs<Context11>(context);

    if (mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format        = mFormatInfo.rtvFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

        d3d11::RenderTargetView rtv;
        ANGLE_TRY(mRenderer->allocateResource(context11, rtvDesc, texture->get(), &rtv));

        mRenderTarget.reset(new TextureRenderTarget11(
            std::move(rtv), *texture, *srv, *blitSRV, mFormatInfo.internalFormat, getFormatSet(),
            getLevelWidth(level), getLevelHeight(level), 1, mSamples));

        *outRT = mRenderTarget.get();
        return angle::Result::Continue;
    }

    ASSERT(mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Format        = mFormatInfo.dsvFormat;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
    dsvDesc.Flags         = 0;

    d3d11::DepthStencilView dsv;
    ANGLE_TRY(mRenderer->allocateResource(context11, dsvDesc, texture->get(), &dsv));

    mRenderTarget.reset(new TextureRenderTarget11(
        std::move(dsv), *texture, *srv, mFormatInfo.internalFormat, getFormatSet(),
        getLevelWidth(level), getLevelHeight(level), 1, mSamples));

    *outRT = mRenderTarget.get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisample::createSRVForSampler(const gl::Context *context,
                                                                  int baseLevel,
                                                                  int mipLevels,
                                                                  DXGI_FORMAT format,
                                                                  const TextureHelper11 &texture,
                                                                  d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format        = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2DMS.SRV", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisample::getSwizzleTexture(const gl::Context *context,
                                                                const TextureHelper11 **outTexture)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisample::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisample::ensureDropStencilTexture(const gl::Context *context,
                                                                       DropStencil *dropStencilOut)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

void TextureStorage11_2DMultisample::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11_2DMultisampleArray::TextureStorage11_2DMultisampleArray(Renderer11 *renderer,
                                                                         GLenum internalformat,
                                                                         GLsizei width,
                                                                         GLsizei height,
                                                                         GLsizei depth,
                                                                         int levels,
                                                                         int samples,
                                                                         bool fixedSampleLocations,
                                                                         const std::string &label)
    : TextureStorage11ImmutableBase(renderer,
                                    GetTextureBindFlags(internalformat,
                                                        renderer->getRenderer11DeviceCaps(),
                                                        BindFlags::RenderTarget()),
                                    GetTextureMiscFlags(internalformat,
                                                        renderer->getRenderer11DeviceCaps(),
                                                        BindFlags::RenderTarget(),
                                                        levels),
                                    internalformat,
                                    label),
      mTexture()
{
    // There are no multisampled compressed formats, so there's no need to adjust texture size
    // according to block size.
    ASSERT(d3d11::GetDXGIFormatSizeInfo(mFormatInfo.texFormat).blockWidth <= 1);
    ASSERT(d3d11::GetDXGIFormatSizeInfo(mFormatInfo.texFormat).blockHeight <= 1);

    mMipLevels            = 1;
    mTextureWidth         = width;
    mTextureHeight        = height;
    mTextureDepth         = depth;
    mSamples              = samples;
    mFixedSampleLocations = fixedSampleLocations;
}

angle::Result TextureStorage11_2DMultisampleArray::onDestroy(const gl::Context *context)
{
    return angle::Result::Continue;
}

TextureStorage11_2DMultisampleArray::~TextureStorage11_2DMultisampleArray() {}

angle::Result TextureStorage11_2DMultisampleArray::copyToStorage(const gl::Context *context,
                                                                 TextureStorage *destStorage)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisampleArray::getResource(const gl::Context *context,
                                                               const TextureHelper11 **outResource)
{
    ANGLE_TRY(ensureTextureExists(context, 1));

    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::ensureTextureExists(const gl::Context *context,
                                                                       int mipLevels)
{
    // For multisampled textures, mipLevels always equals 1.
    ASSERT(mipLevels == 1);

    // if the width or height is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (!mTexture.valid() && mTextureWidth > 0 && mTextureHeight > 0)
    {
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width          = mTextureWidth;
        desc.Height         = mTextureHeight;
        desc.MipLevels      = mipLevels;
        desc.ArraySize      = mTextureDepth;
        desc.Format         = mFormatInfo.texFormat;
        desc.Usage          = D3D11_USAGE_DEFAULT;
        desc.BindFlags      = getBindFlags() & ~D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags      = getMiscFlags();

        const gl::TextureCaps &textureCaps =
            mRenderer->getNativeTextureCaps().get(mFormatInfo.internalFormat);
        GLuint supportedSamples = textureCaps.getNearestSamples(mSamples);
        desc.SampleDesc.Count   = (supportedSamples == 0) ? 1 : supportedSamples;
        desc.SampleDesc.Quality = mRenderer->getSampleDescQuality(supportedSamples);

        ANGLE_TRY(mRenderer->allocateTexture(GetImplAs<Context11>(context), desc, mFormatInfo,
                                             &mTexture));
        mTexture.setLabels("TexStorage2DMSArray", &mKHRDebugLabel);
    }

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::findRenderTarget(const gl::Context *context,
                                                                    const gl::ImageIndex &index,
                                                                    GLsizei samples,
                                                                    RenderTargetD3D **outRT) const
{
    ASSERT(index.hasLayer());

    const int mipLevel = index.getLevelIndex();
    ASSERT(mipLevel == 0);
    const int layer     = index.getLayerIndex();
    const int numLayers = index.getLayerCount();

    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    TextureStorage11_2DArray::LevelLayerRangeKey key(mipLevel, layer, numLayers);
    if (mRenderTargets.find(key) == mRenderTargets.end())
    {
        ASSERT(outRT);
        *outRT = nullptr;
        return angle::Result::Continue;
    }

    ASSERT(outRT);
    *outRT = mRenderTargets.at(key).get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::createRenderTargetSRV(
    const gl::Context *context,
    const TextureHelper11 &texture,
    const gl::ImageIndex &index,
    DXGI_FORMAT resourceFormat,
    d3d11::SharedSRV *srv) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                           = resourceFormat;
    srvDesc.ViewDimension                    = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
    srvDesc.Texture2DMSArray.FirstArraySlice = index.getLayerIndex();
    srvDesc.Texture2DMSArray.ArraySize       = index.getLayerCount();

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), srv));

    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::getRenderTarget(const gl::Context *context,
                                                                   const gl::ImageIndex &index,
                                                                   GLsizei samples,
                                                                   RenderTargetD3D **outRT)
{
    ASSERT(index.hasLayer());

    const int mipLevel = index.getLevelIndex();
    ASSERT(mipLevel == 0);
    const int layer     = index.getLayerIndex();
    const int numLayers = index.getLayerCount();

    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    TextureStorage11_2DArray::LevelLayerRangeKey key(mipLevel, layer, numLayers);
    if (mRenderTargets.find(key) == mRenderTargets.end())
    {
        const TextureHelper11 *texture = nullptr;
        ANGLE_TRY(getResource(context, &texture));
        d3d11::SharedSRV srv;
        ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.srvFormat, &srv));
        d3d11::SharedSRV blitSRV;
        if (mFormatInfo.blitSRVFormat != mFormatInfo.srvFormat)
        {
            ANGLE_TRY(createRenderTargetSRV(context, *texture, index, mFormatInfo.blitSRVFormat,
                                            &blitSRV));
        }
        else
        {
            blitSRV = srv.makeCopy();
        }

        srv.setLabels("TexStorage2DMSArray.RenderTargetSRV", &mKHRDebugLabel);

        if (mFormatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format                           = mFormatInfo.rtvFormat;
            rtvDesc.ViewDimension                    = D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY;
            rtvDesc.Texture2DMSArray.FirstArraySlice = layer;
            rtvDesc.Texture2DMSArray.ArraySize       = numLayers;

            d3d11::RenderTargetView rtv;
            ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), rtvDesc,
                                                  texture->get(), &rtv));
            rtv.setLabels("TexStorage2DMSArray.RenderTargetRTV", &mKHRDebugLabel);

            mRenderTargets[key].reset(new TextureRenderTarget11(
                std::move(rtv), *texture, srv, blitSRV, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, mSamples));
        }
        else
        {
            ASSERT(mFormatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format                           = mFormatInfo.dsvFormat;
            dsvDesc.ViewDimension                    = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
            dsvDesc.Texture2DMSArray.FirstArraySlice = layer;
            dsvDesc.Texture2DMSArray.ArraySize       = numLayers;
            dsvDesc.Flags                            = 0;

            d3d11::DepthStencilView dsv;
            ANGLE_TRY(mRenderer->allocateResource(GetImplAs<Context11>(context), dsvDesc,
                                                  texture->get(), &dsv));
            dsv.setLabels("TexStorage2DMSArray.RenderTargetDSV", &mKHRDebugLabel);

            mRenderTargets[key].reset(new TextureRenderTarget11(
                std::move(dsv), *texture, srv, mFormatInfo.internalFormat, getFormatSet(),
                getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, mSamples));
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTargets[key].get();
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::createSRVForSampler(
    const gl::Context *context,
    int baseLevel,
    int mipLevels,
    DXGI_FORMAT format,
    const TextureHelper11 &texture,
    d3d11::SharedSRV *outSRV)
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format                           = format;
    srvDesc.ViewDimension                    = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
    srvDesc.Texture2DMSArray.FirstArraySlice = 0;
    srvDesc.Texture2DMSArray.ArraySize       = mTextureDepth;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexStorage2DMSArray.SRV", &mKHRDebugLabel);
    return angle::Result::Continue;
}

angle::Result TextureStorage11_2DMultisampleArray::getSwizzleTexture(
    const gl::Context *context,
    const TextureHelper11 **outTexture)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisampleArray::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_2DMultisampleArray::ensureDropStencilTexture(
    const gl::Context *context,
    DropStencil *dropStencilOut)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

void TextureStorage11_2DMultisampleArray::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

TextureStorage11_Buffer::TextureStorage11_Buffer(Renderer11 *renderer,
                                                 const gl::OffsetBindingPointer<gl::Buffer> &buffer,
                                                 GLenum internalFormat,
                                                 const std::string &label)
    : TextureStorage11(renderer,
                       D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
                       0,
                       internalFormat,
                       label),
      mTexture(),
      mBuffer(buffer),
      mDataSize(GetBoundBufferAvailableSize(buffer))
{
    unsigned int bytesPerPixel =
        static_cast<unsigned int>(d3d11::GetDXGIFormatSizeInfo(mFormatInfo.srvFormat).pixelBytes);
    mMipLevels     = 1;
    mTextureWidth  = static_cast<unsigned int>(mDataSize / bytesPerPixel);
    mTextureHeight = 1;
    mTextureDepth  = 1;
}

TextureStorage11_Buffer::~TextureStorage11_Buffer() {}

angle::Result TextureStorage11_Buffer::initTexture(const gl::Context *context)
{
    if (!mTexture.valid())
    {
        ID3D11Buffer *buffer = nullptr;
        Buffer11 *buffer11   = GetImplAs<Buffer11>(mBuffer.get());
        ANGLE_TRY(buffer11->getBuffer(context, rx::BufferUsage::BUFFER_USAGE_TYPED_UAV, &buffer));
        mTexture.set(buffer, mFormatInfo);
        mTexture.get()->AddRef();
    }
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Buffer::getResource(const gl::Context *context,
                                                   const TextureHelper11 **outResource)
{
    ANGLE_TRY(initTexture(context));
    *outResource = &mTexture;
    return angle::Result::Continue;
}

angle::Result TextureStorage11_Buffer::getMippedResource(const gl::Context *context,
                                                         const TextureHelper11 **)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_Buffer::findRenderTarget(const gl::Context *context,
                                                        const gl::ImageIndex &index,
                                                        GLsizei samples,
                                                        RenderTargetD3D **outRT) const
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_Buffer::getRenderTarget(const gl::Context *context,
                                                       const gl::ImageIndex &index,
                                                       GLsizei samples,
                                                       RenderTargetD3D **outRT)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_Buffer::getSwizzleTexture(const gl::Context *context,
                                                         const TextureHelper11 **outTexture)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_Buffer::getSwizzleRenderTarget(
    const gl::Context *context,
    int mipLevel,
    const d3d11::RenderTargetView **outRTV)
{
    ANGLE_HR_UNREACHABLE(GetImplAs<Context11>(context));
    return angle::Result::Stop;
}

angle::Result TextureStorage11_Buffer::createSRVForSampler(const gl::Context *context,
                                                           int baseLevel,
                                                           int mipLevels,
                                                           DXGI_FORMAT format,
                                                           const TextureHelper11 &texture,
                                                           d3d11::SharedSRV *outSRV)
{
    ASSERT(baseLevel == 0);
    ASSERT(mipLevels == 1);
    ASSERT(outSRV);
    ANGLE_TRY(initTexture(context));
    UINT bytesPerPixel = static_cast<UINT>(d3d11::GetDXGIFormatSizeInfo(format).pixelBytes);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format        = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    ASSERT(mBuffer.getOffset() % bytesPerPixel == 0);
    srvDesc.Buffer.FirstElement = static_cast<UINT>(mBuffer.getOffset() / bytesPerPixel);
    srvDesc.Buffer.NumElements  = static_cast<UINT>(mDataSize / bytesPerPixel);

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexBuffer.SRV", &mKHRDebugLabel);

    return angle::Result::Continue;
}

angle::Result TextureStorage11_Buffer::createSRVForImage(const gl::Context *context,
                                                         int level,
                                                         DXGI_FORMAT format,
                                                         const TextureHelper11 &texture,
                                                         d3d11::SharedSRV *outSRV)
{
    ANGLE_TRY(initTexture(context));
    UINT bytesPerPixel = static_cast<UINT>(d3d11::GetDXGIFormatSizeInfo(format).pixelBytes);
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format        = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    ASSERT(mBuffer.getOffset() % bytesPerPixel == 0);
    srvDesc.Buffer.FirstElement = static_cast<UINT>(mBuffer.getOffset() / bytesPerPixel);
    srvDesc.Buffer.NumElements  = static_cast<UINT>(mDataSize / bytesPerPixel);

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), srvDesc, texture.get(), outSRV));
    outSRV->setLabels("TexBuffer.SRVForImage", &mKHRDebugLabel);

    return angle::Result::Continue;
}
angle::Result TextureStorage11_Buffer::createUAVForImage(const gl::Context *context,
                                                         int level,
                                                         DXGI_FORMAT format,
                                                         const TextureHelper11 &texture,
                                                         d3d11::SharedUAV *outUAV)
{
    ANGLE_TRY(initTexture(context));
    unsigned bytesPerPixel = d3d11::GetDXGIFormatSizeInfo(format).pixelBytes;
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format        = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    ASSERT(mBuffer.getOffset() % bytesPerPixel == 0);
    uavDesc.Buffer.FirstElement = static_cast<UINT>(mBuffer.getOffset() / bytesPerPixel);
    uavDesc.Buffer.NumElements  = static_cast<UINT>(mDataSize / bytesPerPixel);
    uavDesc.Buffer.Flags        = 0;

    ANGLE_TRY(
        mRenderer->allocateResource(GetImplAs<Context11>(context), uavDesc, texture.get(), outUAV));
    outUAV->setLabels("TexBuffer.UAVForImage", &mKHRDebugLabel);

    return angle::Result::Continue;
}

void TextureStorage11_Buffer::associateImage(Image11 *image, const gl::ImageIndex &index) {}
void TextureStorage11_Buffer::disassociateImage(const gl::ImageIndex &index, Image11 *expectedImage)
{}
void TextureStorage11_Buffer::verifyAssociatedImageValid(const gl::ImageIndex &index,
                                                         Image11 *expectedImage)
{}
angle::Result TextureStorage11_Buffer::releaseAssociatedImage(const gl::Context *context,
                                                              const gl::ImageIndex &index,
                                                              Image11 *incomingImage)
{
    return angle::Result::Continue;
}

void TextureStorage11_Buffer::onLabelUpdate()
{
    if (mTexture.valid())
    {
        mTexture.setKHRDebugLabel(&mKHRDebugLabel);
    }
}

}  // namespace rx
