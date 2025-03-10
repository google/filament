// Copyright 2017 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_
#define SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/common/NSRef.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipelineBase> CreateUninitialized(
        Device* device,
        const UnpackedPtr<RenderPipelineDescriptor>& descriptor);

    RenderPipeline(DeviceBase* device, const UnpackedPtr<RenderPipelineDescriptor>& descriptor);
    ~RenderPipeline() override;

    MTLPrimitiveType GetMTLPrimitiveTopology() const;
    MTLWinding GetMTLFrontFace() const;
    MTLCullMode GetMTLCullMode() const;

    void Encode(id<MTLRenderCommandEncoder> encoder);

    id<MTLDepthStencilState> GetMTLDepthStencilState();

    // For each Dawn vertex buffer, give the index in which it will be positioned in the Metal
    // vertex buffer table.
    uint32_t GetMtlVertexBufferIndex(VertexBufferSlot slot) const;

    wgpu::ShaderStage GetStagesRequiringStorageBufferLength() const;

    MaybeError InitializeImpl() override;

  private:
    using RenderPipelineBase::RenderPipelineBase;

    NSRef<MTLVertexDescriptor> MakeVertexDesc() const;
    NSRef<MTLDepthStencilDescriptor> MakeDepthStencilDesc();

    MTLPrimitiveType mMtlPrimitiveTopology;
    MTLWinding mMtlFrontFace;
    MTLCullMode mMtlCullMode;
    NSPRef<id<MTLRenderPipelineState>> mMtlRenderPipelineState;
    NSPRef<id<MTLDepthStencilState>> mMtlDepthStencilState;
    PerVertexBuffer<uint32_t> mMtlVertexBufferIndices;

    wgpu::ShaderStage mStagesRequiringStorageBufferLength = wgpu::ShaderStage::None;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_RENDERPIPELINEMTL_H_
