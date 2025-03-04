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

#ifndef SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_
#define SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_

#include <set>
#include <utility>
#include <vector>

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Error.h"
#include "dawn/native/metal/MultiDrawEncoder.h"

#import <Metal/Metal.h>

namespace dawn::native {
class CommandEncoder;
struct BeginComputePassCmd;
struct BeginRenderPassCmd;
}  // namespace dawn::native

namespace dawn::native::metal {

class CommandRecordingContext;
class Device;
class Texture;
class QuerySet;

void RecordCopyBufferToTexture(CommandRecordingContext* commandContext,
                               id<MTLBuffer> mtlBuffer,
                               uint64_t bufferSize,
                               uint64_t offset,
                               uint32_t bytesPerRow,
                               uint32_t rowsPerImage,
                               Texture* texture,
                               uint32_t mipLevel,
                               const Origin3D& origin,
                               Aspect aspect,
                               const Extent3D& copySize);

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);

    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);
    ~CommandBuffer() override;

    MaybeError FillCommands(CommandRecordingContext* commandContext);

  private:
    using CommandBufferBase::CommandBufferBase;

    MaybeError EncodeComputePass(CommandRecordingContext* commandContext,
                                 BeginComputePassCmd* computePassCmd);

    // Empty occlusion queries aren't filled to zero on Apple GPUs. This set is used to
    // track which results should be explicitly zero'ed as a workaround. Use of empty queries
    // *should* mostly be a degenerate scenario, so this std::set shouldn't be performance-critical.
    // The set is passed as nullptr to `EncodeRenderPass` if the workaround is not in use.
    using EmptyOcclusionQueries = std::set<std::pair<QuerySet*, uint32_t>>;
    MaybeError EncodeRenderPass(id<MTLRenderCommandEncoder> encoder,
                                BeginRenderPassCmd* renderPassCmd,
                                EmptyOcclusionQueries* emptyOcclusionQueries,
                                const std::vector<MultiDrawExecutionData>& multiDrawExecutions);
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_COMMANDBUFFERMTL_H_
