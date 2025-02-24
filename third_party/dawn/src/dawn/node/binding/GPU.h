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

#ifndef SRC_DAWN_NODE_BINDING_GPU_H_
#define SRC_DAWN_NODE_BINDING_GPU_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/binding/Flags.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
// GPU is an implementation of interop::GPU that wraps a dawn::native::Instance.
class GPU final : public interop::GPU {
  public:
    GPU(Flags flags);

    // interop::GPU interface compliance
    interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> requestAdapter(
        Napi::Env env,
        interop::GPURequestAdapterOptions options) override;
    interop::GPUTextureFormat getPreferredCanvasFormat(Napi::Env) override;
    interop::Interface<interop::WGSLLanguageFeatures> getWgslLanguageFeatures(Napi::Env) override;

  private:
    const Flags flags_;
    std::unique_ptr<dawn::native::Instance> instance_;
    std::shared_ptr<AsyncRunner> async_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPU_H_
