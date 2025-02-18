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

#ifndef SRC_DAWN_NODE_BINDING_GPUPIPELINELAYOUT_H_
#define SRC_DAWN_NODE_BINDING_GPUPIPELINELAYOUT_H_

#include <webgpu/webgpu_cpp.h>

#include <string>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUPipelineLayout is an implementation of interop::GPUPipelineLayout that wraps a
// wgpu::PipelineLayout.
class GPUPipelineLayout final : public interop::GPUPipelineLayout {
  public:
    GPUPipelineLayout(const wgpu::PipelineLayoutDescriptor& desc, wgpu::PipelineLayout layout);

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::PipelineLayout&() const { return layout_; }

    // interop::GPUPipelineLayout interface compliance
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::PipelineLayout layout_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUPIPELINELAYOUT_H_
