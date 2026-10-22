// Copyright 2021 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NODE_BINDING_GPUQUEUE_H_
#define SRC_DAWN_NODE_BINDING_GPUQUEUE_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUQueue is an implementation of interop::GPUQueue that wraps a wgpu::Queue.
class GPUQueue final : public interop::GPUQueue {
  public:
    GPUQueue(wgpu::Queue queue, std::shared_ptr<AsyncRunner> async);

    // interop::GPUQueue interface compliance
    void submit(Napi::Env,
                std::vector<interop::Interface<interop::GPUCommandBuffer>> commandBuffers) override;
    interop::Promise<void> onSubmittedWorkDone(Napi::Env) override;
    void writeBuffer(Napi::Env,
                     interop::Interface<interop::GPUBuffer> buffer,
                     interop::GPUSize64 bufferOffset,
                     interop::AllowSharedBufferSource data,
                     interop::GPUSize64 dataOffset,
                     std::optional<interop::GPUSize64> size) override;
    void writeTexture(Napi::Env,
                      interop::GPUTexelCopyTextureInfo destination,
                      interop::AllowSharedBufferSource data,
                      interop::GPUTexelCopyBufferLayout dataLayout,
                      interop::GPUExtent3D size) override;
    void copyExternalImageToTexture(Napi::Env,
                                    interop::GPUCopyExternalImageSourceInfo source,
                                    interop::GPUCopyExternalImageDestInfo destination,
                                    interop::GPUExtent3D copySize) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::Queue queue_;
    std::shared_ptr<AsyncRunner> async_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUQUEUE_H_
