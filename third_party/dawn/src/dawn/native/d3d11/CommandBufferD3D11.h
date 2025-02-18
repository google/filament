// Copyright 2023 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_COMMANDBUFFERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_COMMANDBUFFERD3D11_H_

#include "dawn/native/CommandBuffer.h"

namespace dawn::native {
enum class Command;
struct BeginRenderPassCmd;
struct DispatchCmd;
}  // namespace dawn::native

namespace dawn::native::d3d11 {

class ComputePipeline;
class RenderPipeline;
class ScopedSwapStateCommandRecordingContext;

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);
    MaybeError Execute(const ScopedSwapStateCommandRecordingContext* commandContext);

  private:
    using CommandBufferBase::CommandBufferBase;

    MaybeError ExecuteComputePass(const ScopedSwapStateCommandRecordingContext* commandContext);
    MaybeError ExecuteRenderPass(BeginRenderPassCmd* renderPass,
                                 const ScopedSwapStateCommandRecordingContext* commandContext);
    void HandleDebugCommands(const ScopedSwapStateCommandRecordingContext* commandContext,
                             CommandIterator* iter,
                             Command command);

    MaybeError RecordFirstIndexOffset(RenderPipeline* renderPipeline,
                                      const ScopedSwapStateCommandRecordingContext* commandContext,
                                      uint32_t firstVertex,
                                      uint32_t firstInstance);
    MaybeError RecordNumWorkgroupsForDispatch(
        ComputePipeline* computePipeline,
        const ScopedSwapStateCommandRecordingContext* commandContext,
        DispatchCmd* dispatchCmd);
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMMANDBUFFERD3D11_H_
