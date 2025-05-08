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

#include "dawn/native/vulkan/BindGroupVk.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/common/Range.h"
#include "dawn/common/ityp_stack_vec.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/vulkan/BindGroupLayoutVk.h"
#include "dawn/native/vulkan/BufferVk.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
ResultOrError<Ref<BindGroup>> BindGroup::Create(Device* device,
                                                const BindGroupDescriptor* descriptor) {
    Ref<BindGroup> bindGroup;
    DAWN_TRY_ASSIGN(bindGroup, ToBackend(descriptor->layout->GetInternalBindGroupLayout())
                                   ->AllocateBindGroup(device, descriptor));
    DAWN_TRY(bindGroup->Initialize(descriptor));
    return bindGroup;
}

BindGroup::BindGroup(Device* device,
                     const BindGroupDescriptor* descriptor,
                     DescriptorSetAllocation descriptorSetAllocation)
    : BindGroupBase(this, device, descriptor), mDescriptorSetAllocation(descriptorSetAllocation) {}

BindGroup::~BindGroup() = default;

MaybeError BindGroup::InitializeImpl() {
    // Now do a write of a single descriptor set with all possible chained data allocated on the
    // stack.
    const uint32_t bindingCount = static_cast<uint32_t>((GetLayout()->GetBindingCount()));
    ityp::stack_vec<uint32_t, VkWriteDescriptorSet, kMaxOptimalBindingsPerGroup> writes(
        bindingCount);
    ityp::stack_vec<uint32_t, VkDescriptorBufferInfo, kMaxOptimalBindingsPerGroup> writeBufferInfo(
        bindingCount);
    ityp::stack_vec<uint32_t, VkDescriptorImageInfo, kMaxOptimalBindingsPerGroup> writeImageInfo(
        bindingCount);

    uint32_t numWrites = 0;
    for (BindingIndex bindingIndex : Range(GetLayout()->GetBindingCount())) {
        const BindingInfo& bindingInfo = GetLayout()->GetBindingInfo(bindingIndex);

        auto& write = writes[numWrites];
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = GetHandle();
        write.dstBinding = static_cast<uint32_t>(bindingIndex);
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VulkanDescriptorType(bindingInfo);

        bool shouldWriteDescriptor = MatchVariant(
            bindingInfo.bindingLayout,
            [&](const BufferBindingInfo&) -> bool {
                BufferBinding binding = GetBindingAsBufferBinding(bindingIndex);

                VkBuffer handle = ToBackend(binding.buffer)->GetHandle();
                if (handle == VK_NULL_HANDLE) {
                    // The Buffer was destroyed. Skip this descriptor write since it would be
                    // a Vulkan Validation Layers error. This bind group won't be used as it
                    // is an error to submit a command buffer that references destroyed
                    // resources.
                    return false;
                }
                writeBufferInfo[numWrites].buffer = handle;
                writeBufferInfo[numWrites].offset = binding.offset;
                writeBufferInfo[numWrites].range = binding.size;
                write.pBufferInfo = &writeBufferInfo[numWrites];
                return true;
            },
            [&](const SamplerBindingInfo&) -> bool {
                Sampler* sampler = ToBackend(GetBindingAsSampler(bindingIndex));
                writeImageInfo[numWrites].sampler = sampler->GetHandle();
                write.pImageInfo = &writeImageInfo[numWrites];
                return true;
            },
            [&](const StaticSamplerBindingInfo& layout) -> bool {
                // Static samplers are bound into the Vulkan layout as immutable
                // samplers at BindGroupLayout creation time. There is no work
                // to be done at BindGroup creation time.
                return false;
            },
            [&](const TextureBindingInfo&) -> bool {
                TextureView* view = ToBackend(GetBindingAsTextureView(bindingIndex));

                VkImageView handle = view->GetHandle();
                if (handle == VK_NULL_HANDLE) {
                    // The Texture was destroyed before the TextureView was created.
                    // Skip this descriptor write since it would be
                    // a Vulkan Validation Layers error. This bind group won't be used as it
                    // is an error to submit a command buffer that references destroyed
                    // resources.
                    return false;
                }

                // TODO(crbug.com/41488897: Add GetVkDescriptorSet{Index,
                // Type}(BindingIndex) functions to BindGroupLayoutVk that
                // access vectors holding entries for all BGL entries and
                // eliminate this special-case code in favor of calling those
                // functions to assign `dstBinding` and `descriptorType` above.
                if (auto samplerIndex =
                        ToBackend(GetLayout())->GetStaticSamplerIndexForTexture(bindingIndex)) {
                    // Write the info of the texture at the binding index for the
                    // sampler.
                    write.dstBinding = static_cast<uint32_t>(samplerIndex.value());
                    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                }

                writeImageInfo[numWrites].imageView = handle;
                writeImageInfo[numWrites].imageLayout = VulkanImageLayout(
                    view->GetTexture()->GetFormat(), wgpu::TextureUsage::TextureBinding);

                write.pImageInfo = &writeImageInfo[numWrites];
                return true;
            },
            [&](const StorageTextureBindingInfo&) -> bool {
                TextureView* view = ToBackend(GetBindingAsTextureView(bindingIndex));

                VkImageView handle = VK_NULL_HANDLE;
                if (view->GetTexture()->GetFormat().format == wgpu::TextureFormat::BGRA8Unorm) {
                    handle = view->GetHandleForBGRA8UnormStorage();
                } else {
                    handle = view->GetHandle();
                }
                if (handle == VK_NULL_HANDLE) {
                    // The Texture was destroyed before the TextureView was created.
                    // Skip this descriptor write since it would be
                    // a Vulkan Validation Layers error. This bind group won't be used as it
                    // is an error to submit a command buffer that references destroyed
                    // resources.
                    return false;
                }
                writeImageInfo[numWrites].imageView = handle;
                writeImageInfo[numWrites].imageLayout = VK_IMAGE_LAYOUT_GENERAL;

                write.pImageInfo = &writeImageInfo[numWrites];
                return true;
            },
            [&](const InputAttachmentBindingInfo&) -> bool {
                TextureView* view = ToBackend(GetBindingAsTextureView(bindingIndex));

                VkImageView handle = view->GetHandle();
                if (handle == VK_NULL_HANDLE) {
                    // The Texture was destroyed before the TextureView was created.
                    // Skip this descriptor write since it would be
                    // a Vulkan Validation Layers error. This bind group won't be used as it
                    // is an error to submit a command buffer that references destroyed
                    // resources.
                    return false;
                }
                writeImageInfo[numWrites].imageView = handle;
                writeImageInfo[numWrites].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                write.pImageInfo = &writeImageInfo[numWrites];
                return true;
            });

        if (shouldWriteDescriptor) {
            numWrites++;
        }
    }

    Device* device = ToBackend(GetDevice());
    // TODO(crbug.com/dawn/855): Batch these updates
    device->fn.UpdateDescriptorSets(device->GetVkDevice(), numWrites, writes.data(), 0, nullptr);

    SetLabelImpl();

    return {};
}

void BindGroup::DestroyImpl() {
    BindGroupBase::DestroyImpl();
    ToBackend(GetLayout())->DeallocateDescriptorSet(&mDescriptorSetAllocation);
}

void BindGroup::DeleteThis() {
    // This function must first run the destructor and then deallocate memory. Take a reference to
    // the BindGroupLayout+SlabAllocator before running the destructor so this function can access
    // it afterwards and it's not destroyed prematurely.
    Ref<BindGroupLayout> layout = ToBackend(GetLayout());
    BindGroupBase::DeleteThis();
    layout->DeallocateBindGroup(this);
}

VkDescriptorSet BindGroup::GetHandle() const {
    return mDescriptorSetAllocation.set;
}

void BindGroup::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mDescriptorSetAllocation.set, "Dawn_BindGroup",
                 GetLabel());
}

}  // namespace dawn::native::vulkan
