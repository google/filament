// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/vulkan/ResourceTableVk.h"

#include <vector>

#include "dawn/common/Enumerator.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/ResourceTableDefaultResources.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/SamplerVk.h"
#include "dawn/native/vulkan/TextureVk.h"
#include "dawn/native/vulkan/UtilsVulkan.h"
#include "dawn/native/vulkan/VulkanError.h"
namespace dawn::native::vulkan {

// static
ResultOrError<Ref<ResourceTable>> ResourceTable::Create(Device* device,
                                                        const ResourceTableDescriptor* descriptor) {
    Ref<ResourceTable> table = AcquireRef(new ResourceTable(device, descriptor));
    DAWN_TRY(table->Initialize());
    return table;
}

// static
ResultOrError<VkDescriptorSetLayout> ResourceTable::MakeDescriptorSetLayout(Device* device) {
    // A resource table is a bindgroup made of two entries:
    //
    //  - Binding 0: the metadata storage buffer.
    //  - Binding 1: the variable length, partially bound, update-after-bind array of sampled
    //  textures/samplers.
    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
        {{
             .binding = 0,
             .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
             .descriptorCount = 1,
             .stageFlags = VulkanShaderStages(kAllStages),
             .pImmutableSamplers = nullptr,
         },
         {
             // To store both samplers and images together in the same descriptor set array, we use
             // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, and when adding a texture view to the
             // table, we set an "unused sampler", and similarly when adding a sampler to the table,
             // we set an "unused image".
             //
             // Although this method wastes descriptor memory space, since each descriptor stores
             // both a sampler and an image, this keeps shader emission simpler (one set of arrays
             // per resource type at the same binding, with simple indexing), and allows us to store
             // all descriptors in one descriptor set with
             // VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT enabled.
             .binding = 1,
             .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
             .descriptorCount =
                 kMaxResourceTableSize + uint32_t{ResourceTableDefaultResources::GetCount()},
             .stageFlags = VulkanShaderStages(kAllStages),
             .pImmutableSamplers = nullptr,
         }}};
    std::array<VkDescriptorBindingFlags, 2> flags = {
        0,  //
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
            VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT};

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = flags.size(),
        .pBindingFlags = flags.data(),
    };
    VkDescriptorSetLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagCreateInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = bindings.size(),
        .pBindings = bindings.data(),
    };

    VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo,
                                                                 nullptr, &*dsLayout),
                            "CreateDescriptorSetLayout"));
    return dsLayout;
}

ResourceTable::~ResourceTable() = default;

MaybeError ResourceTable::Initialize() {
    DAWN_TRY(ResourceTableBase::InitializeBase());

    Device* device = ToBackend(GetDevice());

    uint32_t sampledImageCount = uint32_t(GetSizeWithDefaultResources());

    // Allocate mPool.
    {
        std::array<VkDescriptorPoolSize, 2> sizes = {
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1},  // The metadata buffer.
            VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, sampledImageCount},
        };

        VkDescriptorPoolCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 1,
            .poolSizeCount = uint32_t(sizes.size()),
            .pPoolSizes = sizes.data(),
        };

        DAWN_TRY(CheckVkOOMThenSuccess(
            device->fn.CreateDescriptorPool(device->GetVkDevice(), &createInfo, nullptr, &*mPool),
            "CreateDescriptorPool"));
    }

    // Allocate mSet from mPool.
    {
        VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorSetCount = 1,
            .pDescriptorCounts = &sampledImageCount,
        };
        VkDescriptorSetAllocateInfo allocateInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = &variableCountInfo,
            .descriptorPool = mPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &device->GetResourceTableLayout().GetHandle(),
        };

        DAWN_TRY(CheckVkOOMThenSuccess(
            device->fn.AllocateDescriptorSets(device->GetVkDevice(), &allocateInfo, &*mSet),
            "AllocateDescriptorSets"));
    }

    // Only write the metadata buffer in mSet initially, all the other bindings will be written as
    // needed when they are inserted in the ResourceTable.
    {
        Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = metadataBuffer->GetHandle(),
            .offset = 0,
            .range = metadataBuffer->GetSize(),
        };
        VkWriteDescriptorSet write = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = mSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pImageInfo = nullptr,
            .pBufferInfo = &bufferInfo,
            .pTexelBufferView = nullptr,
        };
        device->fn.UpdateDescriptorSets(device->GetVkDevice(), 1, &write, 0, nullptr);
    }

    return {};
}

// Apply updates to resources or to the metadata buffers that are pending.
MaybeError ResourceTable::ApplyPendingUpdates(CommandRecordingContext* recordingContext) {
    Updates updates = AcquireDirtySlotUpdates();

    if (!updates.metadataUpdates.empty()) {
        DAWN_TRY(UpdateMetadataBuffer(recordingContext, updates.metadataUpdates));
    }
    if (!updates.resourceUpdates.empty()) {
        DAWN_TRY(UpdateResourceBindings(updates.resourceUpdates));
    }

    return {};
}

MaybeError ResourceTable::UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                               const std::vector<MetadataUpdate>& updates) {
    // Updates the metadata buffer by scheduling a copy for each u32 that needs to be updated.
    // TODO(https://issues.chromium.org/473442435): If we had a way to use Dawn reentrantly now, we
    // could use a compute shader to dispatch the updates instead of individual copies for each
    // update, and move that logic in the frontend to share it between backends. (also a single
    // dispatch could update multiple metadata buffers potentially).
    Device* device = ToBackend(GetDevice());

    // Allocate enough space for all the data to modify and schedule the copies.
    return device->GetDynamicUploader()->WithUploadReservation(
        sizeof(uint32_t) * updates.size(), kCopyBufferToBufferOffsetAlignment,
        [&](UploadReservation reservation) -> MaybeError {
            uint32_t* stagedData = static_cast<uint32_t*>(reservation.mappedPointer);

            // The metadata buffer will be copied to.
            Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
            DAWN_ASSERT(metadataBuffer->IsInitialized());
            auto scopedUseMetadataBuffer = metadataBuffer->UseInternal();
            metadataBuffer->TransitionUsageNow(recordingContext, wgpu::BufferUsage::CopyDst);

            // Prepare the copies.
            std::vector<VkBufferCopy> copies(updates.size());
            for (auto [i, update] : Enumerate(updates)) {
                stagedData[i] = update.data;

                VkBufferCopy copy{
                    .srcOffset = reservation.offsetInBuffer + i * sizeof(uint32_t),
                    .dstOffset = update.offset,
                    .size = sizeof(uint32_t),
                };
                copies[i] = copy;
            }

            // Enqueue the copy commands all at once.
            device->fn.CmdCopyBuffer(recordingContext->commandBuffer,
                                     ToBackend(reservation.buffer)->GetHandle(),
                                     metadataBuffer->GetHandle(), copies.size(), copies.data());

            // Transition the buffer back to be used as storage as that's how it will be used for
            // shader-side validation.
            metadataBuffer->TransitionUsageNow(recordingContext, kReadOnlyStorageBuffer,
                                               kAllStages);

            return {};
        });
}

MaybeError ResourceTable::UpdateResourceBindings(const std::vector<ResourceUpdate>& updates) {
    Device* device = ToBackend(GetDevice());

    ityp::span<ResourceTableSlot, ResourceTableDefaultResources::Resource> defaultResources;
    DAWN_TRY_ASSIGN(defaultResources,
                    device->GetResourceTableDefaultResources()->GetOrCreate(device));

    // For combined texture samplers, the exact type of texture or sampler we set in the descriptor
    // doesn't matter as long as it's not used by the shader, so we grab any one of each.
    auto textureIndex =
        ResourceTableDefaultResources::IndexOf(tint::ResourceType::kTexture1d_f32_filterable);
    auto samplerIndex =
        ResourceTableDefaultResources::IndexOf(tint::ResourceType::kSampler_filtering);
    auto unusedTextureView = std::get<Ref<TextureViewBase>>(defaultResources[textureIndex]);
    auto unusedSampler = std::get<Ref<SamplerBase>>(defaultResources[samplerIndex]);

    std::vector<VkDescriptorImageInfo> imageWrites;
    std::vector<uint32_t> arrayElements;

    for (const ResourceUpdate& update : updates) {
        // TODO(https://issues.chromium.org/473444515): Support buffer, texel buffers and storage
        // textures.

        MatchVariant(
            update.resource,
            [&](TextureViewBase* textureView) {
                VkImageView handle = ToBackend(textureView)->GetHandle();
                if (handle == nullptr) {
                    return;
                }

                VkDescriptorImageInfo imageWrite = {
                    .sampler = ToBackend(unusedSampler)->GetHandle(),
                    .imageView = handle,
                    .imageLayout = VulkanImageLayout(textureView->GetFormat(),
                                                     wgpu::TextureUsage::TextureBinding),
                };
                imageWrites.push_back(imageWrite);
                arrayElements.push_back(uint32_t{update.slot});
            },
            [&](SamplerBase* sampler) {
                VkSampler handle = ToBackend(sampler)->GetHandle();
                if (handle == nullptr) {
                    return;
                }

                VkDescriptorImageInfo imageWrite = {
                    .sampler = handle,
                    .imageView = ToBackend(unusedTextureView)->GetHandle(),
                    .imageLayout = VK_IMAGE_LAYOUT_GENERAL,
                };
                imageWrites.push_back(imageWrite);
                arrayElements.push_back(uint32_t{update.slot});
            });
    }

    std::vector<VkWriteDescriptorSet> writes;
    for (size_t i = 0; i < imageWrites.size(); i++) {
        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = mSet,
            .dstBinding = 1,
            .dstArrayElement = arrayElements[i],
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &imageWrites[i],
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };
        writes.push_back(write);
    }

    device->fn.UpdateDescriptorSets(device->GetVkDevice(), writes.size(), writes.data(), 0,
                                    nullptr);
    return {};
}

VkDescriptorSet ResourceTable::GetHandle() const {
    return mSet;
}

void ResourceTable::DestroyImpl(DestroyReason reason) {
    if (mPool != VK_NULL_HANDLE) {
        ToBackend(GetDevice())->GetFencedDeleter()->DeleteWhenUnused(mPool);
        mPool = VK_NULL_HANDLE;
        mSet = VK_NULL_HANDLE;
    }
    ResourceTableBase::DestroyImpl(reason);
}

void ResourceTable::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mSet, "Dawn_ResourceTable", GetLabel());
}

}  // namespace dawn::native::vulkan
