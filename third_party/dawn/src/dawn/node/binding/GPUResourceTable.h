// Copyright 2026 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NODE_BINDING_GPURESOURCETABLE_H_
#define SRC_DAWN_NODE_BINDING_GPURESOURCETABLE_H_

#include <webgpu/webgpu_cpp.h>

#include <string>

#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
// GPUResourceTable is an implementation of interop::GPUResourceTable that wraps a
// wgpu::ResourceTable.
class GPUResourceTable final : public interop::GPUResourceTable {
  public:
    GPUResourceTable(const wgpu::ResourceTableDescriptor& desc, wgpu::ResourceTable table);

    // Implicit cast operator to Dawn GPU object
    // NOLINTNEXTLINE(google-explicit-constructor)
    inline operator const wgpu::ResourceTable&() const { return table_; }

    // interop::GPUResourceTable interface compliance
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

    void destroy(Napi::Env) override;
    interop::GPUSize32Out getSize(Napi::Env) override;

    void update(Napi::Env, interop::GPUIndex32 slot, interop::GPUBindingResource resource) override;
    interop::GPUIndex32 insert(Napi::Env, interop::GPUBindingResource resource) override;
    void remove(Napi::Env, interop::GPUIndex32 slot) override;

  private:
    wgpu::ResourceTable table_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPURESOURCETABLE_H_
