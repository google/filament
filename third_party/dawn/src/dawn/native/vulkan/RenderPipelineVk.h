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

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/RenderPipeline.h"
#include "dawn/native/vulkan/PipelineLayoutVk.h"
#include "dawn/native/vulkan/PipelineVk.h"
#include "dawn/native/vulkan/RefCountedVkHandle.h"

namespace dawn::native::vulkan {

class Device;
struct VkPipelineLayoutObject;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipeline> CreateUninitialized(
        Device* device,
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor);

    // Specializations used to JIT pipelines.
    using Specialization = CommonPipelineSpecialization;
    ResultOrError<PipelineHandles> GetOrCreateSpecializedHandle(Specialization&& specialization);
    bool RequiresSpecialization() const;

    VkPipeline GetHandle() const;
    VkPipelineLayout GetVkLayout() const;

    void ApplyDynamicState(VkCommandBuffer& commands, const RenderPipeline* prevPipeline) const;

    // Dawn API
    void SetLabelImpl() override;

  private:
    struct DynamicState {
        VkPrimitiveTopology primitiveTopology;
        VkCullModeFlags cullMode;
        VkFrontFace frontFace;
        VkBool32 depthTestEnable;
        VkBool32 depthWriteEnable;
        VkCompareOp depthCompareOp;
        VkBool32 stencilTestEnable;
        uint16_t packedFrontStencil;
        uint16_t packedBackStencil;
    };

    ~RenderPipeline() override;

    void DestroyImpl(DestroyReason reason) override;
    using RenderPipelineBase::RenderPipelineBase;
    MaybeError InitializeImpl() override;

    bool NeedsPixelCenterPolyfill() const;

    // Initializes a pipeline for the specialization and stores it in mSpecializations.
    struct SpecializationResult {
        Ref<RefCountedVkHandle<VkPipeline>> pipeline;
        Ref<RefCountedVkHandle<VkPipelineLayout>> layout;
    };
    ResultOrError<SpecializationResult> InitializeSpecialization(
        const Specialization& specialization,
        bool buildCacheKey);

    // Helpers used while building the VkPipelineCreateInfo.
    struct PipelineVertexInputStateCreateInfoTemporaryAllocations {
        std::array<VkVertexInputBindingDescription, kMaxVertexBuffers> bindings;
        std::array<VkVertexInputAttributeDescription, kMaxVertexAttributes> attributes;
    };
    VkPipelineVertexInputStateCreateInfo ComputeVertexInputDesc(
        PipelineVertexInputStateCreateInfoTemporaryAllocations* temporaryAllocations);
    VkPipelineDepthStencilStateCreateInfo ComputeDepthStencilDesc();

    // The handles are owned by a ref in mSpecializations.
    PipelineHandles mHandles = {};

    // Caches the specializations as we are most likely to reuse the overtime. Note that noop
    // specialization has mHandle cached directly but mHandle is also kept separately for
    // efficiency.
    bool mRequiresSpecialization = false;
    absl::flat_hash_map<Specialization, SpecializationResult> mSpecializations;

    DynamicState mDynamicState = {};

    // Whether the pipeline has any input attachment being used in the frag shader.
    bool mHasInputAttachment = false;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_RENDERPIPELINEVK_H_
