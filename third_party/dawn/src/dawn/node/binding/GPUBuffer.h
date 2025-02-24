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

#ifndef SRC_DAWN_NODE_BINDING_GPUBUFFER_H_
#define SRC_DAWN_NODE_BINDING_GPUBUFFER_H_

#include <webgpu/webgpu_cpp.h>

#include <memory>
#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUBuffer is an implementation of interop::GPUBuffer that wraps a wgpu::Buffer.
class GPUBuffer final : public interop::GPUBuffer {
  public:
    GPUBuffer(wgpu::Buffer buffer,
              wgpu::BufferDescriptor desc,
              wgpu::Device device,
              std::shared_ptr<AsyncRunner> async);

    // Desc() returns the wgpu::BufferDescriptor used to construct the buffer
    const wgpu::BufferDescriptor& Desc() const { return desc_; }

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::Buffer&() const { return buffer_; }

    // interop::GPUBuffer interface compliance
    interop::Promise<void> mapAsync(Napi::Env env,
                                    interop::GPUMapModeFlags mode,
                                    interop::GPUSize64 offset,
                                    std::optional<interop::GPUSize64> size) override;
    interop::ArrayBuffer getMappedRange(Napi::Env env,
                                        interop::GPUSize64 offset,
                                        std::optional<interop::GPUSize64> size) override;
    void unmap(Napi::Env) override;
    void destroy(Napi::Env) override;
    interop::GPUSize64Out getSize(Napi::Env) override;
    interop::GPUFlagsConstant getUsage(Napi::Env) override;
    interop::GPUBufferMapState getMapState(Napi::Env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    void DetachMappings(Napi::Env env);

    struct Mapping {
        uint64_t start;
        uint64_t end;
        inline bool Intersects(uint64_t s, uint64_t e) const { return s < end && e > start; }
        Napi::Reference<interop::ArrayBuffer> buffer;
    };

    // https://www.w3.org/TR/webgpu/#buffer-interface
    enum class State {
        Unmapped,
        Mapped,
        MappedAtCreation,
        MappingPending,
        Destroyed,
    };

    wgpu::Buffer buffer_;
    wgpu::BufferDescriptor const desc_;
    wgpu::Device const device_;
    std::shared_ptr<AsyncRunner> async_;
    std::vector<Mapping> mappings_;
    bool mapped_;
    std::optional<interop::Promise<void>> pending_map_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUBUFFER_H_
