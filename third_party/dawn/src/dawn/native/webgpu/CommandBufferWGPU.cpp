// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/webgpu/CommandBufferWGPU.h"

#include "dawn/native/webgpu/BufferWGPU.h"
#include "dawn/native/webgpu/DeviceWGPU.h"

namespace dawn::native::webgpu {

// static
Ref<CommandBuffer> CommandBuffer::Create(CommandEncoder* encoder,
                                         const CommandBufferDescriptor* descriptor) {
    return AcquireRef(new CommandBuffer(encoder, descriptor));
}

CommandBuffer::CommandBuffer(CommandEncoder* encoder, const CommandBufferDescriptor* descriptor)
    : CommandBufferBase(encoder, descriptor) {}

WGPUCommandBuffer CommandBuffer::Encode() {
    auto& wgpu = ToBackend(GetDevice())->wgpu;

    // TODO(crbug.com/413053623): Use stored command encoder descriptor
    WGPUCommandEncoder innerEncoder =
        wgpu.deviceCreateCommandEncoder(ToBackend(GetDevice())->GetInnerHandle(), nullptr);

    Command type;
    while (mCommands.NextCommandId(&type)) {
        switch (type) {
            case Command::CopyBufferToBuffer: {
                CopyBufferToBufferCmd* copy = mCommands.NextCommand<CopyBufferToBufferCmd>();
                wgpu.commandEncoderCopyBufferToBuffer(
                    innerEncoder, ToBackend(copy->source)->GetInnerHandle(), copy->sourceOffset,
                    ToBackend(copy->destination)->GetInnerHandle(), copy->destinationOffset,
                    copy->size);
                break;
            }
            default:
                DAWN_UNREACHABLE();
        }
    }

    // TODO(crbug.com/413053623): Store WGPUCommandBufferDescriptor and assign here.
    WGPUCommandBuffer result = wgpu.commandEncoderFinish(innerEncoder, nullptr);
    wgpu.commandEncoderRelease(innerEncoder);
    return result;
}

}  // namespace dawn::native::webgpu
