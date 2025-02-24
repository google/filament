//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerVk.h:
//    Defines the class interface for SamplerVk, implementing SamplerImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SAMPLERVK_H_
#define LIBANGLE_RENDERER_VULKAN_SAMPLERVK_H_

#include "libANGLE/renderer/SamplerImpl.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class SamplerVk : public SamplerImpl
{
  public:
    SamplerVk(const gl::SamplerState &state);
    ~SamplerVk() override;

    void onDestroy(const gl::Context *context) override;
    angle::Result syncState(const gl::Context *context, const bool dirty) override;

    const vk::SamplerHelper &getSampler() const
    {
        ASSERT(mSampler);
        ASSERT(mSampler->valid());
        return *mSampler.get();
    }

  private:
    vk::SharedSamplerPtr mSampler;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SAMPLERVK_H_
