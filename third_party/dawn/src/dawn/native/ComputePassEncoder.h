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

#ifndef SRC_DAWN_NATIVE_COMPUTEPASSENCODER_H_
#define SRC_DAWN_NATIVE_COMPUTEPASSENCODER_H_

#include <utility>
#include <vector>

#include "dawn/native/CommandBufferStateTracker.h"
#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/PassResourceUsageTracker.h"
#include "dawn/native/ProgrammableEncoder.h"

namespace dawn::native {

class SyncScopeUsageTracker;

class ComputePassEncoder final : public ProgrammableEncoder {
  public:
    static Ref<ComputePassEncoder> Create(DeviceBase* device,
                                          const ComputePassDescriptor* descriptor,
                                          CommandEncoder* commandEncoder,
                                          EncodingContext* encodingContext);
    static Ref<ComputePassEncoder> MakeError(DeviceBase* device,
                                             CommandEncoder* commandEncoder,
                                             EncodingContext* encodingContext,
                                             StringView label);

    ~ComputePassEncoder() override;

    ObjectType GetType() const override;

    void APIEnd();

    void APIDispatchWorkgroups(uint32_t workgroupCountX,
                               uint32_t workgroupCountY = 1,
                               uint32_t workgroupCountZ = 1);
    void APIDispatchWorkgroupsIndirect(BufferBase* indirectBuffer, uint64_t indirectOffset);
    void APISetPipeline(ComputePipelineBase* pipeline);

    void APISetBindGroup(uint32_t groupIndex,
                         BindGroupBase* group,
                         uint32_t dynamicOffsetCount = 0,
                         const uint32_t* dynamicOffsets = nullptr);

    void APIWriteTimestamp(QuerySetBase* querySet, uint32_t queryIndex);

    CommandBufferStateTracker* GetCommandBufferStateTrackerForTesting();
    void RestoreCommandBufferStateForTesting(CommandBufferStateTracker state) {
        RestoreCommandBufferState(std::move(state));
    }

  protected:
    ComputePassEncoder(DeviceBase* device,
                       const ComputePassDescriptor* descriptor,
                       CommandEncoder* commandEncoder,
                       EncodingContext* encodingContext);
    ComputePassEncoder(DeviceBase* device,
                       CommandEncoder* commandEncoder,
                       EncodingContext* encodingContext,
                       ErrorTag errorTag,
                       StringView label);

  private:
    void DestroyImpl() override;

    ResultOrError<std::pair<Ref<BufferBase>, uint64_t>> TransformIndirectDispatchBuffer(
        Ref<BufferBase> indirectBuffer,
        uint64_t indirectOffset);

    void RestoreCommandBufferState(CommandBufferStateTracker state);

    CommandBufferStateTracker mCommandBufferState;

    // Adds the bindgroups used for the current dispatch to the SyncScopeResourceUsage and
    // records it in mUsageTracker.
    void AddDispatchSyncScope(SyncScopeUsageTracker scope = {});
    ComputePassResourceUsageTracker mUsageTracker;

    // For render and compute passes, the encoding context is borrowed from the command encoder.
    // Keep a reference to the encoder to make sure the context isn't freed.
    Ref<CommandEncoder> mCommandEncoder;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_COMPUTEPASSENCODER_H_
