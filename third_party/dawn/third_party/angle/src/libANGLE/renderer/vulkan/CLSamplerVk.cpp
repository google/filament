//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerVk.cpp: Implements the class methods for CLSamplerVk.

#include "libANGLE/renderer/vulkan/CLSamplerVk.h"
#include "common/PackedCLEnums_autogen.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"

#include "libANGLE/CLContext.h"
#include "libANGLE/CLSampler.h"
#include "libANGLE/cl_utils.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

using namespace clspv;

namespace rx
{

VkSamplerAddressMode CLSamplerVk::getVkAddressMode()
{
    VkSamplerAddressMode addressMode;
    switch (mSampler.getAddressingMode())
    {
        case cl::AddressingMode::None:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case cl::AddressingMode::ClampToEdge:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
        case cl::AddressingMode::Clamp:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
            break;
        case cl::AddressingMode::Repeat:
            addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            break;
        case cl::AddressingMode::MirroredRepeat:
            addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            break;
        default:
            addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            break;
    }

    if (!mSampler.getNormalizedCoords() && (addressMode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
                                            addressMode != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))
    {
        addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }

    return addressMode;
}

VkFilter CLSamplerVk::getVkFilter()
{
    switch (mSampler.getFilterMode())
    {
        case cl::FilterMode::Nearest:
            return VK_FILTER_NEAREST;
        case cl::FilterMode::Linear:
            return VK_FILTER_LINEAR;
        default:
            return VK_FILTER_LINEAR;
    }
}

VkSamplerMipmapMode CLSamplerVk::getVkMipmapMode()
{
    if (mSampler.getNormalizedCoords())
    {
        switch (mSampler.getFilterMode())
        {
            case cl::FilterMode::Nearest:
                return VK_SAMPLER_MIPMAP_MODE_NEAREST;
            case cl::FilterMode::Linear:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
            default:
                return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
    }

    return VK_SAMPLER_MIPMAP_MODE_NEAREST;
}

uint32_t CLSamplerVk::getSamplerMask()
{
    uint32_t mask = 0;
    if (mSampler.getNormalizedCoords())
    {
        mask |= SamplerNormalizedCoords::CLK_NORMALIZED_COORDS_TRUE;
    }
    switch (mSampler.getAddressingMode())
    {
        case cl::AddressingMode::None:
            break;
        case cl::AddressingMode::ClampToEdge:
            mask |= SamplerAddressingMode::CLK_ADDRESS_CLAMP_TO_EDGE;
            break;
        case cl::AddressingMode::Clamp:
            mask |= SamplerAddressingMode::CLK_ADDRESS_CLAMP;
            break;
        case cl::AddressingMode::Repeat:
            mask |= SamplerAddressingMode::CLK_ADDRESS_REPEAT;
            break;
        case cl::AddressingMode::MirroredRepeat:
            mask |= SamplerAddressingMode::CLK_ADDRESS_MIRRORED_REPEAT;
            break;
        default:
            break;
    }
    switch (mSampler.getFilterMode())
    {
        case cl::FilterMode::Nearest:
            mask |= SamplerFilterMode::CLK_FILTER_NEAREST;
            break;
        case cl::FilterMode::Linear:
        default:
            mask |= SamplerFilterMode::CLK_FILTER_LINEAR;
            break;
    }
    return mask;
}

CLSamplerVk::CLSamplerVk(const cl::Sampler &sampler)
    : CLSamplerImpl(sampler),
      mContext(&sampler.getContext().getImpl<CLContextVk>()),
      mRenderer(mContext->getRenderer()),
      mSamplerHelper(),
      mSamplerHelperNormalized()
{
    VkSamplerAddressMode addressMode = getVkAddressMode();
    VkFilter filter                  = getVkFilter();
    VkSamplerMipmapMode mipmapMode   = getVkMipmapMode();
    VkBool32 unnormalizedCoordinates = !(mSampler.getNormalizedCoords());

    mDefaultSamplerCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0x0,
        .magFilter               = filter,
        .minFilter               = filter,
        .mipmapMode              = mipmapMode,
        .addressModeU            = addressMode,
        .addressModeV            = addressMode,
        .addressModeW            = addressMode,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_FALSE,
        .maxAnisotropy           = 0.0f,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_NEVER,
        .minLod                  = 0.0f,
        .maxLod                  = 0.0f,
        .borderColor             = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = unnormalizedCoordinates,
    };
}

CLSamplerVk::~CLSamplerVk()
{
    mSamplerHelper.destroy(mContext->getDevice());
    if (mSamplerHelperNormalized.valid())
    {
        mSamplerHelperNormalized.destroy(mContext->getDevice());
    }
}

angle::Result CLSamplerVk::create()
{
    ANGLE_TRY(mSamplerHelper.init(mContext, mDefaultSamplerCreateInfo));

    return angle::Result::Continue;
}

angle::Result CLSamplerVk::createNormalized()
{
    if (mSamplerHelperNormalized.valid())
    {
        mDefaultSamplerCreateInfo.unnormalizedCoordinates = false;
        ANGLE_TRY(mSamplerHelperNormalized.init(mContext, mDefaultSamplerCreateInfo));
    }

    return angle::Result::Continue;
}

}  // namespace rx
