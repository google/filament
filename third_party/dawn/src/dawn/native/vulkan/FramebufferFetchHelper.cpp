// Copyright 2026 The Dawn & Tint Authors
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

#include "src/dawn/native/vulkan/FramebufferFetchHelper.h"

#include <utility>

#include "src/dawn/native/Error.h"
#include "src/dawn/native/vulkan/DeviceVk.h"
#include "src/dawn/native/vulkan/ShaderModuleVk.h"
#include "src/dawn/native/vulkan/TextureVk.h"
#include "src/dawn/native/vulkan/UtilsVulkan.h"
#include "src/dawn/native/vulkan/VulkanError.h"
#include "src/utils/log.h"
#include "vulkan/vulkan_core.h"

namespace dawn::native::vulkan {
namespace {

ResultOrError<VkDescriptorSetLayout> MakeFramebufferFetchLayout(Device* device,
                                                                uint32_t attachmentCount) {
    std::array<VkDescriptorSetLayoutBinding, kMaxColorAttachments> bindings;
    for (uint32_t i = 0; i < attachmentCount; ++i) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[i].pImmutableSamplers = nullptr;
    }

    VkDescriptorSetLayoutCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = attachmentCount;
    createInfo.pBindings = bindings.data();

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    DAWN_TRY(CheckVkSuccess(
        device->fn.CreateDescriptorSetLayout(device->GetVkDevice(), &createInfo, nullptr, &*layout),
        "CreateDescriptorSetLayout"));
    return layout;
}

}  // namespace

bool FramebufferFetchHelper::SupportsCoherentRasterization(PCIVendorID vendorId) {
    return gpu_info::IsARM(vendorId);
}

FramebufferFetchHelper::FramebufferFetchHelper(Device* device) : mDevice(device) {
    DAWN_ASSERT(mDevice);
}

FramebufferFetchHelper::~FramebufferFetchHelper() {
    mHolders.Use([this](auto holders) {
        for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
            auto& holder = (*holders)[i];
            if (holder.layout != VK_NULL_HANDLE) {
                mDevice->fn.DestroyDescriptorSetLayout(mDevice->GetVkDevice(), holder.layout,
                                                       nullptr);
                holder.layout = VK_NULL_HANDLE;
            }
            holder.allocator = nullptr;
        }
    });
}

ResultOrError<VkDescriptorSetLayout> FramebufferFetchHelper::GetLayout(uint32_t attachmentCount) {
    DescriptorSetHolder holder;
    DAWN_TRY_ASSIGN(holder, GetDescriptorSetData(attachmentCount));
    return holder.layout;
}

ResultOrError<VkDescriptorSet> FramebufferFetchHelper::GetDescriptorsForRenderPass(
    const BeginRenderPassCmd* cmd) {
    uint32_t attachmentCount = AttachmentCount(cmd->attachmentState->GetColorAttachmentsMask());
    DAWN_CHECK(attachmentCount > 0);

    DescriptorSetHolder holder;
    DAWN_TRY_ASSIGN(holder, GetDescriptorSetData(attachmentCount));

    // Needed to work around some quirks introduced by vulkan_platform.h which causes some platforms
    // to hit compiler errors if Vulkan struct members are assigned to VK_NULL_HANDLE directly.
    static const VkSampler nullSampler = VK_NULL_HANDLE;

    DescriptorSetAllocation allocation;
    DAWN_TRY_ASSIGN(allocation, holder.allocator->Allocate(holder.layout));
    VkDescriptorSet set = allocation.set;

    std::array<VkWriteDescriptorSet, kMaxColorAttachments> writes;
    std::array<VkDescriptorImageInfo, kMaxColorAttachments> imageInfos;
    uint32_t writeCount = 0;

    for (auto i : cmd->attachmentState->GetColorAttachmentsMask()) {
        auto& attachmentInfo = cmd->colorAttachments[i];
        TextureView* textureView = ToBackend(attachmentInfo.view.Get());

        VkImageView imageView;
        if (textureView->GetDimension() == wgpu::TextureViewDimension::e3D) {
            DAWN_TRY_ASSIGN(imageView,
                            textureView->GetOrCreate2DViewOn3D(attachmentInfo.depthSlice));
        } else {
            imageView = textureView->GetHandle();
        }
        DAWN_ASSERT(imageView != VK_NULL_HANDLE);

        auto& imageInfo = imageInfos[writeCount];
        imageInfo.sampler = nullSampler;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageInfo.imageView = imageView;

        auto& write = writes[writeCount];
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.pNext = nullptr;
        write.dstSet = set;
        write.dstBinding = static_cast<uint32_t>(i);
        write.dstArrayElement = 0;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        write.pImageInfo = &imageInfo;
        write.pBufferInfo = nullptr;
        write.pTexelBufferView = nullptr;

        writeCount++;
    }
    DAWN_ASSERT(writeCount == attachmentCount);

    mDevice->fn.UpdateDescriptorSets(mDevice->GetVkDevice(), attachmentCount, writes.data(), 0,
                                     nullptr);

    // This will delete the VkDescriptorSet after the pending command serial has completed so the
    // VkDescriptorSet is valid for the duration of the current submit.
    holder.allocator->Deallocate(&allocation);

    return set;
}

ResultOrError<FramebufferFetchHelper::DescriptorSetHolder>
FramebufferFetchHelper::GetDescriptorSetData(uint32_t attachmentCount) {
    DAWN_CHECK(attachmentCount > 0 && attachmentCount <= kMaxColorAttachments);

    return mHolders.Use([this,
                         attachmentCount](auto holders) -> ResultOrError<DescriptorSetHolder> {
        auto& holder = (*holders)[attachmentCount - 1];

        if (holder.layout == VK_NULL_HANDLE) {
            DAWN_TRY_ASSIGN(holder.layout, MakeFramebufferFetchLayout(mDevice, attachmentCount));

            absl::flat_hash_map<VkDescriptorType, uint32_t> descriptorCountPerType;
            descriptorCountPerType[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] = attachmentCount;
            holder.allocator =
                DescriptorSetAllocator::Create(mDevice, std::move(descriptorCountPerType));
        }

        return holder;
    });
}

}  // namespace dawn::native::vulkan
