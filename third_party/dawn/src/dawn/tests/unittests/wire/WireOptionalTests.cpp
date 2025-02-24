// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/tests/unittests/wire/WireTest.h"

namespace dawn::wire {
namespace {

using testing::_;
using testing::Return;

class WireOptionalTests : public WireTest {
  public:
    WireOptionalTests() {}
    ~WireOptionalTests() override = default;
};

// Test passing nullptr instead of objects - object as value version
TEST_F(WireOptionalTests, OptionalObjectValue) {
    wgpu::BindGroupLayoutDescriptor bglDesc = {};
    bglDesc.entryCount = 0;
    wgpu::BindGroupLayout bgl = device.CreateBindGroupLayout(&bglDesc);

    WGPUBindGroupLayout apiBindGroupLayout = api.GetNewBindGroupLayout();
    EXPECT_CALL(api, DeviceCreateBindGroupLayout(apiDevice, _))
        .WillOnce(Return(apiBindGroupLayout));

    // The `sampler`, `textureView` and `buffer` members of a binding are optional.
    wgpu::BindGroupEntry entry;
    entry.binding = 0;
    entry.sampler = nullptr;
    entry.textureView = nullptr;
    entry.buffer = nullptr;
    entry.nextInChain = nullptr;

    wgpu::BindGroupDescriptor bgDesc = {};
    bgDesc.layout = bgl;
    bgDesc.entryCount = 1;
    bgDesc.entries = &entry;

    wgpu::BindGroup bg = device.CreateBindGroup(&bgDesc);

    WGPUBindGroup apiPlaceholderBindGroup = api.GetNewBindGroup();
    EXPECT_CALL(api, DeviceCreateBindGroup(
                         apiDevice, MatchesLambda([](const WGPUBindGroupDescriptor* desc) -> bool {
                             return desc->nextInChain == nullptr && desc->entryCount == 1 &&
                                    desc->entries[0].binding == 0 &&
                                    desc->entries[0].sampler == nullptr &&
                                    desc->entries[0].buffer == nullptr &&
                                    desc->entries[0].textureView == nullptr;
                         })))
        .WillOnce(Return(apiPlaceholderBindGroup));

    FlushClient();
}

// Test that the wire is able to send optional pointers to structures
TEST_F(WireOptionalTests, OptionalStructPointer) {
    // Create shader module
    wgpu::ShaderModuleDescriptor vertexDescriptor = {};
    wgpu::ShaderModule vsModule = device.CreateShaderModule(&vertexDescriptor);
    WGPUShaderModule apiVsModule = api.GetNewShaderModule();
    EXPECT_CALL(api, DeviceCreateShaderModule(apiDevice, _)).WillOnce(Return(apiVsModule));

    // Create the color state descriptor
    wgpu::BlendComponent blendComponent = {};
    blendComponent.operation = wgpu::BlendOperation::Add;
    blendComponent.srcFactor = wgpu::BlendFactor::One;
    blendComponent.dstFactor = wgpu::BlendFactor::One;
    wgpu::BlendState blendState = {};
    blendState.alpha = blendComponent;
    blendState.color = blendComponent;
    wgpu::ColorTargetState colorTargetState = {};
    colorTargetState.format = wgpu::TextureFormat::RGBA8Unorm;
    colorTargetState.blend = &blendState;
    colorTargetState.writeMask = wgpu::ColorWriteMask::All;

    // Create the depth-stencil state
    wgpu::StencilFaceState stencilFace = {};
    stencilFace.compare = wgpu::CompareFunction::Always;
    stencilFace.failOp = wgpu::StencilOperation::Keep;
    stencilFace.depthFailOp = wgpu::StencilOperation::Keep;
    stencilFace.passOp = wgpu::StencilOperation::Keep;

    wgpu::DepthStencilState depthStencilState = {};
    depthStencilState.format = wgpu::TextureFormat::Depth24PlusStencil8;
    depthStencilState.depthWriteEnabled = wgpu::OptionalBool::False;
    depthStencilState.depthCompare = wgpu::CompareFunction::Always;
    depthStencilState.stencilBack = stencilFace;
    depthStencilState.stencilFront = stencilFace;
    depthStencilState.stencilReadMask = 0xff;
    depthStencilState.stencilWriteMask = 0xff;
    depthStencilState.depthBias = 0;
    depthStencilState.depthBiasSlopeScale = 0.0;
    depthStencilState.depthBiasClamp = 0.0;

    // Create the pipeline layout
    wgpu::PipelineLayoutDescriptor layoutDescriptor = {};
    layoutDescriptor.bindGroupLayoutCount = 0;
    layoutDescriptor.bindGroupLayouts = nullptr;
    wgpu::PipelineLayout layout = device.CreatePipelineLayout(&layoutDescriptor);
    WGPUPipelineLayout apiLayout = api.GetNewPipelineLayout();
    EXPECT_CALL(api, DeviceCreatePipelineLayout(apiDevice, _)).WillOnce(Return(apiLayout));

    // Create pipeline
    wgpu::RenderPipelineDescriptor pipelineDescriptor = {};

    pipelineDescriptor.vertex.module = vsModule;
    pipelineDescriptor.vertex.bufferCount = 0;
    pipelineDescriptor.vertex.buffers = nullptr;

    wgpu::FragmentState fragment = {};
    fragment.module = vsModule;
    fragment.targetCount = 1;
    fragment.targets = &colorTargetState;
    pipelineDescriptor.fragment = &fragment;

    pipelineDescriptor.multisample.count = 1;
    pipelineDescriptor.multisample.mask = 0xFFFFFFFF;
    pipelineDescriptor.multisample.alphaToCoverageEnabled = false;
    pipelineDescriptor.layout = layout;
    pipelineDescriptor.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
    pipelineDescriptor.primitive.frontFace = wgpu::FrontFace::CCW;
    pipelineDescriptor.primitive.cullMode = wgpu::CullMode::None;

    // First case: depthStencil is not null.
    pipelineDescriptor.depthStencil = &depthStencilState;
    wgpu::RenderPipeline pipeline1 = device.CreateRenderPipeline(&pipelineDescriptor);

    WGPURenderPipeline apiPlaceholderPipeline = api.GetNewRenderPipeline();
    EXPECT_CALL(
        api,
        DeviceCreateRenderPipeline(
            apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                return desc->depthStencil != nullptr &&
                       desc->depthStencil->nextInChain == nullptr &&
                       desc->depthStencil->depthWriteEnabled == WGPUOptionalBool_False &&
                       desc->depthStencil->depthCompare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilBack.compare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilBack.failOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilBack.depthFailOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilBack.passOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.compare == WGPUCompareFunction_Always &&
                       desc->depthStencil->stencilFront.failOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.depthFailOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilFront.passOp == WGPUStencilOperation_Keep &&
                       desc->depthStencil->stencilReadMask == 0xff &&
                       desc->depthStencil->stencilWriteMask == 0xff &&
                       desc->depthStencil->depthBias == 0 &&
                       desc->depthStencil->depthBiasSlopeScale == 0.0 &&
                       desc->depthStencil->depthBiasClamp == 0.0;
            })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();

    // Second case: depthStencil is null.
    pipelineDescriptor.depthStencil = nullptr;
    wgpu::RenderPipeline pipeline2 = device.CreateRenderPipeline(&pipelineDescriptor);
    EXPECT_CALL(api,
                DeviceCreateRenderPipeline(
                    apiDevice, MatchesLambda([](const WGPURenderPipelineDescriptor* desc) -> bool {
                        return desc->depthStencil == nullptr;
                    })))
        .WillOnce(Return(apiPlaceholderPipeline));

    FlushClient();
}

}  // anonymous namespace
}  // namespace dawn::wire
