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

VkShaderStageFlags VulkanShaderStageFlags(wgpu::ShaderStage stages) {
    VkShaderStageFlags flags = 0;

    if (stages & wgpu::ShaderStage::Vertex) {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (stages & wgpu::ShaderStage::Fragment) {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    if (stages & wgpu::ShaderStage::Compute) {
        flags |= VK_SHADER_STAGE_COMPUTE_BIT;
    }

    return flags;
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
            return (layout.isUsedForSingleTextureBinding)
                       ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
                       : VK_DESCRIPTOR_TYPE_SAMPLER;
        },
        [](const TextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; },
        [](const StorageTextureBindingInfo&) { return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; },
        [](const InputAttachmentBindingInfo&) { return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT; });
}

// static
ResultOrError<Ref<BindGroupLayout>> BindGroupLayout::Create(
    Device* device,
    const BindGroupLayoutDescriptor* descriptor) {
    Ref<BindGroupLayout> bgl = AcquireRef(new BindGroupLayout(device, descriptor));
    DAWN_TRY(bgl->Initialize());
    return bgl;
}

MaybeError BindGroupLayout::Initialize() {
    // Compute the bindings that will be chained in the DescriptorSetLayout create info. We add
    // one entry per binding set. This might be optimized by computing continuous ranges of
    // bindings of the same type.
    ityp::vector<BindingIndex, VkDescriptorSetLayoutBinding> bindings;
    bindings.reserve(GetBindingCount());

    // Build the mapping from the indices of textures that will be paired with
    // static samplers at the Vk level in combined image sampler entries to
    // their respective sampler indices.
    for (BindingIndex bindingIndex : Range(GetBindingCount())) {
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        if (!std::holds_alternative<StaticSamplerBindingInfo>(bindingInfo.bindingLayout)) {
            continue;
        }

        auto samplerLayout = std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
        if (!samplerLayout.isUsedForSingleTextureBinding) {
            // The client did not specify that this sampler should be paired
            // with a single texture binding.
            continue;
        }

        mTextureToStaticSamplerIndices[GetBindingIndex(samplerLayout.sampledTextureBinding)] =
            bindingIndex;
    }

    for (const auto& [_, bindingIndex] : GetBindingMap()) {
        // This texture will be bound into the VkDescriptorSet at the index for the sampler itself.
        if (mTextureToStaticSamplerIndices.contains(bindingIndex)) {
            continue;
        }

        // Vulkan descriptor set layouts have one entry for binding_array. Only handle their first
        // element as subsequent one will be part of the already added VkDescriptorSetLayoutBinding.
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        if (bindingInfo.indexInArray != BindingIndex(0)) {
            continue;
        }

        VkDescriptorSetLayoutBinding vkBinding;
        vkBinding.binding = uint32_t(bindingIndex);
        vkBinding.descriptorType = VulkanDescriptorType(bindingInfo);
        vkBinding.descriptorCount = uint32_t(bindingInfo.arraySize);
        vkBinding.stageFlags = VulkanShaderStageFlags(bindingInfo.visibility);

        if (std::holds_alternative<StaticSamplerBindingInfo>(bindingInfo.bindingLayout)) {
            auto samplerLayout = std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
            auto sampler = ToBackend(samplerLayout.sampler);
            vkBinding.pImmutableSamplers = &sampler->GetHandle().GetHandle();
        } else {
            vkBinding.pImmutableSamplers = nullptr;
        }

        bindings.emplace_back(vkBinding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    // Record cache key information now since the createInfo is not stored.
    StreamIn(&mCacheKey, createInfo);

    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*mHandle),
                            "CreateDescriptorSetLayout"));

    // Compute the size of descriptor pools used for this layout.
    absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType;

    for (BindingIndex bindingIndex{0}; bindingIndex < GetBindingCount(); ++bindingIndex) {
        if (mTextureToStaticSamplerIndices.contains(bindingIndex)) {
            // This texture will be bound into the VkDescriptorSet at the index
            // for the sampler itself.
            continue;
        }

        // Vulkan descriptor set layouts have one entry for binding_array. Only handle their first
        // element as subsequent one will be part of the already counted descriptors.
        const BindingInfo& bindingInfo = GetBindingInfo(bindingIndex);
        if (bindingInfo.indexInArray != BindingIndex(0)) {
            continue;
        }

        VkDescriptorType vulkanType = VulkanDescriptorType(bindingInfo);

        size_t numVkDescriptors = uint32_t(bindingInfo.arraySize);
        if (vulkanType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
            auto samplerLayout = std::get<StaticSamplerBindingInfo>(bindingInfo.bindingLayout);
            auto sampler = ToBackend(samplerLayout.sampler);
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
                numVkDescriptors = 3;
            }
        }

        // absl:flat_hash_map::operator[] will return 0 if the key doesn't exist.
        descriptorCountPerType[vulkanType] += numVkDescriptors;
    }

    // TODO(enga): Consider deduping allocators for layouts with the same descriptor type
    // counts.
    mDescriptorSetAllocator =
        DescriptorSetAllocator::Create(device, std::move(descriptorCountPerType));

    SetLabelImpl();

    return {};
}

BindGroupLayout::BindGroupLayout(DeviceBase* device, const BindGroupLayoutDescriptor* descriptor)
    : BindGroupLayoutInternalBase(device, descriptor),
      mBindGroupAllocator(MakeFrontendBindGroupAllocator<BindGroup>(4096)) {}

BindGroupLayout::~BindGroupLayout() = default;

void BindGroupLayout::DestroyImpl() {
    BindGroupLayoutInternalBase::DestroyImpl();

    Device* device = ToBackend(GetDevice());

    // DescriptorSetLayout aren't used by execution on the GPU and can be deleted at any time,
    // so we can destroy mHandle immediately instead of using the FencedDeleter.
    // (Swiftshader implements this wrong b/154522740).
    // In practice, the GPU is done with all descriptor sets because bind group deallocation
    // refs the bind group layout so that once the bind group is finished being used, we can
    // recycle its descriptor set.
    if (mHandle != VK_NULL_HANDLE) {
        device->fn.DestroyDescriptorSetLayout(device->GetVkDevice(), mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
    mDescriptorSetAllocator = nullptr;
}

VkDescriptorSetLayout BindGroupLayout::GetHandle() const {
    return mHandle;
}

ResultOrError<Ref<BindGroup>> BindGroupLayout::AllocateBindGroup(
    Device* device,
    const BindGroupDescriptor* descriptor) {
    DescriptorSetAllocation descriptorSetAllocation;
    DAWN_TRY_ASSIGN(descriptorSetAllocation, mDescriptorSetAllocator->Allocate(this));

    return AcquireRef(mBindGroupAllocator->Allocate(device, descriptor, descriptorSetAllocation));
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

std::optional<BindingIndex> BindGroupLayout::GetStaticSamplerIndexForTexture(
    BindingIndex textureBinding) const {
    if (mTextureToStaticSamplerIndices.contains(textureBinding)) {
        return mTextureToStaticSamplerIndices.at(textureBinding);
    }
    return {};
}

void BindGroupLayout::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mHandle, "Dawn_BindGroupLayout", GetLabel());
}

}  // namespace dawn::native::vulkan
