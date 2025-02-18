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

#ifndef SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_
#define SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_

#include <webgpu/webgpu_cpp.h>

#include <string>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUAdapterInfo is an implementation of interop::GPUAdapterInfo.
class GPUAdapterInfo final : public interop::GPUAdapterInfo {
  public:
    explicit GPUAdapterInfo(const wgpu::AdapterInfo& info);

    // interop::GPUAdapterInfo interface compliance
    std::string getVendor(Napi::Env) override;
    std::string getArchitecture(Napi::Env) override;
    std::string getDevice(Napi::Env) override;
    std::string getDescription(Napi::Env) override;
    uint32_t getSubgroupMinSize(Napi::Env) override;
    uint32_t getSubgroupMaxSize(Napi::Env) override;

  private:
    std::string vendor_;
    std::string architecture_;
    std::string device_;
    std::string description_;
    uint32_t subgroup_min_size_;
    uint32_t subgroup_max_size_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUADAPTERINFO_H_
