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

#ifndef SRC_DAWN_NODE_BINDING_GPUCOMMANDENCODER_H_
#define SRC_DAWN_NODE_BINDING_GPUCOMMANDENCODER_H_

#include <webgpu/webgpu_cpp.h>

#include <string>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUCommandEncoder is an implementation of interop::GPUCommandEncoder that wraps a
// wgpu::CommandEncoder.
class GPUCommandEncoder final : public interop::GPUCommandEncoder {
  public:
    GPUCommandEncoder(wgpu::Device device,
                      const wgpu::CommandEncoderDescriptor& desc,
                      wgpu::CommandEncoder enc);

    // interop::GPUCommandEncoder interface compliance
    interop::Interface<interop::GPURenderPassEncoder> beginRenderPass(
        Napi::Env,
        interop::GPURenderPassDescriptor descriptor) override;
    interop::Interface<interop::GPUComputePassEncoder> beginComputePass(
        Napi::Env,
        interop::GPUComputePassDescriptor descriptor) override;
    void clearBuffer(Napi::Env,
                     interop::Interface<interop::GPUBuffer> buffer,
                     interop::GPUSize64 offset,
                     std::optional<interop::GPUSize64> size) override;
    void copyBufferToBuffer(Napi::Env env,
                            interop::Interface<interop::GPUBuffer> source,
                            interop::Interface<interop::GPUBuffer> destination,
                            std::optional<interop::GPUSize64> size) override;
    void copyBufferToBuffer(Napi::Env,
                            interop::Interface<interop::GPUBuffer> source,
                            interop::GPUSize64 sourceOffset,
                            interop::Interface<interop::GPUBuffer> destination,
                            interop::GPUSize64 destinationOffset,
                            std::optional<interop::GPUSize64> size) override;
    void copyBufferToTexture(Napi::Env,
                             interop::GPUTexelCopyBufferInfo source,
                             interop::GPUTexelCopyTextureInfo destination,
                             interop::GPUExtent3D copySize) override;
    void copyTextureToBuffer(Napi::Env,
                             interop::GPUTexelCopyTextureInfo source,
                             interop::GPUTexelCopyBufferInfo destination,
                             interop::GPUExtent3D copySize) override;
    void copyTextureToTexture(Napi::Env,
                              interop::GPUTexelCopyTextureInfo source,
                              interop::GPUTexelCopyTextureInfo destination,
                              interop::GPUExtent3D copySize) override;
    void pushDebugGroup(Napi::Env, std::string groupLabel) override;
    void popDebugGroup(Napi::Env) override;
    void insertDebugMarker(Napi::Env, std::string markerLabel) override;
    void writeTimestamp(Napi::Env,
                        interop::Interface<interop::GPUQuerySet> querySet,
                        interop::GPUSize32 queryIndex) override;
    void resolveQuerySet(Napi::Env,
                         interop::Interface<interop::GPUQuerySet> querySet,
                         interop::GPUSize32 firstQuery,
                         interop::GPUSize32 queryCount,
                         interop::Interface<interop::GPUBuffer> destination,
                         interop::GPUSize64 destinationOffset) override;
    interop::Interface<interop::GPUCommandBuffer> finish(
        Napi::Env env,
        interop::GPUCommandBufferDescriptor descriptor) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::Device device_;
    wgpu::CommandEncoder enc_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUCOMMANDENCODER_H_
