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

#ifndef SRC_DAWN_NATIVE_VULKAN_BINDGROUPVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BINDGROUPVK_H_

#include <vector>

#include "dawn/common/PlacementAllocated.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/BindGroup.h"
#include "dawn/native/vulkan/DescriptorSetAllocation.h"

namespace dawn::native::vulkan {

class Device;

// The sampled texture bindings in Vulkan need to be moved from whatever the binding was going to
// be, to instead use the same slot as the static sampler they will be co-written with in the
// VkDescriptorSet.
using TextureToStaticSamplerMap = absl::flat_hash_map<BindingIndex, BindingIndex>;

class BindGroup final : public BindGroupBase, public PlacementAllocated {
  public:
    static ResultOrError<Ref<BindGroup>> Create(Device* device,
                                                const UnpackedPtr<BindGroupDescriptor>& descriptor);

    BindGroup(Device* device,
              const UnpackedPtr<BindGroupDescriptor>& descriptor,
              DescriptorSetAllocation descriptorSetAllocation);

    // Write the descriptors to the provided VkDescriptorSet, this is also used to write to new
    // allocation from specific vulkan::BindGroupLayout specializations.
    void WriteDescriptorSet(VkDescriptorSet dsSet,
                            const TextureToStaticSamplerMap& textureToStaticSampler) const;

    VkDescriptorSet GetHandle() const;

  private:
    ~BindGroup() override;

    MaybeError InitializeImpl() override;
    void DestroyImpl(DestroyReason reason) override;
    void DeleteThis() override;

    // Dawn API
    void SetLabelImpl() override;

    // The descriptor set in this allocation outlives the BindGroup because it is owned by
    // the BindGroupLayout which is referenced by the BindGroup.
    DescriptorSetAllocation mDescriptorSetAllocation;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BINDGROUPVK_H_
