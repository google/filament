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

#ifndef SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
#define SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_

#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/MutexProtected.h"
#include "dawn/common/NonCopyable.h"
#include "dawn/common/SlabAllocator.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/BindGroupLayoutInternal.h"
#include "dawn/native/vulkan/BindGroupVk.h"
#include "dawn/native/vulkan/SamplerVk.h"

namespace dawn::native {
class CacheKey;
}  // namespace dawn::native

namespace dawn::native::vulkan {

struct DescriptorSetAllocation;
class DescriptorSetAllocator;
class Device;
class OwnedDescriptorSet;

VkDescriptorType VulkanDescriptorType(const BindingInfo& bindingInfo);

// Backend BindGroupLayout implementation for Vulkan. In addition to containing a BindGroupAllocator
// for the CPU-side tracking data, it has a DescriptorSetAllocator that handles efficient allocation
// of the corresponding VkDescriptorSets.
class BindGroupLayout : public BindGroupLayoutInternalBase {
  public:
    static ResultOrError<Ref<BindGroupLayout>> Create(
        Device* device,
        const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);

    VkDescriptorSetLayout GetHandle() const;

    // BindGroupLayouts can be specialized when doing pipeline JITing to return
    // VkDescriptorSetLayouts that have some parts changed.
    using StaticSamplerSpecializationMap =
        absl::flat_hash_map<BindingIndex, StaticSamplerSpecialization>;
    struct Specialization {
        // Replaces the static sampler at BindingIndex with a different sampler created from the
        // StaticSamplerSpecialization.
        StaticSamplerSpecializationMap staticSamplers;

        template <typename H>
        friend H AbslHashValue(H h, const Specialization& s) {
            return H::combine(std::move(h), s.staticSamplers);
        }
        bool operator==(const Specialization& other) const = default;
    };
    ResultOrError<VkDescriptorSetLayout> GetOrCreateSpecializedHandle(
        const Specialization& specialization);

    ResultOrError<Ref<BindGroup>> AllocateBindGroup(
        const UnpackedPtr<BindGroupDescriptor>& descriptor);
    void DeallocateBindGroup(BindGroup* bindGroup);
    void DeallocateDescriptorSet(DescriptorSetAllocation* descriptorSetAllocation);
    void ReduceMemoryUsage() override;

    // Returns a VkDescriptorSet corresponding to a BindGroup but with the necessary modifications
    // for the specialization.
    // TODO(https://crbug.com/496616832): This returns a unique_ptr because
    // ResultOrError<SomethingNonCopyable> doesn't compile at the moment, but unique_ptr has a
    // specialization that works.
    ResultOrError<std::unique_ptr<OwnedDescriptorSet>> GetSpecializedSetFor(
        const BindGroup* bg,
        const Specialization& specialization);

    const TextureToStaticSamplerMap& GetTextureToStaticSamplerMap() const;

  protected:
    BindGroupLayout(DeviceBase* device, const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor);
    ~BindGroupLayout() override;

    MaybeError Initialize();
    void DestroyImpl(DestroyReason reason) override;

    MutexProtected<SlabAllocator<BindGroup>> mBindGroupAllocator;

  private:
    // Dawn API
    void SetLabelImpl() override;

    VkDescriptorSetLayout mHandle = VK_NULL_HANDLE;

    // Caches VkDescriptorSetLayouts for specializations so that the lifetime guarantees are the
    // same as for mHandle. Note that the noop specialization has mHandle cached directly, but
    // mHandle is also kept separate for efficiency when creating BindGroups.
    MutexProtected<absl::flat_hash_map<Specialization, VkDescriptorSetLayout>> mSpecializations;

    // Maps from indices of texture entries that are paired with static samplers
    // to indices of the entries of their respective samplers.
    TextureToStaticSamplerMap mTextureToStaticSampler;

    Ref<DescriptorSetAllocator> mDescriptorSetAllocator;
};

// RAII wrapper around a VkDescriptorSet for use when the VkDescriptorSet is not part of a BindGroup
// object.
class OwnedDescriptorSet : public NonCopyable {
  public:
    OwnedDescriptorSet() = default;
    OwnedDescriptorSet(BindGroupLayout* bgl, DescriptorSetAllocation allocation);
    ~OwnedDescriptorSet();

    VkDescriptorSet GetHandle() const;

  private:
    DescriptorSetAllocation mAllocation;
    Ref<BindGroupLayout> mBindGroupLayout = nullptr;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_BINDGROUPLAYOUTVK_H_
