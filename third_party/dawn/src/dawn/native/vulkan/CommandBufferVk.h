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

#ifndef SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_
#define SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_

#include <set>

#include "dawn/native/CommandBuffer.h"
#include "dawn/native/Error.h"

#include "dawn/common/vulkan_platform.h"

namespace dawn::native {
struct BeginComputePassCmd;
struct BeginRenderPassCmd;
struct TextureCopy;
}  // namespace dawn::native

namespace dawn::native::vulkan {

struct CommandRecordingContext;
class Device;

MaybeError RecordBeginRenderPass(CommandRecordingContext* recordingContext,
                                 Device* device,
                                 BeginRenderPassCmd* renderPass);

class CommandBuffer final : public CommandBufferBase {
  public:
    static Ref<CommandBuffer> Create(CommandEncoder* encoder,
                                     const CommandBufferDescriptor* descriptor);

    MaybeError RecordCommands(CommandRecordingContext* recordingContext);

  private:
    CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor);

    MaybeError RecordComputePass(CommandRecordingContext* recordingContext,
                                 BeginComputePassCmd* computePass,
                                 const ComputePassResourceUsage& resourceUsages);
    MaybeError RecordRenderPass(CommandRecordingContext* recordingContext,
                                BeginRenderPassCmd* renderPass);
    MaybeError RecordCopyImageWithTemporaryBuffer(CommandRecordingContext* recordingContext,
                                                  const TextureCopy& srcCopy,
                                                  const TextureCopy& dstCopy,
                                                  const Extent3D& copySize);
};

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_COMMANDBUFFERVK_H_
