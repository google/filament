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

#include "dawn/utils/ComboRenderPipelineDescriptor.h"

#include "dawn/utils/WGPUHelpers.h"

namespace dawn::utils {

ComboVertexState::ComboVertexState() {
    vertexBufferCount = 0;

    // Fill the default values for vertexBuffers and vertexAttributes in buffers.
    wgpu::VertexAttribute vertexAttribute;
    vertexAttribute.shaderLocation = 0;
    vertexAttribute.offset = 0;
    vertexAttribute.format = wgpu::VertexFormat::Float32;
    for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
        cAttributes[i] = vertexAttribute;
    }
    for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
        cVertexBuffers[i].arrayStride = 0;
        cVertexBuffers[i].stepMode = wgpu::VertexStepMode::Vertex;
        cVertexBuffers[i].attributeCount = 0;
        cVertexBuffers[i].attributes = nullptr;
    }
    // cVertexBuffers[i].attributes points to somewhere in cAttributes.
    // cVertexBuffers[0].attributes points to &cAttributes[0] by default. Assuming
    // cVertexBuffers[0] has two attributes, then cVertexBuffers[1].attributes should point to
    // &cAttributes[2]. Likewise, if cVertexBuffers[1] has 3 attributes, then
    // cVertexBuffers[2].attributes should point to &cAttributes[5].
    cVertexBuffers[0].attributes = &cAttributes[0];
}

ComboRenderPipelineDescriptor::ComboRenderPipelineDescriptor() {
    wgpu::RenderPipelineDescriptor* descriptor = this;

    // Set defaults for the vertex state.
    {
        wgpu::VertexState* vertex = &descriptor->vertex;
        vertex->module = nullptr;
        vertex->bufferCount = 0;

        // Fill the default values for vertexBuffers and vertexAttributes in buffers.
        for (uint32_t i = 0; i < kMaxVertexAttributes; ++i) {
            cAttributes[i].shaderLocation = 0;
            cAttributes[i].offset = 0;
            cAttributes[i].format = wgpu::VertexFormat::Float32;
        }
        for (uint32_t i = 0; i < kMaxVertexBuffers; ++i) {
            cBuffers[i].arrayStride = 0;
            cBuffers[i].stepMode = wgpu::VertexStepMode::Vertex;
            cBuffers[i].attributeCount = 0;
            cBuffers[i].attributes = nullptr;
        }
        // cBuffers[i].attributes points to somewhere in cAttributes.
        // cBuffers[0].attributes points to &cAttributes[0] by default. Assuming
        // cBuffers[0] has two attributes, then cBuffers[1].attributes should point to
        // &cAttributes[2]. Likewise, if cBuffers[1] has 3 attributes, then
        // cBuffers[2].attributes should point to &cAttributes[5].
        cBuffers[0].attributes = &cAttributes[0];
        vertex->buffers = &cBuffers[0];
    }

    // Set the defaults for the primitive state
    {
        wgpu::PrimitiveState* primitive = &descriptor->primitive;
        primitive->topology = wgpu::PrimitiveTopology::TriangleList;
        primitive->stripIndexFormat = wgpu::IndexFormat::Undefined;
        primitive->frontFace = wgpu::FrontFace::CCW;
        primitive->cullMode = wgpu::CullMode::None;
    }

    // Set the defaults for the depth-stencil state
    {
        wgpu::StencilFaceState stencilFace;
        stencilFace.compare = wgpu::CompareFunction::Always;
        stencilFace.failOp = wgpu::StencilOperation::Keep;
        stencilFace.depthFailOp = wgpu::StencilOperation::Keep;
        stencilFace.passOp = wgpu::StencilOperation::Keep;

        cDepthStencil.format = wgpu::TextureFormat::Depth24PlusStencil8;
        cDepthStencil.depthWriteEnabled = wgpu::OptionalBool::False;
        cDepthStencil.depthCompare = wgpu::CompareFunction::Always;
        cDepthStencil.stencilBack = stencilFace;
        cDepthStencil.stencilFront = stencilFace;
        cDepthStencil.stencilReadMask = 0xff;
        cDepthStencil.stencilWriteMask = 0xff;
        cDepthStencil.depthBias = 0;
        cDepthStencil.depthBiasSlopeScale = 0.0;
        cDepthStencil.depthBiasClamp = 0.0;
    }

    // Set the defaults for the multisample state
    {
        wgpu::MultisampleState* multisample = &descriptor->multisample;
        multisample->count = 1;
        multisample->mask = 0xFFFFFFFF;
        multisample->alphaToCoverageEnabled = false;
    }

    // Set the defaults for the fragment state
    {
        cFragment.module = nullptr;
        cFragment.targetCount = 1;
        cFragment.targets = &cTargets[0];
        descriptor->fragment = &cFragment;

        wgpu::BlendComponent blendComponent;
        blendComponent.srcFactor = wgpu::BlendFactor::One;
        blendComponent.dstFactor = wgpu::BlendFactor::Zero;
        blendComponent.operation = wgpu::BlendOperation::Add;

        for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
            cTargets[i].format = wgpu::TextureFormat::RGBA8Unorm;
            cTargets[i].blend = nullptr;
            cTargets[i].writeMask = wgpu::ColorWriteMask::All;

            cBlends[i].color = blendComponent;
            cBlends[i].alpha = blendComponent;
        }
    }
}

wgpu::DepthStencilState* ComboRenderPipelineDescriptor::EnableDepthStencil(
    wgpu::TextureFormat format) {
    this->depthStencil = &cDepthStencil;
    cDepthStencil.format = format;
    return &cDepthStencil;
}

void ComboRenderPipelineDescriptor::DisableDepthStencil() {
    this->depthStencil = nullptr;
}

}  // namespace dawn::utils
