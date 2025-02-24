// Copyright 2019 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_RENDERENCODERBASE_H_
#define SRC_DAWN_NATIVE_RENDERENCODERBASE_H_

#include "dawn/native/AttachmentState.h"
#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/Error.h"
#include "dawn/native/IndirectDrawMetadata.h"
#include "dawn/native/PassResourceUsageTracker.h"
#include "dawn/native/ProgrammableEncoder.h"

namespace dawn::native {

class RenderEncoderBase : public ProgrammableEncoder {
  public:
    RenderEncoderBase(DeviceBase* device,
                      StringView label,
                      EncodingContext* encodingContext,
                      Ref<AttachmentState> attachmentState,
                      bool depthReadOnly,
                      bool stencilReadOnly);

    void APIDraw(uint32_t vertexCount,
                 uint32_t instanceCount = 1,
                 uint32_t firstVertex = 0,
                 uint32_t firstInstance = 0);
    void APIDrawIndexed(uint32_t vertexCount,
                        uint32_t instanceCount,
                        uint32_t firstIndex,
                        int32_t baseVertex,
                        uint32_t firstInstance);

    void APIDrawIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset);
    void APIDrawIndexedIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset);

    void APIMultiDrawIndirect(BufferBase* indirectBuffer,
                              uint64_t indirectOffset,
                              uint32_t maxDrawCount,
                              BufferBase* drawCountBuffer = nullptr,
                              uint64_t drawCountBufferOffset = 0);

    void APIMultiDrawIndexedIndirect(BufferBase* indirectBuffer,
                                     uint64_t indirectOffset,
                                     uint32_t maxDrawCount,
                                     BufferBase* drawCountBuffer = nullptr,
                                     uint64_t drawCountBufferOffset = 0);

    void APISetPipeline(RenderPipelineBase* pipeline);

    void APISetVertexBuffer(uint32_t slot, BufferBase* buffer, uint64_t offset, uint64_t size);
    void APISetIndexBuffer(BufferBase* buffer,
                           wgpu::IndexFormat format,
                           uint64_t offset,
                           uint64_t size);

    void APISetBindGroup(uint32_t groupIndex,
                         BindGroupBase* group,
                         uint32_t dynamicOffsetCount = 0,
                         const uint32_t* dynamicOffsets = nullptr);

    const AttachmentState* GetAttachmentState() const;
    bool IsDepthReadOnly() const;
    bool IsStencilReadOnly() const;
    uint64_t GetDrawCount() const;
    Ref<AttachmentState> AcquireAttachmentState();

  protected:
    // Construct an "error" render encoder base.
    RenderEncoderBase(DeviceBase* device,
                      EncodingContext* encodingContext,
                      ErrorTag errorTag,
                      StringView label);

    void DestroyImpl() override;

    CommandBufferStateTracker mCommandBufferState;
    RenderPassResourceUsageTracker mUsageTracker;
    IndirectDrawMetadata mIndirectDrawMetadata;

    uint64_t mDrawCount = 0;

  private:
    Ref<AttachmentState> mAttachmentState;
    const bool mDisableBaseVertex;
    const bool mDisableBaseInstance;
    bool mDepthReadOnly = false;
    bool mStencilReadOnly = false;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_RENDERENCODERBASE_H_
