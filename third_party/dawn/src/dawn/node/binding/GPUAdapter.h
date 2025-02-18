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

#ifndef SRC_DAWN_NODE_BINDING_GPUADAPTER_H_
#define SRC_DAWN_NODE_BINDING_GPUADAPTER_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>

#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
class Flags;

// GPUAdapter is an implementation of interop::GPUAdapter that wraps a wgpu::Adapter.
class GPUAdapter final : public interop::GPUAdapter {
  public:
    GPUAdapter(wgpu::Adapter adapter, const Flags& flags, std::shared_ptr<AsyncRunner> async);

    // interop::GPUAdapter interface compliance
    interop::Promise<interop::Interface<interop::GPUDevice>> requestDevice(
        Napi::Env env,
        interop::GPUDeviceDescriptor descriptor) override;
    interop::Interface<interop::GPUSupportedFeatures> getFeatures(Napi::Env) override;
    interop::Interface<interop::GPUSupportedLimits> getLimits(Napi::Env) override;
    interop::Interface<interop::GPUAdapterInfo> getInfo(Napi::Env) override;
    bool getIsFallbackAdapter(Napi::Env) override;
    bool getIsCompatibilityMode(Napi::Env) override;
    std::string getFeatureLevel(Napi::Env) override;

  private:
    wgpu::Adapter adapter_;
    const Flags& flags_;
    std::shared_ptr<AsyncRunner> async_;

    // The adapter becomes invalid after the first successful requestDevice and all subsequent
    // requestDevice calls will return an already lost device.
    bool valid_ = true;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUADAPTER_H_
