// Copyright 2018 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_DAWN_NATIVE_VULKAN_SAMPLERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_SAMPLERVK_H_

#include <utility>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/Sampler.h"

namespace dawn::native::vulkan {

class Device;
struct StaticSamplerSpecialization;
class TextureView;

class Sampler final : public SamplerBase {
  public:
    static ResultOrError<Ref<Sampler>> Create(Device* device, const SamplerDescriptor* descriptor);

    // Creates the sampler for that specialization and keeps it cached on the device. This avoids
    // having to keep the samplers alive between frames for specializations (and during processing
    // of the commands that need a specialization). Note that we expect only a small number of
    // different YCbCr samplers. Note also that we cannot rely on the "frontend cache" because it
    // only deduplicates objects but doesn't keep them alive on its own.
    static ResultOrError<Ref<Sampler>> Create(Device* device,
                                              const StaticSamplerSpecialization& specialization);

    const VkSampler& GetHandle() const;

  private:
    ~Sampler() override;
    void DestroyImpl(DestroyReason reason) override;
    using SamplerBase::SamplerBase;
    MaybeError Initialize(const SamplerDescriptor* descriptor);

    // Dawn API
    void SetLabelImpl() override;

    VkSampler mHandle = VK_NULL_HANDLE;
    VkSamplerYcbcrConversion mSamplerYCbCrConversion = VK_NULL_HANDLE;
};

// All the information needed to create a StaticSampler for YCbCr sampling.
struct StaticSamplerSpecialization {
    static StaticSamplerSpecialization From(const TextureView* view, const Sampler* sampler);

    // TODO(https://crbug.com/497675620): All the views use the same RGB identity, range full YCbCr
    // conversion information at the moment. However to take advantage of hardware support we'll
    // need to use different model/range values, at which point the views will need to be
    // specialized either at ExternalTexture creation, or when the rest of the state is specialized.
    static YCbCrVkDescriptor GetYCbCrForTextureView(VkFormat vkFormat,
                                                    uint32_t androidExternalFormat);

    wgpu::FilterMode minFilter;
    wgpu::FilterMode magFilter;
    bool isYCbCr;

    // Members that are only used when isYcbCr
    VkFormat vkFormat;
    uint32_t androidExternalFormat;

    // Assumes that:
    //  - Model is the RGB identity
    //  - Range is full.
    //  - component mapping is the identity.
    //  - chroma offsets are mid-point.
    //  - chroma filter is nearest.

    template <typename H>
    friend H AbslHashValue(H h, const StaticSamplerSpecialization& s) {
        return H::combine(std::move(h), s.minFilter, s.magFilter, s.isYCbCr, s.vkFormat,
                          s.androidExternalFormat);
    }

    bool operator==(const StaticSamplerSpecialization& other) const = default;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_SAMPLERVK_H_
