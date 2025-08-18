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

#ifndef SRC_DAWN_NODE_BINDING_GPUSUPPORTEDLIMITS_H_
#define SRC_DAWN_NODE_BINDING_GPUSUPPORTEDLIMITS_H_

#include <webgpu/webgpu_cpp.h>

#include "dawn/native/DawnNative.h"
#include "dawn/utils/ComboLimits.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUSupportedLimits is an implementation of interop::GPUSupportedLimits.
class GPUSupportedLimits final : public interop::GPUSupportedLimits {
  public:
    explicit GPUSupportedLimits(const dawn::utils::ComboLimits& limits);

    // interop::GPUSupportedLimits interface compliance
    uint32_t getMaxTextureDimension1D(Napi::Env) override;
    uint32_t getMaxTextureDimension2D(Napi::Env) override;
    uint32_t getMaxTextureDimension3D(Napi::Env) override;
    uint32_t getMaxTextureArrayLayers(Napi::Env) override;
    uint32_t getMaxBindGroups(Napi::Env) override;
    uint32_t getMaxBindGroupsPlusVertexBuffers(Napi::Env) override;
    uint32_t getMaxBindingsPerBindGroup(Napi::Env) override;
    uint32_t getMaxDynamicUniformBuffersPerPipelineLayout(Napi::Env) override;
    uint32_t getMaxDynamicStorageBuffersPerPipelineLayout(Napi::Env) override;
    uint32_t getMaxSampledTexturesPerShaderStage(Napi::Env) override;
    uint32_t getMaxSamplersPerShaderStage(Napi::Env) override;
    uint32_t getMaxStorageBuffersPerShaderStage(Napi::Env) override;
    uint32_t getMaxStorageTexturesPerShaderStage(Napi::Env) override;
    uint32_t getMaxUniformBuffersPerShaderStage(Napi::Env) override;
    uint64_t getMaxUniformBufferBindingSize(Napi::Env) override;
    uint64_t getMaxStorageBufferBindingSize(Napi::Env) override;
    uint32_t getMinUniformBufferOffsetAlignment(Napi::Env) override;
    uint32_t getMinStorageBufferOffsetAlignment(Napi::Env) override;
    uint32_t getMaxVertexBuffers(Napi::Env) override;
    uint64_t getMaxBufferSize(Napi::Env) override;
    uint32_t getMaxVertexAttributes(Napi::Env) override;
    uint32_t getMaxVertexBufferArrayStride(Napi::Env) override;
    uint32_t getMaxInterStageShaderVariables(Napi::Env) override;
    uint32_t getMaxColorAttachments(Napi::Env) override;
    uint32_t getMaxColorAttachmentBytesPerSample(Napi::Env) override;
    uint32_t getMaxComputeWorkgroupStorageSize(Napi::Env) override;
    uint32_t getMaxComputeInvocationsPerWorkgroup(Napi::Env) override;
    uint32_t getMaxComputeWorkgroupSizeX(Napi::Env) override;
    uint32_t getMaxComputeWorkgroupSizeY(Napi::Env) override;
    uint32_t getMaxComputeWorkgroupSizeZ(Napi::Env) override;
    uint32_t getMaxComputeWorkgroupsPerDimension(Napi::Env) override;
    uint32_t getMaxStorageBuffersInFragmentStage(Napi::Env) override;
    uint32_t getMaxStorageTexturesInFragmentStage(Napi::Env) override;
    uint32_t getMaxStorageBuffersInVertexStage(Napi::Env) override;
    uint32_t getMaxStorageTexturesInVertexStage(Napi::Env) override;

  private:
    dawn::utils::ComboLimits limits_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUSUPPORTEDLIMITS_H_
