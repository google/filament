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

#ifndef SRC_DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_
#define SRC_DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_

#include <webgpu/webgpu_cpp.h>

#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "src/dawn/node/interop/NodeAPI.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPURenderPassEncoder is an implementation of interop::GPURenderPassEncoder that wraps a
// wgpu::RenderPassEncoder.
class GPURenderPassEncoder final : public interop::GPURenderPassEncoder {
  public:
    GPURenderPassEncoder(const wgpu::RenderPassDescriptor& desc, wgpu::RenderPassEncoder enc);

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::RenderPassEncoder&() const { return enc_; }

    // interop::GPURenderPassEncoder interface compliance
    void setViewport(Napi::Env,
                     float x,
                     float y,
                     float width,
                     float height,
                     float minDepth,
                     float maxDepth) override;
    void setScissorRect(Napi::Env,
                        interop::GPUIntegerCoordinate x,
                        interop::GPUIntegerCoordinate y,
                        interop::GPUIntegerCoordinate width,
                        interop::GPUIntegerCoordinate height) override;
    void setBlendConstant(Napi::Env, interop::GPUColor color) override;
    void setStencilReference(Napi::Env, interop::GPUStencilValue reference) override;
    void beginOcclusionQuery(Napi::Env, interop::GPUSize32 queryIndex) override;
    void endOcclusionQuery(Napi::Env) override;
    void executeBundles(Napi::Env,
                        std::vector<interop::Interface<interop::GPURenderBundle>> bundles) override;
    void end(Napi::Env) override;
    void setBindGroup(Napi::Env,
                      interop::GPUIndex32 index,
                      std::optional<interop::Interface<interop::GPUBindGroup>> bindGroup,
                      std::vector<interop::GPUBufferDynamicOffset> dynamicOffsets) override;
    void setBindGroup(Napi::Env,
                      interop::GPUIndex32 index,
                      std::optional<interop::Interface<interop::GPUBindGroup>> bindGroup,
                      interop::Uint32Array dynamicOffsetsData,
                      interop::GPUSize64 dynamicOffsetsDataStart,
                      interop::GPUSize32 dynamicOffsetsDataLength) override;
    void pushDebugGroup(Napi::Env, std::string groupLabel) override;
    void popDebugGroup(Napi::Env) override;
    void insertDebugMarker(Napi::Env, std::string markerLabel) override;
    void setPipeline(Napi::Env, interop::Interface<interop::GPURenderPipeline> pipeline) override;
    void setIndexBuffer(Napi::Env,
                        interop::Interface<interop::GPUBuffer> buffer,
                        interop::GPUIndexFormat indexFormat,
                        interop::GPUSize64 offset,
                        std::optional<interop::GPUSize64> size) override;
    void setVertexBuffer(Napi::Env,
                         interop::GPUIndex32 slot,
                         std::optional<interop::Interface<interop::GPUBuffer>> buffer,
                         interop::GPUSize64 offset,
                         std::optional<interop::GPUSize64> size) override;
    void draw(Napi::Env,
              interop::GPUSize32 vertexCount,
              interop::GPUSize32 instanceCount,
              interop::GPUSize32 firstVertex,
              interop::GPUSize32 firstInstance) override;
    void drawIndexed(Napi::Env,
                     interop::GPUSize32 indexCount,
                     interop::GPUSize32 instanceCount,
                     interop::GPUSize32 firstIndex,
                     interop::GPUSignedOffset32 baseVertex,
                     interop::GPUSize32 firstInstance) override;
    void drawIndirect(Napi::Env,
                      interop::Interface<interop::GPUBuffer> indirectBuffer,
                      interop::GPUSize64 indirectOffset) override;
    void drawIndexedIndirect(Napi::Env,
                             interop::Interface<interop::GPUBuffer> indirectBuffer,
                             interop::GPUSize64 indirectOffset) override;
    void multiDrawIndirect(Napi::Env,
                           interop::Interface<interop::GPUBuffer> indirectBuffer,
                           interop::GPUSize64 indirectOffset,
                           interop::GPUSize32 maxDrawCount,
                           std::optional<interop::Interface<interop::GPUBuffer>> countBuffer,
                           interop::GPUSize64 countBufferOffset) override;
    void multiDrawIndexedIndirect(
        Napi::Env,
        interop::Interface<interop::GPUBuffer> indirectBuffer,
        interop::GPUSize64 indirectOffset,
        interop::GPUSize32 maxDrawCount,
        std::optional<interop::Interface<interop::GPUBuffer>> drawCountBuffer,
        interop::GPUSize64 drawCountBufferOffset) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::RenderPassEncoder enc_;
    std::string label_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPURENDERPASSENCODER_H_
