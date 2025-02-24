//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLSamplerVk.h: Defines the class interface for CLSamplerVk, implementing CLSamplerImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLSAMPLERVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLSAMPLERVK_H_

#include "clspv/Sampler.h"
#include "libANGLE/renderer/CLSamplerImpl.h"
#include "libANGLE/renderer/vulkan/cl_types.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "vulkan/vulkan_core.h"

namespace rx
{

class CLSamplerVk : public CLSamplerImpl
{
  public:
    CLSamplerVk(const cl::Sampler &sampler);
    ~CLSamplerVk() override;

    vk::SamplerHelper &getSamplerHelper() { return mSamplerHelper; }
    vk::SamplerHelper &getSamplerHelperNormalized() { return mSamplerHelperNormalized; }
    angle::Result create();
    angle::Result createNormalized();

    VkSamplerAddressMode getVkAddressMode();
    VkFilter getVkFilter();
    VkSamplerMipmapMode getVkMipmapMode();
    uint32_t getSamplerMask();

  private:
    CLContextVk *mContext;
    vk::Renderer *mRenderer;

    vk::SamplerHelper mSamplerHelper;
    vk::SamplerHelper mSamplerHelperNormalized;
    VkSamplerCreateInfo mDefaultSamplerCreateInfo;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLSAMPLERVK_H_
