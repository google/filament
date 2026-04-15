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

#include "dawn/native/vulkan/BindGroupLayoutVk.h"

#include <utility>

#include "absl/container/flat_hash_map.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_vector.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/vulkan/DescriptorSetAllocator.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/PhysicalDeviceVk.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

namespace {

// Helper function (and result structure) that precomputes all the information related to static
// bindings that might be for Vulkan BindGroupLayout.
struct VulkanStaticBindings {
    ityp::vector<BindingIndex, VkDescriptorSetLayoutBinding> bindings;
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType;
    TextureToStaticSamplerMap textureToStaticSampler;
};
ResultOrError<VulkanStaticBindings> ComputeVulkanStaticBindings(
    Device* device,
    const BindGroupLayoutInternalBase* layout,
    const BindGroupLayout::StaticSamplerSpecializationMap& staticSamplerSpecializations = {}) {
    VulkanStaticBindings res;

    // Build a map of texture indices to sampler indices. This maps the texture to
    // the sampler which will be used in VK in the combined image sampler entry.
    for (BindingIndex bindingIndex : layout->GetStaticSamplerIndices()) {
        auto samplerBindingInfo =
            std::get<StaticSamplerBindingInfo>(layout->GetBindingInfo(bindingIndex).bindingLayout);
        // This is a static sampler combined with textures dynamically in the shader.
        if (samplerBindingInfo.use == StaticSamplerUse::Freestanding) {
            continue;
        }

        res.textureToStaticSampler[samplerBindingInfo.sampledTextureIndex] = bindingIndex;
    }

    // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
    // one entry per binding set. This could be optimized by computing continuous ranges of
    // bindings of the same type.
    res.bindings.reserve(layout->GetBindingCount());

    for (BindingIndex bindingIndex : Range(layout->GetBindingCount())) {
        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);

        // Skip over bindings that cannot be seen by any shaders as they could cause us to create
        // bindgroups with more bindings than the VkDevice's limits. However keep dynamic buffers
        // as the amount of dynamic offsets need to stay the same as WebGPU's so we can passthrough
        // the dynamic offsets.
        if (bindingInfo.visibility == wgpu::ShaderStage::None &&
            bindingIndex >= layout->GetDynamicBufferCount()) {
            continue;
        }

        // This texture will be bound into the VkDescriptorSet at the index for the sampler itself.
        if (res.textureToStaticSampler.contains(bindingIndex)) {
            continue;
        }

        // Vulkan descriptor set layouts have one entry for binding_array. Only handle their first
        // element as subsequent ones will be part of the already added
        // VkDescriptorSetLayoutBinding.
        if (bindingInfo.indexInArray != BindingIndex(0)) {
            continue;
        }

        VkDescriptorSetLayoutBinding vkBinding{
            .binding = uint32_t(bindingIndex),
            .descriptorType = VulkanDescriptorType(bindingInfo),
            .descriptorCount = uint32_t(bindingInfo.arraySize),
            .stageFlags = VulkanShaderStages(bindingInfo.visibility),
            .pImmutableSamplers = nullptr,
        };
        size_t descriptorCount = vkBinding.descriptorCount;

        // Static samplers are set at VkDescriptorSetLayout creation time.
        if (std::holds_alternative<StaticSamplerBindingInfo>(bindingInfo.bindingLayout)) {
            auto samplerLayout = std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
            auto sampler = ToBackend(samplerLayout.sampler);

            // Override with the specialization's sampler if there's one. This is used to replace
            // samplers with the correct YCbCr sampler when JITing pipelines.
            if (auto it = staticSamplerSpecializations.find(bindingIndex);
                it != staticSamplerSpecializations.end()) {
                DAWN_ASSERT(samplerLayout.use == StaticSamplerUse::InternalForExternalTexture);
                DAWN_TRY_ASSIGN(sampler, Sampler::Create(device, it->second));
            }

            vkBinding.pImmutableSamplers = &sampler->GetHandle().GetHandle();

            if (sampler->IsYCbCr()) {
                // A YCbCr sampler can take up multiple Vk descriptor slots.  There is a
                // recommended Vulkan API to query how many slots a YCbCr sampler should take, but
                // it is not clear how to actually pass the Android external format to that API.
                // However, the spec for that API says the following:
                // "combinedImageSamplerDescriptorCount is a number between 1 and the number of
                // planes in the format. A descriptor set layout binding with immutable Y′CBCR
                // conversion samplers will have a maximum combinedImageSamplerDescriptorCount
                // which is the maximum across all formats supported by its samplers of the
                // combinedImageSamplerDescriptorCount for each format." Hence, we simply hardcode
                // the maximum number of planes that an external format can have here. The number
                // of overall YCbCr descriptors will be relatively small and these pools are not an
                // overall bottleneck on memory usage.
                DAWN_ASSERT(bindingInfo.arraySize == BindingIndex(1));
                descriptorCount = 3;
            }
        }

        res.bindings.emplace_back(vkBinding);

        // absl:flat_hash_map::operator[] will return 0 if the key doesn't exist.
        res.descriptorCountPerType[vkBinding.descriptorType] += descriptorCount;
    }

    return std::move(res);
}

}  // anonymous namespace

VkDescriptorType VulkanDescriptorType(const BindingInfo& bindingInfo) {
    return MatchVariant(
        bindingInfo.bindingLayout,
        [](const BufferBindingInfo& layout) -> VkDescriptorType {
            switch (layout.type) {
                case wgpu::BufferBindingType::Uniform:
                    if (layout.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                case wgpu::BufferBindingType::Storage:
                case kInternalStorageBufferBinding:
                case wgpu::BufferBindingType::ReadOnlyStorage:
                case kInternalReadOnlyStorageBufferBinding:
                    if (layout.hasDynamicOffset) {
                        return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    }
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                case wgpu::BufferBindingType::BindingNotUsed:
                case wgpu::BufferBindingType::Undefined:
                    DAWN_UNREACHABLE();
                    return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            }
            DAWN_UNREACHABLE();
        },
        [](const SamplerBindingInfo&) { return VK_DESCRIPTOR_TYPE_SAMPLER; },
        [](const StaticSamplerBindingInfo& layout) {
            // Make this entry into a combined image sampler iff the client
            // specified a single texture binding to be paired with it.
            return (layout.use == StaticSamplerUse::Freestanding)
                       ? VK_DESCRIPTOR_TYPE_SAMPLER
                       : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        },
        [](const TextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; },
        [](const StorageTextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; },
        [](const TexelBufferBindingInfo& layout) -> VkDescriptorType {
            switch (layout.access) {
                case wgpu::TexelBufferAccess::ReadOnly:
                    // TODO(crbug.com/382544164): Investigate whether read-only texel buffers
                    // should use VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER for broader format
                    // support, or stay on VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER for bindless
                    // compatibility (uniform texel buffers have limited bindless support on
                    // Vulkan and would require a separate descriptor array in the resource
                    // table).
                    [[fallthrough]];
                case wgpu::TexelBufferAccess::ReadWrite:
                    return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                case wgpu::TexelBufferAccess::Undefined:
                    DAWN_UNREACHABLE();
            }
            DAWN_UNREACHABLE();
        },

        [](const InputAttachmentBindingInfo&) { return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; },
        [](const ExternalTextureBindingInfo&) -> VkDescriptorType { DAWN_UNREACHABLE(); });
}

// static
ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor) {
    Ref<BindGroupLayout> bgl = AcquireRef(new BindGroupLayout(device, descriptor));
    DAWN_TRY(bgl->Initialize());
    return bgl;
}

BindGroupLayout::BindGroupLayout(DeviceBase* device,
                                 const UnpackedPtr<BindGroupLayoutDescriptor>& descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {}

BindGroupLayout::~BindGroupLayout() = default;

MaybeError BindGroupLayout::Initialize() {
    Device* device = ToBackend(GetDevice());

    VulkanStaticBindings bindings;
    DAWN_TRY_ASSIGN(bindings, ComputeVulkanStaticBindings(device, this));
    mTextureToStaticSampler = std::move(bindings.textureToStaticSampler);

    mDescriptorSetAllocator =
        DescriptorSetAllocator::Create(device, std::move(bindings.descriptorCountPerType));

    VkDescriptorSetLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = uint32_t{bindings.bindings.size()},
        .pBindings = bindings.bindings.data(),
    };

    // Record cache key information now since the createInfo is not stored.
    StreamIn(&mCacheKey, createInfo);

    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*mHandle),
                            "CreateDescriptorSetLayout"));
    mSpecializations->insert({{}, mHandle});

    SetLabelImpl();

    return {};
}

ResultOrError<VkDescriptorSetLayout> BindGroupLayout::GetOrCreateSpecializedHandle(
    const Specialization& specialization) {
    if (auto specialized = mSpecializations.ConstUse(
            [&](auto specializations) -> std::optional<VkDescriptorSetLayout> {
                if (auto it = specializations->find(specialization); it != specializations->end()) {
                    return it->second;
                }
                return std::nullopt;
            });
        specialized) {
        return *specialized;
    }

    Device* device = ToBackend(GetDevice());
    VulkanStaticBindings bindings;
    DAWN_TRY_ASSIGN(bindings,
                    ComputeVulkanStaticBindings(device, this, specialization.staticSamplers));

    VkDescriptorSetLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = uint32_t{bindings.bindings.size()},
        .pBindings = bindings.bindings.data(),
    };

    VkDescriptorSetLayout specialized;
    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*specialized),
                            "CreateDescriptorSetLayout"));

    return mSpecializations.Use([&](auto specializations) -> ResultOrError<VkDescriptorSetLayout> {
        auto [it, inserted] = specializations->insert({specialization, specialized});
        if (!inserted) {
            device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), specialized, nullptr);
            return it->second;
        }
        return specialized;
    });
}

void BindGroupLayout::DestroyImpl(DestroyReason reason) {
    BindGroupLayoutInternalBase::DestroyImpl(reason);

    Device* device = ToBackend(GetDevice());

    // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
    // so we can destroy mHandle immediately instead of using the FencedDeleter.
    mSpecializations.Use([&](auto specializations) {
        for (auto& [_, handle] : *specializations) {
            device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), handle, nullptr);
        }
        specializations->clear();
    });

    // Handled in the loop above already.
    mHandle = VK_NULL_HANDLE;

    mDescriptorSetAllocator = nullptr;
}

VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
    return mHandle;
}

ResultOrError<Ref<BindGroup>> BindGroupLayout::AllocateBindGroup(
    const UnpackedPtr<BindGroupDescriptor>& descriptor) {
    DescriptorSetAllocation descriptorSetAllocation;
    DAWN_TRY_ASSIGN(descriptorSetAllocation, mDescriptorSetAllocator->Allocate(GetHandle()));

    return AcquireRef(
        mBindGroupAllocator->Allocate(ToBackend(GetDevice()), descriptor, descriptorSetAllocation));
}

void BindGroupLayout::DeallocateBindGroup(BindGroup* bindGroup) {
    mBindGroupAllocator->Deallocate(bindGroup);
}

void BindGroupLayout::DeallocateDescriptorSet(DescriptorSetAllocation* descriptorSetAllocation) {
    mDescriptorSetAllocator->Deallocate(descriptorSetAllocation);
}

void BindGroupLayout::ReduceMemoryUsage() {
    mBindGroupAllocator->DeleteEmptySlabs();
}

ResultOrError<std::unique_ptr<OwnedDescriptorSet>> BindGroupLayout::GetSpecializedSetFor(
    const BindGroup* bg,
    const Specialization& specialization) {
    DAWN_ASSERT(bg->GetLayout() == this);

    VkDescriptorSetLayout dsLayout;
    DAWN_TRY_ASSIGN(dsLayout, GetOrCreateSpecializedHandle(specialization));

    DescriptorSetAllocation dsAllocation;
    DAWN_TRY_ASSIGN(dsAllocation, mDescriptorSetAllocator->Allocate(dsLayout));

    bg->WriteDescriptorSet(dsAllocation.set, mTextureToStaticSampler);
    return std::make_unique<OwnedDescriptorSet>(this, dsAllocation);
}

const TextureToStaticSamplerMap& BindGroupLayout::GetTextureToStaticSamplerMap() const {
    return mTextureToStaticSampler;
}

void BindGroupLayout::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_BindGroupLayout", GetLabel());
}

// OwnedDescriptorSet

OwnedDescriptorSet::OwnedDescriptorSet(BindGroupLayout* bgl, DescriptorSetAllocation allocation)
    : mAllocation(allocation), mBindGroupLayout(bgl) {}

OwnedDescriptorSet::~OwnedDescriptorSet() {
    mBindGroupLayout->DeallocateDescriptorSet(&mAllocation);
    mBindGroupLayout = nullptr;
}

VkDescriptorSet OwnedDescriptorSet::GetHandle() const {
    return mAllocation.set;
}

}  // namespace dawn::native::vulkan
