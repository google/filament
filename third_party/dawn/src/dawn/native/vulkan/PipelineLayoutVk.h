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

#ifndef SRC_DAWN_NATIVE_VULKAN_PIPELINELAYOUTVK_H_
#define SRC_DAWN_NATIVE_VULKAN_PIPELINELAYOUTVK_H_

#include <memory>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Error.h"
#include "dawn/native/PipelineLayout.h"
#include "dawn/native/vulkan/RefCountedVkHandle.h"

namespace dawn::native::vulkan {

class Device;

class PipelineLayout final : public PipelineLayoutBase {
  public:
    static ResultOrError<Ref<PipelineLayout>> Create(
        Device* device,
        const UnpackedPtr<PipelineLayoutDescriptor>& descriptor);

    // Pipeline might use different amounts of immediate data internally which cause difference in
    // VkPipelineLayout push constant range part. Pass internalImmediateDataSize to
    // get correct VkPipelineLayout.
    ResultOrError<Ref<RefCountedVkHandle<VkPipelineLayout>>> GetOrCreateVkLayoutObject(
        const ImmediateConstantMask& immediateConstantMask);

    VkShaderStageFlags GetImmediateDataRangeStage() const;

    // Friend definition of StreamIn which can be found by ADL to override stream::StreamIn<T>.
    friend void StreamIn(stream::Sink* sink, const PipelineLayout& obj) {
        StreamIn(sink, static_cast<const CachedObject&>(obj));
    }

  private:
    ~PipelineLayout() override;
    void DestroyImpl() override;

    using PipelineLayoutBase::PipelineLayoutBase;
    MaybeError Initialize();

    ResultOrError<Ref<RefCountedVkHandle<VkPipelineLayout>>> CreateVkPipelineLayout(
        uint32_t immediateConstantSize);

    // Dawn API
    void SetLabelImpl() override;

    // Multiple VkPipelineLayouts is possible because variant internal immediate data size.
    // Using map to manage 1 PipelineLayout to N VkPipelineLayouts relationship with
    // total immediate data size as key.
    MutexProtected<absl::flat_hash_map<uint32_t, Ref<RefCountedVkHandle<VkPipelineLayout>>>>
        mVkPipelineLayouts;

    // Immediate data requires unique range among shader stages.
    VkShaderStageFlags kImmediateDataRangeShaderStage =
        VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_PIPELINELAYOUTVK_H_
