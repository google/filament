// Copyright 2018 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_
#define SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_

#include <webgpu/webgpu_cpp.h>

#include <array>
#include <vector>

#include "dawn/common/Constants.h"

namespace dawn::utils {

// Primarily used by tests to easily set up the vertex buffer state portion of a RenderPipeline.
class ComboVertexState {
  public:
    ComboVertexState();

    ComboVertexState(const ComboVertexState&) = delete;
    ComboVertexState& operator=(const ComboVertexState&) = delete;
    ComboVertexState(ComboVertexState&&) = delete;
    ComboVertexState& operator=(ComboVertexState&&) = delete;

    size_t vertexBufferCount = 0;
    std::vector<wgpu::VertexBufferLayout> cVertexBuffers =
        std::vector<wgpu::VertexBufferLayout>(kMaxVertexBuffers);
    std::vector<wgpu::VertexAttribute> cAttributes =
        std::vector<wgpu::VertexAttribute>(kMaxVertexAttributes);
};

class ComboRenderPipelineDescriptor : public wgpu::RenderPipelineDescriptor {
  public:
    ComboRenderPipelineDescriptor();

    ComboRenderPipelineDescriptor(const ComboRenderPipelineDescriptor&) = delete;
    ComboRenderPipelineDescriptor& operator=(const ComboRenderPipelineDescriptor&) = delete;
    ComboRenderPipelineDescriptor(ComboRenderPipelineDescriptor&&) = delete;
    ComboRenderPipelineDescriptor& operator=(ComboRenderPipelineDescriptor&&) = delete;

    wgpu::DepthStencilState* EnableDepthStencil(
        wgpu::TextureFormat format = wgpu::TextureFormat::Depth24PlusStencil8);
    void DisableDepthStencil();

    std::array<wgpu::VertexBufferLayout, kMaxVertexBuffers> cBuffers;
    std::array<wgpu::VertexAttribute, kMaxVertexAttributes> cAttributes;
    std::array<wgpu::ColorTargetState, kMaxColorAttachments> cTargets;
    std::array<wgpu::BlendState, kMaxColorAttachments> cBlends;

    wgpu::FragmentState cFragment;
    wgpu::DepthStencilState cDepthStencil;
};

}  // namespace dawn::utils

#endif  // SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_
