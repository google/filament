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

#ifndef SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_
#define SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/vulkan/PipelineVk.h"

namespace dawn::native::vulkan {

class Device;
struct VkPipelineLayoutObject;

class RenderPipeline final : public RenderPipelineBase, public PipelineVk {
  public:
    static Ref<RenderPipeline> CreateUninitialized(
        Device* device,
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor);

    VkPipeline GetHandle() const;

    MaybeError InitializeImpl() override;

    // Dawn API
    void SetLabelImpl() override;

  private:
    ~RenderPipeline() override;
    void DestroyImpl() override;
    using RenderPipelineBase::RenderPipelineBase;

    struct PipelineVertexInputStateCreateInfoTemporaryAllocations {
        std::array<VkVertexInputBindingDescription, kMaxVertexBuffers> bindings;
        std::array<VkVertexInputAttributeDescription, kMaxVertexAttributes> attributes;
    };
    VkPipelineVertexInputStateCreateInfo ComputeVertexInputDesc(
        PipelineVertexInputStateCreateInfoTemporaryAllocations* temporaryAllocations);
    VkPipelineDepthStencilStateCreateInfo ComputeDepthStencilDesc();

    VkPipeline mHandle = VK_NULL_HANDLE;

    // Whether the pipeline has any input attachment being used in the frag shader.
    bool mHasInputAttachment = false;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_
