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

#ifndef SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFER_FETCH_H_
#define SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFER_FETCH_H_

#include "src/dawn/common/GPUInfo.h"
#include "src/dawn/common/MutexProtected.h"
#include "src/dawn/common/Ref.h"
#include "src/dawn/common/vulkan_platform.h"
#include "src/dawn/native/Commands.h"
#include "src/dawn/native/vulkan/DescriptorSetAllocator.h"
#include "src/dawn/native/vulkan/ResourceTableVk.h"
#include "src/dawn/native/vulkan/VulkanInfo.h"

namespace dawn::native::vulkan {

// Helper to create VkDescriptorSets for binding color attachments as input attachments in order
// to implement FramebufferFetch.
class FramebufferFetchHelper {
  public:
    // Checks if hardware supports coherent rasterization natively. FramebufferFetch only requires
    // a subpass self-dependency for color attachments, and no barriers, if this is true.
    static bool SupportsCoherentRasterization(PCIVendorID vendorId);

    explicit FramebufferFetchHelper(Device* device);
    ~FramebufferFetchHelper();

    // Returns layout suitable for a render pass with `attachmentCount` color attachments.
    ResultOrError<VkDescriptorSetLayout> GetLayout(uint32_t attachmentCount);

    // Returns a descriptor set that binds all render pass color attachments as input attachments.
    // The returned descriptor set is only valid to use for the current render pass.
    // It will be destroyed after the render pass is submitted to the GPU.
    ResultOrError<VkDescriptorSet> GetDescriptorsForRenderPass(const BeginRenderPassCmd* cmd);

  private:
    struct DescriptorSetHolder {
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        Ref<DescriptorSetAllocator> allocator;
    };

    // Returns initialized descriptor set layout and allocator for `attachmentCount`. This is safe
    // to call from any thread.
    ResultOrError<DescriptorSetHolder> GetDescriptorSetData(uint32_t attachmentCount);

    raw_ptr<Device> mDevice;
    MutexProtected<std::array<DescriptorSetHolder, kMaxColorAttachments>> mHolders;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_FRAMEBUFFER_FETCH_H_
